#!/usr/bin/env python3
from pathlib import Path
import argparse
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument("--check", action="store_true")
args = parser.parse_args()

ROOT = Path.cwd()

def require_repo_root():
    top = subprocess.check_output(
        ["git", "rev-parse", "--show-toplevel"],
        text=True,
        stderr=subprocess.DEVNULL,
    ).strip()
    if Path(top).resolve() != ROOT.resolve():
        raise SystemExit(f"error: run from repo root: {top}")

def replace_once(text, old, new, label):
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly 1 match, found {count}")
    return text.replace(old, new, 1)

def replace_exact_count(text, old, new, expected, label):
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
texts = {k: p.read_text(encoding="utf-8") for k, p in paths.items()}

texts["current"] = replace_once(
    texts["current"],
    "C5.2 typed-binding initializer ownership complete\nC5.3 plain-assignment value ownership complete\n",
    "C5.2 typed-binding initializer ownership complete\nC5.3 plain-assignment value ownership complete\nC5.4 if/while condition ownership complete\n",
    "CURRENT milestone",
)
texts["current"] = replace_exact_count(
    texts["current"], "AST golden tests: PASS (14)", "AST golden tests: PASS (15)", 2,
    "CURRENT golden counts",
)
texts["current"] = replace_once(
    texts["current"],
    "canonical expression payloads, canonical type-reference payloads, and three\ncanonical statement-expression roles: return value ownership, typed-binding\ninitializer ownership, and plain-assignment value ownership.\n",
    "canonical expression payloads, canonical type-reference payloads, and canonical\nstatement-expression roles for return values, typed-binding initializers,\nplain-assignment values, and if/while conditions.\n",
    "CURRENT canonical roles",
)
texts["current"] = replace_once(
    texts["current"],
    "remaining statement-role ownership beyond return, typed-binding initializer, and plain assignment",
    "remaining statement structure beyond the migrated expression-owning statement roles",
    "CURRENT incomplete statement roles",
)

texts["status"] = replace_once(
    texts["status"],
    "canonical plain-assignment value-expression ownership\nplain-assignment expression_ids as a derived compatibility projection\nunified function/main/nested statement-body parsing\n",
    "canonical plain-assignment value-expression ownership\nplain-assignment expression_ids as a derived compatibility projection\ncanonical if/while condition-expression ownership\nif/while expression_ids as a derived compatibility projection\nunified function/main/nested statement-body parsing\n",
    "status guarantees",
)
texts["status"] = replace_once(
    texts["status"],
    "value ownership, typed-binding initializer ownership, and plain-assignment value\nownership are now migrated, with their generic expression lists treated only as\ncompatibility projections.\n",
    "value ownership, typed-binding initializer ownership, plain-assignment value\nownership, and if/while condition ownership are now migrated, with their generic\nexpression lists treated only as compatibility projections.\n",
    "status validator description",
)
texts["status"] = replace_once(
    texts["status"], "AST golden tests: PASS (14)", "AST golden tests: PASS (15)",
    "status golden count",
)

texts["sitrep"] = replace_once(
    texts["sitrep"],
    "C5.3 canonical plain-assignment value-expression ownership\nC5.3 plain-assignment expression_ids compatibility projection derived from that ownership\nC5.3 function, nested, and main-body assignment regression coverage\n",
    "C5.3 canonical plain-assignment value-expression ownership\nC5.3 plain-assignment expression_ids compatibility projection derived from that ownership\nC5.3 function, nested, and main-body assignment regression coverage\nC5.4 canonical if/while condition-expression ownership\nC5.4 if/while expression_ids compatibility projection derived from that ownership\nC5.4 function, nested, and main-body condition regression coverage\n",
    "SITREP completed",
)
texts["sitrep"] = replace_once(
    texts["sitrep"],
    "if condition ownership\nwhile condition ownership\nelse blocks\n",
    "else blocks\n",
    "SITREP remaining",
)
texts["sitrep"] = replace_once(
    texts["sitrep"],
    "Generalized assignment targets such as field/index lvalues remain a separate\nfuture AST design problem rather than being implied by this migration.\n",
    "Generalized assignment targets such as field/index lvalues remain a separate\nfuture AST design problem rather than being implied by this migration.\n\nFor C5.4, `if` and `while` canonically own a `condition_expression`. The legacy\n`has_condition` and `expression_ids` JSON fields remain derived compatibility\nprojections.\n",
    "SITREP condition law",
)

if args.check:
    print("C5.4 documentation preflight successful: all expected fragments matched; no files changed.")
    raise SystemExit(0)

for k, p in paths.items():
    p.write_text(texts[k], encoding="utf-8")

print("C5.4 documentation updates applied successfully.")
for p in paths.values():
    print(f"  {p.relative_to(ROOT)}")
