#!/bin/sh
set -eu

root=${FLOWCORE_ROOT:?}
flowmini=${FLOWMINI_BIN:?}
analyst=${FLOWANALYST_BIN:?}
parallel=${FLOWPARALLEL_BIN:?}
optimize=${FLOWOPTIMIZE_BIN:?}
prepare=${FLOWPREPARE_BIN:?}
bind=${FLOWBIND_BIN:?}
tiny_lower=${FLOWTINYLOWER_BIN:?}
tiny_validate=${FLOWTINYVALIDATE_BIN:?}
tiny_run=${FLOWTINYRUN_BIN:?}
llvm_lower=${FLOWLOWER_BIN:?}
text_runtime=${FLOWTEXT_RUNTIME:?}

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
policy=$tmpdir/policy
printf '%s\n' \
  'allow libflowtext.so flow_text_concat c memory Text,Text Text' \
  'allow libc.so.6 puts c io Text c_int' > "$policy"
export LD_LIBRARY_PATH=$(dirname "$text_runtime")${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}

source=$root/Lyraform/compiler/examples/text/text_runtime.flow
"$flowmini" --dump-frontend-bundle "$source" > "$tmpdir/frontend.json"
"$analyst" --lowering-plan-version 2 < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
"$parallel" < "$tmpdir/semantic.json" > "$tmpdir/parallel.json"
"$optimize" < "$tmpdir/parallel.json" > "$tmpdir/optimized.json"
"$bind" --policy "$policy" < "$tmpdir/semantic.json" > "$tmpdir/binding.json"
"$prepare" --binding-report "$tmpdir/binding.json" "$tmpdir/optimized.json" > "$tmpdir/lowering.json"

"$llvm_lower" --emit-llvm "$tmpdir/text.ll" "$tmpdir/lowering.json" >/dev/null
clang ${FLOWTEXT_CXX_FLAGS:-} "$tmpdir/text.ll" "$text_runtime" -o "$tmpdir/text.llvm"
if ! "$tiny_lower" "$tmpdir/lowering.json" "$tmpdir/text.tvm" >"$tmpdir/tiny-lower.json" 2>"$tmpdir/tiny-lower.err"; then
    cat "$tmpdir/tiny-lower.json" >&2
    cat "$tmpdir/tiny-lower.err" >&2
    exit 1
fi
"$tiny_validate" "$tmpdir/text.tvm" | grep -q '"status":"valid"'
"$tmpdir/text.llvm" > "$tmpdir/llvm.stdout"
"$tiny_run" --policy "$policy" "$tmpdir/text.tvm" > "$tmpdir/tiny.stdout"
sed '$d' "$tmpdir/tiny.stdout" > "$tmpdir/tiny.output"
cmp -s "$tmpdir/llvm.stdout" "$tmpdir/tiny.output"
test "$(tail -n 1 "$tmpdir/tiny.stdout" | jq -r '.result')" -eq 0
"$tiny_run" --engine computed --policy "$policy" "$tmpdir/text.tvm" > "$tmpdir/tiny.computed.stdout"
sed '$d' "$tmpdir/tiny.computed.stdout" > "$tmpdir/tiny.computed.output"
cmp -s "$tmpdir/llvm.stdout" "$tmpdir/tiny.computed.output"
test "$(tail -n 1 "$tmpdir/tiny.computed.stdout" | jq -r '.result')" -eq 0

printf 'import "%s/Lyraform/compiler/std/abi/libc.flow"\nimport "%s/Lyraform/compiler/std/abi/text.flow"\n\nprogram text_runtime_overflow\n\nfn join(left : Text, right : Text): Text {\n    value : Text(left + right)\n    return value\n}\n\nmain {\n    value : Text(join("' "$root" "$root" > "$tmpdir/overflow.flow"
awk 'BEGIN { for (i = 0; i < 4097; ++i) printf "A" }' >> "$tmpdir/overflow.flow"
printf '%s\n' '", "x"))' '    print value' '}' >> "$tmpdir/overflow.flow"
"$flowmini" --dump-frontend-bundle "$tmpdir/overflow.flow" > "$tmpdir/overflow.frontend.json"
"$analyst" --lowering-plan-version 2 < "$tmpdir/overflow.frontend.json" > "$tmpdir/overflow.semantic.json"
"$parallel" < "$tmpdir/overflow.semantic.json" > "$tmpdir/overflow.parallel.json"
"$optimize" < "$tmpdir/overflow.parallel.json" > "$tmpdir/overflow.optimized.json"
"$bind" --policy "$policy" < "$tmpdir/overflow.semantic.json" > "$tmpdir/overflow.binding.json"
"$prepare" --binding-report "$tmpdir/overflow.binding.json" "$tmpdir/overflow.optimized.json" > "$tmpdir/overflow.lowering.json"
"$tiny_lower" "$tmpdir/overflow.lowering.json" "$tmpdir/overflow.tvm" >/dev/null
set +e
"$tiny_run" --policy "$policy" "$tmpdir/overflow.tvm" > "$tmpdir/overflow.execution.json"
overflow_status=$?
set -e
test "$overflow_status" -ne 0
jq -e '.status == "faulted" and .trap == 7 and .outcome.type == "Outcome" and .outcome.failure_type == "TextFailure" and .outcome.failure_code == "exhausted"' "$tmpdir/overflow.execution.json" >/dev/null

echo 'Runtime Text LLVM/TinyVM parity: PASS (owned bounded concat and governed output)'
