#!/bin/sh
set -eu

execution=${FLOWPARALLEL_EXECUTION_SMOKETEST_BIN:?FLOWPARALLEL_EXECUTION_SMOKETEST_BIN is required}
planner=${FLOWPARALLEL_BIN:?FLOWPARALLEL_BIN is required}
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
flowmini=${FLOWMINI_BIN:-$root/Lyraform/compiler/cmake-build-debug/flowmini}
analyst=${FLOWANALYST_BIN:-$root/Flowanalyst/build/flowanalyst}
fixture="$root/Lyraform/compiler/examples/ast/parallel_independence_probe.flow"
test -x "$execution"
test -x "$planner"
test -x "$flowmini"
test -x "$analyst"
plan=$("$flowmini" --dump-frontend-bundle "$fixture" | "$analyst" | "$planner")
tasks=$(printf '%s\n' "$plan" | jq -r '.dependency_analysis.parallel_candidates')
test "$tasks" -ge 1
report=$(timeout --signal=TERM 30s "$execution" --workers 2 --tasks "$tasks")
printf '%s\n' "$report" | jq -e '.format == "flowparallel.cpu_execution_smoketest" and .status == "ok" and .correctness.matching_result and .correctness.failure_propagated' >/dev/null
set +e
invalid=$("$execution" --workers 2junk 2>/dev/null)
invalid_rc=$?
set -e
test "$invalid_rc" -eq 2
test -z "$invalid"
echo 'Flowparallel CPU execution: PASS'
