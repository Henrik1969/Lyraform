#!/usr/bin/env python3
"""Prove whole-program semantic admission checks unreachable branches."""

import argparse
import json
import pathlib
import subprocess
import tempfile


parser = argparse.ArgumentParser()
parser.add_argument("--build", type=pathlib.Path, required=True)
build = parser.parse_args().build.resolve()
tools = {
    name: build / path
    for name, path in {
        "frontend": "flowmini/flowmini",
        "analyze": "flowanalyst/flowanalyst",
        "bind": "flowbind/flowbind",
        "parallel": "flowtools/flowparallel/flowparallel",
        "optimize": "flowoptimize/flowoptimize",
        "prepare": "flowlower/flowprepare",
        "validate": "flowcontracts/flowvalidate",
        "lower": "flowlower/flowlower",
        "tiny_lower": "tinyvm/flowtinylower",
        "tiny_validate": "tinyvm/flowtinyvalidate",
        "tiny_run": "tinyvm/flowtinyrun",
    }.items()
}


def run(command, data=None):
    return subprocess.run(
        [str(item) for item in command],
        input=data,
        text=True,
        capture_output=True,
        timeout=30,
    )


def require(command, data=None):
    result = run(command, data)
    assert result.returncode == 0, (
        command,
        result.returncode,
        result.stdout,
        result.stderr,
    )
    return result.stdout


def source(expression, condition="false"):
    return f"""program unreachable_invalid_branch

main {{
    if {condition} {{
        {expression} -> return
    }} else {{
        0 -> return
    }}
}}
"""


with tempfile.TemporaryDirectory(prefix="lyraform-unreachable-branch-") as directory:
    root = pathlib.Path(directory)
    invalid = {
        "zero_division": ("1 div 0", "FLOWANALYST_INTEGER_DIVISION_ZERO", "false"),
        "invalid_numeric_operator": ("true + 1", "FLOWANALYST_NUMERIC_OPERATOR_TYPE", "true"),
        "unresolved_name": ("missing_name", "FLOWANALYST_UNRESOLVED_NAME", "false"),
    }
    for name, (expression, diagnostic_code, condition) in invalid.items():
        path = root / f"{name}.flow"
        path.write_text(source(expression, condition))
        frontend = require([tools["frontend"], "--dump-frontend-bundle", path])
        analyzed = run([tools["analyze"], "--lowering-plan-version", "2"], frontend)
        assert analyzed.returncode == 2, (name, analyzed.stdout, analyzed.stderr)
        report = json.loads(analyzed.stdout)
        assert report["status"] == "error", report
        assert report["lowering_plan"]["status"] == "blocked", report
        assert any(item["code"] == diagnostic_code for item in report["diagnostics"]), report
        assert all(
            item.get("provenance", {}).get("line", 0) > 0
            and item.get("provenance", {}).get("column", 0) > 0
            and item.get("provenance", {}).get("ast_path", "").startswith("/statement_pool/")
            for item in report["diagnostics"]
        ), report
        coverage = report["lowering_plan"]["source_operation_coverage"]
        assert coverage["status"] == "complete", report
        assert any(operation["kind"] == "branch" for operation in report["lowering_plan"]["operations"]), report

        # The blocked semantic artifact is not a backend input. Every normal
        # consumer must refuse it rather than inventing an executable plan.
        for stage in ("bind", "parallel"):
            refused = run([tools[stage]], analyzed.stdout)
            assert refused.returncode != 0, (name, stage, refused.stdout, refused.stderr)

    # Control specimen: an unreachable but valid branch remains a normal
    # source program and must survive the complete lowering/execution chain.
    path = root / "valid.flow"
    path.write_text(source("1", "false"))
    frontend = require([tools["frontend"], "--dump-frontend-bundle", path])
    semantic = require([tools["analyze"], "--lowering-plan-version", "2"], frontend)
    report = json.loads(semantic)
    assert report["status"] == "ok", report
    parallel = require([tools["parallel"]], semantic)
    optimized = require([tools["optimize"]], parallel)
    optimized_path = root / "valid.optimized.json"
    optimized_path.write_text(optimized)
    prepared = require([tools["prepare"], optimized_path])
    require([tools["validate"]], prepared)

    llvm = root / "valid.ll"
    require([tools["lower"], "--emit-llvm", llvm], optimized)
    assert "br i1 false" in llvm.read_text(), llvm.read_text()

    tiny = root / "valid.tvm"
    require([tools["tiny_lower"], "-", tiny], prepared)
    require([tools["tiny_validate"], tiny])
    execution = json.loads(require([tools["tiny_run"], tiny]))
    assert execution["status"] == "completed", execution
    assert execution["result"] == 0, execution

print("unreachable invalid branch: PASS")
