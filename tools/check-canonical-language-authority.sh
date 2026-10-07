#!/bin/sh
set -eu

root=${FLOWCORE_ROOT:?}
map=$root/docs/architecture/canonical-language-authority-map-v1.json

fail() {
    printf 'canonical-language authority drift: %s\n' "$1" >&2
    exit 1
}

jq -e '
  .schema == "lyraform.canonical-language-authority-map/v1" and
  .status == "convergence_freeze" and
  .language_surface_frozen == true and
  .whole_language_audit == "docs/architecture/canonical-source-form-audit-v1.json" and
  .baseline_commit == "ecdac7184236dab24423ca7495e882f771cdf287" and
  (.canonical_flow == [
    "structural_frontend",
    "flowanalyst_shared_semantics",
    "flowcontracts_validation",
    "preserving_compiler_stages",
    "llvm_or_tinyvm"
  ]) and
  (.surfaces | length) > 0 and
  ([.surfaces[].id] | length) == ([.surfaces[].id] | unique | length) and
  all(.surfaces[];
    (.id | type == "string" and length > 0) and
    (.recognition | type == "string" and length > 0) and
    (.structural | type == "string" and length > 0) and
    (.semantic | type == "string" and length > 0) and
    (.execution | type == "string" and length > 0) and
    (.authority | type == "string" and length > 0) and
    (.gate | type == "string" and length > 0)) and
  any(.surfaces[]; .id == "initialized_local_scalar_flow" and
    .semantic == "admitted_single_canonical_producer" and
    .authority == "flowanalyst scalar facts + flowcontracts validation") and
  any(.surfaces[]; .id == "deprecated_legacy_source" and
    .recognition == "deprecated_legacy" and
    .semantic == "oracle_only_non_authoritative" and
    .execution == "excluded_from_canonical") and
  any(.surfaces[]; .id == "local_member_reconstruction" and
    .semantic == "admitted" and .execution == "unsupported") and
  any(.surfaces[]; .id == "source_operation_coverage" and
    .semantic == "fail_closed_for_callable_plan_v2") and
  any(.known_bridges[]; .id == "canonical_staged_execution_bridge" and
    .state == "bounded_connected") and
  any(.known_bridges[]; .id == "scalar_operation_identity_bridge" and
    .state == "bounded_connected") and
  any(.known_bridges[]; .id == "target_operation_identity_bridge" and
    .state == "single_producer_validated") and
  any(.known_bridges[]; .id == "guard_disposition_operation_bridge" and
    .state == "bounded_single_producer") and
  any(.known_bridges[]; .id == "text_outcome_ownership_disposition_bridge" and
    .state == "bounded_single_producer") and
  any(.surfaces[]; .id == "provider_and_effect_admission" and
    .semantic == "admitted_single_canonical_producer") and
  any(.known_bridges[]; .id == "provider_effect_authorization_bridge" and
    .state == "bounded_exact_set") and
  any(.surfaces[]; .id == "source_call_operation_projection" and
    .semantic == "operation_linked") and
  any(.known_bridges[]; .id == "call_projection_operation_bridge" and
    .state == "bounded_exact_projection") and
  any(.surfaces[]; .id == "deferred_parallel_candidate_projection" and
    .semantic == "operation_linked_deferred_only" and
    .execution == "not_admitted") and
  any(.known_bridges[]; .id == "parallel_candidate_operation_bridge" and
    .state == "bounded_exact_deferred_projection") and
  any(.known_bridges[]; .id == "call_authority_carrying_stage_bridge" and
    .state == "plan_v2_exact_preservation") and
  any(.known_bridges[]; .id == "remaining_authority_carrying_stage_bridge" and
    .state == "plan_v2_complete_envelope") and
  any(.known_bridges[]; .id == "whole_language_authority_audit" and
    .state == "complete_under_language_freeze") and
  any(.known_bridges[]; .id == "ordinary_aggregate_construction_gap" and
    .state == "decision_required") and
  any(.known_bridges[]; .id == "legacy_source_fallback" and
    .state == "deprecated_oracle_only") and
  any(.known_bridges[]; .id == "general_failure_routing_gap" and
    .state == "declarative_consumer_only") and
  any(.known_bridges[]; .id == "bounded_serial_failure_flow_bridge" and
    .state == "exact_serial_reference_execution") and
  (.forbidden_during_freeze | index("new_syntax")) != null and
  (.forbidden_during_freeze | index("new_language_semantics")) != null and
  (.forbidden_during_freeze | index("self_hosting")) != null
