#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
flowmini=${FLOWMINI_BIN:?FLOWMINI_BIN is required}
analyst=${FLOWANALYST_BIN:?FLOWANALYST_BIN is required}
runner=${CANONICAL_RUNNER:?CANONICAL_RUNNER is required}
parallel=${FLOWPARALLEL_BIN:?FLOWPARALLEL_BIN is required}
optimizer=${FLOWOPTIMIZE_BIN:?FLOWOPTIMIZE_BIN is required}
lower=${FLOWLOWER_BIN:?FLOWLOWER_BIN is required}
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

pass_source="$root/Lyraform/compiler/examples/pass/profile_free_return.flow"
"$flowmini" --dump-frontend-bundle "$pass_source" > "$tmpdir/pass.frontend.json"
"$analyst" --lowering-plan-version 2 < "$tmpdir/pass.frontend.json" > "$tmpdir/pass.semantic.json"
jq -e '
  .status == "ok" and
  .lowering_plan.source_operation_coverage.status == "complete" and
  .lowering_plan.source_operation_coverage.refused_count == 0 and
  (.lowering_plan.source_operation_coverage.statements | length) == 1 and
  .lowering_plan.source_operation_coverage.statements[0].disposition == "lowered"
' "$tmpdir/pass.semantic.json" >/dev/null

jq '.lowering_plan.source_operation_coverage.statements[0].operation_ids[0] = 999' \
    "$tmpdir/pass.semantic.json" > "$tmpdir/hostile.semantic.json"
if "$parallel" < "$tmpdir/hostile.semantic.json" >/dev/null 2>&1; then
    echo 'Flowparallel accepted contradictory source-operation coverage' >&2
    exit 1
fi
"$parallel" < "$tmpdir/pass.semantic.json" > "$tmpdir/pass.parallel.json"
"$optimizer" < "$tmpdir/pass.parallel.json" > "$tmpdir/pass.optimized.json"
jq '.lowering_plan.source_operation_coverage.statements[0].operation_ids[0] = 999' \
    "$tmpdir/pass.optimized.json" > "$tmpdir/hostile.optimized.json"
if "$lower" --emit-llvm "$tmpdir/hostile.ll" < "$tmpdir/hostile.optimized.json" >/dev/null 2>&1; then
    echo 'Flowlower accepted contradictory source-operation coverage' >&2
    exit 1
fi

set +e
FLOWMINI_BIN="$flowmini" FLOWANALYST_BIN="$analyst" FLOWPARALLEL_BIN="$parallel" \
FLOWOPTIMIZE_BIN="$optimizer" FLOWLOWER_BIN="$lower" \
    "$runner" "$pass_source"
run_rc=$?
set -e
test "$run_rc" -eq 42

fail_source="$root/Lyraform/compiler/examples/fail/canonical_operation_gap_print_int.flow"
"$flowmini" --dump-frontend-bundle "$fail_source" > "$tmpdir/fail.frontend.json"
set +e
"$analyst" --lowering-plan-version 2 < "$tmpdir/fail.frontend.json" > "$tmpdir/fail.semantic.json"
analysis_rc=$?
set -e
test "$analysis_rc" -eq 2
jq -e '
  .status == "error" and
  .lowering_plan.status == "blocked" and
  .lowering_plan.source_operation_coverage.status == "refused" and
  .lowering_plan.source_operation_coverage.refused_count == 1 and
  any(.diagnostics[]; .code == "FLOWANALYST_SOURCE_OPERATION_GAP" and .provenance.ast_path == "/statement_pool/1")
' "$tmpdir/fail.semantic.json" >/dev/null

set +e
FLOWMINI_BIN="$flowmini" FLOWANALYST_BIN="$analyst" FLOWPARALLEL_BIN="$parallel" \
FLOWOPTIMIZE_BIN="$optimizer" FLOWLOWER_BIN="$lower" \
    "$runner" "$fail_source" > "$tmpdir/fail.stdout" 2> "$tmpdir/fail.stderr"
runner_rc=$?
set -e
test "$runner_rc" -eq 2
grep -Fq 'FLOWANALYST_SOURCE_OPERATION_GAP' "$tmpdir/fail.stderr"

printf '%s\n' 'Canonical source-operation coverage: PASS'
