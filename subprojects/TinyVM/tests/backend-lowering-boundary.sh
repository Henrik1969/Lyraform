#!/bin/sh
set -eu

lower=$1
validate=$2
run=$3
fixture=$4
build=$5
artifact="$build/backend-empty.tvm"
artifact_again="$build/backend-empty-again.tvm"
report="$build/backend-empty-report.json"

check_failure() {
    name=$1
    code=$2
    stage=$3
    shift 3
    stdout="$build/$name.stdout"
    stderr="$build/$name.stderr"
    set +e
    "$@" >"$stdout" 2>"$stderr"
    status=$?
    set -e
    test "$status" -eq 1
    test ! -s "$stdout"
    jq -e --arg code "$code" --arg stage "$stage" \
        '.status == "failed" and .code == $code and .stage == $stage and .disposition == "no_artifact" and (.message | length > 0)' \
        "$stderr" >/dev/null
}

"$lower" "$fixture" "$artifact" > "$report"
"$lower" "$fixture" "$artifact_again" >/dev/null
cmp -s "$artifact" "$artifact_again"
grep -q '"status":"emitted"' "$report"
"$validate" "$artifact" | grep -q '"status":"valid"'
"$run" "$artifact" | grep -q '"carrier":2,"result":0'

jq '.lowering_plan.operations = [{"id":0,"kind":"switch","block_id":0,"statement_id":0,"operands":[]}]' "$fixture" > "$build/nonempty.json"
rm -f "$build/nonempty.tvm"
set +e
"$lower" "$build/nonempty.json" "$build/nonempty.tvm" > "$build/unsupported.json"
status=$?
set -e
test "$status" -eq 2
grep -q '"status":"unsupported"' "$build/unsupported.json"
test ! -e "$build/nonempty.tvm"

jq '
  .authorization = {"status":"authorized","capabilities":[{"contract":"kernel","library":"libc.so.6","symbol":"fork","convention":"c","effect":"process_ipc","parameter_types":"","return_type":"c_int","status":"authorized"}]} |
  .lowering_plan.operations = [{"id":0,"kind":"external_call","block_id":0,"statement_id":0,"result_symbol_id":1,"operands":[],"provider":{"contract":"kernel","library":"libc.so.6","symbol":"fork","convention":"c","effect":"process_ipc","parameter_types":"","return_type":"c_int"},"effect_contract":{"external":"process_ipc","determinism":"nondeterministic","certainty":"declared"},"argument_resources":[]}]
' "$fixture" > "$build/unsupported-provider.json"
rm -f "$build/unsupported-provider.tvm"
set +e
"$lower" "$build/unsupported-provider.json" "$build/unsupported-provider.tvm" > "$build/unsupported-provider-result.json"
status=$?
set -e
test "$status" -eq 2
grep -q '"status":"unsupported"' "$build/unsupported-provider-result.json"
grep -q 'typed-call slice' "$build/unsupported-provider-result.json"
test ! -e "$build/unsupported-provider.tvm"

for value in \
  '{"kind":"string_literal","type":"c_string","value":"captured"}' \
  '{"kind":"writable_storage","type":"c_pointer","storage":{"bytes":16,"access":"read_write","lifetime":"call"}}'
do
  jq --argjson value "$value" '.lowering_plan.operations = [{"id":0,"kind":"value_definition","block_id":0,"statement_id":0,"result_symbol_id":1,"operands":[$value]}]' "$fixture" > "$build/opaque-value.json"
  "$lower" "$build/opaque-value.json" "$build/opaque-value.tvm" >/dev/null
  "$validate" "$build/opaque-value.tvm" | grep -q '"status":"valid"'
  "$run" "$build/opaque-value.tvm" | grep -q '"carrier":2,"result":0'
done

printf '{"version":' >"$build/backend-malformed.json"
rm -f "$build/backend-malformed.tvm"
check_failure malformed FLOWTINYLOWER_CONTRACT_FAILURE contract \
    "$lower" --diagnostics json "$build/backend-malformed.json" "$build/backend-malformed.tvm"
test ! -e "$build/backend-malformed.tvm"

rm -f "$build/backend-missing.json" "$build/backend-missing.tvm"
check_failure missing FLOWTINYLOWER_INPUT_INVALID input \
    "$lower" --diagnostics json "$build/backend-missing.json" "$build/backend-missing.tvm"
test ! -e "$build/backend-missing.tvm"

dd if=/dev/zero of="$build/backend-oversized.json" bs=1048576 count=17 2>/dev/null
rm -f "$build/backend-oversized.tvm"
check_failure oversized FLOWTINYLOWER_INPUT_INVALID input \
    "$lower" --diagnostics json "$build/backend-oversized.json" "$build/backend-oversized.tvm"
test ! -e "$build/backend-oversized.tvm"

check_failure output FLOWTINYLOWER_OUTPUT_FAILURE output \
    "$lower" --diagnostics json "$fixture" "$build"

echo 'TinyVM backend lowering boundary: PASS'
