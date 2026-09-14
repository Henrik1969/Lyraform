#!/bin/sh
set -eu

benchmark=${FLOWPARALLEL_MATRIX_BENCHMARK_BIN:?FLOWPARALLEL_MATRIX_BENCHMARK_BIN is required}
test -x "$benchmark"
diagnostic_err=$(mktemp)
trap 'rm -f "$diagnostic_err"' EXIT

set +e
"$benchmark" --size 1 --diagnostics json >/dev/null 2>"$diagnostic_err"
status=$?
set -e
test "$status" -eq 1
jq -e '.status == "failed" and .code == "FLOWPARALLEL_MATRIX_BENCHMARK_FAILURE" and .disposition == "no_artifact" and (.message | length > 0)' "$diagnostic_err" >/dev/null
set +e
"$benchmark" --size 64junk --diagnostics json >/dev/null 2>"$diagnostic_err"
status=$?
set -e
test "$status" -eq 1
jq -e '.code == "FLOWPARALLEL_MATRIX_BENCHMARK_FAILURE" and (.message | contains("complete integer"))' "$diagnostic_err" >/dev/null
echo 'Flowparallel matrix benchmark diagnostic boundary: PASS'
