#!/bin/sh
set -eu

root=${FLOWCORE_ROOT:?}
flowmini=${FLOWMINI_BIN:?}
analyst=${FLOWANALYST_BIN:?}
parallel=${FLOWPARALLEL_BIN:?}
optimize=${FLOWOPTIMIZE_BIN:?}
prepare=${FLOWPREPARE_BIN:?}
validate=${FLOWVALIDATE_BIN:?}
lower=${FLOWLOWER_BIN:?}
tiny_lower=${FLOWTINYLOWER_BIN:?}
tiny_validate=${FLOWTINYVALIDATE_BIN:?}
tiny_run=${FLOWTINYRUN_BIN:?}
source="$root/Lyraform/compiler/examples/pass/fn_canonical_demo.flow"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

"$flowmini" --dump-frontend-bundle "$source" > "$tmpdir/frontend.json"
"$analyst" --lowering-plan-version 2 < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
"$parallel" < "$tmpdir/semantic.json" > "$tmpdir/execution.json"
"$optimize" < "$tmpdir/execution.json" > "$tmpdir/optimization.json"
"$prepare" "$tmpdir/optimization.json" > "$tmpdir/lowering.json"
"$validate" "$tmpdir/lowering.json" | grep -q '"classification":"valid"'

jq -e '.lowering_plan.version == 2 and
       ([.lowering_plan.functions[] | select(.entry)] | length) == 1 and
       ([.lowering_plan.functions[] | select(.name == "square") | .parameters[0].symbol_id] == [3]) and
       ([.lowering_plan.operations[] | select(.kind == "call") | .callee_symbol_id] | sort == [2,4]) and
       ([.lowering_plan.operations[] | select(.kind == "return_value") | .operands[0] | select(.kind == "call_result") | .callee_symbol_id] == [2])' \
    "$tmpdir/lowering.json" >/dev/null

for mutation in \
    '.lowering_plan.functions[0].symbol_id = .lowering_plan.functions[1].symbol_id' \
    '.lowering_plan.operations[0].function_symbol_id = 999' \
    '.lowering_plan.operations[0].callee_symbol_id = 999'
do
    jq "$mutation" "$tmpdir/lowering.json" > "$tmpdir/mutated.json"
    if "$validate" "$tmpdir/mutated.json" >/dev/null 2>&1; then
        echo "flowvalidate accepted callable lowering mutation: $mutation" >&2; exit 1
    fi
done

classifier="$root/Lyraform/compiler/examples/bootstrap/shared_scalar_classifier.flow"
"$flowmini" --dump-frontend-bundle "$classifier" > "$tmpdir/classifier.frontend.json"
"$analyst" --lowering-plan-version 2 < "$tmpdir/classifier.frontend.json" > "$tmpdir/classifier.semantic.json"
"$parallel" < "$tmpdir/classifier.semantic.json" > "$tmpdir/classifier.execution.json"
"$optimize" < "$tmpdir/classifier.execution.json" > "$tmpdir/classifier.optimization.json"
"$prepare" --target-policy "$root/Flowlower/target-policies/llvm-host.json" "$tmpdir/classifier.optimization.json" > "$tmpdir/classifier.lowering.json"
"$lower" --emit-llvm "$tmpdir/classifier.ll" "$tmpdir/classifier.lowering.json" > "$tmpdir/classifier.report.json"
clang "$tmpdir/classifier.ll" -o "$tmpdir/classifier"
set +e
"$tmpdir/classifier"
classifier_status=$?
set -e
test "$classifier_status" -eq 1
jq -e '.status == "ready" and .backend.name == "llvm"' "$tmpdir/classifier.report.json" >/dev/null

"$prepare" --target-policy "$root/Flowlower/target-policies/tinyvm-portable.json" "$tmpdir/classifier.optimization.json" > "$tmpdir/classifier.tiny.lowering.json"
"$tiny_lower" "$tmpdir/classifier.tiny.lowering.json" "$tmpdir/classifier.tvm" > "$tmpdir/classifier.tiny.report.json"
"$tiny_lower" "$tmpdir/classifier.tiny.lowering.json" "$tmpdir/classifier-again.tvm" >/dev/null
cmp -s "$tmpdir/classifier.tvm" "$tmpdir/classifier-again.tvm"
"$tiny_validate" "$tmpdir/classifier.tvm" | grep -q '"status":"valid"'
tiny_result=$("$tiny_run" "$tmpdir/classifier.tvm" | jq -r .result)
test "$tiny_result" -eq "$classifier_status"
tiny_computed_result=$("$tiny_run" --engine computed "$tmpdir/classifier.tvm" | jq -r .result)
test "$tiny_computed_result" -eq "$tiny_result"
jq -e '.status == "emitted" and .backend == "tinyvm"' "$tmpdir/classifier.tiny.report.json" >/dev/null