' "$map" >/dev/null || fail 'authority map is incomplete or contradictory'

test "$(grep -Fc 'return flowmini::parseModule(tokens);' "$root/Lyraform/compiler/src/main.cpp")" -eq 1 ||
    fail 'legacy source fallback is no longer one explicit call site'
grep -Fq 'source-path: legacy-compatibility' "$root/Lyraform/compiler/src/main.cpp" ||
    fail 'legacy compatibility path is not visibly classified'
grep -Fq 'scalar_source::analyze(syntax, bundle)' "$root/Lyraform/compiler/src/main.cpp" ||
    fail 'direct scalar adapter no longer exposes its semantic bridge'
grep -Fq '#include <flowcontracts/scalar_analysis.hpp>' "$root/Lyraform/compiler/include/flowmini_scalar_source.h" ||
    fail 'direct scalar path no longer uses the shared scalar analyzer'
grep -Fq 'namespace flowanalyst { namespace scalar = lyraform::scalar::frontend; }' "$root/Flowanalyst/src/scalar_analysis.hpp" ||
    fail 'Flowanalyst no longer names the shared scalar authority'
grep -Fq 'lyraform.source_operation_coverage' "$root/Flowanalyst/src/main.cpp" ||
    fail 'Flowanalyst no longer emits canonical source-operation accounting'
grep -Fq 'operation.result_symbol = scalar != scalar_facts.end()' "$root/Flowanalyst/src/main.cpp" ||
    fail 'canonical scalar operations no longer consume scalar destination authority'
grep -Fq 'std::string(lyraform::scalar::name(scalar->second.destination_type))' "$root/Flowanalyst/src/main.cpp" ||
    fail 'canonical scalar operation projection no longer consumes scalar type authority'
test "$(grep -Fc 'const int destination = scalar != scalar_facts.end()' "$root/Flowanalyst/src/main.cpp")" -eq 1 ||
    fail 'guard transition evaluation no longer consumes scalar destination authority'
grep -Fq 'accounting.obligation_identity = "operation:"' "$root/Flowanalyst/src/main.cpp" ||
    fail 'TextOutcome producer no longer establishes one obligation identity'
grep -Fq 'accounting.value_type,accounting.obligation_identity' "$root/Flowanalyst/src/main.cpp" ||
    fail 'ownership transfer no longer consumes producer accounting authority'
grep -Fq 'validate_provider_authority(root);' "$root/Flowcontracts/include/flowcontracts/artifacts.hpp" ||
    fail 'canonical semantic reports no longer require exact provider use'
grep -Fq 'validate_provider_authority(flowcontracts::json::object(root))' "$root/Flowbind/src/main.cpp" ||
    fail 'Flowbind no longer consumes shared provider authority validation'
grep -Fq 'operation.source_call_projection = true' "$root/Flowanalyst/src/main.cpp" ||
    fail 'Flowanalyst no longer marks source-call operation authority'
grep -Fq 'validate_call_operation_projection(root);' "$root/Flowcontracts/include/flowcontracts/artifacts.hpp" ||
    fail 'canonical semantic reports no longer validate call projection connectivity'
grep -Fq 'validate_parallel_candidate_projection(root);' "$root/Flowcontracts/include/flowcontracts/artifacts.hpp" ||
    fail 'canonical semantic reports no longer validate deferred parallel candidate connectivity'
grep -Fq 'independent_operation_ids' "$root/Flowanalyst/src/main.cpp" ||
    fail 'Flowanalyst no longer connects parallel candidates to operation identities'
grep -Fq '{"effect_facts", report.effect_facts}' "$root/Flowparallel/src/main.cpp" ||
    fail 'Flowparallel no longer preserves callable-plan-v2 effect authority'
grep -Fq '{"parallel_candidates", plan.parallel_candidates}' "$root/Flowoptimize/src/main.cpp" ||
    fail 'Flowoptimize no longer preserves deferred candidate authority'
