#!/bin/sh
set -eu

bin=${FLOWTARGET_ALLOCATION_FAULT_BIN:?FLOWTARGET_ALLOCATION_FAULT_BIN is required}
test -x "$bin"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

set +e
"$bin" --diagnostics json --policy-root "$tmpdir" llvm-host >"$tmpdir/stdout" 2>"$tmpdir/stderr"
status=$?
set -e

test "$status" -eq 1
test ! -s "$tmpdir/stdout"
jq -e '.status == "failed" and .code == "FLOWTARGET_RESOURCE_EXHAUSTED" and .stage == "runtime" and .disposition == "no_artifact"' \
  "$tmpdir/stderr" >/dev/null
echo 'Flowtarget allocation-fault containment: PASS'
