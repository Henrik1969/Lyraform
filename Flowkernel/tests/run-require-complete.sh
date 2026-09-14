#!/bin/sh
set -eu

flowkernel=${1:?flowkernel binary is required}
tmp=$(mktemp)
trap 'rm -f "$tmp"' EXIT
set +e
"$flowkernel" --probe all --require-complete >"$tmp"
status=$?
set -e

jq -e '.format == "flowkernel.probe_report" and (.status == "ok" or .status == "ok-with-skips")' "$tmp" >/dev/null
if jq -e '.status == "ok-with-skips"' "$tmp" >/dev/null; then
    test "$status" -eq 3
else
    test "$status" -eq 0
fi
echo 'Flowkernel require-complete contract: PASS'
