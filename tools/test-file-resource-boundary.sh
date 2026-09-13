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
fixture="$root/Lyraform/compiler/examples/pass/abi_file_resource_main.flow"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT

printf '%s\n' \
  'allow libc.so.6 open c io c_string,c_int c_int' \
  'allow libc.so.6 close c io c_int c_int' \
  'allow libc.so.6 puts c io c_string c_int' > "$work/policy"

"$flowmini" --dump-frontend-bundle "$fixture" > "$work/frontend.json"
"$analyst" --lowering-plan-version 2 < "$work/frontend.json" > "$work/semantic.json"
jq -e '
  .status == "ok" and
  any(.binding_requirements[]; .contract == "file_io" and .symbol == "open" and .parameter_types == "c_string,c_int" and .return_type == "c_int") and
  any(.binding_requirements[]; .contract == "file_io" and .symbol == "close" and .parameter_types == "c_int" and .return_type == "c_int") and
  any(.lowering_plan.operations[]; .kind == "external_call" and .provider.contract == "file_io" and .provider.symbol == "open") and
  any(.lowering_plan.operations[]; .kind == "external_call" and .provider.contract == "file_io" and .provider.symbol == "close")
' "$work/semantic.json" >/dev/null
"$parallel" < "$work/semantic.json" > "$work/parallel.json"
"$optimize" < "$work/parallel.json" > "$work/optimized.json"
"$bind" --policy "$work/policy" < "$work/semantic.json" > "$work/binding.json"
"$prepare" --binding-report "$work/binding.json" "$work/optimized.json" > "$work/lowering.json"
"$lower" --emit-llvm "$work/file.ll" --binding-report "$work/binding.json" < "$work/optimized.json" >/dev/null
grep -q 'call i32 @open' "$work/file.ll"
grep -q 'call i32 @close' "$work/file.ll"
clang "$work/file.ll" -o "$work/file-native"
native_output="$($work/file-native)"
printf '%s\n' 'file open ok' 'file close ok' > "$work/expected"
printf '%s\n' "$native_output" > "$work/native-output"
cmp -s "$work/expected" "$work/native-output"

if ! "$tiny_lower" "$work/lowering.json" "$work/file.tvm" > "$work/tiny-lowering-output.json"; then
  jq -c '.lowering_plan.operations[] | select(.kind == "external_call") | .provider' "$work/lowering.json" >&2
  cat "$work/tiny-lowering-output.json" >&2
  exit 1
fi
"$tiny_validate" "$work/file.tvm" > "$work/tiny-validation.json"
tiny_output="$($tiny_run --policy "$work/policy" "$work/file.tvm" | sed '$d')"
printf '%s\n' "$tiny_output" > "$work/tiny-output"
cmp -s "$work/expected" "$work/tiny-output"
test "$native_output" = "$tiny_output"
echo "file resource boundary: PASS"
