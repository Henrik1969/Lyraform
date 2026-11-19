#!/usr/bin/env python3
"""Vertical tests for the integer quotient/remainder arithmetic canon slice.

This slice admits integer quotient through `div`, integer remainder through `%`,
both with truncation-toward-zero division semantics, while true division is
covered by the separate numeric-promotion slice.
"""

import argparse
import json
import pathlib
import subprocess
import tempfile


parser = argparse.ArgumentParser()
parser.add_argument("--build", type=pathlib.Path, required=True)
args = parser.parse_args()
build = args.build.resolve()
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


def source(expression):
    return f"program division_canon\n\nmain {{\n    {expression} -> return\n}}\n"


positive = {
    "positive": ("7 div 3", 2),
    "negative_left": ("-7 div 3", -2),
    "negative_right": ("7 div -3", -2),
    "both_negative": ("-7 div -3", 2),
    "remainder_positive": ("7 % 3", 1),
    "remainder_negative_left": ("-7 % 3", -1),
    "remainder_negative_right": ("7 % -3", 1),
    "remainder_both_negative": ("-7 % -3", -1),
}


def signed_tiny_result(value):
    value = int(value)
    return value - (1 << 64) if value >= (1 << 63) else value


with tempfile.TemporaryDirectory(prefix="lyraform-division-canon-") as directory:
    temp = pathlib.Path(directory)
    for name, (expression, expected) in positive.items():
        path = temp / f"{name}.flow"
        path.write_text(source(expression))

        frontend = require([tools["frontend"], "--dump-frontend-bundle", path])
        semantic = require(
            [tools["analyze"], "--lowering-plan-version", "2"], frontend
        )
        semantic_report = json.loads(semantic)
        assert semantic_report["status"] == "ok", semantic_report
        assert semantic_report["lowering_plan"]["status"] == "ready", semantic_report

        # Binding is a separate validation gate; the semantic report remains
        # the canonical input to the scheduler.
        require([tools["bind"]], semantic)
        parallel = require([tools["parallel"]], semantic)
        optimized = require([tools["optimize"]], parallel)
        optimized_path = temp / f"{name}.optimized.json"
        optimized_path.write_text(optimized)
        prepared = require([tools["prepare"], optimized_path])
        require([tools["validate"]], prepared)

        # Native lowering and execution.
        llvm = temp / f"{name}.ll"
        require([tools["lower"], "--emit-llvm", llvm], optimized)
        full_llvm = temp / f"{name}.entry.ll"
        full_llvm.write_text(llvm.read_text().replace("@main(", "@review_entry("))
        driver = temp / f"{name}.driver.c"
        driver.write_text(
            "#include <stdio.h>\n"
            "extern int review_entry(void);\n"
            "int main(void) { printf(\"%d\\n\", review_entry()); return 0; }\n"
        )
        native = temp / f"{name}.native"
        require(["clang-18", "-O2", full_llvm, driver, "-o", native])
        assert int(require([native]).strip()) == expected, name

        # TinyVM lowering, validation, and execution.
        tiny = temp / f"{name}.tvm"
        require([tools["tiny_lower"], "-", tiny], prepared)
        require([tools["tiny_validate"], tiny])
        observed = json.loads(require([tools["tiny_run"], tiny]))
        assert observed["status"] == "completed", observed
        assert signed_tiny_result(observed["result"]) == expected, (name, observed)

    negative = {
        "zero_divisor": ("1 div 0", "FLOWANALYST_INTEGER_DIVISION_ZERO"),
        "int32_overflow": (
            "-2147483648 div -1",
            "FLOWANALYST_INTEGER_DIVISION_OVERFLOW",
        ),
        "remainder_zero": (
            "1 % 0",
            "FLOWANALYST_INTEGER_REMAINDER_ZERO",
        ),
        "remainder_overflow": (
            "-2147483648 % -1",
            "FLOWANALYST_INTEGER_REMAINDER_OVERFLOW",
        ),
    }
    for name, (expression, diagnostic_code) in negative.items():
        path = temp / f"{name}.flow"
        path.write_text(source(expression))
        frontend = require([tools["frontend"], "--dump-frontend-bundle", path])
        analyzed = run(
            [tools["analyze"], "--lowering-plan-version", "2"], frontend
        )
        assert analyzed.returncode == 2, (name, analyzed.stdout, analyzed.stderr)
        report = json.loads(analyzed.stdout)
        assert report["status"] == "error", report
        assert report["lowering_plan"]["status"] == "blocked", report
        diagnostic = next(
            item for item in report["diagnostics"] if item["code"] == diagnostic_code
        )
        assert diagnostic["provenance"]["line"] == 4, diagnostic
        assert diagnostic["provenance"]["column"] == 5, diagnostic

print("arithmetic division canon: PASS")
