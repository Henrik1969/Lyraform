#!/usr/bin/env python3
"""Run bounded three-fact ConfigResolve behavior through native LLVM and TinyVM."""
import argparse, pathlib, subprocess, tempfile

parser = argparse.ArgumentParser()
parser.add_argument("--build", type=pathlib.Path, required=True)
parser.add_argument("--source", type=pathlib.Path, default=pathlib.Path(__file__).with_name("finite-resolution.flow"))
args = parser.parse_args()
build = args.build.resolve()
base = args.source.read_text()
marker = "main {\n"
prefix, _ = base.split(marker, 1)

cases = [
    ("highest-tie-last", "first : Fact({present: 1, value: 11, precedence: 10, order: 1})\n second : Fact({present: 1, value: 22, precedence: 20, order: 2})\n third : Fact({present: 1, value: 33, precedence: 20, order: 3})\n result : int(resolve_highest(first, second, third, 0))", 33),
    ("highest-first", "first : Fact({present: 1, value: 11, precedence: 30, order: 1})\n second : Fact({present: 1, value: 22, precedence: 20, order: 2})\n third : Fact({present: 1, value: 33, precedence: 20, order: 3})\n result : int(resolve_highest(first, second, third, 0))", 11),
    ("missing", "first : Fact({present: 0, value: 11, precedence: 30, order: 1})\n second : Fact({present: 0, value: 22, precedence: 20, order: 2})\n third : Fact({present: 0, value: 33, precedence: 20, order: 3})\n result : int(resolve_highest(first, second, third, 0))", 1),
    ("reject-tie", "first : Fact({present: 1, value: 11, precedence: 20, order: 1})\n second : Fact({present: 1, value: 22, precedence: 20, order: 2})\n third : Fact({present: 1, value: 33, precedence: 20, order: 3})\n result : int(resolve_highest(first, second, third, 1))", 3),
    ("first-wins", "first : Fact({present: 1, value: 11, precedence: 10, order: 1})\n second : Fact({present: 1, value: 22, precedence: 20, order: 2})\n third : Fact({present: 1, value: 33, precedence: 30, order: 3})\n result : int(resolve_highest(first, second, third, 2))", 11),
    ("last-wins", "first : Fact({present: 1, value: 11, precedence: 10, order: 1})\n second : Fact({present: 1, value: 22, precedence: 20, order: 2})\n third : Fact({present: 1, value: 33, precedence: 30, order: 3})\n result : int(resolve_highest(first, second, third, 3))", 33),
]

def run(command, *, input=None, accepted=(0,)):
    result = subprocess.run(command, input=input, text=True, capture_output=True)
    if result.returncode not in accepted:
        raise RuntimeError(f"{command}: exit {result.returncode}\n{result.stdout}\n{result.stderr}")
    return result

results = {}
with tempfile.TemporaryDirectory(prefix="lyraform-finite-resolution-") as temp:
    root = pathlib.Path(temp)
    for name, body, expected in cases:
        source = root / f"{name}.flow"
        source.write_text(prefix + marker + " " + body + "\n result -> return\n}\n}\n")
        front = run([build / "flowmini/flowmini", "--dump-frontend-bundle", source]).stdout
        semantic = run([build / "flowanalyst/flowanalyst", "--lowering-plan-version", "2"], input=front).stdout
        parallel = run([build / "flowtools/flowparallel/flowparallel"], input=semantic).stdout
        optimized = run([build / "flowoptimize/flowoptimize"], input=parallel).stdout
        native_ll = root / f"{name}.ll"
        run([build / "flowlower/flowlower", "--target", "Demo", "--emit-llvm", native_ll], input=optimized)
        native = root / f"{name}.bin"
        run(["clang-18", native_ll, "-o", native])
        native_result = run([native], accepted=tuple(range(0, 256))).returncode
        if native_result != expected:
            raise RuntimeError(f"{name}: native {native_result}, expected {expected}")
        optimized_path = root / f"{name}.optimized.json"
        optimized_path.write_text(optimized)
        prepared = run([build / "flowlower/flowprepare", "--target", "Demo", optimized_path]).stdout
        tiny = root / f"{name}.tvm"
        run([build / "tinyvm/flowtinylower", "-", tiny], input=prepared)
        vm = run([build / "tinyvm/flowtinyrun", tiny]).stdout
        if f'"status":"completed"' not in vm.replace(" ", "") or f'"result":{expected}' not in vm.replace(" ", ""):
            raise RuntimeError(f"{name}: TinyVM mismatch\n{vm}")
        results[name] = {"native": native_result, "tinyvm": expected}
print({"cases": len(results), "results": results})
