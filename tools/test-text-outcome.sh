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
validator=${FLOWVALIDATE_BIN:?}
text_runtime=${FLOWTEXT_RUNTIME:?}

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
policy=$tmpdir/policy
printf '%s\n' \
  'allow libflowtext.so flow_text_concat_value c memory Text,Text TextOutcome' \
  'allow libflowtext.so flow_text_dispose c memory Text c_int' \
  'allow libc.so.6 puts c io Text c_int' \
  'allow libc.so.6 puts c io c_string c_int' > "$policy"
export LD_LIBRARY_PATH=$(dirname "$text_runtime")${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}

run_case() {
    source=$1
    expected=$2
    name=$3
    "$flowmini" --dump-frontend-bundle "$source" > "$tmpdir/$name.frontend.json"
    jq -e 'any(.symbol_table.symbols[]; .name == "TextOutcome") and any(.symbol_table.symbols[]; .name == "TextFailure")' "$tmpdir/$name.frontend.json" >/dev/null
    "$analyst" --lowering-plan-version 2 < "$tmpdir/$name.frontend.json" > "$tmpdir/$name.semantic.json"
    jq -e '
      any(.lowering_plan.operations[]; .kind == "text_outcome" and .provider.symbol == "flow_text_concat_value" and .provider.return_type == "TextOutcome" and .result_outcome.type == "Outcome") and
      any(.lowering_plan.disposition_facts[];
        .completion=="exactly_one" and
        [.possible_dispositions[].kind]==["success","failure"] and
        .possible_dispositions[0].payload_type=="Text" and
        .possible_dispositions[1].payload_type=="TextFailure" and
        .obligation.kind=="must_account" and .obligation.status=="accounted" and
        .obligation.proof=="complementary_zero_code_branches" and
        (.obligation.success_value_operation_ids|length)>=2 and
        (.obligation.failure_recovery_operation_ids|length)>=1) and
      any(.lowering_plan.operations[]; .kind == "external_call" and .provider.symbol == "flow_text_dispose") and
      any(.lowering_plan.operations[]; .kind == "branch")
    ' "$tmpdir/$name.semantic.json" >/dev/null
    producer_line=$(awk '/concat_outcome/ {print NR; exit}' "$source")
    jq -e --argjson line "$producer_line" '
        all(.lowering_plan.disposition_facts[]; .provenance.line==$line and .provenance.column==5)' \
        "$tmpdir/$name.semantic.json" >/dev/null
    "$parallel" < "$tmpdir/$name.semantic.json" > "$tmpdir/$name.parallel.json"
    "$optimize" < "$tmpdir/$name.parallel.json" > "$tmpdir/$name.optimized.json"
    LD_LIBRARY_PATH="$LD_LIBRARY_PATH" "$bind" --policy "$policy" < "$tmpdir/$name.semantic.json" > "$tmpdir/$name.binding.json"
    "$prepare" --binding-report "$tmpdir/$name.binding.json" "$tmpdir/$name.optimized.json" > "$tmpdir/$name.lowering.json"
    jq -S '.lowering_plan.disposition_facts' "$tmpdir/$name.semantic.json" > "$tmpdir/$name.dispositions.json"
    for stage in parallel optimized lowering; do
        jq -S '.lowering_plan.disposition_facts' "$tmpdir/$name.$stage.json" > "$tmpdir/$name.$stage.dispositions.json"
        cmp -s "$tmpdir/$name.dispositions.json" "$tmpdir/$name.$stage.dispositions.json"
        "$validator" "$tmpdir/$name.$stage.json" >/dev/null
    done
    "$llvm_lower" --emit-llvm "$tmpdir/$name.ll" --binding-report "$tmpdir/$name.binding.json" < "$tmpdir/$name.optimized.json" >/dev/null
    test "$(grep -c 'call i32 @flow_text_dispose' "$tmpdir/$name.ll")" -eq 1
    clang ${FLOWTEXT_CXX_FLAGS:-} "$tmpdir/$name.ll" "$text_runtime" -o "$tmpdir/$name.llvm"
    "$tiny_lower" "$tmpdir/$name.lowering.json" "$tmpdir/$name.tvm" >/dev/null
    "$tiny_validate" "$tmpdir/$name.tvm" | grep -q '"status":"valid"'
    "$tmpdir/$name.llvm" > "$tmpdir/$name.llvm.out"
    "$tiny_run" --policy "$policy" "$tmpdir/$name.tvm" > "$tmpdir/$name.tiny.out"
    sed '$d' "$tmpdir/$name.tiny.out" > "$tmpdir/$name.tiny.program.out"
    printf '%s\n' "$expected" > "$tmpdir/$name.expected"
    cmp -s "$tmpdir/$name.expected" "$tmpdir/$name.llvm.out"
    cmp -s "$tmpdir/$name.expected" "$tmpdir/$name.tiny.program.out"
}

