#!/usr/bin/env python3
"""Vertical tests for the bounded float64 exceptional/precision contract."""

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


def source(expression, predicate):
    return f"""program floating_exceptional_precision

fn probe(): float64 {{
    {expression} -> return
}}

main {{
    result : float64(probe())
    if {predicate} {{
        0 -> return
    }} else {{
        1 -> return
    }}
}}
"""


cases = {
    "positive_infinity": (
        "1.0 / 0.0",
        "result > 1.0e308",
        "isinf(value) && !signbit(value)",
    ),
    "negative_infinity": (
        "-1.0 / 0.0",
        "result < -1.0e308",
        "isinf(value) && signbit(value)",
    ),
    "nan": (
        "0.0 / 0.0",
        "result != result",
        "isnan(value)",
    ),
    "negative_zero": (
        "-0.0",
        "1.0 / result < 0.0",
        "value == 0.0 && signbit(value)",
    ),
    "round_tie_to_even": (
        "1.0 + 1.1102230246251565e-16",
        "result == 1.0",
        "value == 1.0",
    ),
    "round_to_next": (
        "1.0 + 3.3306690738754696e-16",
        "result == 1.0000000000000004",
        "value == 1.0000000000000004",
    ),
    "overflow_to_infinity": (
        "1.0e308 * 1.0e308",
        "result > 1.0e308",
        "isinf(value) && !signbit(value)",
    ),
}


with tempfile.TemporaryDirectory(prefix="lyraform-floating-exceptional-") as directory:
    root = pathlib.Path(directory)
    for name, (expression, predicate, native_check) in cases.items():
        path = root / f"{name}.flow"
        path.write_text(source(expression, predicate))
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

        llvm = root / f"{name}.ll"
        require([tools["lower"], "--emit-llvm", llvm], optimized)
        llvm_text = llvm.read_text()
        function = re.search(r'define double @"?flow\.function\.(\d+)"?\(', llvm_text)
        assert function, llvm_text
        entry_llvm = root / f"{name}.entry.ll"
        entry_llvm.write_text(llvm_text.replace("@main(", "@flow_main("))
        driver = root / f"{name}.driver.c"
        driver.write_text(
            "#include <math.h>\n"
            f'extern double review_probe(void) __asm__("flow.function.{function.group(1)}");\n'
            "int main(void) { double value = review_probe(); return "
            f"!({native_check}); }}\n"
        )
        native = root / f"{name}.native"
        require(["clang-18", "-O2", entry_llvm, driver, "-lm", "-o", native])
        native_result = run([native])
        assert native_result.returncode == 0, (name, native_result.stdout, native_result.stderr)

        tiny = root / f"{name}.tvm"
        require([tools["tiny_lower"], "-", tiny], prepared)
        require([tools["tiny_validate"], tiny])
        execution = json.loads(require([tools["tiny_run"], tiny]))
        assert execution["status"] == "completed", execution
        assert execution["result"] == 0, (name, execution)

    literal = root / "nonfinite_literal.flow"
    literal.write_text(source("1.0e309", "result == result"))
    frontend = require([tools["frontend"], "--dump-frontend-bundle", literal])
    analyzed = run([tools["analyze"], "--lowering-plan-version", "2"], frontend)
    assert analyzed.returncode == 2, (analyzed.stdout, analyzed.stderr)
    report = json.loads(analyzed.stdout)
    assert report["status"] == "error", report
    assert any(item["code"] == "FLOWANALYST_FLOAT_LITERAL_RANGE" for item in report["diagnostics"]), report

print("floating exceptional/precision: PASS (IEEE binary64 values, signed zero, exceptional results, and ties-to-even witnesses)")
