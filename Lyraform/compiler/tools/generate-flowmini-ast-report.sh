#!/usr/bin/env bash
set -euo pipefail

# Flowmini v25 AST report generator
# ---------------------------------
#
# Purpose:
#   Start from a blank slate:
#       configure/build Flowmini v25
#       generate AST gallery inputs
#       run --dump-ast over pass examples and gallery examples
#       validate JSON
#       run the Flowmini suite
#       generate a Markdown report for docs/sessions
#
# Location:
#   This script is version-local and is expected to live at:
#
#       Lyraform/compiler/tools/generate-flowmini-ast-report.sh
#
# Usage:
#   cd ~/Projekter/scratchpad/flow_Policy_envelope_pattern/Lyraform/compiler
#   tools/generate-flowmini-ast-report.sh
#
# Optional env:
#   FLOWMINI_ROOT=/path/to/Lyraform/compiler
#   REPO_ROOT=/path/to/repo/root
#   BUILD_DIR=/path/to/build-dir
#   JOBS=20
#
# Important Henrik-machine rule:
#   Interactive rm is padded by safe-rm.
#   This script uses /usr/bin/rm explicitly for internal generated cleanup.

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
FLOWMINI_ROOT_DEFAULT="$(cd -- "$SCRIPT_DIR/.." && pwd)"
REPO_ROOT_DEFAULT="$(cd -- "$FLOWMINI_ROOT_DEFAULT/../.." && pwd)"

FLOWMINI_ROOT="${FLOWMINI_ROOT:-$FLOWMINI_ROOT_DEFAULT}"
REPO_ROOT="${REPO_ROOT:-$REPO_ROOT_DEFAULT}"
BUILD_DIR="${BUILD_DIR:-$FLOWMINI_ROOT/cmake-build-ast-report}"
JOBS="${JOBS:-20}"

STAMP="$(date +%Y%m%d_%H%M%S)"
REPORT_BASE="$FLOWMINI_ROOT/test-report/ast-report"
REPORT_DIR="$REPORT_BASE/$STAMP"
REPORT_LATEST="$REPORT_BASE/latest"

DUMPS_DIR="$REPORT_DIR/dumps"
ERRORS_DIR="$REPORT_DIR/errors"
INPUTS_DIR="$REPORT_DIR/inputs"
SUMMARY_JSON="$REPORT_DIR/summary.json"
SUMMARY_MD="$REPORT_DIR/summary.md"
SUITE_LOG="$REPORT_DIR/flowmini_suite.log"
BAD_JSON_LIST="$REPORT_DIR/bad-json.txt"

DOCS_DIR="$REPO_ROOT/docs/sessions"
REPORT_MD="$DOCS_DIR/flowmini-v25-ast-test-report-$STAMP.md"
REPORT_MD_LATEST="$DOCS_DIR/flowmini-v25-ast-test-report-latest.md"

if [[ ! -d "$FLOWMINI_ROOT" ]]; then
    echo "error: FLOWMINI_ROOT does not exist: $FLOWMINI_ROOT" >&2
    exit 1
fi

if [[ ! -f "$FLOWMINI_ROOT/CMakeLists.txt" ]]; then
    echo "error: FLOWMINI_ROOT is not a CMake project: $FLOWMINI_ROOT" >&2
    exit 1
fi

mkdir -p "$REPORT_BASE" "$REPORT_DIR" "$DUMPS_DIR" "$ERRORS_DIR" "$INPUTS_DIR" "$DOCS_DIR"

/usr/bin/rm -f -- "$REPORT_LATEST"
ln -sfn -- "$REPORT_DIR" "$REPORT_LATEST"

echo "== Flowmini AST report =="
echo "repo root:     $REPO_ROOT"
echo "flowmini root: $FLOWMINI_ROOT"
echo "build dir:     $BUILD_DIR"
echo "report dir:    $REPORT_DIR"
echo "jobs:          $JOBS"
echo

echo "== generating AST gallery inputs =="

cat > "$INPUTS_DIR/function_signature_gallery.flow" <<'EOF'
program function_signature_gallery