run_case "$root/Lyraform/compiler/examples/text/text_outcome.flow" 'Lyraform' success
run_case "$root/Lyraform/compiler/examples/text/text_outcome_return.flow" 'Lyraform' returned

sed -e "s|../../std/abi/libc.flow|$root/Lyraform/compiler/std/abi/libc.flow|" \
    -e "s|../../std/abi/text.flow|$root/Lyraform/compiler/std/abi/text.flow|" \
    "$root/Lyraform/compiler/examples/text/text_outcome.flow" > "$tmpdir/base.flow"

refuse_case() {
    source=$1
    name=$2
    "$flowmini" --dump-frontend-bundle "$source" > "$tmpdir/$name.frontend.json"
    set +e
    "$analyst" --lowering-plan-version 2 "$tmpdir/$name.frontend.json" > "$tmpdir/$name.semantic.json"
    status=$?
    set -e
    test "$status" -eq 2
    producer_line=$(awk '/concat_outcome/ {print NR; exit}' "$source")
    jq -e --arg suffix "/$name.flow" --argjson line "$producer_line" '
        any(.diagnostics[]; .code=="FLOWANALYST_DANGLING_OUTCOME_WIRE" and
            (.provenance.source|endswith($suffix)) and .provenance.line==$line and .provenance.column==5)' \
        "$tmpdir/$name.semantic.json" >/dev/null
    jq -e '.status=="error" and .lowering_plan.status=="blocked" and
        (.lowering_plan.disposition_facts|length)==0 and
        any(.diagnostics[];
            .code=="FLOWANALYST_DANGLING_OUTCOME_WIRE" and
            (.message|contains("inspect both variants")) and
            (.message|contains("only direct unique return transfer is supported; further propagation and explicit policy sinks remain unsupported")) and
            .root_cause==true and .provenance.line>0)' "$tmpdir/$name.semantic.json" >/dev/null
}

awk '/    zero :/{print "}"; exit} {print}' "$tmpdir/base.flow" > "$tmpdir/unhandled.flow"
refuse_case "$tmpdir/unhandled.flow" unhandled

awk '/    if code != zero/{skip=1;next} skip&&/^    }/{skip=0;next} !skip' "$tmpdir/base.flow" > "$tmpdir/missing-failure.flow"
refuse_case "$tmpdir/missing-failure.flow" missing-failure

sed 's/if code != zero/if code == zero/' "$tmpdir/base.flow" > "$tmpdir/non-complementary.flow"
refuse_case "$tmpdir/non-complementary.flow" non-complementary

sed '/puts("concat failed") -> status/d' "$tmpdir/base.flow" > "$tmpdir/empty-failure.flow"
refuse_case "$tmpdir/empty-failure.flow" empty-failure

sed 's/puts("concat failed")/puts_text(outcome.value)/' "$tmpdir/base.flow" > "$tmpdir/failure-value.flow"
refuse_case "$tmpdir/failure-value.flow" failure-value

