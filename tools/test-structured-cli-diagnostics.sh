#!/bin/sh
set -eu

root=${FLOWCORE_ROOT:?}
flowmini=${FLOWMINI_BIN:?}
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

set +e
"$flowmini" --diagnostics json \
    "$root/Lyraform/compiler/examples/fail/bad_if_int.flow" \
    >"$tmpdir/stdout" 2>"$tmpdir/stderr"
status=$?
set -e

test "$status" -eq 1
test ! -s "$tmpdir/stdout"
jq -e '
    .status == "failed" and
    .code == "FLOW_DIAGNOSTIC_ERROR" and
    .stage == "lowerer" and
    (.message | length > 0) and
    .disposition == "no_artifact"
' "$tmpdir/stderr" >/dev/null

set +e
"$flowmini" --diagnostics text \
    "$root/Lyraform/compiler/examples/fail/bad_if_int.flow" \
    >"$tmpdir/text-out" 2>"$tmpdir/text-err"
status=$?
set -e

test "$status" -eq 1
grep -Fq 'unsupported diagnostics format: text' "$tmpdir/text-err"
printf '%s\n' 'structured CLI diagnostics: PASS'