fn no_args(): int32 {
    zero : int32(0)
    zero -> return
}

fn one_arg(x : int32): int32 {
    x -> return
}

fn two_args(a : int32, b : int32): int32 {
    a + b -> return
}

fn bool_arg(flag : Bool): Bool {
    flag -> return
}

fn many_args(a : int32, b : int32, c : int32, d : int32): int32 {
    a + b + c + d -> return
}

main {
    one : int32(1)
    two : int32(2)
    three : int32(3)
    four : int32(4)
    out : int32(0)

    many_args(one, two, three, four) -> out
    print out
}
EOF

cat > "$INPUTS_DIR/function_signature_spacing.flow" <<'EOF'
program function_signature_spacing

fn compact(a:int32,b:int32):int32 {
    a + b -> return
}

fn spaced (
    a : int32,
    b : int32
) : int32 {
    a + b -> return
}

main {
    one : int32(1)
    two : int32(2)
    out : int32(0)

    compact(one, two) -> out
    print out

    spaced(one, two) -> out
    print out
}
EOF

echo "== configuring/building =="
cmake -S "$FLOWMINI_ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug
cmake --build "$BUILD_DIR" --target flowmini -j "$JOBS"

BIN="$BUILD_DIR/flowmini"

if [[ ! -x "$BIN" ]]; then
    echo "error: built binary not found/executable: $BIN" >&2
    exit 1
fi

echo
echo "== collecting AST dumps =="

dump_one() {
    local input="$1"
    local label="$2"

    local safe_label
    safe_label="$(printf '%s' "$label" | sed 's#[/ ]#__#g; s#[^A-Za-z0-9_.-]#_#g')"

    local out="$DUMP_DIR/${safe_label}.ast.json"
    local err="$ERRORS_DIR/${safe_label}.err"

    if "$BIN" --dump-ast "$input" > "$out" 2> "$err"; then
        if [[ ! -s "$err" ]]; then
            /usr/bin/rm -f -- "$err"
        fi
        echo "ok:   $label"
    else
        echo "fail: $label"
    fi
}

cd "$FLOWMINI_ROOT"

while IFS= read -r file; do
    dump_one "$file" "$file"
done < <(find examples/pass -name '*.flow' | sort)

while IFS= read -r file; do
    dump_one "$file" "gallery/$(basename "$file")"
done < <(find "$INPUTS_DIR" -name '*.flow' | sort)

echo
echo "== validating JSON =="

: > "$BAD_JSON_LIST"

while IFS= read -r file; do
    if ! python3 -m json.tool "$file" > /dev/null 2>&1; then
        echo "$file" >> "$BAD_JSON_LIST"
        echo "bad json: $file"
    fi
done < <(find "$DUMPS_DIR" -name '*.ast.json' | sort)

echo
echo "== running suite =="
set +e
cmake --build "$BUILD_DIR" --target flowmini_suite -j "$JOBS" > "$SUITE_LOG" 2>&1
SUITE_STATUS=$?
set -e

if [[ "$SUITE_STATUS" -eq 0 ]]; then
    echo "suite: PASS"
else
    echo "suite: FAIL, see $SUITE_LOG"
fi

echo
echo "== summarizing AST dumps =="

mkdir -p "$REPORT_DIR" "$DUMPS_DIR" "$ERRORS_DIR"

python3 - "$DUMPS_DIR" "$ERRORS_DIR" "$BAD_JSON_LIST" "$SUMMARY_JSON" "$SUMMARY_MD" <<'PY'
import json
import pathlib
import sys

dumps_dir = pathlib.Path(sys.argv[1])
errors_dir = pathlib.Path(sys.argv[2])
bad_json_list = pathlib.Path(sys.argv[3])
summary_json = pathlib.Path(sys.argv[4])
summary_md = pathlib.Path(sys.argv[5])

bad_json = set()
if bad_json_list.exists():
    bad_json = {line.strip() for line in bad_json_list.read_text().splitlines() if line.strip()}