sed '/dispose(outcome.value) -> status/d' "$tmpdir/base.flow" > "$tmpdir/missing-dispose.flow"
refuse_case "$tmpdir/missing-dispose.flow" missing-dispose

awk '{print} /dispose\(outcome.value\) -> status/{print}' "$tmpdir/base.flow" > "$tmpdir/double-dispose.flow"
refuse_case "$tmpdir/double-dispose.flow" double-dispose

awk '{print} /outcome : TextOutcome/{print "    copy : TextOutcome(outcome)"}' "$tmpdir/base.flow" > "$tmpdir/copied.flow"
refuse_case "$tmpdir/copied.flow" copied

# Previously accepted counterexamples: execution order, mutable guards, exits.
awk '/puts_text\(outcome.value\)/ {saved=$0; next} /dispose\(outcome.value\)/ {print; print saved; next} {print}' "$tmpdir/base.flow" > "$tmpdir/use-after-dispose.flow"
refuse_case "$tmpdir/use-after-dispose.flow" use-after-dispose
for variant in overwrite-code overwrite-zero early-return; do
    case $variant in
        overwrite-code) statement='0 -> code' ;;
        overwrite-zero) statement='1 -> zero' ;;
        early-return) statement='0 -> return' ;;
    esac
    awk -v statement="$statement" '/    if code == zero/ {print "    " statement} {print}' "$tmpdir/base.flow" > "$tmpdir/$variant.flow"
    refuse_case "$tmpdir/$variant.flow" "$variant"
done
awk '/dispose\(outcome.value\)/ {print "        0 -> return"} {print}' "$tmpdir/base.flow" > "$tmpdir/return-before-dispose.flow"
refuse_case "$tmpdir/return-before-dispose.flow" return-before-dispose
sed 's/puts("concat failed") -> status/0 -> return/' "$tmpdir/base.flow" > "$tmpdir/failure-return.flow"
refuse_case "$tmpdir/failure-return.flow" failure-return

sed 's/puts_text(outcome.value) -> status/puts_text(outcome.value) -> code/' "$tmpdir/base.flow" > "$tmpdir/branch-overwrite-code.flow"
refuse_case "$tmpdir/branch-overwrite-code.flow" branch-overwrite-code
sed 's/puts_text(outcome.value) -> status/puts_text(outcome.value) -> zero/' "$tmpdir/base.flow" > "$tmpdir/branch-overwrite-zero.flow"
refuse_case "$tmpdir/branch-overwrite-zero.flow" branch-overwrite-zero

# A copied success payload would hide later alias uses from cleanup accounting.
sed 's/puts_text(outcome.value) -> status/borrowed : Text(outcome.value)/' "$tmpdir/base.flow" > "$tmpdir/copied-value.flow"
refuse_case "$tmpdir/copied-value.flow" copied-value

# Equivalent supported shapes: literal zero and reversed sibling branches.
sed 's/code == zero/0 == code/; s/code != zero/0 != code/' "$tmpdir/base.flow" > "$tmpdir/literal-zero.flow"
run_case "$tmpdir/literal-zero.flow" 'Lyraform' literal-zero
python3 - "$tmpdir/base.flow" "$tmpdir/reversed.flow" <<'PYCASE'
from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
a=s.index('    if code == zero {')
b=s.index('    if code != zero {')
c=s.index('    }', b)+len('    }')
Path(sys.argv[2]).write_text(s[:a]+s[b:c]+'\n'+s[a:b]+s[c:])
PYCASE
run_case "$tmpdir/reversed.flow" 'Lyraform' reversed

# The same ownership law closes the producer obligation in the caller.
sed -e "s|../../std/abi/libc.flow|$root/Lyraform/compiler/std/abi/libc.flow|" \
    -e "s|../../std/abi/text.flow|$root/Lyraform/compiler/std/abi/text.flow|" \
    "$root/Lyraform/compiler/examples/text/text_outcome_return.flow" > "$tmpdir/return-base.flow"
