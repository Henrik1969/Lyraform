#!/bin/sh
set -eu

flowkernel=${1:?flowkernel binary is required}
stdout=$(mktemp)
stderr=$(mktemp)
trap 'rm -f "$stdout" "$stderr"' EXIT
set +e
"$flowkernel" --diagnostics json --probe unknown >"$stdout" 2>"$stderr"
status=$?
set -e
test "$status" -eq 1
test ! -s "$stdout"
jq -e '.status == "failed" and .code == "FLOWKERNEL_FAILURE" and (.message | length > 0) and .disposition == "no_artifact"' "$stderr" >/dev/null
echo 'Flowkernel structured diagnostics: PASS'
