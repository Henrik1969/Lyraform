#!/bin/sh
set -eu

bin=${FLOWLOWER_ALLOCATION_FAULT_BIN:?FLOWLOWER_ALLOCATION_FAULT_BIN is required}
test -x "$bin"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

set +e
printf '%s' '{"format":"flowoptimize.optimization_report","version":1,"status":"ready"}' \
  | "$bin" --diagnostics json >"$tmpdir/stdout" 2>"$tmpdir/stderr"
status=$?
set -e

test "$status" -eq 1
test ! -s "$tmpdir/stdout"
jq -e '.status == "failed" and .code == "FLOWLOWER_RESOURCE_EXHAUSTED" and .stage == "runtime" and .disposition == "no_artifact"' \
  "$tmpdir/stderr" >/dev/null
echo 'Flowlower allocation-fault containment: PASS'
