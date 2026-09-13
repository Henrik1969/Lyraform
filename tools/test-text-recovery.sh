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
text_runtime=${FLOWTEXT_RUNTIME:?}

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
policy=$tmpdir/policy
printf '%s\n' \
  'allow libflowtext.so flow_text_concat_status c memory Text,Text c_int' \
  'allow libflowtext.so flow_text_concat c memory Text,Text Text' \
  'allow libc.so.6 puts c io Text c_int' \
  'allow libc.so.6 puts c io c_string c_int' > "$policy"
export LD_LIBRARY_PATH=$(dirname "$text_runtime")${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}

run_case() {
    source=$1
    expected=$2
    name=$3
    "$flowmini" --dump-frontend-bundle "$source" > "$tmpdir/$name.frontend.json"
    "$analyst" --lowering-plan-version 2 < "$tmpdir/$name.frontend.json" > "$tmpdir/$name.semantic.json"
    jq -e '
      any(.lowering_plan.operations[]; .kind == "branch") and
      any(.lowering_plan.operations[]; .kind == "external_call" and .provider.symbol == "flow_text_concat_status" and .provider.return_type == "c_int") and
      any(.lowering_plan.operations[]; .kind == "text_outcome" and .provider.symbol == "flow_text_concat")
    ' "$tmpdir/$name.semantic.json" >/dev/null
    "$parallel" < "$tmpdir/$name.semantic.json" > "$tmpdir/$name.parallel.json"
    "$optimize" < "$tmpdir/$name.parallel.json" > "$tmpdir/$name.optimized.json"
    "$bind" --policy "$policy" < "$tmpdir/$name.semantic.json" > "$tmpdir/$name.binding.json"
    "$prepare" --binding-report "$tmpdir/$name.binding.json" "$tmpdir/$name.optimized.json" > "$tmpdir/$name.lowering.json"
    "$llvm_lower" --emit-llvm "$tmpdir/$name.ll" --binding-report "$tmpdir/$name.binding.json" < "$tmpdir/$name.optimized.json" >/dev/null
    clang "$tmpdir/$name.ll" "$text_runtime" -o "$tmpdir/$name.llvm"
    "$tiny_lower" "$tmpdir/$name.lowering.json" "$tmpdir/$name.tvm" >/dev/null
    "$tiny_validate" "$tmpdir/$name.tvm" | grep -q '"status":"valid"'
    "$tmpdir/$name.llvm" > "$tmpdir/$name.llvm.out"
    "$tiny_run" --policy "$policy" "$tmpdir/$name.tvm" > "$tmpdir/$name.tiny.out"
    sed '$d' "$tmpdir/$name.tiny.out" > "$tmpdir/$name.tiny.program.out"
    printf '%s\n' "$expected" > "$tmpdir/$name.expected"
    cmp -s "$tmpdir/$name.expected" "$tmpdir/$name.llvm.out"
    cmp -s "$tmpdir/$name.expected" "$tmpdir/$name.tiny.program.out"
}

run_case "$root/Lyraform/compiler/examples/text/text_recovery.flow" 'Lyraform' success

printf 'import "%s/Lyraform/compiler/std/abi/libc.flow"\nimport "%s/Lyraform/compiler/std/abi/text.flow"\n\nprogram text_recovery_overflow\n\nfn status(left : Text, right : Text): c_int {\n    code : c_int(concat_status(left, right))\n    code -> return\n}\n\nfn join(left : Text, right : Text): Text {\n    value : Text(left + right)\n    return value\n}\n\nmain {\n    zero : c_int(0)\n    code : c_int(status("' "$root" "$root" > "$tmpdir/overflow.flow"
awk 'BEGIN { for (i = 0; i < 4097; ++i) printf "A" }' >> "$tmpdir/overflow.flow"
printf '%s\n' '", "x"))' '    if code == zero {' '        value : Text(join("Lyra", "form"))' '        print value' '    }' '    if code != zero {' '        puts("concat failed") -> code' '    }' '}' >> "$tmpdir/overflow.flow"
run_case "$tmpdir/overflow.flow" 'concat failed' overflow

echo 'Text recovery boundary: PASS (Flow status carrier, explicit branch, LLVM/TinyVM parity)'
