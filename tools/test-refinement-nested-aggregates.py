#!/usr/bin/env python3
"""Bounded composition of refined fields through nested ordinary aggregates."""
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


def source(value, update=""):
    return ("program refinement_nested_aggregates\n"
            "type Positive32 refines int32 {\n"
            " invariant value > 0\n"
            "}\n"
            "record Balance { amount : Positive32\n }\n"
            "record Wallet { balance : Balance\n }\n"
            "target Demo {\nmain {\n"
            f" wallet : Wallet({{balance: {{amount: {value}}}}})\n"
            f"{update}"
            " projected : c_int(wallet.balance.amount)\n"
            " 0 -> return\n"
            "}\n}\n")


with tempfile.TemporaryDirectory(prefix="lyraform-refinement-nested-aggregates-") as directory:
    root = pathlib.Path(directory)

    valid = root / "valid.flow"
    valid.write_text(source("7"))
    frontend = good([tools["frontend"], "--dump-frontend-bundle", valid])
    semantic = good([tools["analyze"], "--lowering-plan-version", "2"], frontend)
    report = json.loads(semantic)
    assert report["status"] == "ok" and report["lowering_plan"]["status"] == "ready", report
    catalog = report["lowering_plan"]["ordinary_record_types"]["types"]
    wallet = next(item for item in catalog if item["name"] == "Wallet")
    balance = next(item for item in catalog if item["name"] == "Balance")
    assert wallet["fields"][0]["type"]["kind"] == "record", catalog
    assert balance["fields"] == [{"name": "amount", "type": {"kind": "scalar", "name": "int32"}}], catalog

    parallel = good([tools["parallel"]], semantic)
    optimized = good([tools["optimize"]], parallel)
    prepared = good([tools["prepare"], "--target", "Demo"], optimized)
    good([tools["validate"]], prepared)
    llvm = root / "nested.ll"
    good([tools["lower"], "--target", "Demo", "--emit-llvm", llvm], optimized)
    good([tools["tiny_lower"], "-", root / "nested.tvm"], prepared)

    member = root / "member.flow"
    member.write_text(source("7", " 9 -> wallet.balance.amount\n"))
    member_frontend = good([tools["frontend"], "--dump-frontend-bundle", member])
    member_semantic = good([tools["analyze"], "--lowering-plan-version", "2"], member_frontend)
    member_report = json.loads(member_semantic)
    fact = next(item["target_fact"] for item in member_report["lowering_plan"]["operations"]
                if "target_fact" in item)
    assert [item["name"] for item in fact["members"]] == ["balance", "amount"], fact
    assert fact["assignability"]["state"] == "assignable", fact
    assert fact["assignability"]["update"] == "reconstruct_and_rebind", fact
    assert fact["destination_type"] == "int32", fact
    assert fact["execution"] == "unsupported", fact
    good([tools["parallel"]], member_semantic)

    for name, value, code in (
        ("invalid", "-1", "FLOWANALYST_REFINED_RECORD_FIELD_VIOLATION"),
        ("unknown", "source_value", "FLOWANALYST_REFINED_RECORD_FIELD_UNPROVEN"),
    ):
        path = root / (name + ".flow")
        path.write_text(source(value))
        parsed = good([tools["frontend"], "--dump-frontend-bundle", path])
        result = run([tools["analyze"], "--lowering-plan-version", "2"], parsed)
        assert result.returncode == 2, (name, result.stdout, result.stderr)
        assert any(item["code"] == code for item in json.loads(result.stdout)["diagnostics"])

    invalid_member = root / "invalid-member.flow"
    invalid_member.write_text(source("7", " -1 -> wallet.balance.amount\n"))
    parsed = good([tools["frontend"], "--dump-frontend-bundle", invalid_member])
    result = run([tools["analyze"], "--lowering-plan-version", "2"], parsed)
    assert result.returncode == 2, (result.stdout, result.stderr)
    assert any(item["code"] == "FLOWANALYST_REFINED_MEMBER_VIOLATION"
               for item in json.loads(result.stdout)["diagnostics"])

    print(json.dumps({"nested_construction": "admitted", "nested_projection": "lowered",
                      "nested_member": "semantically_admitted_backend_refused",
                      "invalid_nested_field": "refused", "unknown_nested_field": "refused",
                      "invalid_nested_member": "refused"}))
