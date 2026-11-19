#!/usr/bin/env python3
"""Bounded R4 refined aggregate fields and member-boundary evidence."""
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
    "tiny_lower": "tinyvm/flowtinylower",
}.items()}


def run(command, data=None):
    return subprocess.run(list(map(str, command)), input=data, text=True,
                          capture_output=True, timeout=30)


def good(command, data=None):
    result = run(command, data)
    assert result.returncode == 0, (command, result.returncode, result.stdout, result.stderr)
    return result.stdout


def source(field_value, member_update=""):
    return ("program refinement_aggregate_fields\n"
            "type Positive32 refines int32 {\n"
            " invariant value > 0\n"
            "}\n"
            "record Wallet { balance : Positive32\n }\n"
            "target Demo {\n"
            "main {\n"
            f" wallet : Wallet({{balance: {field_value}}})\n"
            f"{member_update}"
            " value : c_int(wallet.balance)\n"
            " 0 -> return\n"
            "}\n"
            "}\n")


with tempfile.TemporaryDirectory(prefix="lyraform-refinement-aggregate-fields-") as directory:
    root = pathlib.Path(directory)

    valid_path = root / "valid.flow"
    valid_path.write_text(source("7"))
    frontend = good([tools["frontend"], "--dump-frontend-bundle", valid_path])
    semantic = good([tools["analyze"], "--lowering-plan-version", "2"], frontend)
    report = json.loads(semantic)
    assert report["status"] == "ok" and report["lowering_plan"]["status"] == "ready", report
    catalog = report["lowering_plan"]["ordinary_record_types"]
    wallet = next(item for item in catalog["types"] if item["name"] == "Wallet")
    assert wallet["fields"] == [{"name": "balance", "type": {"kind": "scalar", "name": "int32"}}], catalog
    assert any(item.get("kind") == "record_construct" for operation in report["lowering_plan"]["operations"]
               for item in operation.get("operands", [])), report["lowering_plan"]

    bound = good([tools["bind"]], semantic)
    parallel = good([tools["parallel"]], semantic)
    optimized = good([tools["optimize"]], parallel)
    prepared_path = root / "prepared.json"
    prepared_path.write_text(good([tools["prepare"], "--target", "Demo"], optimized))
    good([tools["validate"]], prepared_path.read_text())
    llvm = root / "valid.ll"
    good([tools["lower"], "--target", "Demo", "--emit-llvm", llvm], optimized)
    native = root / "native"
    native_ir = root / "native.ll"
    native_ir.write_text(llvm.read_text().replace("@main(", "@review_entry("))
    driver = root / "driver.c"
    driver.write_text("extern int review_entry(void);\nint main(void) { return review_entry() == 0 ? 0 : 1; }\n")
    good(["clang-18", native_ir, driver, "-o", native])
    assert run([native]).returncode == 0

    invalid_path = root / "invalid.flow"
    invalid_path.write_text(source("-1"))
    invalid_frontend = good([tools["frontend"], "--dump-frontend-bundle", invalid_path])
    invalid = run([tools["analyze"], "--lowering-plan-version", "2"], invalid_frontend)
    assert invalid.returncode == 2, (invalid.stdout, invalid.stderr)
    invalid_report = json.loads(invalid.stdout)
    diagnostic = next(item for item in invalid_report["diagnostics"]
                       if item["code"] == "FLOWANALYST_REFINED_RECORD_FIELD_VIOLATION")
    assert diagnostic["provenance"]["line"] > 0, diagnostic

    unknown_path = root / "unknown.flow"
    unknown_path.write_text(source("source_value"))
    unknown_frontend = good([tools["frontend"], "--dump-frontend-bundle", unknown_path])
    unknown = run([tools["analyze"], "--lowering-plan-version", "2"], unknown_frontend)
    assert unknown.returncode == 2, (unknown.stdout, unknown.stderr)
    unknown_report = json.loads(unknown.stdout)
    assert any(item["code"] == "FLOWANALYST_REFINED_RECORD_FIELD_UNPROVEN"
               for item in unknown_report["diagnostics"]), unknown_report

    member_path = root / "member.flow"
    member_path.write_text(source("7", " 9 -> wallet.balance\n"))
    member_frontend = good([tools["frontend"], "--dump-frontend-bundle", member_path])
    member_semantic = good([tools["analyze"], "--lowering-plan-version", "2"], member_frontend)
    member_report = json.loads(member_semantic)
    fact = next(item["target_fact"] for item in member_report["lowering_plan"]["operations"]
                if "target_fact" in item)
    assert fact["assignability"]["state"] == "assignable", fact
    assert fact["destination_type"] == "int32", fact
    assert fact["assignability"]["source_type"] == "int32", fact
    member_parallel = good([tools["parallel"]], member_semantic)
    member_optimized = good([tools["optimize"]], member_parallel)
    member_prepared = root / "member-prepared.json"
    member_prepared.write_text(good([tools["prepare"], "--target", "Demo"], member_optimized))
    good([tools["validate"]], member_prepared.read_text())
    assert run([tools["lower"], "--emit-llvm", root / "member.ll", member_optimized]).returncode != 0
    assert run([tools["tiny_lower"], member_prepared, root / "member.tvm"]).returncode != 0

    for name, value, code in (
        ("member-invalid", "-1", "FLOWANALYST_REFINED_MEMBER_VIOLATION"),
        ("member-unknown", "source_value", "FLOWANALYST_REFINED_MEMBER_UNPROVEN"),
    ):
        path = root / (name + ".flow")
        path.write_text(source("7", f" {value} -> wallet.balance\n"))
        parsed = good([tools["frontend"], "--dump-frontend-bundle", path])
        result = run([tools["analyze"], "--lowering-plan-version", "2"], parsed)
        assert result.returncode == 2, (name, result.stdout, result.stderr)
        report = json.loads(result.stdout)
        assert any(item["code"] == code for item in report["diagnostics"]), report

    print(json.dumps({"field_construction": "admitted", "field_projection": "executed",
                      "member_update": "semantically_admitted_backend_refused",
                      "invalid_field": "refused", "unknown_field": "refused",
                      "invalid_member": "refused", "unknown_member": "refused"}))
