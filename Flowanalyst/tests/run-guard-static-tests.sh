#!/bin/sh
set -eu
build=$1
root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
tmpdir=$(mktemp -d /tmp/lyraform-guard-static.XXXXXX)
trap 'rm -rf "$tmpdir"' EXIT
compiler="$build/flowmini/flowmini"
analyst="$build/flowanalyst/flowanalyst"
validator="$build/flowcontracts/flowvalidate"

analyze() {
    source=$1
    "$compiler" --dump-frontend-bundle "$source" > "$tmpdir/frontend.json"
    "$analyst" "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
    "$analyst" "$tmpdir/frontend.json" > "$tmpdir/repeated.json"
    cmp "$tmpdir/semantic.json" "$tmpdir/repeated.json"
}

analyze "$root/guard-static.flow"
jq -e '.status=="ok" and .lowering_plan.status=="ready" and
    ([.lowering_plan.guard_facts[].event]==["activate","transition","deactivate"]) and
    ([.lowering_plan.guard_facts[].classification]==["proven_safe","proven_safe","deactivated"]) and
    all(.lowering_plan.guard_facts[]; .format=="lyraform.guard_fact" and .version==1 and
        .fact_id>=0 and
        .guard_name=="positive" and .guard_symbol_id>=0 and .scope_id>=0 and
        (.dependencies|length)==1 and .provenance.line>0 and .provenance.column>0) and
    (.lowering_plan.disposition_facts|length)==1 and
    all(.lowering_plan.disposition_facts[];
        .format=="lyraform.disposition_fact" and .version==1 and .fact_id>=0 and
        .completion=="exactly_one" and
        (.possible_dispositions|length)==1 and
        .possible_dispositions[0].kind=="success" and
        .possible_dispositions[0].payload_type=="int" and
        .possible_dispositions[0].commit=="atomic_destination" and
        .possible_dispositions[0].route.kind=="destination" and
        (.eliminated_dispositions|length)==1 and
        .eliminated_dispositions[0].kind=="failure" and
        .eliminated_dispositions[0].payload_type=="GuardViolation" and
        .eliminated_dispositions[0].reason=="proven_guard_preservation" and
        (.eliminated_dispositions[0].proof_guard_fact_ids|length)==1 and
        .provenance.line>0 and .provenance.column>0) and
    any(.lowering_plan.operations[]; .kind=="assignment" and .statement_id==4)' "$tmpdir/semantic.json" >/dev/null
jq -S '.lowering_plan.guard_facts' "$tmpdir/semantic.json" > "$tmpdir/facts.json"
jq -S '.lowering_plan.disposition_facts' "$tmpdir/semantic.json" > "$tmpdir/dispositions.json"
"$build/flowtools/flowparallel/flowparallel" "$tmpdir/semantic.json" > "$tmpdir/execution.json"
"$build/flowoptimize/flowoptimize" "$tmpdir/execution.json" > "$tmpdir/optimization.json"
"$build/flowlower/flowprepare" "$tmpdir/optimization.json" > "$tmpdir/backend.json"
for stage in semantic execution optimization backend; do
    jq -S '.lowering_plan.guard_facts' "$tmpdir/$stage.json" > "$tmpdir/stage-facts.json"
    cmp "$tmpdir/facts.json" "$tmpdir/stage-facts.json"
    jq -S '.lowering_plan.disposition_facts' "$tmpdir/$stage.json" > "$tmpdir/stage-dispositions.json"
    cmp "$tmpdir/dispositions.json" "$tmpdir/stage-dispositions.json"
    "$validator" "$tmpdir/$stage.json" >/dev/null
done
"$build/flowlower/flowlower" --emit-llvm "$tmpdir/program.ll" "$tmpdir/optimization.json" >/dev/null
"$build/tinyvm/flowtinylower" "$tmpdir/backend.json" "$tmpdir/program.tvm" >/dev/null

# Controls above establish accepted backend formats. Project each hostile plan
# into those formats so a format refusal cannot masquerade as guard evidence.
hostile_count=0
reject_authority() {
    hostile=$1
    jq --slurpfile hostile "$hostile" '.lowering_plan=$hostile[0].lowering_plan' \
        "$tmpdir/optimization.json" > "$tmpdir/hostile-optimization.json"
    jq --slurpfile hostile "$hostile" '.lowering_plan=$hostile[0].lowering_plan' \
        "$tmpdir/backend.json" > "$tmpdir/hostile-backend.json"
    if "$validator" "$hostile" > "$tmpdir/rejection.json" 2>&1; then exit 1; fi
    jq -e '.classification=="invalid" and ((.path + " " + .reason)|test("guard|disposition"; "i"))' \
        "$tmpdir/rejection.json" >/dev/null
    if "$build/flowlower/flowlower" --emit-llvm "$tmpdir/forbidden.ll" "$tmpdir/hostile-optimization.json" > "$tmpdir/rejection.txt" 2>&1; then exit 1; fi
    grep -Eiq 'guard|disposition' "$tmpdir/rejection.txt"
    if "$build/tinyvm/flowtinylower" "$tmpdir/hostile-backend.json" "$tmpdir/forbidden.tvm" > "$tmpdir/rejection.txt" 2>&1; then exit 1; fi
    grep -Eiq 'guard|disposition' "$tmpdir/rejection.txt"
    hostile_count=$((hostile_count + 1))
}

