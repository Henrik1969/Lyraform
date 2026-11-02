#!/usr/bin/env python3
"""Prove immutable Text fields in finite ordinary records on both backends."""
import json
import pathlib
import subprocess
import sys
import tempfile

root = pathlib.Path(__file__).resolve().parents[2]
build = pathlib.Path(sys.argv[1])
source = pathlib.Path(sys.argv[2]) if len(sys.argv) > 2 else root / "experiments/configresolve/text-record.flow"

def run(command, *, input=None):
    result = subprocess.run(command, input=input, text=True, capture_output=True)
    if result.returncode:
        raise SystemExit(f"failed: {' '.join(map(str, command))}\n{result.stdout}\n{result.stderr}")
    return result.stdout

with tempfile.TemporaryDirectory(prefix="lyraform-configresolve-text-record-") as directory:
    out = pathlib.Path(directory)
    front = run([build / "flowmini/flowmini", "--dump-frontend-bundle", source])
    semantic = run([build / "flowanalyst/flowanalyst", "--lowering-plan-version", "2"], input=front)
    plan = json.loads(semantic)["lowering_plan"]
    assert plan["status"] == "ready" and plan["ordinary_record_execution"]["bound"] == "finite_scalar_nested"
    parallel = run([build / "flowtools/flowparallel/flowparallel"], input=semantic)
    optimized = run([build / "flowoptimize/flowoptimize"], input=parallel)
    native_ir = out / "program.ll"
    run([build / "flowlower/flowlower", "--emit-llvm", native_ir, "--target", "Demo"], input=optimized)
    native = out / "native"
    run(["clang-18", native_ir, "-o", native])
    native_result = subprocess.run([native]).returncode
    prepared = run([build / "flowlower/flowprepare", "--target", "Demo"], input=optimized)
    prepared_path = out / "prepared.json"
    prepared_path.write_text(prepared)
    artifact = out / "program.tvm"
    run([build / "tinyvm/flowtinylower", prepared_path, artifact])
    tiny = json.loads(run([build / "tinyvm/flowtinyrun", artifact]))
    assert native_result == 7 and tiny["result"] == 7 and tiny["trap"] == 0
    print(json.dumps({"native": native_result, "tinyvm": tiny["result"], "trap": tiny["trap"]}, sort_keys=True))
