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
source=$root/Lyraform/compiler/examples/pass/text_value.flow
policy=$tmpdir/policy
printf '%s\n' 'allow libc.so.6 puts c io Text c_int' > "$policy"

"$flowmini" --dump-frontend-bundle "$source" > "$tmpdir/frontend.json"
"$analyst" --lowering-plan-version 2 < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
"$parallel" < "$tmpdir/semantic.json" > "$tmpdir/execution.json"
"$optimize" < "$tmpdir/execution.json" > "$tmpdir/optimization.json"
"$bind" --policy "$policy" < "$tmpdir/semantic.json" > "$tmpdir/binding.json"
# Both backends consume the independently captured prepared artifact.
"$prepare" --binding-report "$tmpdir/binding.json" "$tmpdir/optimization.json" > "$tmpdir/lowering.json"
"$llvm_lower" --emit-llvm "$tmpdir/text.ll" "$tmpdir/lowering.json" >/dev/null
clang "$tmpdir/text.ll" -o "$tmpdir/text.llvm"
"$tiny_lower" "$tmpdir/lowering.json" "$tmpdir/text.tvm" >/dev/null
"$tiny_validate" "$tmpdir/text.tvm" | grep -q '"status":"valid"'

"$tmpdir/text.llvm" > "$tmpdir/llvm.stdout"
"$tiny_run" --policy "$policy" "$tmpdir/text.tvm" > "$tmpdir/tiny.stdout"
sed '$d' "$tmpdir/tiny.stdout" > "$tmpdir/tiny.output"
cmp -s "$tmpdir/llvm.stdout" "$tmpdir/tiny.output"
test "$(tail -n 1 "$tmpdir/tiny.stdout" | jq -r '.result')" -eq 0

echo 'Text compile-time boundary LLVM/TinyVM parity: PASS'
