#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
bin=${FLOWPARALLEL_BIN:?FLOWPARALLEL_BIN is required}
flowmini=${FLOWMINI_BIN:-$root/Lyraform/compiler/cmake-build-debug/flowmini}
analyst=${FLOWANALYST_BIN:-$root/Flowanalyst/build/flowanalyst}
fixture="$root/Lyraform/compiler/examples/ast/parallel_independence_probe.flow"
test -x "$bin"
test -x "$flowmini"
test -x "$analyst"

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

"$flowmini" --dump-frontend-bundle "$fixture" | "$analyst" > "$tmpdir/semantic.json"

report=$("$bin" "$tmpdir/semantic.json")
printf '%s\n' "$report" | jq -e --arg source "$fixture" '
  .format == "flowparallel.execution_plan" and
  .version == 1 and
  .status == "ready" and
  .source.path == $source and
  .cost_model.status == "deferred" and
  .cost_model.minimum_speedup == 1.25 and
  .provider_selection.status == "deferred" and
  .dependency_analysis.parallel_candidates == 2 and
  .fallback.required == true and
  .runtime.capabilities_format == "frankencore.runtime_capabilities"
' >/dev/null

set +e
blocked=$(printf '%s' '{"format":"flowanalyst.semantic_report","version":1,"status":"error"}' | "$bin")
blocked_rc=$?
set -e
test "$blocked_rc" -eq 2
printf '%s\n' "$blocked" | jq -e '.status == "blocked"' >/dev/null

# Legal formatting and object-key order cannot change authority or output.
jq -cS . "$tmpdir/semantic.json" > "$tmpdir/reordered.json"
"$bin" "$tmpdir/reordered.json" > "$tmpdir/reordered.plan.json"
printf '%s\n' "$report" > "$tmpdir/original.plan.json"
cmp "$tmpdir/original.plan.json" "$tmpdir/reordered.plan.json"

reject() {
    name=$1
    if "$bin" "$tmpdir/$name.json" >"$tmpdir/$name.out" 2>"$tmpdir/$name.err"; then
        echo "$name hostile semantic report unexpectedly accepted" >&2
        exit 1
    fi
    test -s "$tmpdir/$name.err"
}

printf '%s' '{"format":"wrong","version":1,"status":"ok","nested":{"format":"flowanalyst.semantic_report"}}' > "$tmpdir/nested-fake.json"
reject nested-fake
printf '%s' '{"format":"wrong","format":"flowanalyst.semantic_report","version":1,"status":"ok"}' > "$tmpdir/duplicate-key.json"
reject duplicate-key
printf '%s' '{"format":"wrong","version":1,"status":"ok","note":"\\\"format\\\":\\\"flowanalyst.semantic_report\\\""}' > "$tmpdir/escaped-fake.json"
reject escaped-fake
sed '$s/.$//' "$tmpdir/semantic.json" > "$tmpdir/truncated.json"
reject truncated
jq 'del(.lowering_plan)' "$tmpdir/semantic.json" > "$tmpdir/missing-plan.json"
reject missing-plan
jq '.lowering_plan.operations = [(.lowering_plan.operations[0]),(.lowering_plan.operations[0])]' "$tmpdir/semantic.json" > "$tmpdir/duplicate-operation.json"
reject duplicate-operation
jq '(.analysis_graph.matrix_views[] | select(.name == "region_dependency") | .entries[0].row) = .analysis_graph.matrix_views[0].rows' "$tmpdir/semantic.json" > "$tmpdir/out-of-range-matrix.json"
reject out-of-range-matrix
set +e
printf '%s' '{"format":"wrong"}' | "$bin" --diagnostics json >"$tmpdir/diagnostic.out" 2>"$tmpdir/diagnostic.err"
diagnostic_rc=$?
set -e
test "$diagnostic_rc" -ne 0
test ! -s "$tmpdir/diagnostic.out"
jq -e '.status == "failed" and .code == "FLOWPARALLEL_CONTRACT_FAILURE" and .stage == "contract" and .disposition == "no_artifact" and (.message | length > 0)' "$tmpdir/diagnostic.err" >/dev/null

set +e
"$bin" --diagnostics json extra >"$tmpdir/diagnostic.out" 2>"$tmpdir/diagnostic.err"
diagnostic_rc=$?
set -e
test "$diagnostic_rc" -eq 1
test ! -s "$tmpdir/diagnostic.out"
jq -e '.code == "FLOWPARALLEL_INPUT_INVALID" and .stage == "input" and .disposition == "no_artifact"' "$tmpdir/diagnostic.err" >/dev/null

reject_unsupported() {
    request=$1
    payload=$2
    set +e
    output=$(printf '%s' "$payload" | "$bin")
    status=$?
    set -e
    test "$status" -eq 2
    printf '%s\n' "$output" | jq -e --arg request "$request" \
        '.status == "unsupported" and .request == $request and .fallback.emitted == false' >/dev/null
}

reject_unsupported parallel_effectful_v1 \
    '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","schedule_policy":"parallel_effectful_v1"}'
reject_unsupported parallel_independent_v1 \
    '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","schedule_policy":"parallel_independent_v1"}'
reject_unsupported parallel_reentrant_v1 \
    '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","schedule_policy":"parallel_reentrant_v1"}'
reject_unsupported cancellation \
    '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","cancellation":"required"}'
reject_unsupported async \
    '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","async":"requested"}'
reject_unsupported backpressure \
    '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","backpressure":"requested"}'
reject_unsupported reentrancy \
    '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","reentrancy":"requested"}'
reject_unsupported nested \
    '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","nested":"requested"}'
reject_unsupported distributed \
    '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","distributed":"requested"}'
reject_unsupported retry \
    '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","retry":"automatic"}'
reject_unsupported irreversible \
    '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","irreversible":"requested"}'

set +e
printf '%s' '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","async":true}' | \
    "$bin" --diagnostics json >"$tmpdir/control-type.out" 2>"$tmpdir/control-type.err"
control_type_rc=$?
set -e
test "$control_type_rc" -eq 1
test ! -s "$tmpdir/control-type.out"
jq -e '.code == "FLOWPARALLEL_CONTRACT_FAILURE" and .stage == "contract" and .disposition == "no_artifact"' \
    "$tmpdir/control-type.err" >/dev/null

echo 'Flowparallel unsupported-scheduling refusal: PASS'
echo 'Flowparallel tests: PASS'
