#!/bin/sh
set -eu

bin=${FLOWPARALLEL_GRAPH_REFERENCE_ALLOCATION_FAULT_BIN:?FLOWPARALLEL_GRAPH_REFERENCE_ALLOCATION_FAULT_BIN is required}
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
jq -e '.status == "failed" and .code == "FLOWPARALLEL_GRAPH_REFERENCE_RESOURCE_EXHAUSTED" and .disposition == "no_artifact"' \
  "$tmpdir/stderr" >/dev/null
echo 'Flowparallel graph-reference allocation-fault containment: PASS'
