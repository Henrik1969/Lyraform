#!/bin/sh
set -eu
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
cat > "$tmpdir/identity.flow" <<'FLOW'
program separate_identity
abi first {
    library "libc.so.6"
    convention c
    extern fn renamed(value : c_int): c_int {
        symbol "abs"
        effect pure
    }
}
abi second {
    library "libc.so.6"
    convention c
    extern fn renamed(value : c_int): c_int {
        symbol "abs"
        effect readonly
    }
}
fn abs(value : int): int {
    return value + 1
}
main {
    input : c_int(-40)
    one : c_int(0)
    two : c_int(0)
    first.renamed(input) -> one
    second.renamed(input) -> two
    return abs(one) + two
}
FLOW
"$FLOWMINI_BIN" --dump-frontend-bundle "$tmpdir/identity.flow" > "$tmpdir/frontend.json"
"$FLOWANALYST_BIN" --lowering-plan-version 2 < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
jq -e '[.binding_requirements[] | [.contract, .symbol, .effect]] == [["first", "abs", "pure"], ["second", "abs", "readonly"]] and
    [.lowering_plan.operations[] | select(.kind == "external_call") | [.callee, .provider.contract, .provider.symbol, .provider.effect]] ==
        [["first.renamed", "first", "abs", "pure"], ["second.renamed", "second", "abs", "readonly"]] and
    any(.lowering_plan.operations[]; .kind == "call" and .callee == "abs" and (has("provider") | not)) and
    (. as $report | all(.external_operations[];
      . as $projection | any($report.lowering_plan.operations[];
        .id == $projection.operation_id and .source_call_projection == true and
        .expression_id == $projection.expression_id and .statement_id == $projection.statement_id and
        .callee == $projection.callee and .callee_symbol_id == $projection.callee_symbol_id and
        .arguments == $projection.arguments)))' "$tmpdir/semantic.json" >/dev/null

reject_call_projection() {
    mutation=$1
    jq "$mutation" "$tmpdir/semantic.json" > "$tmpdir/hostile.semantic.json"
    if "$FLOWPARALLEL_BIN" < "$tmpdir/hostile.semantic.json" >/dev/null 2>&1; then
        echo "call projection mutation was accepted: $mutation" >&2
        exit 1
    fi
}
reject_call_projection 'del(.lowering_plan.operations[0].source_call_projection)'
reject_call_projection '.external_operations[0].operation_id = 999'
reject_call_projection '.external_operations[0].callee = "drifted"'
reject_call_projection '.external_operations += [.external_operations[0]]'
reject_call_projection 'del(.external_operations[0])'
reject_call_projection '.external_operations[0].purity = "pure"'
reject_call_projection '(.effect_facts[] | select(.effect == "pure")) |= (.effect = "unknown" | .certainty = "unresolved")'
reject_call_projection '(.lowering_plan.operations[] | select(.kind == "return_value") | .source_call_projection) = true'
printf '%s\n' 'allow libc.so.6 abs c pure c_int c_int' 'allow libc.so.6 abs c readonly c_int c_int' > "$tmpdir/policy.conf"
"$FLOWBIND_BIN" --policy "$tmpdir/policy.conf" < "$tmpdir/semantic.json" > "$tmpdir/binding.json"
"$FLOWPARALLEL_BIN" < "$tmpdir/semantic.json" > "$tmpdir/execution.json"
"$FLOWOPTIMIZE_BIN" < "$tmpdir/execution.json" > "$tmpdir/optimization.json"
"$FLOWLOWER_BIN" --binding-report "$tmpdir/binding.json" --emit-llvm "$tmpdir/program.ll" < "$tmpdir/optimization.json" > "$tmpdir/report.json"
test "$(grep -c '^declare i32 @abs(' "$tmpdir/program.ll")" -eq 1
grep -Eq '^define i32 @flow.function.[0-9]+\(' "$tmpdir/program.ll"
! grep -q '^define i32 @abs(' "$tmpdir/program.ll"
clang "$tmpdir/program.ll" -o "$tmpdir/program"
set +e
"$tmpdir/program"
status=$?
set -e
test "$status" -eq 81
# Omitting the second contract's effect grant must not authorize it via the first.
head -1 "$tmpdir/policy.conf" > "$tmpdir/one-policy.conf"
if "$FLOWBIND_BIN" --policy "$tmpdir/one-policy.conf" < "$tmpdir/semantic.json" > "$tmpdir/denied.json"; then
    echo 'second provider was authorized through the first' >&2; exit 1
fi
jq -e '.status == "blocked"' "$tmpdir/denied.json" >/dev/null
# Ordinary source names do not discover unused external capabilities.
python3 - "$tmpdir/identity.flow" "$tmpdir/local.flow" <<'PYTHON'
import sys
s=open(sys.argv[1]).read().replace('extern fn renamed(', 'extern fn abs(')
s=s[:s.index('main {')] + 'main { return abs(41) }\n'
open(sys.argv[2], 'w').write(s)
PYTHON
"$FLOWMINI_BIN" --dump-frontend-bundle "$tmpdir/local.flow" > "$tmpdir/local.frontend.json"
"$FLOWANALYST_BIN" --lowering-plan-version 2 < "$tmpdir/local.frontend.json" > "$tmpdir/local.semantic.json"
jq -e '.binding_requirements == [] and all(.lowering_plan.operations[]; .kind != "external_call")' "$tmpdir/local.semantic.json" >/dev/null
# Conflicting library identities cannot collapse into one LLVM declaration.
jq '(.lowering_plan.operations[] | select(.kind == "external_call" and .provider.contract == "second") | .provider.library) = "libm.so.6"' "$tmpdir/optimization.json" > "$tmpdir/conflict.json"
jq '(.capabilities[] | select(.contract == "second") | .library) = "libm.so.6"' "$tmpdir/binding.json" > "$tmpdir/conflict.binding.json"
if "$FLOWLOWER_BIN" --binding-report "$tmpdir/conflict.binding.json" --emit-llvm "$tmpdir/conflict.ll" < "$tmpdir/conflict.json" > "$tmpdir/conflict.report" 2> "$tmpdir/conflict.log"; then
    echo 'different native providers collapsed by symbol name' >&2; exit 1
fi
grep -Fq 'native symbol has conflicting provider libraries or ABI declarations: abs' "$tmpdir/conflict.log"
test ! -e "$tmpdir/conflict.ll"
echo 'Exact provider and source identity: PASS'
