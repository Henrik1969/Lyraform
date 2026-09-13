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
printf '%s\n' 'allow libc.so.6 strnlen c pure c_string,c_size_t c_size_t' 'allow libc.so.6 puts c io c_string c_int' > "$policy"

fixture=$root/Lyraform/compiler/examples/pass/abi_strnlen_main.flow
"$flowmini" --dump-frontend-bundle "$fixture" > "$tmpdir/frontend.json"
"$analyst" --lowering-plan-version 2 < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
jq -e '
  .status == "ok" and
  any(.binding_requirements[]; .symbol == "strnlen" and .parameter_types == "c_string,c_size_t" and .return_type == "c_size_t") and
  any(.lowering_plan.operations[]; .kind == "external_call" and .provider.symbol == "strnlen")
' "$tmpdir/semantic.json" >/dev/null
"$parallel" < "$tmpdir/semantic.json" > "$tmpdir/parallel.json"
"$optimize" < "$tmpdir/parallel.json" > "$tmpdir/optimized.json"
"$bind" --policy "$policy" < "$tmpdir/semantic.json" > "$tmpdir/binding.json"
"$prepare" --binding-report "$tmpdir/binding.json" "$tmpdir/optimized.json" > "$tmpdir/lowering.json"
"$llvm_lower" --emit-llvm "$tmpdir/strnlen.ll" --binding-report "$tmpdir/binding.json" < "$tmpdir/optimized.json" >/dev/null
grep -q 'call i64 @strnlen' "$tmpdir/strnlen.ll"
clang "$tmpdir/strnlen.ll" -o "$tmpdir/strnlen"
"$tmpdir/strnlen" > "$tmpdir/llvm.out"
"$tiny_lower" "$tmpdir/lowering.json" "$tmpdir/strnlen.tvm" >/dev/null
"$tiny_validate" "$tmpdir/strnlen.tvm" | grep -q '"status":"valid"'
"$tiny_run" --policy "$policy" "$tmpdir/strnlen.tvm" > "$tmpdir/tiny.out"
sed '$d' "$tmpdir/tiny.out" > "$tmpdir/tiny.program.out"
printf '%s\n' 'strnlen ok' > "$tmpdir/expected"
cmp -s "$tmpdir/expected" "$tmpdir/llvm.out"
cmp -s "$tmpdir/expected" "$tmpdir/tiny.program.out"

echo 'strnlen Flow boundary: PASS (bounded c_string length parity on LLVM/TinyVM)'
