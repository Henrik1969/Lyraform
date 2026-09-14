#!/bin/sh
set -eu

bin=${FLOWMINI_ALLOCATION_FAULT_BIN:?FLOWMINI_ALLOCATION_FAULT_BIN is required}
test -x "$bin"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
printf 'fn main() {\n}\n' >"$tmpdir/input.flow"

set +e
"$bin" --diagnostics json "$tmpdir/input.flow" >"$tmpdir/stdout" 2>"$tmpdir/stderr"
status=$?
set -e

test "$status" -eq 1
test ! -s "$tmpdir/stdout"
jq -e '.status == "failed" and .code == "FLOW_RESOURCE_EXHAUSTED" and .stage == "runtime" and .disposition == "no_artifact"' \
  "$tmpdir/stderr" >/dev/null
echo 'Flowmini allocation-fault containment: PASS'
