#!/usr/bin/env python3
from pathlib import Path
import argparse
import subprocess
import sys

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

# Flowmini/CURRENT.md
texts["current"] = replace_once(
    texts["current"],
    """Flowmini v0.24 explicit AST stabilization
Road C5 statement deepening in progress
C5.1 return expression ownership complete
""",
    """Flowmini v0.24 explicit AST stabilization
Road C5 statement deepening in progress
C5.1 return expression ownership complete
C5.2 typed-binding initializer ownership complete
""",
    "CURRENT milestone",
)

texts["current"] = replace_once(
    texts["current"],
    "AST golden tests: PASS (12)",
    "AST golden tests: PASS (13)",
    "CURRENT status golden count",
)
texts["current"] = replace_once(
    texts["current"],
    "AST golden tests: PASS (12)",
    "AST golden tests: PASS (13)",
    "CURRENT expected golden count",
)

texts["current"] = replace_once(
    texts["current"],
    """canonical expression payloads, canonical type-reference payloads, and the
first canonical statement-expression role: return value ownership.
""",
    """canonical expression payloads, canonical type-reference payloads, and two
canonical statement-expression roles: return value ownership and typed-binding
initializer ownership.
""",
    "CURRENT canonical roles",
)

texts["current"] = replace_once(
    texts["current"],
    "remaining statement-role ownership beyond return",
    "remaining statement-role ownership beyond return and typed-binding initializer",
    "CURRENT incomplete statement roles",
)

# v0.24 explicit AST status
texts["status"] = replace_once(
    texts["status"],
    """canonical return value-expression ownership
return expression_ids as a derived compatibility projection
unified function/main/nested statement-body parsing
""",
    """canonical return value-expression ownership
return expression_ids as a derived compatibility projection
canonical typed-binding initializer-expression ownership
typed-binding expression_ids as a derived compatibility projection
empty initializer parentheses preserved without fabricating expression nodes
unified function/main/nested statement-body parsing
""",
    "status AST guarantees",
)

texts["status"] = replace_once(
    texts["status"],
    """it also checks statement-role ownership as those roles become canonical; return
value ownership is the first migrated statement role.
""",
    """it also checks statement-role ownership as those roles become canonical. Return
value ownership and typed-binding initializer ownership are now migrated, with
their generic expression lists treated only as compatibility projections.
""",
    "status validator description",
)

texts["status"] = replace_once(
    texts["status"],
    "AST golden tests: PASS (12)",
    "AST golden tests: PASS (13)",
    "status golden count",
)

# SITREP C5 section
texts["sitrep"] = replace_once(
    texts["sitrep"],
    """C5.1 canonical return value-expression ownership
C5.1 return expression_ids compatibility projection derived from that ownership
C5.1 unified function/main/nested statement-body parsing
C5.1 direct and nested return regression coverage
""",
    """C5.1 canonical return value-expression ownership
C5.1 return expression_ids compatibility projection derived from that ownership
C5.1 unified function/main/nested statement-body parsing
C5.1 direct and nested return regression coverage
C5.2 canonical typed-binding initializer-expression ownership
C5.2 typed-binding expression_ids compatibility projection derived from that ownership
C5.2 empty initializer parentheses remain structural and do not fabricate expressions
C5.2 direct and nested typed-binding regression coverage
""",
    "SITREP C5 completed",
)

texts["sitrep"] = replace_once(
    texts["sitrep"],
    """typed-binding initializer ownership
assignment value ownership
if condition ownership
""",
    """assignment value ownership
if condition ownership
""",
    "SITREP C5 remaining",
)

texts["sitrep"] = replace_once(
    texts["sitrep"],
    """migration, but they are not the canonical meaning of a migrated statement role.
""",
    """migration, but they are not the canonical meaning of a migrated statement role.

For typed bindings, empty `()` remains meaningful structural syntax but does
not manufacture a fake initializer expression. Canonical initializer ownership
exists only when an actual expression is present.
""",
    "SITREP C5 law extension",
)

if args.check:
    print("C5.2 documentation preflight successful: all expected live-document fragments matched; no files changed.")
    raise SystemExit(0)

for key, path in paths.items():
    path.write_text(texts[key], encoding="utf-8")

print("C5.2 documentation updates applied successfully.")
for path in paths.values():
    print(f"  {path.relative_to(ROOT)}")
