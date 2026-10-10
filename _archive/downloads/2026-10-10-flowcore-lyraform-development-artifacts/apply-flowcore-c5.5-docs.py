#!/usr/bin/env python3
from pathlib import Path
import argparse
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument("--check", action="store_true", help="preflight only; write nothing")
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

for p in paths.values():
    if not p.is_file():
        raise SystemExit(f"error: missing expected file: {p}")

texts = {k: p.read_text(encoding="utf-8") for k, p in paths.items()}

# ----------------------------------------------------------------------
# Flowmini/CURRENT.md
# ----------------------------------------------------------------------
texts["current"] = replace_once(
    texts["current"],
    "C5.3 plain-assignment value ownership complete\nC5.4 if/while condition ownership complete\n",
    "C5.3 plain-assignment value ownership complete\nC5.4 if/while condition ownership complete\nC5.5 optional if-owned else branches complete\n",
    "CURRENT milestone",
)

texts["current"] = replace_exact_count(
    texts["current"],
    "AST golden tests: PASS (15)",
    "AST golden tests: PASS (16)",
    2,
    "CURRENT golden counts",
)

texts["current"] = replace_once(
    texts["current"],
    "statement-expression roles for return values, typed-binding initializers,\nplain-assignment values, and if/while conditions.\n",
    "statement-expression roles for return values, typed-binding initializers,\nplain-assignment values, and if/while conditions. If statements also own an\noptional explicit else branch when one is present in source.\n",
    "CURRENT canonical statement summary",
)

texts["current"] = replace_once(
    texts["current"],
    "remaining statement structure beyond the migrated expression-owning statement roles\nelse blocks\n",
    "remaining statement structure beyond the migrated expression-owning roles and if/else branches\n",
    "CURRENT incomplete else removal",
)

# ----------------------------------------------------------------------
# Explicit AST status
# ----------------------------------------------------------------------
texts["status"] = replace_once(
    texts["status"],
    "canonical if/while condition-expression ownership\nif/while expression_ids as a derived compatibility projection\nunified function/main/nested statement-body parsing\n",
    "canonical if/while condition-expression ownership\nif/while expression_ids as a derived compatibility projection\noptional if-owned else branches\nnested else-branch statement bodies\nabsence of synthetic else branches when source omits else\nunified function/main/nested statement-body parsing\n",
    "status AST guarantees",
)

texts["status"] = replace_once(
    texts["status"],
    "semantic validation and lowering of arbitrary postfix chains\nelse blocks\nsemantic validation of type references\n",
    "semantic validation and lowering of arbitrary postfix chains\nsemantic validation of type references\n",
    "status incomplete else removal",
)

texts["status"] = replace_once(
    texts["status"],
    "AST golden tests: PASS (15)",
    "AST golden tests: PASS (16)",
    "status golden count",
)

# ----------------------------------------------------------------------
# C5 section in historical SITREP
# ----------------------------------------------------------------------
texts["sitrep"] = replace_once(
    texts["sitrep"],
    "C5.4 canonical if/while condition-expression ownership\nC5.4 if/while expression_ids compatibility projection derived from that ownership\nC5.4 function, nested, and main-body condition regression coverage\n",
    "C5.4 canonical if/while condition-expression ownership\nC5.4 if/while expression_ids compatibility projection derived from that ownership\nC5.4 function, nested, and main-body condition regression coverage\nC5.5 optional else branch owned directly by If\nC5.5 nested else-body statement population\nC5.5 no synthetic else branch when source omits else\nC5.5 nested if/else regression coverage\n",
    "SITREP C5 completed",
)

texts["sitrep"] = replace_once(
    texts["sitrep"],
    "For C5.4, `if` and `while` canonically own a `condition_expression`. The legacy\n`has_condition` and `expression_ids` JSON fields remain derived compatibility\nprojections.\n",
    "For C5.4, `if` and `while` canonically own a `condition_expression`. The legacy\n`has_condition` and `expression_ids` JSON fields remain derived compatibility\nprojections.\n\nFor C5.5, `else` is not a standalone statement kind. It is an optional branch\nowned by the immediately preceding `if`. The existing `body` remains the true\nbranch; `else_body` is present only when the source contains an explicit\n`else { ... }`. No synthetic empty false branch is created. `else if` sugar\nremains deliberately outside the C5.5 boundary; equivalent nested structure can\nalready be expressed as an `if` inside `else_body`.\n",
    "SITREP else law",
)

texts["sitrep"] = replace_once(
    texts["sitrep"],
    "else blocks\nexpression statements\nflow statements if retained\n",
    "expression statements\nflow statements if retained\n",
    "SITREP C5 remaining",
)

if args.check:
    print("C5.5 documentation preflight successful: all expected fragments matched; no files changed.")
    raise SystemExit(0)

for k, p in paths.items():
    p.write_text(texts[k], encoding="utf-8")

print("C5.5 documentation updates applied successfully.")
for p in paths.values():
    print(f"  {p.relative_to(ROOT)}")
