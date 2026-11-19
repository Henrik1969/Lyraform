#!/usr/bin/env python3
"""Vertical tests for mixed numeric promotion and true division.

The slice gives +, -, and * the common numeric result domain and gives /
true-division semantics.  An integer operand is explicitly promoted to the
float64 carrier when the selected result domain is floating.  Integer quotient
and remainder remain separate integer-only operators.
"""

import argparse
import json
import pathlib
import re
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


def source(expression, expected):
    return f"""program numeric_promotion_canon

fn probe(): float64 {{
    {expression} -> return
}}

main {{
    result : float64(probe())
    if result == {expected} {{
        1 -> return
    }} else {{
        2 -> return
    }}
}}
"""


def pipeline(root, name, expression, expected):
    path = root / f"{name}.flow"
    path.write_text(source(expression, expected))
    frontend = require([tools["frontend"], "--dump-frontend-bundle", path])
    semantic = require([tools["analyze"], "--lowering-plan-version", "2"], frontend)
    report = json.loads(semantic)
    assert report["status"] == "ok", report
    assert report["lowering_plan"]["status"] == "ready", report
    require([tools["bind"]], semantic)
    parallel = require([tools["parallel"]], semantic)
    optimized = require([tools["optimize"]], parallel)
    optimized_path = root / f"{name}.optimized.json"
    optimized_path.write_text(optimized)
    prepared = require([tools["prepare"], optimized_path])
    require([tools["validate"]], prepared)

    # Native lowering and execution observe the non-entry float function
    # directly, while retaining the ordinary main entry in the artifact.
    llvm = root / f"{name}.ll"
    require([tools["lower"], "--emit-llvm", llvm], optimized)
    text = re.sub(r"@flow\.function\.[0-9]+", "@review_probe", llvm.read_text())
    text = re.sub(r"@main\b", "@flow_main", text)
    entry_llvm = root / f"{name}.entry.ll"
    entry_llvm.write_text(text)
    driver = root / f"{name}.driver.c"
    driver.write_text(
        "#include <stdio.h>\n"
        "extern double review_probe(void);\n"
        "int main(void) { printf(\"%.17g\\n\", review_probe()); return 0; }\n"
    )
    native = root / f"{name}.native"
    require(["clang-18", "-O2", entry_llvm, driver, "-o", native])
    observed = float(require([native]).strip())
    assert abs(observed - float(expected)) <= 1e-15, (name, observed, expected)

    # TinyVM lowers the same callable through its f64 carrier and executes it
    # from main.  The integer result is a deliberate observable witness for
    # the float comparison in the main branch.
    tiny = root / f"{name}.tvm"
    require([tools["tiny_lower"], "-", tiny], prepared)
    require([tools["tiny_validate"], tiny])
    execution = json.loads(require([tools["tiny_run"], tiny]))
    assert execution["status"] == "completed", execution
    assert execution["result"] == 1, (name, execution)


with tempfile.TemporaryDirectory(prefix="lyraform-numeric-promotion-") as directory:
    root = pathlib.Path(directory)
    positive = {
        "add_mixed": ("2.5 + 5", "7.5"),
        "subtract_mixed": ("5 - 2.5", "2.5"),
        "multiply_mixed": ("2.5 * 5", "12.5"),
        "true_division_mixed": ("5 / 2.5", "2.0"),
        "true_division_integer_operands": ("8 / 2", "4.0"),
        "true_division_fraction": ("1 / 3", "0.3333333333333333"),
    }
    for name, (expression, expected) in positive.items():
        pipeline(root, name, expression, expected)

    negative = {
        "integer_quotient_rejects_float": ("2.5 div 5", "FLOWANALYST_INTEGER_DIVISION_TYPE"),
        "remainder_rejects_float": ("2.5 % 5", "FLOWANALYST_INTEGER_REMAINDER_TYPE"),
    }
    for name, (expression, diagnostic_code) in negative.items():
        path = root / f"{name}.flow"
        path.write_text(source(expression, "0.0"))
        frontend = require([tools["frontend"], "--dump-frontend-bundle", path])
        analyzed = run([tools["analyze"], "--lowering-plan-version", "2"], frontend)
        assert analyzed.returncode == 2, (name, analyzed.stdout, analyzed.stderr)
        report = json.loads(analyzed.stdout)
        assert report["status"] == "error", report
        assert any(item["code"] == diagnostic_code for item in report["diagnostics"]), report

print("numeric promotion canon: PASS")
