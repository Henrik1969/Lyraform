#!/usr/bin/env python3
from pathlib import Path
import argparse
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument("--check", action="store_true", help="preflight only; write nothing")
args = parser.parse_args()

ROOT = Path.cwd()

def require_repo_root() -> None:
    try:
        top = subprocess.check_output(
            ["git", "rev-parse", "--show-toplevel"],
            text=True,
            stderr=subprocess.DEVNULL,
        ).strip()
    except subprocess.CalledProcessError:
        raise SystemExit("error: not inside a Git repository")
    if Path(top).resolve() != ROOT.resolve():
        raise SystemExit(f"error: run from repo root: {top}")

def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly 1 match, found {count}")
    return text.replace(old, new, 1)

def replace_exact_count(text: str, old: str, new: str, expected: int, label: str) -> str:
    count = text.count(old)
    if count != expected:
        raise RuntimeError(f"{label}: expected exactly {expected} matches, found {count}")
    return text.replace(old, new)

require_repo_root()

paths = {
    "current": ROOT / "Flowmini/CURRENT.md",
    "status": ROOT / "Flowmini/flowmini_v24_explicit_ast/docs/v0.24-explicit-ast-status.md",
    "sitrep": ROOT / "Flowmini/flowmini_v24_explicit_ast/docs/v0.24-shallow-expression-ast-sitrep.md",
}

for path in paths.values():
    if not path.is_file():
        raise SystemExit(f"error: missing expected file: {path}")

texts = {key: path.read_text(encoding="utf-8") for key, path in paths.items()}

# ----------------------------------------------------------------------
# Flowmini/CURRENT.md
# ----------------------------------------------------------------------
texts["current"] = replace_once(
    texts["current"],
    """C5.1 return expression ownership complete
C5.2 typed-binding initializer ownership complete
""",
    """C5.1 return expression ownership complete
C5.2 typed-binding initializer ownership complete
C5.3 plain-assignment value ownership complete
""",
    "CURRENT milestone",
)

texts["current"] = replace_exact_count(
    texts["current"],
    "AST golden tests: PASS (13)",
    "AST golden tests: PASS (14)",
    2,
    "CURRENT golden counts",
)

texts["current"] = replace_once(
    texts["current"],
    """canonical expression payloads, canonical type-reference payloads, and two
canonical statement-expression roles: return value ownership and typed-binding
initializer ownership.
""",
    """canonical expression payloads, canonical type-reference payloads, and three
canonical statement-expression roles: return value ownership, typed-binding
initializer ownership, and plain-assignment value ownership.
""",
    "CURRENT canonical roles",
)

texts["current"] = replace_once(
    texts["current"],
    "remaining statement-role ownership beyond return and typed-binding initializer",
    "remaining statement-role ownership beyond return, typed-binding initializer, and plain assignment",
    "CURRENT incomplete statement roles",
)

# ----------------------------------------------------------------------
# v0.24 explicit AST status
# ----------------------------------------------------------------------
texts["status"] = replace_once(
    texts["status"],
    """canonical typed-binding initializer-expression ownership
typed-binding expression_ids as a derived compatibility projection
empty initializer parentheses preserved without fabricating expression nodes
unified function/main/nested statement-body parsing
""",
    """canonical typed-binding initializer-expression ownership
typed-binding expression_ids as a derived compatibility projection
empty initializer parentheses preserved without fabricating expression nodes
canonical plain-assignment value-expression ownership
plain-assignment expression_ids as a derived compatibility projection
unified function/main/nested statement-body parsing
""",
    "status AST guarantees",
)

texts["status"] = replace_once(
    texts["status"],
    """value ownership and typed-binding initializer ownership are now migrated, with
their generic expression lists treated only as compatibility projections.
""",
    """value ownership, typed-binding initializer ownership, and plain-assignment value
ownership are now migrated, with their generic expression lists treated only as
compatibility projections.
""",
    "status validator description",
)

texts["status"] = replace_once(
    texts["status"],
    "AST golden tests: PASS (13)",
    "AST golden tests: PASS (14)",
    "status golden count",
)

# ----------------------------------------------------------------------
# C5 section in the historical SITREP.
# Do not alter the old PASS(8) checkpoint sections.
# ----------------------------------------------------------------------
texts["sitrep"] = replace_once(
    texts["sitrep"],
    """C5.2 canonical typed-binding initializer-expression ownership
C5.2 typed-binding expression_ids compatibility projection derived from that ownership
C5.2 empty initializer parentheses remain structural and do not fabricate expressions
C5.2 direct and nested typed-binding regression coverage
""",
    """C5.2 canonical typed-binding initializer-expression ownership
C5.2 typed-binding expression_ids compatibility projection derived from that ownership
C5.2 empty initializer parentheses remain structural and do not fabricate expressions
C5.2 direct and nested typed-binding regression coverage
C5.3 canonical plain-assignment value-expression ownership
C5.3 plain-assignment expression_ids compatibility projection derived from that ownership
C5.3 function, nested, and main-body assignment regression coverage
""",
    "SITREP C5 completed",
)

texts["sitrep"] = replace_once(
    texts["sitrep"],
    """assignment value ownership
if condition ownership
while condition ownership
""",
    """if condition ownership
while condition ownership
""",
    "SITREP C5 remaining",
)

texts["sitrep"] = replace_once(
    texts["sitrep"],
    """exists only when an actual expression is present.
""",
    """exists only when an actual expression is present.

For C5.3, the currently supported plain assignment form keeps its identifier
target in `name` and canonically owns only the right-hand-side value expression.
Generalized assignment targets such as field/index lvalues remain a separate
future AST design problem rather than being implied by this migration.
""",
    "SITREP assignment boundary",
)

if args.check:
    print("C5.3 documentation preflight successful: all expected live-document fragments matched; no files changed.")
    raise SystemExit(0)

for key, path in paths.items():
    path.write_text(texts[key], encoding="utf-8")

print("C5.3 documentation updates applied successfully.")
for path in paths.values():
    print(f"  {path.relative_to(ROOT)}")
