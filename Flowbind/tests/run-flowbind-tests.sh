#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
bin=${FLOWBIND_BIN:?FLOWBIND_BIN is required}
flowmini=${FLOWMINI_BIN:-$root/Lyraform/compiler/cmake-build-debug/flowmini}
flowanalyst=${FLOWANALYST_BIN:-$root/Flowanalyst/build/flowanalyst}
fixture=$root/Lyraform/compiler/examples/pass/abi_libc_demo.flow
test -x "$flowmini"
test -x "$flowanalyst"
test -x "$bin"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
policy="$tmpdir/policy"
printf '%s\n' \
  'allow libc.so.6 strlen c pure c_string c_size_t' \
  'allow libc.so.6 abs c pure' \
  'allow libc.so.6 labs c pure' \
  'allow libc.so.6 puts c io' \
  'allow libc.so.6 open c io' \
  'allow libc.so.6 read c io' \
  'allow libc.so.6 write c io' \
  'allow libc.so.6 sendfile c io' \
  'allow libc.so.6 close c io' \
  'allow libc.so.6 flowcore_symbol_that_does_not_exist c pure' \
  "allow $tmpdir/provider-that-is-not-present.so exported_symbol c pure" > "$policy"

for hostile in \
  '{"format":"flowanalyst.semantic_report","format":"flowanalyst.semantic_report","version":1,"status":"ok","binding_requirements":[]}' \
  '{"decoy":{"format":"flowanalyst.semantic_report","version":1,"status":"ok"},"binding_requirements":[]}' \
  '{"format":"flowanalyst.semantic_report","version":9223372036854775808,"status":"ok","binding_requirements":[]}'
do
  if printf '%s' "$hostile" | "$bin" --policy "$policy" >/dev/null 2>&1; then
    echo 'Flowbind accepted malformed or non-authoritative envelope' >&2
    exit 1
  fi
done

escaped_authority='{"f\u006frmat":"flowanalyst.semantic_report","version":1,"status":"ok","binding_requirements":[]}'
printf '%s' "$escaped_authority" | "$bin" --policy "$policy" | jq -e '.status == "ready"' >/dev/null

report=$("$flowmini" --dump-frontend-bundle "$fixture" | "$flowanalyst" | "$bin" --policy "$policy")
printf '%s\n' "$report" | grep -q '"status": "ready"'
printf '%s\n' "$report" | grep -q '"execution": "not-performed"'
printf '%s\n' "$report" | grep -q '"carrier_types_supported": true'
printf '%s\n' "$report" | grep -q '"provider_signature_evidence": "not-provided"'

set +e
wrong_signature=$(printf '%s' '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","binding_requirements":[{"contract":"bad","library":"libc.so.6","convention":"c","symbol":"strlen","effect":"pure","parameter_types":"c_string","return_type":"c_int"}]}' | "$bin" --policy "$policy")
wrong_signature_rc=$?
set -e
test "$wrong_signature_rc" -eq 2
printf '%s\n' "$wrong_signature" | grep -q 'denied by capability policy'

file_fixture=$root/Lyraform/compiler/examples/apps/flowcat/flowcat.flow
file_semantic=$("$flowmini" --dump-frontend-bundle "$file_fixture" | "$flowanalyst")
file_report=$(printf '%s\n' "$file_semantic" | "$bin" --policy "$policy")
printf '%s\n' "$file_report" | jq -e '.status == "ready" and (.symbols | index("open")) != null and (.symbols | index("sendfile")) != null and (.symbols | index("close")) != null' >/dev/null

hostile_file_semantic=$(printf '%s\n' "$file_semantic" | jq '(.lowering_plan.operations[] | select(.provider.symbol == "sendfile") | .operands[3].type) = "c_long"')
set +e
printf '%s\n' "$hostile_file_semantic" | "$bin" --policy "$policy" >/dev/null 2>&1
hostile_file_rc=$?
set -e
test "$hostile_file_rc" -eq 1

hostile_memory_semantic=$(printf '%s\n' "$file_semantic" | jq '(.lowering_plan.operations[] | select(.provider.symbol == "sendfile") | .argument_resources[2].memory_effect) = "read"')
set +e
printf '%s\n' "$hostile_memory_semantic" | "$bin" --policy "$policy" >/dev/null 2>&1
hostile_memory_rc=$?
set -e
test "$hostile_memory_rc" -eq 1

hostile_effect_semantic=$(printf '%s\n' "$file_semantic" | jq '(.lowering_plan.operations[] | select(.provider.symbol == "sendfile") | .effect_contract.determinism) = "deterministic"')
set +e
printf '%s\n' "$hostile_effect_semantic" | "$bin" --policy "$policy" >/dev/null 2>&1
hostile_effect_rc=$?
set -e
test "$hostile_effect_rc" -eq 1

