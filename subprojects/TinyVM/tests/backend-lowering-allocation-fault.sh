#!/bin/sh
set -eu

lower=${1:?flowtinylower allocation-fault binary is required}
test -x "$lower"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

set +e
"$lower" "$tmpdir/input.json" "$tmpdir/output.tvm" >"$tmpdir/stdout" 2>"$tmpdir/stderr"
status=$?
set -e

test "$status" -eq 1
test ! -s "$tmpdir/stdout"
test ! -e "$tmpdir/output.tvm"
grep -q 'allocation failed' "$tmpdir/stderr"
echo 'TinyVM lowerer allocation-fault containment: PASS'
