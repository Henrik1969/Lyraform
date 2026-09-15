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
dd if=/dev/zero of="$tmpdir/oversized.json" bs=1048576 count=17 2>/dev/null
printf '%s\n' '{"format":"frankencore.runtime_capabilities","version":1,"cpu":{"logical_processors":1},"memory":{"total_bytes":1,"available_bytes":1},"cuda":{"status":"unavailable","driver":"","device_count":0,"diagnostic":"test"}}' >"$tmpdir/capabilities.json"

check_stdin() {
    name=$1
    expected_code=$2
    expected_stage=$3
    shift 3
    set +e
    output=$(cat "$tmpdir/malformed.json" | "$@" 2>"$tmpdir/$name.err")
    status=$?
    set -e
    test "$status" -eq 1
    test -z "$output"
    jq -e --arg code "$expected_code" --arg stage "$expected_stage" '.status == "failed" and .code == $code and ($stage == "" or .stage == $stage) and .disposition == "no_artifact" and (.message | length > 0)' "$tmpdir/$name.err" >/dev/null
}

check_file() {
    name=$1
    expected_code=$2
    expected_stage=$3
    shift 3
    set +e
    output=$("$@" 2>"$tmpdir/$name.err")
    status=$?
    set -e
    test "$status" -eq 1
    test -z "$output"
    jq -e --arg code "$expected_code" --arg stage "$expected_stage" '.status == "failed" and .code == $code and ($stage == "" or .stage == $stage) and .disposition == "no_artifact" and (.message | length > 0)' "$tmpdir/$name.err" >/dev/null
}

check_oversized_stdin() {
    name=$1
    shift
    set +e
    output=$(dd if=/dev/zero bs=1048576 count=17 2>/dev/null | "$@" --diagnostics json 2>"$tmpdir/$name.err")
    status=$?
    set -e
    test "$status" -eq 1
    test -z "$output"
    jq -e '.status == "failed" and .disposition == "no_artifact" and (.message | contains("16 MiB input limit"))' "$tmpdir/$name.err" >/dev/null
}

check_stdin plan FLOWPARALLEL_CONTRACT_FAILURE '' "$flowparallel" --diagnostics json
check_stdin cpu FLOWPARALLEL_CPU_CONTRACT_FAILURE contract "$cpu" --diagnostics json
check_stdin cuda FLOWPARALLEL_CUDA_CONTRACT_FAILURE contract "$cuda" --diagnostics json
check_stdin graph_reference FLOWPARALLEL_GRAPH_REFERENCE_FAILURE '' "$graph_reference" --diagnostics json
check_stdin graph_cuda FLOWPARALLEL_GRAPH_CUDA_FAILURE '' "$graph_cuda" --diagnostics json
check_oversized_stdin plan_oversized "$flowparallel"
check_oversized_stdin cpu_oversized "$cpu"
check_oversized_stdin cuda_oversized "$cuda"
check_oversized_stdin graph_reference_oversized "$graph_reference"
check_oversized_stdin graph_cuda_oversized "$graph_cuda"
check_file runtime_planner FLOWPARALLEL_RUNTIME_PLANNER_CONTRACT_FAILURE contract "$runtime_planner" --plan "$tmpdir/malformed.json" --capabilities "$tmpdir/capabilities.json" --diagnostics json
check_file graph_planner FLOWPARALLEL_GRAPH_PLANNER_CONTRACT_FAILURE contract "$graph_planner" --graph "$tmpdir/malformed.json" --capabilities "$tmpdir/capabilities.json" --diagnostics json
check_file runtime_planner_oversized FLOWPARALLEL_RUNTIME_PLANNER_INPUT_INVALID input "$runtime_planner" --plan "$tmpdir/oversized.json" --capabilities "$tmpdir/capabilities.json" --diagnostics json
check_file graph_planner_oversized FLOWPARALLEL_GRAPH_PLANNER_INPUT_INVALID input "$graph_planner" --graph "$tmpdir/oversized.json" --capabilities "$tmpdir/capabilities.json" --diagnostics json
check_file cuda_execute FLOWPARALLEL_CUDA_EXECUTE_FAILURE '' "$cuda_execute" --size 1 --diagnostics json
check_file benchmark FLOWPARALLEL_MATRIX_BENCHMARK_FAILURE '' "$benchmark" --size 1 --diagnostics json

echo 'Flowparallel exception containment: 16/16 boundaries PASS'
