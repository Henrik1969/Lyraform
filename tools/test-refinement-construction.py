#!/usr/bin/env python3
"""Bounded R1 refinement construction: prove, refuse, and execute."""
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
    "frontend": build / "flowmini/flowmini",
    "analyze": build / "flowanalyst/flowanalyst",
    "bind": build / "flowbind/flowbind",
    "parallel": build / "flowtools/flowparallel/flowparallel",
    "optimize": build / "flowoptimize/flowoptimize",
    "prepare": build / "flowlower/flowprepare",
    "lower": build / "flowlower/flowlower",
    "validate": build / "flowcontracts/flowvalidate",
    "tiny_lower": build / "tinyvm/flowtinylower",
    "tiny_run": build / "tinyvm/flowtinyrun",
}


def run(command, data=None):
    return subprocess.run(list(map(str, command)), input=data, text=True,
                          capture_output=True, timeout=30)


def good(command, data=None):
    result = run(command, data)
    assert result.returncode == 0, (command, result.returncode, result.stdout, result.stderr)
    return result.stdout


def source(initializer, tail="0 -> return"):
    return ("program refinement_construction\n"
            "type Positive32 refines int32 {\n"
            " invariant value > 0\n"
            "}\n"
            "main {\n"
            f"x : Positive32({initializer})\n"
            f"{tail}\n"
            "}\n")


with tempfile.TemporaryDirectory(prefix="lyraform-refinement-construction-") as directory:
    root = pathlib.Path(directory)

    valid_path = root / "valid.flow"
    valid_path.write_text(source("7"))
    frontend = good([tools["frontend"], "--dump-frontend-bundle", valid_path])
    semantic = good([tools["analyze"], "--lowering-plan-version", "2"], frontend)
    report = json.loads(semantic)
    assert report["status"] == "ok" and report["lowering_plan"]["status"] == "ready", report
    assert not any("REFINED_VALUE" in item["code"] for item in report["diagnostics"]), report["diagnostics"]
    assert any(operand.get("type") == "int32" and operand.get("value") == "7"
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
    assert llvm.exists()
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

    invalid_path = root / "invalid.flow"
    invalid_path.write_text(source("-1"))
    invalid_frontend = good([tools["frontend"], "--dump-frontend-bundle", invalid_path])
    invalid = run([tools["analyze"], "--lowering-plan-version", "2"], invalid_frontend)
    assert invalid.returncode == 2, (invalid.stdout, invalid.stderr)
    invalid_report = json.loads(invalid.stdout)
    assert invalid_report["lowering_plan"]["status"] == "blocked", invalid_report
    diagnostic = next(item for item in invalid_report["diagnostics"]
                      if item["code"] == "FLOWANALYST_REFINED_INVARIANT_VIOLATION")
    assert diagnostic["provenance"]["line"] > 0 and diagnostic["provenance"]["column"] > 0, diagnostic
    assert pathlib.Path(diagnostic["provenance"]["source"]).name == invalid_path.name, diagnostic

    unknown_path = root / "unknown.flow"
    unknown_path.write_text(source("source_value"))
    unknown_frontend = good([tools["frontend"], "--dump-frontend-bundle", unknown_path])
    unknown = run([tools["analyze"], "--lowering-plan-version", "2"], unknown_frontend)
    assert unknown.returncode == 2, (unknown.stdout, unknown.stderr)
    unknown_report = json.loads(unknown.stdout)
    assert any(item["code"] == "FLOWANALYST_REFINED_VALUE_UNSUPPORTED"
               for item in unknown_report["diagnostics"]), unknown_report

    print(json.dumps({"valid": "admitted", "invalid": "refused",
                      "unknown": "refused", "llvm": "checked", "tinyvm": "checked"}))