jq -e '.lowering_plan.disposition_facts[0] as $fact |
    $fact.obligation.ownership_transfer as $transfer |
    $transfer.format=="lyraform.ownership_transfer" and $transfer.version==1 and
    $transfer.kind=="function_return" and $transfer.mode=="unique" and
    $transfer.obligation_identity==$fact.obligation.identity and
    $transfer.source_owner_symbol_id!=$transfer.destination_owner_symbol_id and
    $transfer.destination_owner_symbol_id==$fact.obligation.owner_symbol_id' \
    "$tmpdir/returned.semantic.json" >/dev/null
sed '/produced -> return/i\    copy : TextOutcome(produced)' "$tmpdir/return-base.flow" > "$tmpdir/return-copy.flow"
refuse_case "$tmpdir/return-copy.flow" return-copy
sed '/outcome : TextOutcome(make())/a\    another : TextOutcome(make())' "$tmpdir/return-base.flow" > "$tmpdir/return-fanout.flow"
refuse_case "$tmpdir/return-fanout.flow" return-fanout
sed '/outcome : TextOutcome(make())/a\    copy : TextOutcome(outcome)' "$tmpdir/return-base.flow" > "$tmpdir/return-caller-copy.flow"
refuse_case "$tmpdir/return-caller-copy.flow" return-caller-copy
sed 's/fn make(): TextOutcome/fn make(): Text/' "$tmpdir/return-base.flow" > "$tmpdir/return-type.flow"
refuse_case "$tmpdir/return-type.flow" return-type
awk '{print} /produced -> return/ {print}' "$tmpdir/return-base.flow" > "$tmpdir/return-duplicate.flow"
refuse_case "$tmpdir/return-duplicate.flow" return-duplicate
awk '/    zero :/ {print "}"; exit} {print}' "$tmpdir/return-base.flow" > "$tmpdir/return-unhandled.flow"
refuse_case "$tmpdir/return-unhandled.flow" return-unhandled
python3 - "$tmpdir/return-base.flow" "$tmpdir/return-failure.flow" <<'PYFAIL'
from pathlib import Path
import sys
Path(sys.argv[2]).write_text(Path(sys.argv[1]).read_text().replace('"Lyra", "form"', '"'+'A'*4097+'", "x"'))
PYFAIL
run_case "$tmpdir/return-failure.flow" 'concat failed' return-failure

# Every hostile consumer gets its own accepted artifact format. A valid
# control must succeed before a rejection can count as contract evidence.
"$validator" "$tmpdir/success.semantic.json" >/dev/null
"$llvm_lower" --emit-llvm "$tmpdir/control.ll" --binding-report "$tmpdir/success.binding.json" "$tmpdir/success.optimized.json" >/dev/null
"$tiny_lower" "$tmpdir/success.lowering.json" "$tmpdir/control.tvm" >/dev/null
hostile_count=0
hostile_baseline=success
reject_hostile_plan() {
    mutation=$1
    for stage in semantic parallel optimized lowering; do
        jq "$mutation" "$tmpdir/$hostile_baseline.$stage.json" > "$tmpdir/hostile.$stage.json"
    done
    if "$validator" "$tmpdir/hostile.semantic.json" >"$tmpdir/rejection" 2>&1; then exit 1; fi
    jq -e '(.path + " " + .reason) | test("disposition|outcome"; "i")' "$tmpdir/rejection" >/dev/null
    if "$llvm_lower" --emit-llvm "$tmpdir/forbidden.ll" --binding-report "$tmpdir/$hostile_baseline.binding.json" "$tmpdir/hostile.optimized.json" >"$tmpdir/rejection" 2>&1; then exit 1; fi
    grep -Eiq 'disposition|outcome' "$tmpdir/rejection"
    if "$tiny_lower" "$tmpdir/hostile.lowering.json" "$tmpdir/forbidden.tvm" >"$tmpdir/rejection" 2>&1; then exit 1; fi
    grep -Eiq 'disposition|outcome' "$tmpdir/rejection"
    if "$parallel" "$tmpdir/hostile.semantic.json" >"$tmpdir/rejection" 2>&1; then exit 1; fi
    grep -Eiq 'disposition|outcome' "$tmpdir/rejection"
    if "$optimize" "$tmpdir/hostile.parallel.json" >"$tmpdir/rejection" 2>&1; then exit 1; fi
    grep -Eiq 'disposition|outcome' "$tmpdir/rejection"
    if "$bind" --policy "$policy" "$tmpdir/hostile.semantic.json" >"$tmpdir/rejection" 2>&1; then exit 1; fi
    grep -Eiq 'disposition|outcome' "$tmpdir/rejection"
    if "$prepare" --binding-report "$tmpdir/$hostile_baseline.binding.json" "$tmpdir/hostile.optimized.json" >"$tmpdir/rejection" 2>&1; then exit 1; fi
    grep -Eiq 'disposition|outcome' "$tmpdir/rejection"
    hostile_count=$((hostile_count + 1))
}

