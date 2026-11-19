#!/usr/bin/env python3
"""Verify bounded required/range validation over structured ConfigResolve facts."""
import argparse, pathlib, subprocess, tempfile

parser = argparse.ArgumentParser()
parser.add_argument("--build", type=pathlib.Path, required=True)
parser.add_argument("--source", type=pathlib.Path, default=pathlib.Path(__file__).with_name("finite-resolution.flow"))
args = parser.parse_args(); build = args.build.resolve(); base = args.source.read_text()
marker = "main {\n"; prefix, _ = base.split(marker, 1)

cases = [
    ("valid", "first : Fact({present: 1, value: 11, precedence: 10, order: 1})\n second : Fact({present: 1, value: 22, precedence: 20, order: 2})\n third : Fact({present: 1, value: 33, precedence: 20, order: 3})\n rule : Policy({required: 1, minimum: 0, maximum: 100})\n result : int32(resolve_checked(first, second, third, rule, 0))", 33),
    ("out-of-range-first", "first : Fact({present: 1, value: 101, precedence: 30, order: 1})\n second : Fact({present: 1, value: 22, precedence: 20, order: 2})\n third : Fact({present: 1, value: 33, precedence: 20, order: 3})\n rule : Policy({required: 1, minimum: 0, maximum: 100})\n result : int32(resolve_checked(first, second, third, rule, 0))", 4),
    ("out-of-range-later", "first : Fact({present: 1, value: 11, precedence: 10, order: 1})\n second : Fact({present: 1, value: -1, precedence: 20, order: 2})\n third : Fact({present: 1, value: 33, precedence: 20, order: 3})\n rule : Policy({required: 1, minimum: 0, maximum: 100})\n result : int32(resolve_checked(first, second, third, rule, 0))", 4),
    ("missing-optional", "first : Fact({present: 0, value: 0, precedence: 10, order: 1})\n second : Fact({present: 0, value: 0, precedence: 20, order: 2})\n third : Fact({present: 0, value: 0, precedence: 20, order: 3})\n rule : Policy({required: 0, minimum: 0, maximum: 100})\n result : int32(resolve_checked(first, second, third, rule, 0))", 1),
]

def run(command, *, input=None, accepted=(0,)):
    result = subprocess.run(command, input=input, text=True, capture_output=True)
    if result.returncode not in accepted: raise RuntimeError(f"{command}: exit {result.returncode}\n{result.stdout}\n{result.stderr}")
    return result

results = {}
with tempfile.TemporaryDirectory(prefix="lyraform-finite-policy-") as temp:
    root = pathlib.Path(temp)
    for name, body, expected in cases:
        source = root / f"{name}.flow"; source.write_text(prefix + marker + " " + body + "\n result -> return\n}\n}\n")
        front = run([build / "flowmini/flowmini", "--dump-frontend-bundle", source]).stdout
        semantic = run([build / "flowanalyst/flowanalyst", "--lowering-plan-version", "2"], input=front).stdout
        parallel = run([build / "flowtools/flowparallel/flowparallel"], input=semantic).stdout
        optimized = run([build / "flowoptimize/flowoptimize"], input=parallel).stdout
        ll = root / f"{name}.ll"; run([build / "flowlower/flowlower", "--target", "Demo", "--emit-llvm", ll], input=optimized)
        native = root / f"{name}.bin"; run(["clang-18", ll, "-o", native])
        native_result = run([native], accepted=tuple(range(256))).returncode
        if native_result != expected: raise RuntimeError(f"{name}: native {native_result}, expected {expected}")
        opt = root / f"{name}.json"; opt.write_text(optimized)
        prepared = run([build / "flowlower/flowprepare", "--target", "Demo", opt]).stdout
        tvm = root / f"{name}.tvm"; run([build / "tinyvm/flowtinylower", "-", tvm], input=prepared)
        vm = run([build / "tinyvm/flowtinyrun", tvm]).stdout.replace(" ", "")
        if '"status":"completed"' not in vm or f'"result":{expected}' not in vm or '"trap":0' not in vm: raise RuntimeError(f"{name}: TinyVM mismatch\n{vm}")
        results[name] = expected
print({"cases": len(results), "results": results})
