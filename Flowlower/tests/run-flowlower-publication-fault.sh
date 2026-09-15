#!/bin/sh
set -eu

fault_bin=${FLOWLOWER_PUBLICATION_FAULT_BIN:?}
lower=${FLOWLOWER_BIN:?}
fixture_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
fixture="$fixture_dir/captured-empty-lowering.json"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
output="$tmpdir/output.ll"

for mode in write sync close rename
do
    printf 'previous\n' >"$output"
    set +e
    FLOWLOWER_PUBLICATION_FAULT=$mode "$fault_bin" --diagnostics json \
        --emit-llvm "$output" "$fixture" >"$tmpdir/$mode.stdout" 2>"$tmpdir/$mode.stderr"
    status=$?
    set -e
    test "$status" -eq 1
    test ! -s "$tmpdir/$mode.stdout"
    jq -e '.status == "failed" and .code == "FLOWLOWER_OUTPUT_FAILURE" and .stage == "output" and .disposition == "no_artifact"' \
        "$tmpdir/$mode.stderr" >/dev/null
    printf 'previous\n' | cmp -s - "$output"
    if find "$tmpdir" -maxdepth 1 -name 'output.ll.tmp.*' | grep -q .; then
        echo "temporary LLVM output remained after $mode failure" >&2
        exit 1
    fi
done

for mode in directory-sync directory-close
do
    printf 'previous\n' >"$output"
    set +e
    FLOWLOWER_PUBLICATION_FAULT=$mode "$fault_bin" --diagnostics json \
        --emit-llvm "$output" "$fixture" >"$tmpdir/$mode.stdout" 2>"$tmpdir/$mode.stderr"
    status=$?
    set -e
    test "$status" -eq 1
    test ! -s "$tmpdir/$mode.stdout"
    jq -e '
      .status == "failed" and
      .code == "FLOWLOWER_OUTPUT_DURABILITY_UNCERTAIN" and
      .stage == "output" and
      .disposition == "artifact_published_durability_uncertain" and
      (.message | contains("parent directory durability is uncertain"))
    ' "$tmpdir/$mode.stderr" >/dev/null
    grep -q 'define i32 @main()' "$output"
    if find "$tmpdir" -maxdepth 1 -name 'output.ll.tmp.*' | grep -q .; then
        echo "temporary LLVM output remained after $mode failure" >&2
        exit 1
    fi
done

printf 'previous\n' >"$output"
set +e
FLOWLOWER_PUBLICATION_FAULT=abrupt-write "$fault_bin" --diagnostics json \
    --emit-llvm "$output" "$fixture" >"$tmpdir/abrupt-write.stdout" 2>"$tmpdir/abrupt-write.stderr"
status=$?
set -e
test "$status" -eq 86
test ! -s "$tmpdir/abrupt-write.stdout"
printf 'previous\n' | cmp -s - "$output"
test -e "$output.tmp.lyraform-v1"

"$lower" --emit-llvm "$output" "$fixture" >"$tmpdir/success.json"
grep -q 'define i32 @main()' "$output"
test ! -e "$output.tmp.lyraform-v1"
jq -e '.status == "ready" and .artifact.status == "emitted"' "$tmpdir/success.json" >/dev/null
echo 'Flowlower atomic publication faults: PASS'
