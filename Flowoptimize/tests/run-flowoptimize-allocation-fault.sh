#!/bin/sh
set -eu

bin=${FLOWOPTIMIZE_ALLOCATION_FAULT_BIN:?FLOWOPTIMIZE_ALLOCATION_FAULT_BIN is required}
test -x "$bin"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

set +e
printf '%s' '{"format":"flowanalyst.semantic_report","version":1,"status":"ok"}' \
  | "$bin" --diagnostics json >"$tmpdir/stdout" 2>"$tmpdir/stderr"
status=$?
set -e

test "$status" -eq 1
test ! -s "$tmpdir/stdout"
jq -e '.status == "failed" and .code == "FLOWOPTIMIZE_RESOURCE_EXHAUSTED" and .stage == "runtime" and .disposition == "no_artifact"' \
  "$tmpdir/stderr" >/dev/null
echo 'Flowoptimize allocation-fault containment: PASS'
