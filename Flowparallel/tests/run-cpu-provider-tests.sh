#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cpu=${FLOWPARALLEL_CPU_BIN:?FLOWPARALLEL_CPU_BIN is required}
planner=${FLOWPARALLEL_BIN:?FLOWPARALLEL_BIN is required}
flowmini=${FLOWMINI_BIN:-$root/Lyraform/compiler/cmake-build-debug/flowmini}
analyst=${FLOWANALYST_BIN:-$root/Flowanalyst/build/flowanalyst}
fixture="$root/Lyraform/compiler/examples/ast/parallel_independence_probe.flow"
test -x "$cpu"
test -x "$planner"
test -x "$flowmini"
test -x "$analyst"

plan=$("$flowmini" --dump-frontend-bundle "$fixture" | "$analyst" | "$planner")
parallel=$(printf '%s\n' "$plan" | "$cpu" --observed-speedup 2.0 --minimum-speedup 1.25 --workers 4)
printf '%s\n' "$parallel" | jq -e '.status == "ready" and .decision == "parallel" and .provider == "cpu.threadpool" and .workers >= 2 and .execution == "not-performed"' >/dev/null

serial=$(printf '%s\n' "$plan" | "$cpu" --observed-speedup 0.5 --minimum-speedup 1.25 --workers 4)
printf '%s\n' "$serial" | jq -e '.status == "ready" and .decision == "serial" and .provider == "cpu.serial" and .workers == 1' >/dev/null

reject_unsupported() {
    request=$1
    payload=$2
    set +e
    output=$(printf '%s' "$payload" | "$cpu")
    status=$?
    set -e
    test "$status" -eq 2
    printf '%s\n' "$output" | jq -e --arg request "$request" \
        '.status == "unsupported" and .request == $request and .fallback.emitted == false' >/dev/null
}

reject_unsupported parallel_effectful_v1 \
    '{"format":"flowparallel.execution_plan","version":1,"status":"ready","schedule_policy":"parallel_effectful_v1"}'
reject_unsupported cancellation \
    '{"format":"flowparallel.execution_plan","version":1,"status":"ready","cancellation":"requested"}'
reject_unsupported async \
    '{"format":"flowparallel.execution_plan","version":1,"status":"ready","async":"requested"}'
reject_unsupported backpressure \
    '{"format":"flowparallel.execution_plan","version":1,"status":"ready","backpressure":"requested"}'

set +e
malformed=$(printf '%s' '{"format":"flowparallel.execution_plan","version":1,"format":"forged","status":"ready"}' | "$cpu" 2>/dev/null)
malformed_rc=$?
set -e
test "$malformed_rc" -ne 0
test -z "$malformed"

set +e
wrong_type=$(printf '%s' '{"format":"flowparallel.execution_plan","version":1,"status":true,"dependency_analysis":{"parallel_candidates":0}}' | "$cpu" 2>/dev/null)
wrong_type_rc=$?
set -e
test "$wrong_type_rc" -ne 0
test -z "$wrong_type"

set +e
wrong_version=$(printf '%s' '{"format":"flowparallel.execution_plan","version":2,"status":"ready","dependency_analysis":{"parallel_candidates":0}}' | "$cpu" 2>/dev/null)
wrong_version_rc=$?
set -e
test "$wrong_version_rc" -ne 0
test -z "$wrong_version"

set +e
nested=$(printf '%s' '{"format":"flowparallel.execution_plan","version":1,"status":"ready","dependency_analysis":{"parallel_candidates":0},"metadata":{"cancellation":"requested"}}' | "$cpu")
nested_rc=$?
set -e
test "$nested_rc" -eq 0
printf '%s\n' "$nested" | jq -e '.status == "ready" and .execution == "not-performed"' >/dev/null

set +e
blocked=$(printf '%s' '{"format":"flowparallel.execution_plan","version":1,"status":"blocked"}' | "$cpu")
blocked_rc=$?
set -e
test "$blocked_rc" -eq 2
printf '%s\n' "$blocked" | jq -e '.status == "blocked"' >/dev/null
echo 'Flowparallel CPU provider: PASS'
