#!/bin/sh
set -eu

build=$1
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
specimen="$root/Lyraform/compiler/examples/disposition/text_outcome_consumer.flow"
tmpdir=$(mktemp -d /tmp/lyraform-source-disposition.XXXXXX)
trap 'rm -rf "$tmpdir"' EXIT
compiler="$build/flowmini/flowmini"
analyst="$build/flowanalyst/flowanalyst"
validator="$build/flowcontracts/flowvalidate"

analyze() {
    source=$1
    "$compiler" --dump-frontend-bundle "$source" > "$tmpdir/frontend.json"
    "$analyst" --lowering-plan-version 2 "$tmpdir/frontend.json" > "$tmpdir/report.json"
}

analyze "$specimen"
"$analyst" --lowering-plan-version 2 "$tmpdir/frontend.json" > "$tmpdir/repeated.json"
cmp "$tmpdir/report.json" "$tmpdir/repeated.json"
jq -e '
    .status=="ok" and (.diagnostics|length)==0 and .lowering_plan.status=="blocked" and
    (.lowering_plan.source_disposition.format=="lyraform.source_disposition_topology") and
    (.lowering_plan.source_disposition.version==1) and
    (.lowering_plan.source_disposition.status=="semantic") and
    (.lowering_plan.source_disposition.execution=="unsupported") and
    (.lowering_plan.source_disposition.producer.completion=="exactly_one") and
    (.lowering_plan.source_disposition.producer.commit_law=="atomic_tagged_result") and
    (.lowering_plan.source_disposition.producer.success_type.spelling=="Text") and
    ([.lowering_plan.source_disposition.producer.failure_types[].spelling]==["TextFailure"]) and
    ([.lowering_plan.source_disposition.producer.fault_types[].spelling]==["TextIntegrityFault"]) and
    (.lowering_plan.source_disposition.response_functions[0].input_projection=="immutable_failure_envelope") and
    (.lowering_plan.source_disposition.response_functions[0].response_class=="recover") and
    (.lowering_plan.source_disposition.response_functions[0].outgoing_type.spelling=="Text") and
    (.lowering_plan.source_disposition.consumer.closed_set==true) and
    (.lowering_plan.source_disposition.consumer.selection_requirement=="fixed_single_route_gate_3") and
    (.lowering_plan.source_disposition.junction.pairing=="same_producer_attempt_exclusive") and
    (.lowering_plan.source_disposition.containment.scope_kind=="activation") and
    (.lowering_plan.source_disposition.bridge.status=="declarative") and
    (.lowering_plan.source_disposition.bridge.policy_selection=="not_materialized_gate_3") and
    (all(.lowering_plan.source_disposition.execution_claims[]; .==false))' \
    "$tmpdir/report.json" >/dev/null
jq '.lowering_plan.source_disposition' "$tmpdir/report.json" > "$tmpdir/topology.json"
"$validator" "$tmpdir/topology.json" > "$tmpdir/validated.json"
jq -e '.classification=="valid" and .format=="lyraform.source_disposition_topology"' \
    "$tmpdir/validated.json" >/dev/null
"$validator" "$tmpdir/report.json" > "$tmpdir/validated-report.json"
jq -e '.classification=="valid" and .format=="flowanalyst.semantic_report"' \
    "$tmpdir/validated-report.json" >/dev/null

hostile_count=0
reject_topology() {
    mutation=$1
    jq "$mutation" "$tmpdir/topology.json" > "$tmpdir/hostile.json"
    if "$validator" "$tmpdir/hostile.json" > "$tmpdir/rejected.json" 2>&1; then exit 1; fi
    jq -e '.classification=="invalid"' "$tmpdir/rejected.json" >/dev/null
    hostile_count=$((hostile_count + 1))
}

for mutation in \
    '.producer.operation_id=999' \
    '.producer.disposition_id=999' \
    '.producer.function_symbol_id=999' \
    '.producer.owner_symbol_id=999' \
    '.producer.graph_node_id="foreign"' \
    '.producer.success_type.identity="type:Bool"' \
    '.response_functions[0].incoming_failure_type.spelling="OtherFailure"' \
    '.response_functions[0].outgoing_type.spelling="Bool" | .response_functions[0].outgoing_type.identity="type:Bool"' \
    '.response_functions[0].input_projection="mutable"' \
    '.response_functions[0].obligation_transition="linked_successor"' \
    '.consumer.accepted_failure_types=[]' \
    '.consumer.routes[0].function_symbol_id=999' \
    '.junction.producer_node_id="foreign"' \
    '.junction.success_wire_id=.junction.failure_wire_id' \
    '.containment.scope_kind="process"' \
    '.containment.provenance.line=0' \
    '.containment.provenance.source="foreign.flow"' \
    '.module.identity="module:foreign"' \
    '.bridge.module_revision="foreign"' \
    '.bridge.wire_mapping[0].graph_wire_id="foreign"' \
    '.status="ready"' \
    '.execution="ready"' \
    '.execution_claims.graph_ir_ready=true'; do
    reject_topology "$mutation"
