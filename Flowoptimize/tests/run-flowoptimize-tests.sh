#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
optimizer=${FLOWOPTIMIZE_BIN:?FLOWOPTIMIZE_BIN is required}
analyst=${FLOWANALYST_BIN:-$root/Flowanalyst/build/flowanalyst}
parallel=${FLOWPARALLEL_BIN:-$root/Flowparallel/build/flowparallel}
flowmini=${FLOWMINI_BIN:-$root/Lyraform/compiler/cmake-build-debug/flowmini}
fixture="$root/Lyraform/compiler/examples/ast/call_expression_probe.flow"
test -x "$flowmini"
test -x "$parallel"
test -x "$optimizer"

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
"$flowmini" --dump-frontend-bundle "$fixture" | "$analyst" | "$parallel" > "$tmpdir/plan.json"
report=$("$optimizer" "$tmpdir/plan.json")
printf '%s\n' "$report" | jq -e '.format == "flowoptimize.optimization_report" and .status == "ready"' >/dev/null
printf '%s\n' "$report" | jq -e '.transforms[0].kind == "coo_deduplicate" and .transforms[0].status == "not-needed" and .state.transformation == "identity"' >/dev/null
printf '%s\n' "$report" | jq -e --arg source "$fixture" '
  .source.path == $source and
  .projections[0].kind == "graph_to_matrix" and
  .projections[0].status == "available" and
  .provider_policy.selection == "runtime" and
  (.provider_policy.candidates | index("cuda")) != null and
  .provider_policy.cuda.fallback == "cpu"
' >/dev/null

decision="$tmpdir/decision.json"
printf '%s\n' '{"format":"flowparallel.graph_provider_decision","version":1,"status":"verified","provider":"cpu.reference","representation":"sparse","reason":"test fallback","fallback":"cpu.reference"}' > "$decision"
decision_report=$($flowmini --dump-frontend-bundle "$fixture" | "$analyst" | "$parallel" | "$optimizer" --provider-decision "$decision")
printf '%s\n' "$decision_report" | jq -e '.status == "ready" and .provider_policy.decision.status == "verified" and .provider_policy.decision.provider == "cpu.reference" and .state.canonical_graph == "unchanged" and .state.transformation == "identity"' >/dev/null

jq '.graph_projection.rows = 2 | .graph_projection.columns = 2 | .graph_projection.entries = [{"row":0,"column":1,"value":true},{"row":0,"column":1,"value":true},{"row":1,"column":0,"value":true}]' "$tmpdir/plan.json" > "$tmpdir/duplicate-plan.json"
duplicate_report=$("$optimizer" "$tmpdir/duplicate-plan.json")
printf '%s\n' "$duplicate_report" | jq -e '.transforms[0].status == "applied" and .transforms[0].input_entries == 3 and .transforms[0].output_entries == 2 and .transforms[0].semantics_preserved == true' >/dev/null

printf '%s\n' '{"format":"flowparallel.graph_provider_decision","version":1,"status":"verified","provider":"unknown.provider","representation":"dense"}' > "$decision"
if $flowmini --dump-frontend-bundle "$fixture" | "$analyst" | "$parallel" | "$optimizer" --provider-decision "$decision" >/dev/null 2>&1; then
    echo 'unsupported provider decision unexpectedly accepted' >&2
    exit 1
fi

if printf '%s' '{"format":"flowparallel.execution_plan","version":1,"status":"blocked"}' | "$optimizer" >/dev/null 2>&1; then
    echo 'rejected semantic report unexpectedly accepted' >&2
    exit 1
fi

# Typed authority attacks fail before optimization.
printf '%s' '{"format":"wrong","version":1,"status":"ready","nested":{"format":"flowparallel.execution_plan"}}' > "$tmpdir/nested-fake.json"
if "$optimizer" "$tmpdir/nested-fake.json" >/dev/null 2>&1; then echo 'nested fake authority unexpectedly accepted' >&2; exit 1; fi
printf '%s' '{"format":"flowparallel.execution_plan","format":"wrong","version":1,"status":"ready"}' > "$tmpdir/duplicate-key.json"
if "$optimizer" "$tmpdir/duplicate-key.json" >/dev/null 2>&1; then echo 'duplicate authority unexpectedly accepted' >&2; exit 1; fi
jq '(.graph_projection.entries[0].row) = .graph_projection.rows' "$tmpdir/plan.json" > "$tmpdir/out-of-range.json"
if "$optimizer" "$tmpdir/out-of-range.json" >/dev/null 2>&1; then echo 'out-of-range matrix unexpectedly accepted' >&2; exit 1; fi
jq '.cancellation = "required"' "$tmpdir/plan.json" > "$tmpdir/unsupported-schedule.json"
if "$optimizer" "$tmpdir/unsupported-schedule.json" >/dev/null 2>&1; then
    echo 'unsupported scheduling request was stripped by optimization' >&2
    exit 1
fi

set +e
printf '%s' '{"format":"wrong","version":1}' | "$optimizer" --diagnostics json >"$tmpdir/hostile-stdout" 2>"$tmpdir/hostile-stderr"
hostile_rc=$?
set -e
test "$hostile_rc" -eq 1
test ! -s "$tmpdir/hostile-stdout"
jq -e '.status == "failed" and .code == "FLOWOPTIMIZE_CONTRACT_FAILURE" and .stage == "contract" and (.message | length > 0) and .disposition == "no_artifact"' "$tmpdir/hostile-stderr" >/dev/null

printf '%s' '{"format":"flowparallel.execution_plan","version":1}' | "$optimizer" --diagnostics json --provider-decision "$tmpdir/missing-decision.json" >"$tmpdir/missing-stdout" 2>"$tmpdir/missing-stderr" || test "$?" -eq 1
test ! -s "$tmpdir/missing-stdout"
jq -e '.status == "failed" and .code == "FLOWOPTIMIZE_INPUT_INVALID" and .stage == "input" and .disposition == "no_artifact"' "$tmpdir/missing-stderr" >/dev/null
echo 'Flowoptimize tests: PASS'
