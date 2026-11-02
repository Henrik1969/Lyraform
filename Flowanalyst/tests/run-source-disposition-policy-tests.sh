#!/bin/sh
set -eu

build=$1
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$root"
tmpdir=$(mktemp -d /tmp/lyraform-source-disposition-policy.XXXXXX)
trap 'rm -rf "$tmpdir"' EXIT

compiler="$build/flowmini/flowmini"
analyst="$build/flowanalyst/flowanalyst"
validator="$build/flowcontracts/flowvalidate"
fixed_source="$root/Lyraform/compiler/examples/disposition/text_outcome_consumer.flow"
fixed_host="$root/Lyraform/compiler/examples/disposition/policy/text_outcome_consumer.host-policy.json"
two_source="$root/Lyraform/compiler/examples/disposition/text_outcome_two_routes.flow"
two_policy="$root/Lyraform/compiler/examples/disposition/policy/text_outcome_two_routes.policy.json"
two_host="$root/Lyraform/compiler/examples/disposition/policy/text_outcome_two_routes.host-policy.json"

"$compiler" --dump-frontend-bundle "$fixed_source" > "$tmpdir/fixed-frontend.json"
"$analyst" --lowering-plan-version 2 --host-disposition-policy "$fixed_host" \
    "$tmpdir/fixed-frontend.json" > "$tmpdir/fixed-report.json"
"$analyst" --lowering-plan-version 2 --host-disposition-policy "$fixed_host" \
    "$tmpdir/fixed-frontend.json" > "$tmpdir/fixed-repeated.json"
cmp "$tmpdir/fixed-report.json" "$tmpdir/fixed-repeated.json"
jq -e '
    .status=="ok" and .lowering_plan.status=="blocked" and
    .lowering_plan.source_disposition_policy.format=="lyraform.source_disposition_policy_selection" and
    .lowering_plan.source_disposition_policy.version==1 and
    .lowering_plan.source_disposition_policy.status=="selected" and
    .lowering_plan.source_disposition_policy.execution=="unsupported" and
    .lowering_plan.source_disposition_policy.selection_kind=="fixed_single_route" and
    .lowering_plan.source_disposition_policy.policy.profile=="compiler.fixed_single_route" and
    .lowering_plan.source_disposition_policy.policy.authority=="lyraform.semantic-policy-selector" and
    (.lowering_plan.source_disposition_policy.selections|length)==1 and
    .lowering_plan.source_disposition_policy.selections[0].route_id==0 and
    .lowering_plan.source_disposition_policy.selections[0].response_function_symbol_id==40 and
    .lowering_plan.source_disposition_policy.selections[0].response_transition.response_class=="recover" and
    .lowering_plan.source_disposition_policy.selections[0].response_transition.obligation_transition=="close_original" and
    .lowering_plan.host_disposition_policy.action=="report_and_fail" and
    .lowering_plan.host_disposition_policy.execution=="unsupported" and
    (all(.lowering_plan.source_disposition.execution_claims[]; .==false))' \
    "$tmpdir/fixed-report.json" >/dev/null
"$validator" "$tmpdir/fixed-report.json" > "$tmpdir/fixed-validation.json"
jq -e '.classification=="valid"' "$tmpdir/fixed-validation.json" >/dev/null

set +e
"$analyst" --lowering-plan-version 2 "$tmpdir/fixed-frontend.json" \
    > "$tmpdir/missing-host.json"
status=$?
set -e
test "$status" -eq 2
jq -e '.status=="error" and any(.diagnostics[]; .code=="FLOWANALYST_HOST_POLICY_MISSING")' \
    "$tmpdir/missing-host.json" >/dev/null

set +e
"$analyst" --lowering-plan-version 2 --disposition-policy "$two_policy" \
    --host-disposition-policy "$fixed_host" "$tmpdir/fixed-frontend.json" \
    > "$tmpdir/extra-fixed-policy.json"
status=$?
set -e
test "$status" -eq 2
jq -e '.status=="error" and any(.diagnostics[]; .code=="FLOWANALYST_DISPOSITION_POLICY_EXTRA")' \
    "$tmpdir/extra-fixed-policy.json" >/dev/null

"$compiler" --dump-frontend-bundle "$two_source" > "$tmpdir/two-frontend.json"
set +e
"$analyst" --lowering-plan-version 2 --host-disposition-policy "$two_host" \
    "$tmpdir/two-frontend.json" > "$tmpdir/missing-route-policy.json"
status=$?
set -e
test "$status" -eq 2
jq -e '.status=="error" and any(.diagnostics[]; .code=="FLOWANALYST_DISPOSITION_POLICY_MISSING")' \
    "$tmpdir/missing-route-policy.json" >/dev/null

"$analyst" --lowering-plan-version 2 --disposition-policy "$two_policy" \
    --host-disposition-policy "$two_host" "$tmpdir/two-frontend.json" \
    > "$tmpdir/two-report.json"
"$analyst" --lowering-plan-version 2 --disposition-policy "$two_policy" \
    --host-disposition-policy "$two_host" "$tmpdir/two-frontend.json" \
    > "$tmpdir/two-repeated.json"