done

report_hostile_count=0
reject_report() {
    mutation=$1
    jq "$mutation" "$tmpdir/report.json" > "$tmpdir/hostile-report.json"
    if "$validator" "$tmpdir/hostile-report.json" > "$tmpdir/rejected-report.json" 2>&1; then exit 1; fi
    jq -e '.classification=="invalid"' "$tmpdir/rejected-report.json" >/dev/null
    report_hostile_count=$((report_hostile_count + 1))
}
for mutation in \
    '.lowering_plan.source_disposition.producer.operation_id=999' \
    '.lowering_plan.source_disposition.producer.return_operation_id=999' \
    '.lowering_plan.source_disposition.producer.owner_symbol_id=999' \
    '.lowering_plan.source_disposition.bridge.wire_mapping[0].graph_wire_id="foreign"' \
    '.lowering_plan.source_disposition.module.provenance.source="foreign.flow"' \
    '.lowering_plan.status="ready"'; do
    reject_report "$mutation"
done

refusal_count=0
refuse_source() {
    name=$1
    expected=$2
    script=$3
    source="$tmpdir/$name.flow"
    sed "$script" "$specimen" | sed "s#../../std#$root/Lyraform/compiler/std#" > "$source"
    "$compiler" --dump-frontend-bundle "$source" > "$tmpdir/frontend.json"
    set +e
    "$analyst" --lowering-plan-version 2 "$tmpdir/frontend.json" > "$tmpdir/refused.json"
    status=$?
    set -e
    test "$status" -eq 2
    jq -e --arg code "$expected" --arg name "$name.flow" '
        .status=="error" and .lowering_plan.status=="blocked" and
        (.lowering_plan|has("source_disposition")|not) and
        any(.diagnostics[]; .code==$code and (.provenance.source|endswith($name)) and
            .provenance.line>0 and .provenance.column>0)' "$tmpdir/refused.json" >/dev/null
    refusal_count=$((refusal_count + 1))
}

refuse_source wrong_recovery FLOWANALYST_DISPOSITION_REJOIN_TYPE \
    's/recover Text/recover c_int/'
refuse_source wrong_envelope FLOWANALYST_DISPOSITION_ENVELOPE_TYPE \
    's/failure TextFailure/failure TextIntegrityFault/'
refuse_source contradictory_set FLOWANALYST_DISPOSITION_CARRIER_MISMATCH \
    's/fails { TextFailure }/fails { TextIntegrityFault }/'
refuse_source missing_failure FLOWANALYST_DISPOSITION_DANGLING_ROUTE \
    '/wire producer_node.failure/d'
refuse_source duplicate_success FLOWANALYST_DISPOSITION_COEMISSION \
    '/wire producer_node.out/p'
refuse_source fault_to_consumer FLOWANALYST_DISPOSITION_FAULT_TO_CONSUMER \
    's/wire producer_node.fault   => quarantine.fault/wire producer_node.fault   => handled.fault/'
refuse_source unknown_response FLOWANALYST_DISPOSITION_UNKNOWN_RESPONSE \
    's/    use_fallback/    missing_response/'
refuse_source unaccounted_transform FLOWANALYST_DISPOSITION_UNACCOUNTED_TRANSFORM \
    's/recover Text/transform Text/'
refuse_source cycle FLOWANALYST_DISPOSITION_CYCLE_UNSUPPORTED \
    '/main {/i\
wire display.out => producer_node.in'
refuse_source mixed_producer FLOWANALYST_DISPOSITION_AMBIGUOUS_PRODUCER \
    's/fn display_text(value : Text): c_int/fn display_text(value : Text): c_int fails { TextFailure }/'

printf '%s\n' "Source disposition Gate 2: PASS (1 deterministic source topology; $refusal_count early semantic refusals with verified source locations; $hostile_count topology and $report_hostile_count semantic-report mutations rejected; execution unsupported)"
