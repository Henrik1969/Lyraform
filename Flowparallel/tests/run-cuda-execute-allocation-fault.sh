#!/bin/sh
set -eu

bin=${FLOWPARALLEL_CUDA_EXECUTE_ALLOCATION_FAULT_BIN:?FLOWPARALLEL_CUDA_EXECUTE_ALLOCATION_FAULT_BIN is required}
test -x "$bin"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

set +e
"$bin" --size 2 --diagnostics json >"$tmpdir/stdout" 2>"$tmpdir/stderr"
status=$?
set -e

test "$status" -eq 1
test ! -s "$tmpdir/stdout"
jq -e '.status == "failed" and .code == "FLOWPARALLEL_CUDA_EXECUTE_RESOURCE_EXHAUSTED" and .disposition == "no_artifact"' \
  "$tmpdir/stderr" >/dev/null
echo 'Flowparallel CUDA execution allocation-fault containment: PASS'
