#!/bin/sh
set -eu

bin=${FLOWPARALLEL_MATRIX_BENCHMARK_ALLOCATION_FAULT_BIN:?FLOWPARALLEL_MATRIX_BENCHMARK_ALLOCATION_FAULT_BIN is required}
test -x "$bin"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

set +e
"$bin" --size 32 --iterations 2 --diagnostics json >"$tmpdir/stdout" 2>"$tmpdir/stderr"
status=$?
set -e

test "$status" -eq 1
test ! -s "$tmpdir/stdout"
jq -e '.status == "failed" and .code == "FLOWPARALLEL_MATRIX_BENCHMARK_RESOURCE_EXHAUSTED" and .stage == "runtime" and .disposition == "no_artifact"' \
  "$tmpdir/stderr" >/dev/null
echo 'Flowparallel matrix-benchmark allocation-fault containment: PASS'
