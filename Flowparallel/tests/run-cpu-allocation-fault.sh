#!/bin/sh
set -eu

bin=${FLOWPARALLEL_CPU_ALLOCATION_FAULT_BIN:?FLOWPARALLEL_CPU_ALLOCATION_FAULT_BIN is required}
test -x "$bin"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

set +e
printf '%s' '{"format":"flowparallel.execution_plan","version":1,"status":"ready"}' \
  | "$bin" --diagnostics json >"$tmpdir/stdout" 2>"$tmpdir/stderr"
status=$?
set -e

test "$status" -eq 1
test ! -s "$tmpdir/stdout"
jq -e '.status == "failed" and .code == "FLOWPARALLEL_CPU_RESOURCE_EXHAUSTED" and .stage == "runtime" and .disposition == "no_artifact"' \
  "$tmpdir/stderr" >/dev/null
echo 'Flowparallel CPU allocation-fault containment: PASS'