cmp "$tmpdir/two-report.json" "$tmpdir/two-repeated.json"
jq -e '
    .status=="ok" and .lowering_plan.status=="blocked" and
    .lowering_plan.source_disposition_policy.selection_kind=="supplied" and
    (.lowering_plan.source_disposition_policy.selections|length)==1 and
    .lowering_plan.source_disposition_policy.selections[0].route_id==1 and
    .lowering_plan.source_disposition_policy.selections[0].response_function_symbol_id==43 and
    .lowering_plan.source_disposition_policy.selections[0].response_transition.outgoing_type.identity=="type:Text" and
    .lowering_plan.host_disposition_policy.action=="report_and_fail" and
    (all(.lowering_plan.source_disposition.execution_claims[]; .==false))' \
    "$tmpdir/two-report.json" >/dev/null

jq -S . "$two_policy" > "$tmpdir/external-policy.json"
jq -S '.lowering_plan.source_disposition_policy' "$tmpdir/two-report.json" \
    > "$tmpdir/embedded-policy.json"
cmp "$tmpdir/external-policy.json" "$tmpdir/embedded-policy.json"
jq -S . "$two_host" > "$tmpdir/external-host.json"
jq -S '.lowering_plan.host_disposition_policy' "$tmpdir/two-report.json" \
    > "$tmpdir/embedded-host.json"
cmp "$tmpdir/external-host.json" "$tmpdir/embedded-host.json"
jq -S '.lowering_plan.source_disposition' "$tmpdir/missing-route-policy.json" \
    > "$tmpdir/topology-before-selection.json"
jq -S '.lowering_plan.source_disposition' "$tmpdir/two-report.json" \
    > "$tmpdir/topology-after-selection.json"
cmp "$tmpdir/topology-before-selection.json" "$tmpdir/topology-after-selection.json"
"$validator" "$tmpdir/two-report.json" > "$tmpdir/two-validation.json"
jq -e '.classification=="valid"' "$tmpdir/two-validation.json" >/dev/null

policy_hostile_count=0
reject_policy() {
    mutation=$1
    jq "$mutation" "$two_policy" > "$tmpdir/hostile-policy.json"
    set +e
    "$analyst" --lowering-plan-version 2 --disposition-policy "$tmpdir/hostile-policy.json" \
        --host-disposition-policy "$two_host" "$tmpdir/two-frontend.json" \
        > "$tmpdir/hostile-policy-report.json"
    status=$?
    set -e
    test "$status" -eq 2
    jq -e '.status=="error" and any(.diagnostics[]; .code=="FLOWANALYST_DISPOSITION_POLICY_INVALID")' \
        "$tmpdir/hostile-policy-report.json" >/dev/null
    policy_hostile_count=$((policy_hostile_count + 1))
}

for mutation in \
    '.module.identity="module:foreign"' \
    '.module.revision="stale"' \
    '.selection_kind="fixed_single_route"' \
    '.selections=[]' \
    '.selections += [.selections[0]]' \
    '.selections[0].declaration_id=999' \
    '.selections[0].semantic_id="consumer-declaration:999"' \
    '.selections[0].instance_structural_id=999' \
    '.selections[0].instance_node_id="foreign"' \
    '.selections[0].incoming_failure_type.identity="type:OtherFailure"' \
    '.selections[0].route_id=99' \
    '.selections[0].response_function_symbol_id=40' \
    '.selections[0].response_transition.response_class="transform"' \
    '.selections[0].response_transition.outgoing_type.identity="type:Bool"' \
    '.selections[0].response_transition.obligation_transition="linked_successor"' \
    '.policy.profile=""' \
    '.policy.revision=""' \
    '.policy.authority=""' \
    '.unexpected="undeclared meaning"' \
    '.module.unexpected="undeclared meaning"' \
    '.selections[0].unexpected="undeclared meaning"' \
    '.selections[0].incoming_failure_type.unexpected="undeclared meaning"' \
    '.selections[0].response_transition.unexpected="undeclared meaning"' \
    '.selections[0].response_transition.outgoing_type.unexpected="undeclared meaning"' \
    '.policy.unexpected="undeclared meaning"' \
    '.policy.provenance.unexpected="undeclared meaning"'; do
    reject_policy "$mutation"
done

host_hostile_count=0
reject_host_policy() {
    mutation=$1
    jq "$mutation" "$two_host" > "$tmpdir/hostile-host.json"
    set +e
    "$analyst" --lowering-plan-version 2 --disposition-policy "$two_policy" \
        --host-disposition-policy "$tmpdir/hostile-host.json" "$tmpdir/two-frontend.json" \
        > "$tmpdir/hostile-host-report.json"
    status=$?
    set -e
    test "$status" -eq 2
    jq -e '.status=="error" and any(.diagnostics[]; .code=="FLOWANALYST_HOST_POLICY_INVALID")' \
        "$tmpdir/hostile-host-report.json" >/dev/null
    host_hostile_count=$((host_hostile_count + 1))
}

