#!/bin/sh
set -eu

flowparallel=${FLOWPARALLEL_BIN:?FLOWPARALLEL_BIN is required}
cpu=${FLOWPARALLEL_CPU_BIN:?FLOWPARALLEL_CPU_BIN is required}
cuda=${FLOWPARALLEL_CUDA_BIN:?FLOWPARALLEL_CUDA_BIN is required}
runtime_planner=${FLOWPARALLEL_RUNTIME_PLANNER_BIN:?FLOWPARALLEL_RUNTIME_PLANNER_BIN is required}
graph_reference=${FLOWPARALLEL_GRAPH_REFERENCE_BIN:?FLOWPARALLEL_GRAPH_REFERENCE_BIN is required}
graph_planner=${FLOWPARALLEL_GRAPH_PLANNER_BIN:?FLOWPARALLEL_GRAPH_PLANNER_BIN is required}
graph_cuda=${FLOWPARALLEL_GRAPH_CUDA_BIN:?FLOWPARALLEL_GRAPH_CUDA_BIN is required}
cuda_execute=${FLOWPARALLEL_CUDA_EXECUTE_BIN:?FLOWPARALLEL_CUDA_EXECUTE_BIN is required}
benchmark=${FLOWPARALLEL_MATRIX_BENCHMARK_BIN:?FLOWPARALLEL_MATRIX_BENCHMARK_BIN is required}

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
printf '%s\n' '{"format":"wrong"}' >"$tmpdir/malformed.json"
printf '%s\n' '{"format":"frankencore.runtime_capabilities","version":1,"cpu":{"logical_processors":1},"memory":{"total_bytes":1,"available_bytes":1},"cuda":{"status":"unavailable","driver":"","device_count":0,"diagnostic":"test"}}' >"$tmpdir/capabilities.json"

check_stdin() {
    name=$1
    expected_code=$2
    shift 2
    set +e
    output=$(cat "$tmpdir/malformed.json" | "$@" 2>"$tmpdir/$name.err")
    status=$?
    set -e
    test "$status" -eq 1
    test -z "$output"
    jq -e --arg code "$expected_code" '.status == "failed" and .code == $code and .disposition == "no_artifact" and (.message | length > 0)' "$tmpdir/$name.err" >/dev/null
}

check_file() {
    name=$1
    expected_code=$2
    shift 2
    set +e
    output=$("$@" 2>"$tmpdir/$name.err")
    status=$?
    set -e
    test "$status" -eq 1
    test -z "$output"
    jq -e --arg code "$expected_code" '.status == "failed" and .code == $code and .disposition == "no_artifact" and (.message | length > 0)' "$tmpdir/$name.err" >/dev/null
}

check_stdin plan FLOWPARALLEL_CONTRACT_FAILURE "$flowparallel" --diagnostics json
check_stdin cpu FLOWPARALLEL_CPU_FAILURE "$cpu" --diagnostics json
check_stdin cuda FLOWPARALLEL_CUDA_FAILURE "$cuda" --diagnostics json
check_stdin graph_reference FLOWPARALLEL_GRAPH_REFERENCE_FAILURE "$graph_reference" --diagnostics json
check_stdin graph_cuda FLOWPARALLEL_GRAPH_CUDA_FAILURE "$graph_cuda" --diagnostics json
check_file runtime_planner FLOWPARALLEL_RUNTIME_PLANNER_FAILURE "$runtime_planner" --plan "$tmpdir/malformed.json" --capabilities "$tmpdir/capabilities.json" --diagnostics json
check_file graph_planner FLOWPARALLEL_GRAPH_PLANNER_FAILURE "$graph_planner" --graph "$tmpdir/malformed.json" --capabilities "$tmpdir/capabilities.json" --diagnostics json
check_file cuda_execute FLOWPARALLEL_CUDA_EXECUTE_FAILURE "$cuda_execute" --size 1 --diagnostics json
check_file benchmark FLOWPARALLEL_MATRIX_BENCHMARK_FAILURE "$benchmark" --size 1 --diagnostics json

echo 'Flowparallel exception containment: 9/9 boundaries PASS'
