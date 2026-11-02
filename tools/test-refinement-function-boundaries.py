#!/usr/bin/env python3
"""Bounded R3 refinement function-argument and return boundaries."""
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


def source(argument="7", returned="7"):
    return ("program refinement_function_boundaries\n"
            "type Positive32 refines int32 {\n"
            " invariant value > 0\n"
            "}\n"
            "fn accept(x : Positive32): int32 {\n"
            " 0 -> return\n"
            "}\n"
            "fn make(): Positive32 {\n"
            f" {returned} -> return\n"
            "}\n"
            "main {\n"
            f"a : c_int(accept({argument}))\n"
            "b : c_int(make())\n"
            "0 -> return\n"
            "}\n")


with tempfile.TemporaryDirectory(prefix="lyraform-refinement-function-boundaries-") as directory:
    root = pathlib.Path(directory)

    valid_path = root / "valid.flow"
    valid_path.write_text(source())
    frontend = good([tools["frontend"], "--dump-frontend-bundle", valid_path])
    semantic = good([tools["analyze"], "--lowering-plan-version", "2"], frontend)
    report = json.loads(semantic)
    assert report["status"] == "ok" and report["lowering_plan"]["status"] == "ready", report
    assert not any("REFINED_" in item["code"] for item in report["diagnostics"]), report["diagnostics"]
    functions = {item["name"]: item for item in report["lowering_plan"]["functions"]}
    assert functions["accept"]["parameters"][0]["type"] == "int32", functions
    assert functions["make"]["return_type"] == "int32", functions

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

    for name, text, code in (
        ("bad-argument", source(argument="-1"), "FLOWANALYST_REFINED_ARGUMENT_VIOLATION"),
        ("bad-return", source(returned="-1"), "FLOWANALYST_REFINED_RETURN_VIOLATION"),
        ("unknown-return", source(returned="source_value"), "FLOWANALYST_REFINED_RETURN_UNPROVEN"),
    ):
        path = root / (name + ".flow")
        path.write_text(text)
        parsed = good([tools["frontend"], "--dump-frontend-bundle", path])
        result = run([tools["analyze"], "--lowering-plan-version", "2"], parsed)
        assert result.returncode == 2, (name, result.stdout, result.stderr)
        report = json.loads(result.stdout)
        assert report["lowering_plan"]["status"] == "blocked", report
        diagnostic = next(item for item in report["diagnostics"] if item["code"] == code)
        assert diagnostic["provenance"]["line"] > 0 and diagnostic["provenance"]["column"] > 0, diagnostic
        assert pathlib.Path(diagnostic["provenance"]["source"]).name == path.name, diagnostic

    print(json.dumps({"valid": "admitted", "bad_argument": "refused",
                      "bad_return": "refused", "unknown_return": "refused",
                      "llvm": "checked", "tinyvm": "checked"}))
