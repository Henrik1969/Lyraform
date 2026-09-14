#!/bin/sh
set -eu

clock=${CLOCK_BIN:?}
revision=${REVISION_BIN:?}
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

set +e
"$clock" --diagnostics json --clock unknown >"$tmpdir/clock.out" 2>"$tmpdir/clock.err"
clock_status=$?
"$revision" --diagnostics json --old-revision 8 --new-revision 8 >"$tmpdir/revision.out" 2>"$tmpdir/revision.err"
revision_status=$?
set -e

test "$clock_status" -eq 1
test "$revision_status" -eq 1
test ! -s "$tmpdir/clock.out"
test ! -s "$tmpdir/revision.out"
jq -e '.status == "failed" and .code == "FRANKENCORE_CLOCK_FAILURE" and .disposition == "no_artifact"' \
    "$tmpdir/clock.err" >/dev/null
jq -e '.status == "failed" and .code == "FRANKENCORE_REVISION_FAILURE" and .disposition == "no_artifact"' \
    "$tmpdir/revision.err" >/dev/null
printf '%s\n' 'reference structured diagnostics: PASS'
