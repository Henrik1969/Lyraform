#!/usr/bin/env bash
set -euo pipefail

flowmini=${FLOWMINI_BIN:?FLOWMINI_BIN is required}
analyst=${FLOWANALYST_BIN:?FLOWANALYST_BIN is required}
parallel=${FLOWPARALLEL_BIN:?FLOWPARALLEL_BIN is required}
optimizer=${FLOWOPTIMIZE_BIN:?FLOWOPTIMIZE_BIN is required}
lower=${FLOWLOWER_BIN:?FLOWLOWER_BIN is required}
source=${1:?source path is required}
shift

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

"$flowmini" --dump-frontend-bundle "$source" > "$tmpdir/frontend.json"
set +e
"$analyst" --lowering-plan-version 2 < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
analysis_rc=$?
set -e
if [[ "$analysis_rc" -ne 0 ]]; then
    jq -r '.diagnostics[]? | "\(.code): \(.message) [\(.provenance.source):\(.provenance.line):\(.provenance.column)]"' \
        "$tmpdir/semantic.json" >&2 || true
    exit "$analysis_rc"
fi

if ! jq -e '
    .status == "ok" and
    .lowering_plan.status == "ready" and
    .lowering_plan.version == 2 and
    .lowering_plan.source_operation_coverage.format == "lyraform.source_operation_coverage" and
    .lowering_plan.source_operation_coverage.version == 1 and
    .lowering_plan.source_operation_coverage.status == "complete" and
    .lowering_plan.source_operation_coverage.refused_count == 0
' "$tmpdir/semantic.json" >/dev/null; then
    printf '%s\n' 'Igor: canonical execution refused: source-to-operation coverage is not complete' >&2
    exit 2
fi

"$parallel" < "$tmpdir/semantic.json" > "$tmpdir/parallel.json"
"$optimizer" < "$tmpdir/parallel.json" > "$tmpdir/optimized.json"
"$lower" --emit-llvm "$tmpdir/program.ll" < "$tmpdir/optimized.json" > "$tmpdir/lowering.json"
clang "$tmpdir/program.ll" -o "$tmpdir/program"

set +e
"$tmpdir/program" "$@"
program_rc=$?
set -e
exit "$program_rc"
