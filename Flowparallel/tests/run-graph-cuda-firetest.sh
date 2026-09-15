#!/bin/sh
set -eu

cuda=${FLOWPARALLEL_GRAPH_CUDA_BIN:?FLOWPARALLEL_GRAPH_CUDA_BIN is required}
reference=${FLOWPARALLEL_GRAPH_REFERENCE_BIN:?FLOWPARALLEL_GRAPH_REFERENCE_BIN is required}
test -x "$cuda"; test -x "$reference"
fixture=$(mktemp)
trap 'rm -f "$fixture"' EXIT

run_case() {
    name=$1
    json=$2
    printf '%s\n' "$json" > "$fixture"
    cpu=$($reference "$fixture")
    gpu=$($cuda "$fixture")
    printf '%s\n' "$gpu"
    jq -n --arg name "$name" --argjson cpu "$cpu" --argjson gpu "$gpu" \
        -e '$gpu.status == "verified" and $gpu.provider == "cuda.cublas.boolean_threshold" and $gpu.reachable_pairs == $gpu.cpu_reachable_pairs and $gpu.reachable_pairs == $cpu.reachable_pairs and ($gpu.end_to_end_speedup | numbers) > 0' >/dev/null
}

run_case empty '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","matrix_views":[{"name":"region_dependency","rows":4,"columns":4,"entries":[]}]}'
run_case chain '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","matrix_views":[{"name":"region_dependency","rows":4,"columns":4,"entries":[{"row":0,"column":1},{"row":1,"column":2},{"row":2,"column":3}]}]}'
run_case cycle '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","matrix_views":[{"name":"region_dependency","rows":4,"columns":4,"entries":[{"row":0,"column":1},{"row":1,"column":0}]}]}'
run_case dense '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","matrix_views":[{"name":"region_dependency","rows":4,"columns":4,"entries":[{"row":0,"column":1},{"row":0,"column":2},{"row":0,"column":3},{"row":1,"column":0},{"row":1,"column":2},{"row":1,"column":3},{"row":2,"column":0},{"row":2,"column":1},{"row":2,"column":3},{"row":3,"column":0},{"row":3,"column":1},{"row":3,"column":2}]}]}'
diagnostic_err=$(mktemp)
trap 'rm -f "$fixture" "$diagnostic_err"' EXIT
set +e
printf '%s\n' '{"format":"wrong"}' | "$cuda" --diagnostics json > /dev/null 2>"$diagnostic_err"
diagnostic_rc=$?
set -e
test "$diagnostic_rc" -ne 0
jq -e '.status == "failed" and .code == "FLOWPARALLEL_GRAPH_CUDA_CONTRACT_FAILURE" and .stage == "contract" and .disposition == "no_artifact" and (.message | length > 0)' "$diagnostic_err" >/dev/null
echo 'Flowparallel CUDA graph firetest: 4/4 shapes passed'
