#!/usr/bin/env python3
"""Bounded aggregate cross-field invariant construction checks."""
import json
import pathlib
import subprocess
import sys
import tempfile

build = pathlib.Path(sys.argv[1])
cases = [
    ("proven", "{lower: 2, upper: 7}", set()),
    ("violated", "{lower: 9, upper: 7}", {"FLOWANALYST_RECORD_INVARIANT_VIOLATION"}),
    ("unproven", "{lower: calculate(), upper: 7}", {"FLOWANALYST_RECORD_INVARIANT_UNPROVEN"}),
]

with tempfile.TemporaryDirectory(prefix="lyraform-record-cross-field-") as directory:
    source = pathlib.Path(directory) / "input.flow"
    for name, literal, expected in cases:
        functions = ""
        if name == "unproven":
            functions = "fn calculate(): int32 {\n value : int32(2)\n value -> return\n}\n"
        source.write_text(
            "program aggregate_invariants\n"
            "record Range { lower : int32\nupper : int32\n"
            "invariant lower <= upper\n}\n"
            + functions
            + "main {\n item : Range(" + literal + ")\n 0 -> return\n}\n"
        )
        frontend = subprocess.run(
            [build / "flowmini/flowmini", "--dump-frontend-bundle", source],
            capture_output=True, text=True, check=True,
        ).stdout
        parsed = json.loads(frontend)
        declaration = next(
            item for item in parsed["ast"]["declaration_pool"]
            if item["kind"] == "record" and item["name"] == "Range"
        )
        assert len(declaration["invariants"]) == 1, (name, declaration)
        result = subprocess.run(
            [build / "flowanalyst/flowanalyst", "--lowering-plan-version", "1"],
            input=frontend, capture_output=True, text=True,
        )
        report = json.loads(result.stdout)
        issues = {
            item["code"] for item in report["diagnostics"]
            if item["code"].startswith("FLOWANALYST_RECORD_INVARIANT_")
        }
        assert issues == expected, (name, issues, report)
        assert all(
            item["provenance"]["line"] > 0
            for item in report["diagnostics"]
            if item["code"].startswith("FLOWANALYST_RECORD_INVARIANT_")
        )
        assert result.returncode == (2 if expected else 0), (name, report)

print("PASS: aggregate cross-field invariant construction is proven, refused, or rejected explicitly")