items = []
totals = {
    "files": 0,
    "valid_json": 0,
    "bad_json": len(bad_json),
    "failed_dumps": 0,
    "programs": 0,
    "units": 0,
    "functions": 0,
    "main_blocks": 0,
    "parameters": 0,
    "functions_with_return_type": 0,
}

for path in sorted(dumps_dir.glob("*.ast.json")):
    rel = path.name
    totals["files"] += 1

    if str(path) in bad_json:
        items.append({
            "file": rel,
            "valid_json": False,
            "error": "invalid JSON",
        })
        continue

    try:
        data = json.loads(path.read_text())
    except Exception as exc:
        items.append({
            "file": rel,
            "valid_json": False,
            "error": str(exc),
        })
        continue

    totals["valid_json"] += 1

    source_unit = data.get("source_unit", {})
    unit_kind = source_unit.get("kind", "unknown")
    unit_name = source_unit.get("name", "")
    declarations = source_unit.get("declarations", [])

    if unit_kind == "program":
        totals["programs"] += 1
    elif unit_kind == "unit":
        totals["units"] += 1

    decl_items = []
    for decl in declarations:
        kind = decl.get("kind", "unknown")

        if kind == "function":
            totals["functions"] += 1
            param_count = int(decl.get("parameter_count", 0) or 0)
            totals["parameters"] += param_count

            return_type = decl.get("return_type", "")
            if return_type:
                totals["functions_with_return_type"] += 1

            decl_items.append({
                "kind": "function",
                "name": decl.get("name", ""),
                "parameter_count": param_count,
                "parameters": decl.get("parameters", []),
                "return_type": return_type,
                "body_statement_count": decl.get("body_statement_count", 0),
            })
        elif kind == "main_block":
            totals["main_blocks"] += 1
            decl_items.append({
                "kind": "main_block",
                "body_statement_count": decl.get("body_statement_count", 0),
            })
        else:
            decl_items.append(decl)

    items.append({
        "file": rel,
        "valid_json": True,
        "source_unit": {
            "kind": unit_kind,
            "name": unit_name,
            "declaration_count": source_unit.get("declaration_count", len(declarations)),
        },
        "declarations": decl_items,
        "expression_pool_size": data.get("expression_pool_size", 0),
    })

failed_errs = sorted(errors_dir.glob("*.err"))
totals["failed_dumps"] = len(failed_errs)

summary = {
    "totals": totals,
    "items": items,
    "failed_errors": [p.name for p in failed_errs],
}

summary_json.write_text(json.dumps(summary, indent=2) + "\n")

lines = []
lines.append("# Flowmini v25 AST Dump Summary")
lines.append("")
lines.append("## Totals")
lines.append("")

for key, value in totals.items():
    lines.append(f"- `{key}`: {value}")

lines.append("")
lines.append("## Per-file Summary")
lines.append("")

for item in items:
    lines.append(f"### {item['file']}")

    if not item.get("valid_json"):
        lines.append("")
        lines.append(f"- invalid JSON: {item.get('error', '')}")
        lines.append("")
        continue

    su = item["source_unit"]

    lines.append("")
    lines.append(f"- source unit: `{su['kind']}` `{su['name']}`")
    lines.append(f"- declaration count: `{su['declaration_count']}`")
    lines.append(f"- expression pool size: `{item.get('expression_pool_size', 0)}`")
    lines.append("")
    lines.append("Declarations:")
    lines.append("")

    for decl in item["declarations"]:
        if decl.get("kind") == "function":
            params = decl.get("parameters", [])

            if params:
                param_text = ", ".join(
                    f"{p.get('name', '')}: {p.get('type', '')}"
                    for p in params
                )
            else:
                param_text = ""

            lines.append(
                f"- function `{decl.get('name', '')}`"
                f" params={decl.get('parameter_count', 0)}"
                f" ({param_text})"
                f" return=`{decl.get('return_type', '')}`"
                f" body_statements={decl.get('body_statement_count', 0)}"
            )
        elif decl.get("kind") == "main_block":
            lines.append(
                f"- main_block body_statements={decl.get('body_statement_count', 0)}"
            )
        else:
            lines.append(f"- {decl.get('kind', 'unknown')}: `{decl}`")

    lines.append("")