for mutation in \
    '.module.identity="module:foreign"' \
    '.module.revision="stale"' \
    '.containment.node_id="foreign"' \
    '.containment.scope_kind="process"' \
    '.action="halt_and_quarantine"' \
    '.action="restart"' \
    '.policy.profile=""' \
    '.policy.revision=""' \
    '.policy.authority=""' \
    '.unexpected="undeclared meaning"' \
    '.module.unexpected="undeclared meaning"' \
    '.containment.unexpected="undeclared meaning"' \
    '.policy.unexpected="undeclared meaning"' \
    '.policy.provenance.unexpected="undeclared meaning"'; do
    reject_host_policy "$mutation"
done

# The source-resolved typed rejoin destination must survive every plan stage.
jq -e '.lowering_plan.source_disposition.rejoin as $r |
    $r.node_id == "display" and $r.input_type == "Text" and
    $r.wire_id == .lowering_plan.source_disposition.junction.rejoin_wire_id and
    any(.lowering_plan.functions[];
        .symbol_id == $r.function_symbol_id and .availability == "definition" and
        .parameters[0].symbol_id == $r.parameter_symbol_id and .parameters[0].type == $r.input_type)'     "$tmpdir/fixed-report.json" > /dev/null
printf '%s\n'     'allow libc.so.6 puts c io Text c_int'     'allow libflowtext.so flow_text_concat_value c memory Text,Text TextOutcome' > "$tmpdir/bind.policy"
export LD_LIBRARY_PATH="$build/flowtools/reference/text${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
"$build/flowbind/flowbind" --policy "$tmpdir/bind.policy" "$tmpdir/fixed-report.json" > "$tmpdir/bound.json"
"$build/flowtools/flowparallel/flowparallel" < "$tmpdir/fixed-report.json" > "$tmpdir/parallel.json"
"$build/flowoptimize/flowoptimize" < "$tmpdir/parallel.json" > "$tmpdir/optimized.json"
"$build/flowlower/flowprepare" --binding-report "$tmpdir/bound.json" "$tmpdir/optimized.json" > "$tmpdir/prepared.json"
jq -S '.lowering_plan.source_disposition.rejoin' "$tmpdir/fixed-report.json" > "$tmpdir/rejoin.json"
for stage in parallel optimized prepared; do
    "$validator" "$tmpdir/$stage.json" > /dev/null
    jq -S '.lowering_plan.source_disposition.rejoin' "$tmpdir/$stage.json" > "$tmpdir/stage-rejoin.json"
    cmp "$tmpdir/rejoin.json" "$tmpdir/stage-rejoin.json"
    jq '.lowering_plan.source_disposition.rejoin.parameter_symbol_id=99999' "$tmpdir/$stage.json" > "$tmpdir/foreign-rejoin.json"
    if "$validator" "$tmpdir/foreign-rejoin.json" > /dev/null; then
        echo "foreign rejoin parameter accepted at $stage" >&2; exit 1
    fi
done

report_hostile_count=0
reject_report() {
    mutation=$1
    jq "$mutation" "$tmpdir/two-report.json" > "$tmpdir/hostile-report.json"
    set +e
    "$validator" "$tmpdir/hostile-report.json" > "$tmpdir/hostile-validation.json"
    status=$?
    set -e
    test "$status" -eq 1
    jq -e '.classification=="invalid"' "$tmpdir/hostile-validation.json" >/dev/null
    report_hostile_count=$((report_hostile_count + 1))
}

for mutation in \
    '.lowering_plan.source_disposition_policy.selections[0].route_id=99' \
    '.lowering_plan.source_disposition_policy.selections[0].response_function_symbol_id=40' \
    '.lowering_plan.source_disposition_policy.selections[0].response_transition.response_class="transform"' \
    '.lowering_plan.source_disposition_policy.module.revision="stale"' \
    '.lowering_plan.host_disposition_policy.action="restart"' \
    '.lowering_plan.source_disposition_policy.unexpected="undeclared meaning"' \
    '.lowering_plan.host_disposition_policy.unexpected="undeclared meaning"' \
    'del(.lowering_plan.host_disposition_policy)' \
    'del(.lowering_plan.source_disposition_policy)' \
    '.lowering_plan.source_disposition.rejoin.function_symbol_id=99999' \
    '.lowering_plan.source_disposition.rejoin.parameter_symbol_id=99999' \
    '.lowering_plan.source_disposition.rejoin.input_type="c_int"' \
    '.lowering_plan.source_disposition.rejoin.wire_id="foreign"' \
    '.lowering_plan.source_disposition.rejoin.provenance.line=0' \
    '(.lowering_plan.source_disposition.response_functions[0] | .response_class)="transform" | (.lowering_plan.source_disposition.response_functions[0] | .obligation_transition)="linked_successor"' \
    '.lowering_plan.status="ready"'; do
    reject_report "$mutation"
done

printf '%s\n' \
    "Source disposition Gate 3: PASS (fixed and supplied selection deterministic; external/embedded artifacts identical; topology unchanged; $policy_hostile_count route-policy, $host_hostile_count host-policy, and $report_hostile_count report mutations rejected; execution unsupported)"
