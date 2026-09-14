#!/bin/sh
set -eu

bin=${FLOWPARALLEL_GRAPH_PLANNER_ALLOCATION_FAULT_BIN:?FLOWPARALLEL_GRAPH_PLANNER_ALLOCATION_FAULT_BIN is required}
test -x "$bin"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

set +e
"$bin" --graph "$tmpdir/graph.json" --capabilities "$tmpdir/capabilities.json" --diagnostics json \
  >"$tmpdir/stdout" 2>"$tmpdir/stderr"
status=$?
set -e

test "$status" -eq 1
test ! -s "$tmpdir/stdout"
jq -e '.status == "failed" and .code == "FLOWPARALLEL_GRAPH_PLANNER_RESOURCE_EXHAUSTED" and .disposition == "no_artifact"' \
  "$tmpdir/stderr" >/dev/null
echo 'Flowparallel graph-planner allocation-fault containment: PASS'
