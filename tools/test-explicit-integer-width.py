#!/usr/bin/env python3
"""Explicit-width admission, without claiming backend migration is complete."""
import argparse
import json
import pathlib
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument("--build", type=pathlib.Path, required=True)
build = parser.parse_args().build

def run(tool, data, *args):
    return subprocess.run([str(build / tool), *args], input=data, text=True,
                          capture_output=True, timeout=30)

sources = [
    "main { x : int(1)\n0 -> return\n}",
    "fn f(x : int): c_int {0 -> return\n}\nmain {0 -> return\n}",
    "fn f(): int {0 -> return\n}\nmain {0 -> return\n}",
    "main {x : list<int>(0)\n0 -> return\n}",
    "type Count refines int { }\nmain {0 -> return\n}",
    "record Box { value : int\n}\nmain {0 -> return\n}",
]
with tempfile.TemporaryDirectory() as directory:
    root = pathlib.Path(directory)
    for index, body in enumerate(sources):
        path = root / f"case-{index}.flow"
        path.write_text("program explicit_width\n" + body + "\n")
        frontend = run("flowmini/flowmini", None, "--dump-frontend-bundle", str(path))
        assert frontend.returncode == 0, frontend.stderr
        result = run("flowanalyst/flowanalyst", frontend.stdout, "--lowering-plan-version", "2")
        assert result.returncode == 2, (index, result.stdout, result.stderr)
        report = json.loads(result.stdout)
        assert report["lowering_plan"]["status"] == "blocked", report
        assert "FLOWANALYST_INTEGER_WIDTH_REQUIRED" in result.stdout, result.stdout
    path = root / "positive.flow"
    path.write_text("program explicit_width\nmain { int : c_int(1)\n0 -> return\n}\n")
    frontend = run("flowmini/flowmini", None, "--dump-frontend-bundle", str(path))
    assert frontend.returncode == 0, frontend.stderr
    result = run("flowanalyst/flowanalyst", frontend.stdout, "--lowering-plan-version", "2")
    assert result.returncode == 0, (result.stdout, result.stderr)
    original = json.loads(result.stdout)
    attacks = 0
    for version in (1, 2):
        for status in ("ready", "blocked", "invalid", None):
            report = json.loads(json.dumps(original))
            plan = report["lowering_plan"]
            plan["version"] = version
            if status is None:
                plan.pop("status", None)
            else:
                plan["status"] = status
            plan["operations"][0]["operands"][0]["type"] = "int"
            for operation in plan["operations"]:
                operation.pop("scalar_fact", None)
            for stage in ("flowcontracts/flowvalidate", "flowbind/flowbind", "flowtools/flowparallel/flowparallel"):
                result = run(stage, json.dumps(report))
                assert result.returncode != 0, (stage, result.stdout)
                assert "bare int has no executable width" in result.stderr + result.stdout
                attacks += 1
    def good(stage, data, *args):
        result = run(stage, data, *args)
        assert result.returncode == 0, (stage, result.stdout, result.stderr)
        return result.stdout
    parallel = good("flowtools/flowparallel/flowparallel", json.dumps(original))
    optimized = good("flowoptimize/flowoptimize", parallel)
    prepared = good("flowlower/flowprepare", optimized)
    for stage, artifact, args in (
        ("flowoptimize/flowoptimize", parallel, []),
        ("flowlower/flowprepare", optimized, []),
        ("flowlower/flowlower", optimized, ["--emit-llvm", str(root / "bad.ll")]),
        ("flowlower/flowlower", prepared, ["--emit-llvm", str(root / "bad-prepared.ll")]),
        ("tinyvm/flowtinylower", prepared, ["-", str(root / "bad.tvm")]),
    ):
        report = json.loads(artifact)
        report["lowering_plan"]["operations"][0]["operands"][0]["type"] = "int"
        result = run(stage, json.dumps(report), *args)
        assert result.returncode != 0, (stage, result.stdout)
        assert "bare int has no executable width" in result.stderr + result.stdout
        attacks += 1
    assert not list(root.glob("bad*")), "refused artifact published executable output"
    print(json.dumps(dict(negative_sources=len(sources), positive_value_name=1,
                          hostile_artifacts=attacks)))
