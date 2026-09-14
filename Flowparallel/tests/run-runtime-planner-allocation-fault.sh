#!/bin/sh
set -eu

bin=${FLOWPARALLEL_RUNTIME_PLANNER_ALLOCATION_FAULT_BIN:?FLOWPARALLEL_RUNTIME_PLANNER_ALLOCATION_FAULT_BIN is required}
test -x "$bin"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

set +e
"$bin" --plan "$tmpdir/plan.json" --capabilities "$tmpdir/capabilities.json" --diagnostics json \
  >"$tmpdir/stdout" 2>"$tmpdir/stderr"
status=$?
set -e

test "$status" -eq 1
test ! -s "$tmpdir/stdout"
jq -e '.status == "failed" and .code == "FLOWPARALLEL_RUNTIME_PLANNER_RESOURCE_EXHAUSTED" and .stage == "runtime" and .disposition == "no_artifact"' \
  "$tmpdir/stderr" >/dev/null
echo 'Flowparallel runtime-planner allocation-fault containment: PASS'
