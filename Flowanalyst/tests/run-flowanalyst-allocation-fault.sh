#!/bin/sh
set -eu

bin=${FLOWANALYST_ALLOCATION_FAULT_BIN:?FLOWANALYST_ALLOCATION_FAULT_BIN is required}
test -x "$bin"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

set +e
printf '%s' '{"format":"flowmini.frontend_bundle","version":2,"symbol_table":{}}' \
  | "$bin" --diagnostics json >"$tmpdir/stdout" 2>"$tmpdir/stderr"
status=$?
set -e

test "$status" -eq 1
test ! -s "$tmpdir/stdout"
jq -e '.status == "failed" and .code == "FLOWANALYST_RESOURCE_EXHAUSTED" and .stage == "runtime" and .disposition == "no_artifact"' \
  "$tmpdir/stderr" >/dev/null
echo 'Flowanalyst allocation-fault containment: PASS'