"$analyst" --lowering-plan-version 2 "$tmpdir/frontend.json" > "$tmpdir/semantic-v2.json"
jq -e '.status=="ok" and
    all(.lowering_plan.disposition_facts[]; .function_symbol_id>=0) and
    (.lowering_plan as $plan |
        all($plan.disposition_facts[];
            . as $fact |
            any($plan.operations[];
                .id==$fact.operation_id and .function_symbol_id==$fact.function_symbol_id)))' \
    "$tmpdir/semantic-v2.json" >/dev/null
"$validator" "$tmpdir/semantic-v2.json" >/dev/null

analyze "$root/guard-composition.flow"
jq -e '.status=="ok" and
    ([.lowering_plan.guard_facts[]|select(.event=="transition" and .statement_id==3)]|length)==2 and
    ([.lowering_plan.guard_facts[]|select(.event=="transition" and .statement_id==5)]|length)==1 and
    any(.lowering_plan.guard_facts[]; .event=="deactivate" and .guard_name=="small") and
    any(.lowering_plan.guard_facts[]; .event=="deactivate" and .guard_name=="positive")' "$tmpdir/semantic.json" >/dev/null

# Imported text expands the parser stream; facts/diagnostics must still point
# to the original file and original line, not the concatenated input.
printf 'import "%s/../../Lyraform/compiler/std/abi/text.flow"\n' "$root" > "$tmpdir/imported-guard.flow"
cat "$root/guard-static.flow" >> "$tmpdir/imported-guard.flow"
analyze "$tmpdir/imported-guard.flow"
transition_line=$(awk '/4 -> x/ {print NR; exit}' "$tmpdir/imported-guard.flow")
jq -e --argjson line "$transition_line" '
    any(.lowering_plan.guard_facts[]; .event=="transition" and
        (.provenance.source|endswith("/imported-guard.flow")) and .provenance.line==$line) and
    all(.lowering_plan.disposition_facts[]; .provenance.line==$line)' "$tmpdir/semantic.json" >/dev/null
sed 's/4 -> x/-1 -> x/' "$tmpdir/imported-guard.flow" > "$tmpdir/imported-violation.flow"
"$compiler" --dump-frontend-bundle "$tmpdir/imported-violation.flow" > "$tmpdir/imported.frontend.json"
set +e
"$analyst" "$tmpdir/imported.frontend.json" > "$tmpdir/imported-refusal.json"
imported_status=$?
set -e
test "$imported_status" -eq 2
jq -e --argjson line "$transition_line" '
    any(.diagnostics[]; .code=="FLOWANALYST_GUARD_TRANSITION_VIOLATION" and
        (.provenance.source|endswith("/imported-violation.flow")) and .provenance.line==$line)' \
    "$tmpdir/imported-refusal.json" >/dev/null

refuse() {
    body=$1
    code=$2
    printf 'program guard_refusal\nfn sensor(): int {\n4 -> return\n}\nmain {\nx : int(1)\n%s\n}\n' "$body" > "$tmpdir/refused.flow"
    "$compiler" --dump-frontend-bundle "$tmpdir/refused.flow" > "$tmpdir/frontend.json"
    set +e
    "$analyst" "$tmpdir/frontend.json" > "$tmpdir/refused.json"
    status=$?
    set -e
    test "$status" -eq 2
    jq -e --arg code "$code" '.status=="error" and .lowering_plan.status=="blocked" and
        (.lowering_plan.disposition_facts|length)==0 and
        any(.diagnostics[]; .code==$code) and
        ($code=="FLOWANALYST_GUARD_NOT_ACTIVE" or
            any(.lowering_plan.guard_facts[]; .classification=="proven_violation" or .classification=="not_provable"))' "$tmpdir/refused.json" >/dev/null
    set +e
    "$validator" "$tmpdir/refused.json" > "$tmpdir/blocked-validation.json"
    validation_status=$?
    set -e
    test "$validation_status" -eq 2
    jq -e '.classification=="blocked"' "$tmpdir/blocked-validation.json" >/dev/null
    if test "$code" = FLOWANALYST_GUARD_NOT_ACTIVE; then return; fi
    jq '.status="ok" | .lowering_plan.status="ready"' "$tmpdir/refused.json" > "$tmpdir/forged.json"
    reject_authority "$tmpdir/forged.json"
}
refuse 'guard positive : x < 0' FLOWANALYST_GUARD_ACTIVATION_VIOLATION
refuse 'guard positive : x > 0
-1 -> x' FLOWANALYST_GUARD_TRANSITION_VIOLATION
refuse 'guard positive : x > 0
sensor() -> x' FLOWANALYST_GUARD_NOT_PROVABLE
refuse 'unguard absent' FLOWANALYST_GUARD_NOT_ACTIVE