set +e
bad=$(printf '%s' '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","binding_requirements":[{"contract":"bad","library":"libc.so.6","convention":"c","symbol":"flowcore_symbol_that_does_not_exist","effect":"pure","parameter_types":"","return_type":"c_int"}]}' | "$bin" --policy "$policy")
bad_rc=$?
set -e
test "$bad_rc" -eq 2
printf '%s\n' "$bad" | grep -q 'unavailable'

set +e
missing_provider=$(printf '%s' "{\"format\":\"flowanalyst.semantic_report\",\"version\":1,\"status\":\"ok\",\"binding_requirements\":[{\"contract\":\"missing-provider\",\"library\":\"$tmpdir/provider-that-is-not-present.so\",\"convention\":\"c\",\"symbol\":\"exported_symbol\",\"effect\":\"pure\",\"parameter_types\":\"\",\"return_type\":\"c_int\"}]}" | "$bin" --policy "$policy")
missing_provider_rc=$?
set -e
test "$missing_provider_rc" -eq 2
printf '%s\n' "$missing_provider" | jq -e '
  .status == "blocked" and
  any(.failures[]; contains("library unavailable")) and
  (has("symbols") | not) and
  (has("execution") | not)
' >/dev/null

set +e
bad_type=$(printf '%s' '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","binding_requirements":[{"contract":"bad","library":"libc.so.6","convention":"c","symbol":"abs","effect":"pure","parameter_types":"not_an_abi_type","return_type":"c_int"}]}' | "$bin" --policy "$policy")
bad_type_rc=$?
set -e
test "$bad_type_rc" -eq 2
printf '%s\n' "$bad_type" | grep -q 'unsupported parameter ABI type'

set +e
bad_return=$(printf '%s' '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","binding_requirements":[{"contract":"bad","library":"libc.so.6","convention":"c","symbol":"abs","effect":"pure","parameter_types":"c_int","return_type":"native_struct"}]}' | "$bin" --policy "$policy")
bad_return_rc=$?
set -e
test "$bad_return_rc" -eq 2
printf '%s\n' "$bad_return" | grep -q 'unsupported return ABI type'

set +e
bad_convention=$(printf '%s' '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","binding_requirements":[{"contract":"bad","library":"libc.so.6","convention":"fastcall","symbol":"abs","effect":"pure","parameter_types":"c_int","return_type":"c_int"}]}' | "$bin" --policy "$policy")
bad_convention_rc=$?
set -e
test "$bad_convention_rc" -eq 2
printf '%s\n' "$bad_convention" | grep -q "unsupported calling convention 'fastcall'"

set +e
denied=$(printf '%s' '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","binding_requirements":[{"contract":"bad","library":"libc.so.6","convention":"c","symbol":"abs","effect":"pure","parameter_types":"c_int","return_type":"c_int"}]}' | "$bin")
denied_rc=$?
set -e
test "$denied_rc" -eq 2
printf '%s\n' "$denied" | grep -q 'denied by capability policy'

set +e
printf '%s' '{"format":"wrong","version":1}' | "$bin" --diagnostics json >"$tmpdir/hostile-stdout" 2>"$tmpdir/hostile-stderr"
hostile_rc=$?
set -e
test "$hostile_rc" -eq 1
test ! -s "$tmpdir/hostile-stdout"
jq -e '.status == "failed" and .code == "FLOWBIND_INPUT_INVALID" and .stage == "input" and (.message | length > 0) and .disposition == "no_artifact"' "$tmpdir/hostile-stderr" >/dev/null
set +e
jq -nc '{format:"flowanalyst.semantic_report",version:1,status:"ok",binding_requirements:[range(0;100001)|{contract:"test",library:"libc.so.6",convention:"c",symbol:"abs",effect:"pure",parameter_types:"c_int",return_type:"c_int"}]}' \
  | "$bin" --policy "$policy" --diagnostics json >"$tmpdir/oversized-stdout" 2>"$tmpdir/oversized-stderr"
oversized_rc=$?
set -e
test "$oversized_rc" -eq 1
test ! -s "$tmpdir/oversized-stdout"
jq -e '.status == "failed" and .code == "FLOWBIND_INPUT_INVALID" and .stage == "input" and .disposition == "no_artifact" and (.message | contains("100000-entry limit"))' "$tmpdir/oversized-stderr" >/dev/null

printf '%s\n' 'deny malformed-policy-line' > "$tmpdir/invalid-policy"
set +e
policy_diagnostic=$(printf '%s' '{"format":"flowanalyst.semantic_report","version":1,"status":"ok","binding_requirements":[]}' | "$bin" --policy "$tmpdir/invalid-policy" --diagnostics json 2>&1 >/dev/null)
policy_diagnostic_rc=$?
set -e
test "$policy_diagnostic_rc" -eq 1
printf '%s\n' "$policy_diagnostic" | jq -e '.code == "FLOWBIND_POLICY_FAILURE" and .stage == "policy" and .disposition == "no_artifact"' >/dev/null
echo 'Flowbind tests: PASS'
