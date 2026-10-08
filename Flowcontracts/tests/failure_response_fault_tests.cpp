#include <flowcontracts/failure_response_fault.hpp>

#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition) {
    if (!condition) throw std::runtime_error("response-fault containment regression");
}

flowcontracts::FailureFlowClosedSetPlan original_plan() {
    using namespace flowcontracts;
    FailureFlowClosedSetPlan result;
    result.id = 100;
    result.schedule = "serial_closed_failure_set_v1";
    result.producer = {10, 20, 21, "Reading", {"ReadFailure"}, "failure",
                       "exactly_one", "failure_publishes_no_normal_state",
                       "sensor.flow:4:5"};
    result.wires = {{110, 10, 20, "failure", 30, 40,
                     "failure_envelope", "ReadFailure"}};
    result.consumer = {30, 31, {"ReadFailure"}, {{40, "ReadFailure", 50}}};
    result.functions = {{50, "failure_envelope", "ReadFailure",
                         "Reading", "definition"}};
    result.selections = {{30, "ReadFailure", 40, "recovery_policy", "v1"}};
    result.transitions = {{60, 30, 40, 50, "ReadFailure", "failure_envelope",
                           "recover", "success", "Reading", "close_original",
                           "preserve_origin_commit", "link_response_to_origin",
                           "separate_obligations"}};
    return result;
}

flowcontracts::FaultContainmentPlan containment_plan() {
    using namespace flowcontracts;
    return {
        101, "serial_halt_quarantine_v1",
        {11, 22, 50, "ProviderIntegrityFault", "fault", "exactly_one",
         "fault_publishes_no_normal_state", "use_cache:response"},
        {111, 11, 22, "fault", 41, "fault_envelope", "ProviderIntegrityFault"},
        {41, 51, "activation", "ProviderIntegrityFault",
         "halt_and_quarantine", "halted_quarantined", "suppressed", "none",
         "preserve_fault_envelope"},
        {41, "ProviderIntegrityFault", "halt_and_quarantine",
         "safety_profile", "v1"}};
}

flowcontracts::FailureResponseFaultPlan plan() {
    using namespace flowcontracts;
    return {700, "serial_response_fault_containment_v1", original_plan(),
            {70, 100, "ReadFailure", 40, 50, 60, 101, 11, 22,
             "ProviderIntegrityFault", "preserve_unresolved_in_contained_scope",
             "distinct_linked_obligation", "preserve_correlation",
             "response_fault_publishes_no_transition",
             "response_fault_provenance_to_fault_origin",
             "halt_quarantine_no_rejoin"},
            containment_plan()};
}

flowcontracts::FailureEnvelope origin() {
    using namespace flowcontracts;
    return {10, 200, 20, "read-attempt", "correlation-1", "ReadFailure",
            json::Object{{"reason", "sensor_unavailable"}}, "no_commit",
            "sensor.flow:4:5/provider:thermometer"};
}

template <typename Mutation>
void require_plan_refusal(Mutation mutation) {
    auto hostile = plan();
    mutation(hostile);
    require(!flowcontracts::failure_response_fault_plan_refusal(hostile).empty());
}
}

