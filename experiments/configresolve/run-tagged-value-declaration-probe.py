#!/usr/bin/env python3
"""Prove the closed tagged-value declaration is accepted generically."""
import pathlib
import subprocess
import sys
import tempfile

root = pathlib.Path(__file__).resolve().parents[2]
build = pathlib.Path(sys.argv[1])
source = root / "experiments/configresolve/tagged-value-declaration.flow"
binary = build / "flowmini/flowmini"
if not binary.exists():
    binary = build / "flowmini"
result = subprocess.run(
    [binary, "--emit-flowir", "-", source],
    text=True,
    capture_output=True,
)
if result.returncode:
    raise SystemExit(f"tagged-value declaration rejected:\n{result.stdout}\n{result.stderr}")
artifact = result.stdout
for case in ("Null", "Bool", "Int", "Text", "Double"):
    if case not in source.read_text():
        raise SystemExit(f"missing declared case: {case}")
if "module configresolve_tagged_value_declaration" not in artifact:
    raise SystemExit("unexpected FlowIR output")
if "const.int" not in artifact:
    raise SystemExit("probe main did not lower")
base = source.read_text()
with tempfile.TemporaryDirectory(prefix="lyraform-tagged-value-hostile-") as directory:
    duplicate = pathlib.Path(directory) / "duplicate.flow"
    duplicate.write_text(base.replace("    case Double : float64\n", "    case Double : float64\n    case Int : int\n"))
    rejected = subprocess.run([binary, "--emit-flowir", "-", duplicate], text=True, capture_output=True)
    if rejected.returncode == 0 or "duplicate variant case" not in (rejected.stdout + rejected.stderr):
        raise SystemExit("duplicate variant case was not refused")
    unknown = pathlib.Path(directory) / "unknown.flow"
    unknown.write_text(base.replace("case Text : Text", "case Text : MissingText"))
    rejected = subprocess.run([binary, "--emit-flowir", "-", unknown], text=True, capture_output=True)
    if rejected.returncode == 0 or "unknown or sealed payload type" not in (rejected.stdout + rejected.stderr):
        raise SystemExit("unknown variant payload was not refused")
print("{\"declaration\": \"accepted\", \"cases\": 5, \"execution\": \"refused-until-construction\"}")
