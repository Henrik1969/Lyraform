#!/usr/bin/env python3
"""Bounded R2 refinement assignment: prove, refuse, and execute."""
import argparse
import json
import pathlib
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument("--build", type=pathlib.Path, required=True)
args = parser.parse_args()
build = args.build.resolve()

tools = {name: build / path for name, path in {
    "frontend": "flowmini/flowmini", "analyze": "flowanalyst/flowanalyst",
    "bind": "flowbind/flowbind", "parallel": "flowtools/flowparallel/flowparallel",
    "optimize": "flowoptimize/flowoptimize", "prepare": "flowlower/flowprepare",
    "lower": "flowlower/flowlower", "validate": "flowcontracts/flowvalidate",
    "tiny_lower": "tinyvm/flowtinylower", "tiny_run": "tinyvm/flowtinyrun",
}.items()}


def run(command, data=None):
    return subprocess.run(list(map(str, command)), input=data, text=True,
                          capture_output=True, timeout=30)


def good(command, data=None):
    result = run(command, data)
    assert result.returncode == 0, (command, result.returncode, result.stdout, result.stderr)
    return result.stdout


def source(assignment):
    return ("program refinement_assignment\n"
            "type Positive32 refines int32 {\n"
            " invariant value > 0\n"
            "}\n"
            "main {\n"
            "x : Positive32(7)\n"
            f"{assignment} -> x\n"
            "0 -> return\n"
            "}\n")


with tempfile.TemporaryDirectory(prefix="lyraform-refinement-assignment-") as directory:
    root = pathlib.Path(directory)

    valid_path = root / "valid.flow"
    valid_path.write_text(source("9"))
    frontend = good([tools["frontend"], "--dump-frontend-bundle", valid_path])
    semantic = good([tools["analyze"], "--lowering-plan-version", "2"], frontend)
    report = json.loads(semantic)
    assert report["status"] == "ok" and report["lowering_plan"]["status"] == "ready", report
    assert not any("REFINED_ASSIGNMENT" in item["code"] for item in report["diagnostics"]), report["diagnostics"]
    assert any(operand.get("type") == "int32" and operand.get("value") == "9"
               for operation in report["lowering_plan"]["operations"]
               for operand in operation.get("operands", [])), report["lowering_plan"]

    good([tools["bind"]], semantic)
    parallel = good([tools["parallel"]], semantic)
    optimized = good([tools["optimize"]], parallel)
    optimized_path = root / "optimized.json"
    optimized_path.write_text(optimized)
    prepared = good([tools["prepare"], optimized_path])
    good([tools["validate"]], prepared)
    llvm = root / "valid.ll"
    good([tools["lower"], "--emit-llvm", llvm], optimized)
    native_ir = root / "native.ll"
    native_ir.write_text(llvm.read_text().replace("@main(", "@review_entry("))
    driver = root / "driver.c"
    driver.write_text("#include <stdio.h>\nextern int review_entry(void);\n"
                      "int main(void) { printf(\"%d\\n\", review_entry()); return 0; }\n")
    native = root / "native"
    good(["clang-18", native_ir, driver, "-o", native])
    assert good([native]).strip() == "0"
    tiny = root / "valid.tvm"
    good([tools["tiny_lower"], "-", tiny], prepared)
    observed = json.loads(good([tools["tiny_run"], tiny]))
    assert observed["status"] == "completed" and observed["result"] == 0, observed

    for name, assignment, code in (
        ("invalid", "-1", "FLOWANALYST_REFINED_ASSIGNMENT_VIOLATION"),
        ("unknown", "source_value", "FLOWANALYST_REFINED_ASSIGNMENT_UNPROVEN"),
    ):
        path = root / (name + ".flow")
        path.write_text(source(assignment))
        parsed = good([tools["frontend"], "--dump-frontend-bundle", path])
        result = run([tools["analyze"], "--lowering-plan-version", "2"], parsed)
        assert result.returncode == 2, (name, result.stdout, result.stderr)
        report = json.loads(result.stdout)
        assert report["lowering_plan"]["status"] == "blocked", report
        diagnostic = next(item for item in report["diagnostics"] if item["code"] == code)
        assert diagnostic["provenance"]["line"] > 0 and diagnostic["provenance"]["column"] > 0, diagnostic
        assert pathlib.Path(diagnostic["provenance"]["source"]).name == path.name, diagnostic

    print(json.dumps({"valid": "admitted", "invalid": "refused",
                      "unknown": "refused", "llvm": "checked", "tinyvm": "checked"}))
