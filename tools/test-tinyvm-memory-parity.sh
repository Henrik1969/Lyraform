#!/bin/sh
set -eu

root=${FLOWCORE_ROOT:?}
flowmini=${FLOWMINI_BIN:?}
analyst=${FLOWANALYST_BIN:?}
parallel=${FLOWPARALLEL_BIN:?}
optimize=${FLOWOPTIMIZE_BIN:?}
prepare=${FLOWPREPARE_BIN:?}
bind=${FLOWBIND_BIN:?}
llvm_lower=${FLOWLOWER_BIN:?}
tiny_lower=${FLOWTINYLOWER_BIN:?}
tiny_validate=${FLOWTINYVALIDATE_BIN:?}
tiny_run=${FLOWTINYRUN_BIN:?}

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
policy=$tmpdir/policy
printf '%s\n' \
  'allow libc.so.6 memset c io c_pointer,c_int,c_size_t c_pointer' \
  'allow libc.so.6 memcpy c io c_pointer,c_pointer,c_size_t c_pointer' \
  'allow libc.so.6 memmove c io c_pointer,c_pointer,c_size_t c_pointer' \
  'allow libc.so.6 memcmp c pure c_pointer,c_pointer,c_size_t c_int' > "$policy"

source=$root/Lyraform/compiler/examples/pass/abi_memory_demo.flow
"$flowmini" --dump-frontend-bundle "$source" > "$tmpdir/frontend.json"
"$analyst" < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
"$parallel" < "$tmpdir/semantic.json" > "$tmpdir/parallel.json"
"$optimize" < "$tmpdir/parallel.json" > "$tmpdir/optimized.json"
"$bind" --policy "$policy" < "$tmpdir/semantic.json" > "$tmpdir/binding.json"
"$prepare" --binding-report "$tmpdir/binding.json" "$tmpdir/optimized.json" > "$tmpdir/lowering.json"
"$llvm_lower" --emit-llvm "$tmpdir/memory.ll" --binding-report "$tmpdir/binding.json" < "$tmpdir/optimized.json" >/dev/null
clang "$tmpdir/memory.ll" -o "$tmpdir/memory.llvm"
if ! "$tiny_lower" "$tmpdir/lowering.json" "$tmpdir/memory.tvm" >"$tmpdir/tiny-lower.json" 2>"$tmpdir/tiny-lower.err"; then
  cat "$tmpdir/tiny-lower.json" >&2
  cat "$tmpdir/tiny-lower.err" >&2
  exit 1
fi
"$tiny_validate" "$tmpdir/memory.tvm" | grep -q '"status":"valid"'
set +e
"$tmpdir/memory.llvm"
llvm_status=$?
set -e
printf '%s\n' "$llvm_status" > "$tmpdir/llvm.output"
"$tiny_run" --policy "$policy" "$tmpdir/memory.tvm" > "$tmpdir/tiny.output"
tail -n 1 "$tmpdir/tiny.output" | jq -e '.status == "completed" and .result == 0' >/dev/null
tail -n 1 "$tmpdir/tiny.output" | jq -r '.result' > "$tmpdir/tiny.result"
cmp -s "$tmpdir/llvm.output" "$tmpdir/tiny.result"
"$tiny_run" --engine computed --policy "$policy" "$tmpdir/memory.tvm" > "$tmpdir/tiny.computed.output"
tail -n 1 "$tmpdir/tiny.computed.output" | jq -e '.status == "completed" and .result == 0' >/dev/null
tail -n 1 "$tmpdir/tiny.computed.output" | jq -r '.result' > "$tmpdir/tiny.computed.result"
cmp -s "$tmpdir/llvm.output" "$tmpdir/tiny.computed.result"

# A count beyond the declared storage must remain a governed runtime fault.
jq '.lowering_plan.operations |= map(if .kind == "value_definition" and .operands[0].type == "c_size_t" then .operands[0].value = "9" else . end)' \
  "$tmpdir/lowering.json" > "$tmpdir/out_of_bounds.json"
"$tiny_lower" "$tmpdir/out_of_bounds.json" "$tmpdir/out_of_bounds.tvm" >/dev/null
set +e
"$tiny_run" --policy "$policy" "$tmpdir/out_of_bounds.tvm" > "$tmpdir/out_of_bounds.output"
status=$?
set -e
test "$status" -ne 0
tail -n 1 "$tmpdir/out_of_bounds.output" | jq -e '.status == "faulted" and .trap == 7' >/dev/null

echo 'TinyVM memory ABI parity: PASS (bounded storage, exact imports, and bounds fault)'
