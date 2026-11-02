#!/bin/sh
set -eu

runtime_close_fault_bin=${RUNTIME_CLOSE_FAULT_BIN:?RUNTIME_CLOSE_FAULT_BIN is required}
test -x "$runtime_close_fault_bin"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
"$runtime_close_fault_bin" >"$tmpdir/stdout" 2>"$tmpdir/stderr"
test -s "$tmpdir/stdout"
test ! -s "$tmpdir/stderr"
grep -Fq '"status": "unknown"' "$tmpdir/stdout"
grep -Fq 'CUDA driver library cleanup failed' "$tmpdir/stdout"
echo 'Frankencore runtime CUDA cleanup fault containment: PASS'
