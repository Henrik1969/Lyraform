#!/bin/sh
set -eu

compiler=${FLOWMINI_BIN:?FLOWMINI_BIN is required}
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

printf '%b' 'program valid_utf8\nmain {\n    return 0\n}\n// caf\303\251\n' > "$tmpdir/valid.flow"
"$compiler" --dump-frontend-bundle "$tmpdir/valid.flow" | jq -e '.format == "flowmini.frontend_bundle"' >/dev/null

printf '%s' 'program invalid_utf8
main {
    return 0
}
' > "$tmpdir/invalid.flow"
printf '\360\200\200\200' >> "$tmpdir/invalid.flow"

set +e
output=$("$compiler" --diagnostics json "$tmpdir/invalid.flow" 2>"$tmpdir/error.json")
status=$?
set -e

test "$status" -eq 1
test -z "$output"
jq -e '.status == "failed" and .code == "FLOW_DIAGNOSTIC_ERROR" and .stage == "source" and (.message | contains("invalid UTF-8 at byte")) and .disposition == "no_artifact"' "$tmpdir/error.json" >/dev/null
echo 'Flowmini UTF-8 source boundary: PASS'