# Contextual spelling does not steal the established typed-binding form.
printf 'program guard_identifier\nmain {\nguard : int(1)\n}\n' > "$tmpdir/collision.flow"
"$compiler" --dump-frontend-bundle "$tmpdir/collision.flow" > "$tmpdir/collision.json"
jq -e '.parse_validity.state=="canonical_valid" and .parse_validity.coverage=="complete" and
    any(.ast.statement_pool[]; .kind=="let" and .name=="guard") and
    (all(.ast.statement_pool[]; .kind!="guard_activate"))' "$tmpdir/collision.json" >/dev/null

for malformed in 'guard positive x > 0' 'guard positive :' 'unguard positive extra'; do
    printf 'program malformed_guard\nmain {\nx : int(1)\n%s\n}\n' "$malformed" > "$tmpdir/malformed.flow"
    "$compiler" --dump-frontend-bundle "$tmpdir/malformed.flow" > "$tmpdir/malformed.json"
    jq -e '.parse_validity.state=="recovered" and any(.ast.statement_pool[]; .kind=="unknown")' "$tmpdir/malformed.json" >/dev/null
done

analyze "$root/guard-static.flow"
for mutation in \
    '.fact_id=-1' \
    '.guard_symbol_id=-1' \
    '.dependencies=[]' \
    '.dependencies=[999]' \
    '.predicate_expression_id=-1' \
    '.provenance.line=-1' \
    '.classification="not_provable"' \
    '.execution="unsupported"' \
    '.affected_operation_id=999' \
    '.event="invented"' \
    '.version=99'; do
    jq "(.lowering_plan.guard_facts[]|select(.event==\"transition\"))|=($mutation)" "$tmpdir/semantic.json" > "$tmpdir/hostile.json"
    reject_authority "$tmpdir/hostile.json"
done

analyze "$root/guard-static.flow"
for mutation in \
    '.fact_id=-1' \
    '.version=99' \
    '.operation_id=999' \
    '.statement_id=999' \
    '.expression_id=999' \
    '.scope_id=999' \
    '.completion="at_most_one"' \
    '.possible_dispositions=[]' \
    '.possible_dispositions[0].kind="failure"' \
    '.possible_dispositions[0].payload_type="Bool"' \
    '.possible_dispositions[0].commit="partial"' \
    '.possible_dispositions[0].route.symbol_id=999' \
    '.eliminated_dispositions=[]' \
    '.eliminated_dispositions[0].payload_type="OtherFailure"' \
    '.eliminated_dispositions[0].proof_guard_fact_ids=[]' \
    '.eliminated_dispositions[0].proof_guard_fact_ids=[999]' \
    '.provenance.line=-1'; do
    jq "(.lowering_plan.disposition_facts[0])|=($mutation)" "$tmpdir/semantic.json" > "$tmpdir/hostile-disposition.json"
    reject_authority "$tmpdir/hostile-disposition.json"
done

for structural_mutation in \
    '.lowering_plan |= del(.disposition_facts)' \
    '.lowering_plan.disposition_facts=[]' \
    '.lowering_plan.disposition_facts += [.lowering_plan.disposition_facts[0]]'; do
    jq "$structural_mutation" "$tmpdir/semantic.json" > "$tmpdir/hostile-disposition.json"
    reject_authority "$tmpdir/hostile-disposition.json"
done

jq '.lowering_plan.disposition_facts=[]' "$tmpdir/semantic.json" > "$tmpdir/dangling-authority.json"
set +e
"$validator" "$tmpdir/dangling-authority.json" > "$tmpdir/dangling-diagnostic.json"
dangling_status=$?
set -e
test "$dangling_status" -eq 1
jq -e '.classification=="invalid" and
    (.path|contains("disposition_facts")) and
    (.reason|contains("proven guarded operation lacks disposition authority"))' \
    "$tmpdir/dangling-diagnostic.json" >/dev/null

echo 'Static guard lifecycle: PASS (activation, composition, selective unguard, canonical disposition authority v1 with v2 owner linkage, explanatory dangling-authority diagnostic, 5 semantic refusals, 3 malformed forms, 1 collision, 4 preserved stages, 11 hostile guard facts, 20 hostile disposition facts, 2 backends)'

echo "Guard/disposition hostile boundary evidence: $hostile_count mutations rejected at 3 format-correct consumers"