summary_md.write_text("\n".join(lines) + "\n")
PY

GIT_BRANCH="$(git -C "$REPO_ROOT" branch --show-current 2>/dev/null || true)"
GIT_COMMIT="$(git -C "$REPO_ROOT" rev-parse --short HEAD 2>/dev/null || true)"

SUITE_SUMMARY="$(grep -E 'total:|pass:|bad:' "$SUITE_LOG" || true)"
BAD_JSON_CONTENT="$(cat "$BAD_JSON_LIST")"
GALLERY_FILES="$(find "$INPUTS_DIR" -name '*.flow' -printf '%f\n' | sort)"

echo
echo "== writing Markdown report =="

cat > "$REPORT_MD" <<EOF
# Flowmini v25 AST Test Report

Generated: $(date -Is)

## Context

- Repository root: \`$REPO_ROOT\`
- Flowmini root: \`$FLOWMINI_ROOT\`
- Build dir: \`$BUILD_DIR\`
- Binary: \`$BIN\`
- Branch: \`$GIT_BRANCH\`
- Commit: \`$GIT_COMMIT\`

## Purpose

This report was generated from a blank slate by configuring/building Flowmini v25, running \`--dump-ast\` across pass examples and generated AST gallery inputs, validating JSON output, and running the Flowmini suite.

The purpose is to inspect current AST feature coverage and expose blanks/edge cases.

## Current AST Doctrine

\`\`\`text
TokenTree remembers what the source looked like.
AST states what the source means.
\`\`\`

Current \`--dump-ast\` runs after import expansion, so imported declarations may appear in the AST dump.

## Suite Result

Command:

\`\`\`bash
cmake --build "$BUILD_DIR" --target flowmini_suite -j "$JOBS"
\`\`\`

Exit status:

\`\`\`text
$SUITE_STATUS
\`\`\`

Summary:

\`\`\`text
$SUITE_SUMMARY
\`\`\`

Full suite log:

\`\`\`text
$SUITE_LOG
\`\`\`

## JSON Validation

Bad JSON list:

\`\`\`text
$BAD_JSON_CONTENT
\`\`\`

If the block above is empty, all AST JSON files parsed successfully with \`python3 -m json.tool\`.

## Generated Gallery Inputs

Generated under:

\`\`\`text
$INPUTS_DIR
\`\`\`

Files:

\`\`\`text
$GALLERY_FILES
\`\`\`

## Summary

The compact summary is included below.

EOF

cat "$SUMMARY_MD" >> "$REPORT_MD"

cat >> "$REPORT_MD" <<EOF

## Artifact Paths

- Report directory: \`$REPORT_DIR\`
- AST dumps: \`$DUMPS_DIR\`
- Errors: \`$ERRORS_DIR\`
- Summary JSON: \`$SUMMARY_JSON\`
- Summary Markdown: \`$SUMMARY_MD\`
- Suite log: \`$SUITE_LOG\`

## Known Current Limitations

- Function bodies are still skeletal.
- Statements are not yet parsed into AST.
- Expressions are not yet parsed into AST.
- Raw AST vs expanded AST is not separated yet.
- Imported declarations do not yet carry origin metadata.
- Type references are still early/simple and will need richer structure for generic/container/domain types.

## Current Strategic Status

\`\`\`text
v25 Step 1: AST data structures exist                 PASS
v25 Step 2: --dump-ast exists                         PASS
v25 Step 3: source-unit header populated              PASS
v25 Step 4: top-level fn/main declaration shells      PASS
v25 Step 5A: function signatures                      TESTED / VERIFY SUMMARY
\`\`\`
EOF

cp "$REPORT_MD" "$REPORT_MD_LATEST"

echo
echo "== done =="
echo "report:        $REPORT_MD"
echo "latest report: $REPORT_MD_LATEST"
echo "report dir:    $REPORT_DIR"
echo
echo "Open with:"
echo "  nvim \"$REPORT_MD\""S
