#!/usr/bin/env bash
set -euo pipefail

root="${FLOWCORE_ROOT:?}"
flowmini="${FLOWMINI_BIN:?}"
analyst="${FLOWANALYST_BIN:?}"
parallel="${FLOWPARALLEL_BIN:?}"
optimize="${FLOWOPTIMIZE_BIN:?}"
prepare="${FLOWPREPARE_BIN:?}"
bind="${FLOWBIND_BIN:?}"
lower="${FLOWLOWER_BIN:?}"
tiny_lower="${FLOWTINYLOWER_BIN:?}"
tiny_validate="${FLOWTINYVALIDATE_BIN:?}"
tiny_run="${FLOWTINYRUN_BIN:?}"
fixture="$root/Lyraform/compiler/examples/pass/abi_file_io_main.flow"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT

printf '%s\n' \
  'allow libc.so.6 open c io c_string,c_int c_int' \
  'allow libc.so.6 read c io c_int,c_pointer,c_size_t c_long' \
  'allow libc.so.6 write c io c_int,c_pointer,c_size_t c_long' \
  'allow libc.so.6 close c io c_int c_int' \
  'allow libc.so.6 puts c io c_string c_int' > "$work/policy"

"$flowmini" --dump-frontend-bundle "$fixture" > "$work/frontend.json"
"$analyst" --lowering-plan-version 2 < "$work/frontend.json" > "$work/semantic.json"
jq -e '
  .status == "ok" and
  any(.binding_requirements[]; .symbol == "read" and .parameter_types == "c_int,c_pointer,c_size_t" and .return_type == "c_long") and
  any(.binding_requirements[]; .symbol == "write" and .parameter_types == "c_int,c_pointer,c_size_t" and .return_type == "c_long") and
  any(.lowering_plan.operations[]; .kind == "external_call" and .provider.symbol == "read") and
  any(.lowering_plan.operations[]; .kind == "external_call" and .provider.symbol == "write")
' "$work/semantic.json" >/dev/null
"$parallel" < "$work/semantic.json" > "$work/parallel.json"
"$optimize" < "$work/parallel.json" > "$work/optimized.json"
"$bind" --policy "$work/policy" < "$work/semantic.json" > "$work/binding.json"
"$prepare" --binding-report "$work/binding.json" "$work/optimized.json" > "$work/lowering.json"
"$lower" --emit-llvm "$work/file.ll" --binding-report "$work/binding.json" < "$work/optimized.json" >/dev/null
grep -q 'call i64 @read' "$work/file.ll"
grep -q 'call i64 @write' "$work/file.ll"
clang "$work/file.ll" -o "$work/file-native"
native_output="$($work/file-native)"
printf '%s\n' 'read bounded ok' 'write bounded ok' 'write failure ok' > "$work/expected"
printf '%s\n' "$native_output" > "$work/native-output"
cmp -s "$work/expected" "$work/native-output"

"$tiny_lower" "$work/lowering.json" "$work/file.tvm" >/dev/null
"$tiny_validate" "$work/file.tvm" > "$work/tiny-validation.json"
tiny_output="$($tiny_run --policy "$work/policy" "$work/file.tvm" | sed '$d')"
printf '%s\n' "$tiny_output" > "$work/tiny-output"
cmp -s "$work/expected" "$work/tiny-output"
test "$native_output" = "$tiny_output"

refusal_fixture="$root/Lyraform/compiler/examples/fail/abi_file_sendfile_unsupported.flow"
printf '%s\n' \
  'allow libc.so.6 sendfile c io c_int,c_int,c_pointer,c_size_t c_long' > "$work/refusal-policy"
"$flowmini" --dump-frontend-bundle "$refusal_fixture" > "$work/refusal-frontend.json"
"$analyst" --lowering-plan-version 2 < "$work/refusal-frontend.json" > "$work/refusal-semantic.json"
"$parallel" < "$work/refusal-semantic.json" > "$work/refusal-parallel.json"
"$optimize" < "$work/refusal-parallel.json" > "$work/refusal-optimized.json"
"$bind" --policy "$work/refusal-policy" < "$work/refusal-semantic.json" > "$work/refusal-binding.json"
"$prepare" --binding-report "$work/refusal-binding.json" "$work/refusal-optimized.json" > "$work/refusal-lowering.json"
if "$tiny_lower" "$work/refusal-lowering.json" "$work/refusal.tvm" > "$work/refusal-result.json" 2> "$work/refusal-error"; then
  echo "sendfile was unexpectedly admitted by TinyVM" >&2
  exit 1
fi
jq -e '.status == "unsupported" and (.reason | contains("not admitted"))' "$work/refusal-result.json" >/dev/null
echo "file I/O boundary: PASS"