for mutation in \
    'del(.lowering_plan.disposition_facts)' \
    '.lowering_plan.disposition_facts=[]' \
    '.lowering_plan.disposition_facts += [.lowering_plan.disposition_facts[0]]' \
    '.lowering_plan.disposition_facts[0].obligation.success_branch_operation_id as $id | (.lowering_plan.operations[]|select(.id==$id).operands[0].left.symbol_id)=999'; do
    reject_hostile_plan "$mutation"
done

for mutation in \
    '.completion="at_most_one"' \
    '.possible_dispositions[0].payload_type="Bool"' \
    '.possible_dispositions[1].payload_type="OtherFailure"' \
    '.possible_dispositions[1].failure_codes=[]' \
    '.possible_dispositions[1].route.symbol_id=999' \
    '.obligation.kind="optional"' \
    '.obligation.status="ignored"' \
    '.obligation.owner_symbol_id=999' \
    '.obligation.code_projection_operation_id=999' \
    '.obligation.code_symbol_id=999' \
    '.obligation.success_branch_operation_id=999' \
    '.obligation.failure_branch_operation_id=999' \
    '.obligation.success_value_operation_ids=[]' \
    '.obligation.dispose_operation_id=999' \
    '.obligation.failure_recovery_operation_ids=[]' \
    '.obligation.proof="assumed"'; do
    reject_hostile_plan "(.lowering_plan.disposition_facts[0])|=($mutation)"
done

# Attack execution relations while retaining the declared accounting sets.
for mutation in \
    '.lowering_plan.disposition_facts[0].obligation as $o | (.lowering_plan.operations[]|select(.id==$o.dispose_operation_id)) |= (.statement_id=0 | .target_fact.statement_id=0 | .target_fact.provenance.ast_path="/statement_pool/0/payload/target")' \
    '.lowering_plan.disposition_facts[0].obligation as $o | (.lowering_plan.operations[]|select(.id==$o.failure_branch_operation_id).block_id)=999' \
    '.lowering_plan.disposition_facts[0].obligation as $o | (.lowering_plan.operations[]|select(.id==$o.success_branch_operation_id).else_block_id)=$o.failure_block_id' \
    '.lowering_plan.disposition_facts[0].obligation as $o | (.lowering_plan.operations[]|select(.id==$o.code_projection_operation_id).statement_id)=999' \
    '.lowering_plan.disposition_facts[0].obligation as $o | (.lowering_plan.operations[]|select(.id==$o.success_branch_operation_id).function_symbol_id)=999'; do
    reject_hostile_plan "$mutation"
done

reject_hostile_plan '
    .lowering_plan.disposition_facts[0].obligation as $o |
    ($o.success_value_operation_ids[] | select(.!=$o.dispose_operation_id)) as $use |
    (.lowering_plan.operations[] | select(.id==$use).argument_resources[0].ownership)="owned"'
