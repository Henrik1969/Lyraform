#!/bin/sh
set -eu

compiler=${FLOWMINI_BIN:?FLOWMINI_BIN is required}
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

printf '%b' 'program valid_utf8\nmain {\n    return 0\n}\n// caf\303\251 \342\202\254 \360\237\214\215\n' > "$tmpdir/valid.flow"
"$compiler" --dump-frontend-bundle "$tmpdir/valid.flow" | jq -e '.format == "flowmini.frontend_bundle"' >/dev/null

prefix='program invalid_utf8
main {
    return 0
}
'

check_invalid() {
    name=$1
    bytes=$2
    printf '%s' "$prefix" > "$tmpdir/$name.flow"
    printf '%b' "$bytes" >> "$tmpdir/$name.flow"
    set +e
    output=$("$compiler" --diagnostics json "$tmpdir/$name.flow" 2>"$tmpdir/$name.error.json")
    status=$?
    set -e
    test "$status" -eq 1
    test -z "$output"
    jq -e '.status == "failed" and .code == "FLOW_DIAGNOSTIC_ERROR" and .stage == "source" and (.message | contains("invalid UTF-8 at byte")) and .disposition == "no_artifact"' "$tmpdir/$name.error.json" >/dev/null
}

check_invalid truncated_2 '\302'
check_invalid bad_continuation '\342\050\241'
check_invalid overlong_4 '\360\200\200\200'
check_invalid surrogate '\355\240\200'
check_invalid out_of_range '\364\220\200\200'
check_invalid truncated_4 '\360\220\200'

dd if=/dev/zero of="$tmpdir/oversized.flow" bs=1048576 count=17 2>/dev/null
set +e
output=$("$compiler" --diagnostics json "$tmpdir/oversized.flow" 2>"$tmpdir/oversized.error.json")
status=$?
set -e
test "$status" -eq 1
test -z "$output"
jq -e '.status == "failed" and .code == "FLOW_DIAGNOSTIC_ERROR" and .disposition == "no_artifact" and (.message | contains("16 MiB input limit"))' "$tmpdir/oversized.error.json" >/dev/null
echo 'Flowmini UTF-8 source boundary: PASS'
