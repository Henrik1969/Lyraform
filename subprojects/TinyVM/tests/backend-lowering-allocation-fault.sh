#!/bin/sh
set -eu

lower=${1:?flowtinylower allocation-fault binary is required}
test -x "$lower"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

set +e
"$lower" --diagnostics json "$tmpdir/input.json" "$tmpdir/output.tvm" >"$tmpdir/stdout" 2>"$tmpdir/stderr"
status=$?
set -e

test "$status" -eq 1
test ! -s "$tmpdir/stdout"
test ! -e "$tmpdir/output.tvm"
jq -e '.status == "failed" and .code == "FLOWTINYLOWER_RESOURCE_EXHAUSTED" and .stage == "runtime" and .message == "allocation failed" and .disposition == "no_artifact"' \
  "$tmpdir/stderr" >/dev/null
echo 'TinyVM lowerer allocation-fault containment: PASS'