int main() {
    using namespace flowcontracts;
    try {
        const auto authority = plan();
        require(failure_response_fault_plan_refusal(authority).empty());
        const auto fact = failure_response_fault_plan_fact(authority);
        const auto decoded = read_failure_response_fault_plan(fact, "$.plan");
        require(failure_response_fault_plan_refusal(decoded).empty());
        require(json::serialize(failure_response_fault_plan_fact(decoded)) ==
                json::serialize(fact));

        const std::map<long long, FailureResponseCallable> responses{{50,
            [](const FailureEnvelope& input) {
                require(input.obligation == 200);
                return FailureResponseResult{
                    "fault", "ProviderIntegrityFault",
                    json::Object{{"provider", "cache"}, {"reason", "corrupt"}},
                    "use_cache:response:fault"};
            }}};
        const std::map<long long, FaultContainmentCallable> containment{{41,
            [](const FaultEnvelope& fault,
               const FaultContainmentAuthority& authority) {
                require(fault.obligation == 201);
                require(fault.attempt == "cache-response-attempt");
                require(fault.correlation == "correlation-1");
                require(fault.provenance == "use_cache:response:fault");
                require(authority.scope == 51);
                return FaultContainmentResult{"contained", 51,
                    "halted_quarantined", "suppressed", "none",
                    "runtime:activation:51:quarantined"};
            }}};
        const auto source = origin();
        const auto receipt = execute_failure_response_fault_containment(
            authority, source, "cache-response-attempt", 201,
            responses, containment);
        require(receipt.original_obligation == 200);
        require(receipt.fault_obligation == 201);
        require(receipt.original_obligation_state == "unresolved_contained");
        require(receipt.final_state == "contained_fault_no_rejoin");
        require(receipt.normal_publication == "suppressed");
        require(receipt.continuation == "none" && receipt.rejoin == "none");
        require(failure_response_fault_receipt_refusal(
                    receipt, authority, source).empty());

        require_plan_refusal([](auto& value) { value.id = -1; });
        require_plan_refusal([](auto& value) { value.schedule = "recover_fault"; });
        require_plan_refusal([](auto& value) { value.link.original_plan = 999; });
        require_plan_refusal([](auto& value) { value.link.original_function = 999; });
        require_plan_refusal([](auto& value) { value.link.containment_plan = 999; });
        require_plan_refusal([](auto& value) { value.link.fault_disposition = 999; });
        require_plan_refusal([](auto& value) { value.link.fault_operation = 999; });
        require_plan_refusal([](auto& value) { value.link.fault_type = "Other"; });
        require_plan_refusal([](auto& value) {
            value.containment.producer.function = 999;
        });
        require_plan_refusal([](auto& value) {
            value.link.original_obligation_law = "close";
        });
        require_plan_refusal([](auto& value) { value.link.fault_obligation_law = "reuse"; });
        require_plan_refusal([](auto& value) { value.link.correlation_law = "replace"; });
        require_plan_refusal([](auto& value) { value.link.commit_law = "publish"; });
        require_plan_refusal([](auto& value) { value.link.provenance_law = "erase"; });
        require_plan_refusal([](auto& value) { value.link.containment_law = "resume"; });

        try {
            (void)execute_failure_response_fault_containment(
                authority, source, "cache-response-attempt", 200,
                responses, containment);
            require(false);
        } catch (const json::Error&) {}
        const std::map<long long, FailureResponseCallable> expected_failure{{50,
            [](const FailureEnvelope&) {
                return FailureResponseResult{"failure", "CacheUnavailable",
                                             "offline", "cache:failure"};
            }}};
        try {
            (void)execute_failure_response_fault_containment(
                authority, source, "cache-response-attempt", 201,
                expected_failure, containment);
            require(false);
        } catch (const json::Error&) {}
        const std::map<long long, FailureResponseCallable> success{{50,
            [](const FailureEnvelope&) {
                return FailureResponseResult{"success", "Reading", 21,
                                             "cache:success"};
            }}};
        try {
            (void)execute_failure_response_fault_containment(
                authority, source, "cache-response-attempt", 201,
                success, containment);
            require(false);
        } catch (const json::Error&) {}

        auto hostile_receipt = receipt;
        hostile_receipt.original_obligation_state = "closed";
        require(!failure_response_fault_receipt_refusal(
                    hostile_receipt, authority, source).empty());
        hostile_receipt = receipt;
        hostile_receipt.rejoin = "resume";
        require(!failure_response_fault_receipt_refusal(
                    hostile_receipt, authority, source).empty());
        hostile_receipt = receipt;
        hostile_receipt.fault_envelope.obligation = 200;
        require(!failure_response_fault_receipt_refusal(
                    hostile_receipt, authority, source).empty());

        auto future = json::object(fact);
        future["version"] = 2;
        try {
            (void)read_failure_response_fault_plan(future, "$.plan");
            require(false);
        } catch (const json::Error&) {}

        std::cout << "Canonical response fault: distinct fault obligation, exact containment, unresolved original, no publication and no rejoin PASS\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
