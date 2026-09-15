#!/bin/sh
set -eu

fault_bin=${FLOWMINI_PUBLICATION_FAULT_BIN:?}
compiler=${FLOWMINI_BIN:?}
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
source_file="$root/examples/pass/fn_demo.flow"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
output="$tmpdir/output.flowir"

for mode in write sync close rename
do
    printf 'previous\n' >"$output"
    set +e
    FLOWMINI_PUBLICATION_FAULT=$mode "$fault_bin" --diagnostics json \
        --emit-flowir "$output" "$source_file" >"$tmpdir/$mode.stdout" 2>"$tmpdir/$mode.stderr"
    status=$?
    set -e
    test "$status" -eq 1
    test ! -s "$tmpdir/$mode.stdout"
    jq -e '.status == "failed" and .code == "FLOW_OUTPUT_FAILURE" and .stage == "output" and .disposition == "no_artifact"' \
        "$tmpdir/$mode.stderr" >/dev/null
    printf 'previous\n' | cmp -s - "$output"
    if find "$tmpdir" -maxdepth 1 -name 'output.flowir.tmp.*' | grep -q .; then
        echo "temporary FlowIR output remained after $mode failure" >&2
        exit 1
    fi
done

printf 'previous\n' >"$output"
set +e
FLOWMINI_PUBLICATION_FAULT=directory-sync "$fault_bin" --diagnostics json \
    --emit-flowir "$output" "$source_file" >"$tmpdir/directory-sync.stdout" 2>"$tmpdir/directory-sync.stderr"
status=$?
set -e
test "$status" -eq 1
test ! -s "$tmpdir/directory-sync.stdout"
jq -e '
  .status == "failed" and
  .code == "FLOW_OUTPUT_DURABILITY_UNCERTAIN" and
  .stage == "output" and
  .disposition == "artifact_published_durability_uncertain" and
  (.message | contains("parent directory durability is uncertain"))
' "$tmpdir/directory-sync.stderr" >/dev/null
grep -q '^module ' "$output"
if find "$tmpdir" -maxdepth 1 -name 'output.flowir.tmp.*' | grep -q .; then
    echo "temporary FlowIR output remained after directory-sync failure" >&2
    exit 1
fi

"$compiler" --emit-flowir "$output" "$source_file"
test -s "$output"
echo 'Flowmini atomic publication faults: PASS'
