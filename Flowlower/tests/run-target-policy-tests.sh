#!/bin/sh
set -eu

target=${FLOWTARGET_BIN:?}
validate=${FLOWVALIDATE_BIN:?}
root=$(CDPATH= cd -- "$(dirname -- "$0")/../target-policies" && pwd)
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

for name in llvm-host tinyvm-portable; do
    "$target" --policy-root "$root" "$name" > "$tmpdir/$name.json"
    "$validate" --canonical "$tmpdir/$name.json" > "$tmpdir/$name.canonical.json"
    cmp -s "$tmpdir/$name.json" "$tmpdir/$name.canonical.json"
    grep -q '"classification":"valid"' <<EOF
$("$validate" "$tmpdir/$name.json")
EOF
done

test "$(jq -r .backend.name "$tmpdir/llvm-host.json")" = llvm
test "$(jq -r .backend.name "$tmpdir/tinyvm-portable.json")" = tinyvm
test "$(jq -r .fallback.mode "$tmpdir/llvm-host.json")" = none
test "$(jq -r .fallback.mode "$tmpdir/tinyvm-portable.json")" = none

if "$target" --policy-root "$root" missing >/dev/null 2>"$tmpdir/missing.err"; then
    echo 'flowtarget resolved a missing policy' >&2; exit 1
fi
grep -q 'target policy is unavailable' "$tmpdir/missing.err"
set +e
"$target" --policy-root "$root" missing --diagnostics json >"$tmpdir/missing.stdout" 2>"$tmpdir/missing.json"
missing_rc=$?
set -e
test "$missing_rc" -eq 1
test ! -s "$tmpdir/missing.stdout"
jq -e '.status == "failed" and .code == "FLOWTARGET_INPUT_INVALID" and .stage == "input" and (.message | length > 0) and .disposition == "no_artifact"' "$tmpdir/missing.json" >/dev/null
if "$target" --policy-root "$root" ../llvm-host >/dev/null 2>"$tmpdir/name.err"; then
    echo 'flowtarget accepted a path-like target name' >&2; exit 1
fi
grep -q 'invalid target policy name' "$tmpdir/name.err"
set +e
"$target" --policy-root "$root" ../llvm-host --diagnostics json >"$tmpdir/name.stdout" 2>"$tmpdir/name.json"
name_rc=$?
set -e
test "$name_rc" -eq 1
test ! -s "$tmpdir/name.stdout"
jq -e '.status == "failed" and .code == "FLOWTARGET_INPUT_INVALID" and .stage == "input" and .disposition == "no_artifact"' "$tmpdir/name.json" >/dev/null

jq '.name = "wrong-name"' "$root/llvm-host.json" > "$tmpdir/mismatch.json"
mkdir "$tmpdir/policies"
cp "$tmpdir/mismatch.json" "$tmpdir/policies/requested.json"
if "$target" --policy-root "$tmpdir/policies" requested >/dev/null 2>"$tmpdir/mismatch.err"; then
    echo 'flowtarget accepted mismatched policy identity' >&2; exit 1
fi
grep -q 'resolved policy identity does not match' "$tmpdir/mismatch.err"

echo 'target policy boundary: PASS'
