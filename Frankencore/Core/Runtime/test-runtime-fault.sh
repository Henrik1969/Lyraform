#!/bin/sh
set -eu

runtime_fault_bin=${RUNTIME_FAULT_BIN:?RUNTIME_FAULT_BIN is required}
test -x "$runtime_fault_bin"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
set +e
"$runtime_fault_bin" >"$tmpdir/stdout" 2>"$tmpdir/stderr"
status=$?
set -e
test "$status" -eq 1
test ! -s "$tmpdir/stdout"
grep -Fq 'injected runtime capability discovery failure' "$tmpdir/stderr"
echo 'Frankencore runtime discovery fault containment: PASS'