# Receiver prerequisites: typed ordinary results and fresh invocation locals.
cat > "$tmpdir/scalars.flow" <<'FLOW'
program arbitrary_typed_results
abi libc {
    library "libc.so.6"
    convention c
    extern fn length(value : c_string): c_size_t {
        symbol "strlen"
        effect pure
    }
}
fn wide(value : c_long): c_long {
    if value > 0 { return value }
    return 4294967297
}
fn truth(value : int): Bool {
    if value == 3 { return true } else { return false }
}
fn label(value : c_string): c_string { return value }
fn fresh(value : int): int {
    local : int(0)
    local + value -> local
    return local
}
main {
    large : c_long(4294967297)
    roundtrip : c_long(0)
    wide(large) -> roundtrip
    zero : c_long(0)
    wide(zero) -> roundtrip
    decision : Bool(false)
    truth(3) -> decision
    text : c_string("fresh frame")
    output : c_string("")
    label(text) -> output
    size : c_size_t(0)
    expected : c_size_t(11)
    libc.length(output) -> size
    first : int(0)
    second : int(0)
    fresh(3) -> first
    fresh(4) -> second
    if decision {
        if roundtrip == large {
            if size == expected {
                return first + second + 35
            }
        }
    }
    return 7
}
FLOW
"$flowmini" --dump-frontend-bundle "$tmpdir/scalars.flow" | "$analyst" --lowering-plan-version 2 > "$tmpdir/scalars.semantic.json"
"$parallel" < "$tmpdir/scalars.semantic.json" | "$optimize" > "$tmpdir/scalars.optimization.json"
printf '%s\n' 'allow libc.so.6 strlen c pure c_string c_size_t' > "$tmpdir/scalars.policy"
"$FLOWBIND_BIN" --policy "$tmpdir/scalars.policy" < "$tmpdir/scalars.semantic.json" > "$tmpdir/scalars.binding.json"
"$prepare" --binding-report "$tmpdir/scalars.binding.json" "$tmpdir/scalars.optimization.json" > "$tmpdir/scalars.lowering.json"
"$lower" --emit-llvm "$tmpdir/scalars.ll" "$tmpdir/scalars.lowering.json" > "$tmpdir/scalars.report.json"
clang "$tmpdir/scalars.ll" -o "$tmpdir/scalars"
set +e
"$tmpdir/scalars"
scalar_status=$?
set -e
test "$scalar_status" -eq 42
"$tiny_lower" "$tmpdir/scalars.lowering.json" "$tmpdir/scalars.tvm" > "$tmpdir/scalars.tiny.report.json"
"$tiny_validate" "$tmpdir/scalars.tvm" >/dev/null
for engine in switch computed; do
    test "$("$tiny_run" --policy "$tmpdir/scalars.policy" --engine "$engine" "$tmpdir/scalars.tvm" | jq -r .result)" -eq 42
done
# A receiver must not manufacture a result for a missing return path.
sed 's/if value == 3 { return true } else { return false }/if value == 3 { return true }/' "$tmpdir/scalars.flow" > "$tmpdir/missing-result.flow"
"$flowmini" --dump-frontend-bundle "$tmpdir/missing-result.flow" | "$analyst" --lowering-plan-version 2 > "$tmpdir/missing-result.semantic.json"
"$parallel" < "$tmpdir/missing-result.semantic.json" | "$optimize" > "$tmpdir/missing-result.optimization.json"
"$prepare" --binding-report "$tmpdir/scalars.binding.json" "$tmpdir/missing-result.optimization.json" > "$tmpdir/missing-result.lowering.json"
if "$lower" --emit-llvm "$tmpdir/missing-result.ll" "$tmpdir/missing-result.lowering.json" > "$tmpdir/missing-result.report.json" 2>/dev/null; then
    echo 'missing callable result accepted by LLVM' >&2; exit 1
fi
if "$tiny_lower" "$tmpdir/missing-result.lowering.json" "$tmpdir/missing-result.tvm" > "$tmpdir/missing-result.tiny.report.json" 2>/dev/null; then
    echo 'missing callable result accepted by TinyVM' >&2; exit 1
fi

jq '(.lowering_plan.functions[] | select(.name == "truth")).return_type = "c_long"' "$tmpdir/scalars.lowering.json" > "$tmpdir/wrong-result.lowering.json"
if "$lower" --emit-llvm "$tmpdir/wrong-result.ll" "$tmpdir/wrong-result.lowering.json" > "$tmpdir/wrong-result.report.json" 2>/dev/null; then
    echo 'mismatched callable result accepted by LLVM' >&2; exit 1
fi
if "$tiny_lower" "$tmpdir/wrong-result.lowering.json" "$tmpdir/wrong-result.tvm" > "$tmpdir/wrong-result.tiny.report.json" 2>/dev/null; then
    echo 'mismatched callable result accepted by TinyVM' >&2; exit 1
fi

echo 'callable lowering boundary: PASS'
