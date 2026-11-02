#!/usr/bin/env python3
"""Verify record-valued branch returns through native LLVM and TinyVM."""
import argparse, pathlib, subprocess, tempfile

parser = argparse.ArgumentParser()
parser.add_argument("--build", type=pathlib.Path, required=True)
parser.add_argument("--source", type=pathlib.Path, default=pathlib.Path(__file__).with_name("record-result-branch.flow"))
args = parser.parse_args()
build = args.build.resolve(); source = args.source.read_text()

def run(command, *, input=None, accepted=(0,)):
    result = subprocess.run(command, input=input, text=True, capture_output=True)
    if result.returncode not in accepted:
        raise RuntimeError(f"{command}: exit {result.returncode}\n{result.stdout}\n{result.stderr}")
    return result

results = {}
with tempfile.TemporaryDirectory(prefix="lyraform-record-result-branch-") as temp:
    root = pathlib.Path(temp)
    for flag, expected in ((0, 11), (1, 22)):
        current = root / f"branch-{flag}.flow"
        current.write_text(source.replace("choose(0)", f"choose({flag})"))
        front = run([build / "flowmini/flowmini", "--dump-frontend-bundle", current]).stdout
        semantic = run([build / "flowanalyst/flowanalyst", "--lowering-plan-version", "2"], input=front).stdout
        parallel = run([build / "flowtools/flowparallel/flowparallel"], input=semantic).stdout
        optimized = run([build / "flowoptimize/flowoptimize"], input=parallel).stdout
        ll = root / f"branch-{flag}.ll"
        run([build / "flowlower/flowlower", "--target", "Demo", "--emit-llvm", ll], input=optimized)
        native = root / f"branch-{flag}.bin"; run(["clang-18", ll, "-o", native])
        native_result = run([native], accepted=tuple(range(256))).returncode
        if native_result != expected: raise RuntimeError(f"flag {flag}: native {native_result}, expected {expected}")
        opt = root / f"branch-{flag}.json"; opt.write_text(optimized)
        prepared = run([build / "flowlower/flowprepare", "--target", "Demo", opt]).stdout
        tvm = root / f"branch-{flag}.tvm"; run([build / "tinyvm/flowtinylower", "-", tvm], input=prepared)
        vm = run([build / "tinyvm/flowtinyrun", tvm]).stdout.replace(" ", "")
        if '"status":"completed"' not in vm or f'"result":{expected}' not in vm or '"trap":0' not in vm:
            raise RuntimeError(f"flag {flag}: TinyVM mismatch\n{vm}")
        results[str(flag)] = expected
print({"cases": len(results), "results": results})