grep -Fq 'validate_parallel_candidate_projection(root);' "$root/Flowcontracts/include/flowcontracts/validate.hpp" ||
    fail 'downstream carrying artifacts no longer revalidate candidate authority'
grep -Fq 'validate_targets(root);' "$root/Flowcontracts/include/flowcontracts/validate.hpp" ||
    fail 'optimization artifacts no longer validate the carried target catalog'
grep -Fq 'validate_abi_contracts(root);' "$root/Flowcontracts/include/flowcontracts/validate.hpp" ||
    fail 'optimization artifacts no longer validate carried ABI contracts'
grep -Fq 'plan_version == 2 || !aggregate_layouts.empty()' "$root/Flowlower/src/prepare.cpp" ||
    fail 'backend preparation no longer preserves an explicit empty plan-v2 layout set'
grep -Fq '.lowering_plan.source_operation_coverage.status == "complete"' "$root/tools/igor-run-canonical.sh" ||
    fail 'canonical Igor execution no longer requires complete source-operation accounting'
grep -Fq 'run|run-canonical)' "$root/igor" ||
    fail 'Igor run is no longer routed through canonical execution'
grep -Fq 'run-legacy)' "$root/igor" ||
    fail 'deprecated legacy execution is not explicitly isolated'
grep -Fq 'legacy runtime is deprecated, non-canonical' "$root/igor" ||
    fail 'legacy execution no longer emits its deprecation warning'
grep -Fq 'parameter_projection != "failure_envelope"' "$root/Flowcontracts/include/flowcontracts/failure_consumer.hpp" ||
    fail 'declarative failure consumers no longer require the canonical envelope projection'
grep -Fq '"lyraform.failure_response_transition"' "$root/Flowcontracts/include/flowcontracts/failure_response_transition.hpp" ||
    fail 'declarative failure-response transition authority is missing'
grep -Fq 'transition.response_class == "recover"' "$root/Flowcontracts/include/flowcontracts/failure_response_transition.hpp" ||
    fail 'bounded recovery response-transition validation is missing'
grep -Fq 'transition.response_class == "transform"' "$root/Flowcontracts/include/flowcontracts/failure_response_transition.hpp" ||
    fail 'bounded transformation response-transition validation is missing'
grep -Fq 'transition.origin_commit_law != "preserve_origin_commit"' "$root/Flowcontracts/include/flowcontracts/failure_response_transition.hpp" ||
    fail 'failure-response transitions no longer preserve origin commit evidence'
grep -Fq 'transition.response_attempt_failure_law != "separate_obligations"' "$root/Flowcontracts/include/flowcontracts/failure_response_transition.hpp" ||
    fail 'response-attempt failures no longer remain separate obligations'
grep -Fq '"lyraform.failure_flow_plan"' "$root/Flowcontracts/include/flowcontracts/failure_flow_execution.hpp" ||
    fail 'bounded executable failure-flow plan is missing'
grep -Fq 'plan.schedule != "serial_explicit_failure_route_v1"' "$root/Flowcontracts/include/flowcontracts/failure_flow_execution.hpp" ||
    fail 'bounded failure-flow execution no longer requires its serial schedule'
grep -Fq 'callables.find(route.function)' "$root/Flowcontracts/include/flowcontracts/failure_flow_execution.hpp" ||
    fail 'bounded failure-flow execution no longer dispatches by exact function identity'
grep -Fq 'failure_flow_receipt_refusal(receipt, plan, envelope)' "$root/Flowcontracts/include/flowcontracts/failure_flow_execution.hpp" ||
    fail 'bounded failure-flow execution no longer validates its completion receipt'
grep -Fq 'Status: accepted architectural law' "$root/docs/architecture/decisions/0063-bounded-disposition-evidence-epochs.md" ||
    fail 'bounded evidence-epoch law is missing or no longer accepted'
grep -Fq 'LLVM/TinyVM continue to refuse all member targets' "$root/docs/checkpoints/2026-09-19-v1-bounded-local-member-assignability.md" ||
    fail 'member execution gap is no longer documented'

printf '%s\n' 'Canonical language authority freeze: PASS'
