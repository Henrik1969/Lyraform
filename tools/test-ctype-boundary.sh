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
fixture="$root/Lyraform/compiler/examples/pass/abi_ctype_main.flow"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT

printf '%s\n' \
  'allow libc.so.6 tolower c pure c_int c_int' \
  'allow libc.so.6 toupper c pure c_int c_int' \
  'allow libc.so.6 puts c io c_string c_int' > "$work/policy"

"$flowmini" --dump-frontend-bundle "$fixture" > "$work/frontend.json"
"$analyst" --lowering-plan-version 2 < "$work/frontend.json" > "$work/semantic.json"
jq -e '
  .lowering_plan.operations
  | map(select(.kind == "external_call"))
  | map(.provider | {library,symbol,convention,effect,parameter_types,return_type})
  | unique
  | sort_by(.symbol)
  == [
    {library:"libc.so.6",symbol:"puts",convention:"c",effect:"io",parameter_types:"c_string",return_type:"c_int"},
    {library:"libc.so.6",symbol:"tolower",convention:"c",effect:"pure",parameter_types:"c_int",return_type:"c_int"},
    {library:"libc.so.6",symbol:"toupper",convention:"c",effect:"pure",parameter_types:"c_int",return_type:"c_int"}
  ]
' "$work/semantic.json" >/dev/null
"$parallel" < "$work/semantic.json" > "$work/parallel.json"
"$optimize" < "$work/parallel.json" > "$work/optimized.json"
"$bind" --policy "$work/policy" < "$work/semantic.json" > "$work/binding.json"
printf '%s\n' \
  'allow libc.so.6 tolower c pure c_int c_int' \
  'allow libc.so.6 puts c io c_string c_int' > "$work/incomplete-policy"
if "$bind" --policy "$work/incomplete-policy" < "$work/semantic.json" > "$work/rejected-binding.json" 2> "$work/rejected-binding.err"; then
  echo "ctype incomplete policy was unexpectedly accepted" >&2
  exit 1
fi
"$prepare" --binding-report "$work/binding.json" "$work/optimized.json" > "$work/lowering.json"
"$lower" --emit-llvm "$work/ctype.ll" --binding-report "$work/binding.json" < "$work/optimized.json" >/dev/null
grep -q 'call i32 @tolower' "$work/ctype.ll"
grep -q 'call i32 @toupper' "$work/ctype.ll"
clang "$work/ctype.ll" -o "$work/ctype-native"
native_output="$("$work/ctype-native")"
grep -q '^tolower ok$' <<< "$native_output"
grep -q '^toupper ok$' <<< "$native_output"

"$tiny_lower" "$work/lowering.json" "$work/ctype.tvm" >/dev/null
"$tiny_validate" "$work/ctype.tvm" > "$work/tiny-validation.json"
tiny_output="$("$tiny_run" --policy "$work/policy" "$work/ctype.tvm" | sed '$d')"
grep -q '^tolower ok$' <<< "$tiny_output"
grep -q '^toupper ok$' <<< "$tiny_output"

test "$native_output" = "$tiny_output"
echo "ctype library boundary: PASS"
