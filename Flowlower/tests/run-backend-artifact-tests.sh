#!/bin/sh
set -eu

prepare=${FLOWPREPARE_BIN:?}
validate=${FLOWVALIDATE_BIN:?}
lower=${FLOWLOWER_BIN:?}
fixture_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

set +e
printf '%s' '{"format":"wrong","version":1}' | "$prepare" --diagnostics json >"$tmpdir/hostile-stdout" 2>"$tmpdir/hostile-stderr"
hostile_rc=$?
set -e
test "$hostile_rc" -eq 1
test ! -s "$tmpdir/hostile-stdout"
jq -e '.status == "failed" and .code == "FLOWPREPARE_CONTRACT_FAILURE" and .stage == "contract" and (.message | length > 0) and .disposition == "no_artifact"' "$tmpdir/hostile-stderr" >/dev/null

"$prepare" "$fixture_dir/captured-empty-optimization.json" > "$tmpdir/prepared.json"
cmp -s "$fixture_dir/captured-empty-lowering.json" "$tmpdir/prepared.json"
"$validate" --canonical "$tmpdir/prepared.json" > "$tmpdir/canonical.json"
cmp -s "$tmpdir/prepared.json" "$tmpdir/canonical.json"

# Captured replay deliberately invokes only the validator and LLVM consumer.
"$validate" "$fixture_dir/captured-empty-lowering.json" | grep -q '"classification":"valid"'
"$lower" --emit-llvm "$tmpdir/empty.ll" "$fixture_dir/captured-empty-lowering.json" > "$tmpdir/report.json"
grep -q 'define i32 @main()' "$tmpdir/empty.ll"
grep -q '"status": "emitted"' "$tmpdir/report.json"

for mutation in \
  '.format = "flowcore.unknown"' \
  '.target.name = ""' \
  '.lowering_plan.operations = [{"id":0,"kind":"external_call","operands":[],"provider":{"contract":"c","library":"libc.so.6","symbol":"abs","convention":"c","effect":"pure","parameter_types":"c_int","return_type":"c_int"},"effect_contract":{"external":"pure","determinism":"deterministic","certainty":"declared"},"argument_resources":[]}]' \
  '.authorization.status = "authorized"'
do
  jq "$mutation" "$fixture_dir/captured-empty-lowering.json" > "$tmpdir/mutated.json"
  if "$validate" "$tmpdir/mutated.json" >/dev/null 2>&1; then
    echo "flowvalidate accepted backend artifact mutation: $mutation" >&2
    exit 1
  fi
done

echo 'backend lowering artifact boundary: PASS'