reject_hostile_plan '
    .lowering_plan.operations += [{id:9999, kind:"return_value", statement_id:3,
      expression_id:9999, scope_id:.lowering_plan.disposition_facts[0].scope_id,
      function_symbol_id:.lowering_plan.disposition_facts[0].function_symbol_id,
      block_id:0, operands:[{kind:"integer_literal",type:"c_int",value:"0"}]}]'

# A second actual cleanup must be rejected independently, not merely because
# an authorization list happens to reject the changed provider later.
reject_hostile_plan '
    .lowering_plan.disposition_facts[0].obligation as $o |
    (.lowering_plan.operations[] | select(.id==$o.dispose_operation_id)) as $dispose |
    ($o.success_value_operation_ids[] | select(.!=$o.dispose_operation_id)) as $use |
    (.lowering_plan.operations[] | select(.id==$use)) |=
      (.callee=$dispose.callee | .callee_symbol_id=$dispose.callee_symbol_id |
       .provider=$dispose.provider | .effect_contract=$dispose.effect_contract |
       .argument_resources=$dispose.argument_resources)'

# Mutate the generic transfer fact while keeping each stage's accepted format.
hostile_baseline=returned
for mutation in \
    'del(.lowering_plan.disposition_facts[0].obligation.ownership_transfer)' \
    '.lowering_plan.disposition_facts[0].obligation.ownership_transfer.mode="shared"' \
    '.lowering_plan.disposition_facts[0].obligation.ownership_transfer.value_type="OtherOwnedType"' \
    '.lowering_plan.disposition_facts[0].obligation.ownership_transfer.obligation_identity="new-completion"' \
    '.lowering_plan.disposition_facts[0].obligation.ownership_transfer.producer_operation_id=999' \
    '.lowering_plan.disposition_facts[0].obligation.ownership_transfer.return_operation_id=999' \
    '.lowering_plan.disposition_facts[0].obligation.ownership_transfer.call_operation_id=999' \
    '.lowering_plan.disposition_facts[0].obligation.ownership_transfer.source_owner_symbol_id=999' \
    '.lowering_plan.disposition_facts[0].obligation.ownership_transfer.destination_owner_symbol_id=999' \
    '.lowering_plan.disposition_facts[0].obligation.ownership_transfer.source_function_symbol_id=999' \
    '.lowering_plan.disposition_facts[0].obligation.ownership_transfer.destination_function_symbol_id=999' \
    '.lowering_plan.disposition_facts[0].obligation.ownership_transfer as $t | (.lowering_plan.operations[]|select(.id==$t.return_operation_id).operands[0].symbol_id)=999' \
    '.lowering_plan.disposition_facts[0].obligation.ownership_transfer as $t | (.lowering_plan.operations[]|select(.id==$t.call_operation_id).operands)=[{kind:"integer_literal",type:"c_int",value:"0"}]'; do
    reject_hostile_plan "$mutation"
done

printf 'import "%s/Lyraform/compiler/std/abi/libc.flow"\nimport "%s/Lyraform/compiler/std/abi/text.flow"\n\nprogram text_outcome_overflow\n\nmain {\n    outcome : TextOutcome(concat_outcome("' "$root" "$root" > "$tmpdir/overflow.flow"
awk 'BEGIN { for (i = 0; i < 4097; ++i) printf "A" }' >> "$tmpdir/overflow.flow"
printf '%s\n' '", "x"))' '    zero : c_int(0)' '    code : c_int(outcome.code)' '    status : c_int(0)' '    if code == zero {' '        puts_text(outcome.value) -> status' '        dispose(outcome.value) -> status' '    }' '    if code != zero {' '        puts("concat failed") -> status' '    }' '}' >> "$tmpdir/overflow.flow"
run_case "$tmpdir/overflow.flow" 'concat failed' overflow

echo "TextOutcome boundary: PASS (6 execution cases, 23 source refusals, $hostile_count hostile mutations at 7 valid consumer boundaries, 3 preserved stages, ordered exactly-once cleanup)"
