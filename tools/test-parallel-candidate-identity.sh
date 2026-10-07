#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
fixture="$root/Lyraform/compiler/examples/ast/parallel_independence_probe.flow"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

"$FLOWMINI_BIN" --dump-frontend-bundle "$fixture" > "$tmpdir/frontend.json"
"$FLOWANALYST_BIN" --lowering-plan-version 2 < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"

jq -e '
  (.parallel_candidates | length) == 2 and
  (. as $report | all(.parallel_candidates[];
    .status == "deferred" and
    .proof == "pure-callee-disjoint-inputs" and
    (.independent_operation_ids | length) == 1 and
    (.independent_with | length) == 1 and
    (. as $candidate | any($report.lowering_plan.operations[];
      .id == $candidate.operation_id and
      .source_call_projection == true and
      .expression_id == $candidate.call_expression and
      .statement_id == $candidate.statement_id and
      .callee == $candidate.callee)) and
    (. as $candidate | any($report.lowering_plan.operations[];
      .id == $candidate.independent_operation_ids[0] and
      .expression_id == $candidate.independent_with[0]))))
' "$tmpdir/semantic.json" >/dev/null
"$FLOWPARALLEL_BIN" < "$tmpdir/semantic.json" > "$tmpdir/execution.json"
jq -e '.dependency_analysis.parallel_candidates == 2' "$tmpdir/execution.json" >/dev/null

reject() {
    mutation=$1
    jq "$mutation" "$tmpdir/semantic.json" > "$tmpdir/hostile.json"
    if "$FLOWPARALLEL_BIN" < "$tmpdir/hostile.json" >/dev/null 2>&1; then
        echo "parallel candidate mutation was accepted: $mutation" >&2
        exit 1
    fi
}

reject '.parallel_candidates[0].operation_id = 999'
reject '.parallel_candidates[0].call_expression = 999'
reject '.parallel_candidates[0].callee = "drifted"'
reject '.parallel_candidates[0].independent_operation_ids[0] = 999'
reject '.parallel_candidates[0].independent_operation_ids[0] = .parallel_candidates[0].operation_id'
reject '.parallel_candidates[0].independent_with[0] = 999'
reject 'del(.parallel_candidates[0])'
reject '.parallel_candidates += [.parallel_candidates[0]]'
reject '.parallel_candidates[0].proof = "trust-me"'
reject '.parallel_candidates[0].status = "ready"'
reject '.parallel_candidates = []'

# The validator derives independence from canonical operation evidence rather
# than trusting a mutually edited pair of candidate records.
reject '(.lowering_plan.operations[1].operands[0]) = {"expression_id":999,"kind":"identifier","type":"int","symbol_id":77} | (.lowering_plan.operations[0].operands[0]) = {"expression_id":998,"kind":"identifier","type":"int","symbol_id":77}'
reject '.lowering_plan.operations[1].result_symbol_id = .lowering_plan.operations[0].result_symbol_id'
reject '.lowering_plan.operations[1].scope_id = 999'

echo 'Canonical parallel candidate identity: PASS'
