#include <flowcontracts/fault_containment.hpp>

#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition) {
    if (!condition) throw std::runtime_error("fault-containment regression");
}

flowcontracts::FaultContainmentPlan plan() {
    using namespace flowcontracts;
    return {
        100, "serial_halt_quarantine_v1",
        {10, 20, 21, "ProviderIntegrityFault", "fault", "exactly_one",
         "fault_publishes_no_normal_state", "sensor.flow:4:5"},
        {30, 10, 20, "fault", 40, "fault_envelope",
         "ProviderIntegrityFault"},
        {40, 50, "activation", "ProviderIntegrityFault",
         "halt_and_quarantine", "halted_quarantined", "suppressed", "none",
         "preserve_fault_envelope"},
        {40, "ProviderIntegrityFault", "halt_and_quarantine",
         "safety_profile", "v1"}};
}

flowcontracts::FaultEnvelope envelope() {
    using namespace flowcontracts;
    return {10, 200, 20, "provider-attempt", "correlation-1",
            "ProviderIntegrityFault",
            json::Object{{"provider", "thermometer"}, {"reason", "corrupt"}},
            "no_commit", "sensor.flow:4:5/provider:thermometer"};
}

template <typename Mutation>
void require_plan_refusal(Mutation mutation) {
    auto hostile = plan();
    mutation(hostile);
    require(!flowcontracts::fault_containment_plan_refusal(hostile).empty());
}
}

int main() {
    using namespace flowcontracts;
    try {
        const auto authority = plan();
        require(fault_containment_plan_refusal(authority).empty());
        const auto plan_fact = fault_containment_plan_fact(authority);
        const auto decoded_plan = read_fault_containment_plan(plan_fact, "$.plan");
        require(fault_containment_plan_refusal(decoded_plan).empty());
        require(json::serialize(fault_containment_plan_fact(decoded_plan)) ==
                json::serialize(plan_fact));

        const auto source = envelope();
        const auto envelope_fact = fault_envelope_fact(source);
        const auto decoded_envelope = read_fault_envelope(envelope_fact, "$.fault");
        require(fault_envelope_refusal(decoded_envelope, authority).empty());

        const std::map<long long, FaultContainmentCallable> authorities{{40,
            [](const FaultEnvelope& input,
               const FaultContainmentAuthority& containment) {
                require(input.obligation == 200);
                require(input.fault_type == "ProviderIntegrityFault");
                require(containment.scope == 50);
                return FaultContainmentResult{"contained", 50,
                    "halted_quarantined", "suppressed", "none",
                    "runtime:activation:50:quarantined"};
            }}};
        const auto receipt = execute_fault_containment(
            authority, source, authorities);
        require(receipt.obligation == 200);
        require(receipt.scope == 50);
        require(receipt.scope_state == "halted_quarantined");
        require(receipt.normal_publication == "suppressed");
        require(receipt.continuation == "none");
        require(receipt.obligation_transition == "contained_no_continuation");
        require(fault_containment_receipt_refusal(
                    receipt, authority, source).empty());
        const auto receipt_fact = fault_containment_receipt_fact(receipt);
        const auto decoded_receipt = read_fault_containment_receipt(
            receipt_fact, "$.receipt");
        require(fault_containment_receipt_refusal(
                    decoded_receipt, authority, source).empty());
        require(json::serialize(fault_containment_receipt_fact(decoded_receipt)) ==
                json::serialize(receipt_fact));

        require_plan_refusal([](auto& value) { value.id = -1; });
        require_plan_refusal([](auto& value) { value.schedule = "implicit"; });
        require_plan_refusal([](auto& value) { value.producer.fault_port = "failure"; });
        require_plan_refusal([](auto& value) {
            value.producer.fault_commit_law = "publish_partial";
        });
        require_plan_refusal([](auto& value) { value.wire.authority = 999; });
        require_plan_refusal([](auto& value) { value.wire.to_port = "failure_envelope"; });
        require_plan_refusal([](auto& value) { value.authority.scope_kind = "process"; });
        require_plan_refusal([](auto& value) { value.authority.action = "recover"; });
        require_plan_refusal([](auto& value) { value.authority.scope_state = "running"; });
        require_plan_refusal([](auto& value) {
            value.authority.normal_publication = "allowed";
        });
        require_plan_refusal([](auto& value) { value.authority.continuation = "resume"; });
        require_plan_refusal([](auto& value) { value.authority.evidence_law = "drop"; });
        require_plan_refusal([](auto& value) { value.selection.action = "terminate"; });

        auto hostile_envelope = source;
        hostile_envelope.fault_type = "ReadFailure";
        require(!fault_envelope_refusal(hostile_envelope, authority).empty());
        hostile_envelope = source;
        hostile_envelope.origin_commit = "committed";
        require(!fault_envelope_refusal(hostile_envelope, authority).empty());

        const std::map<long long, FaultContainmentCallable> recovering{{40,
            [](const FaultEnvelope&, const FaultContainmentAuthority&) {
                return FaultContainmentResult{"success", 50, "running",
                    "allowed", "resume", "unsafe"};
            }}};
        try {
            (void)execute_fault_containment(authority, source, recovering);
            require(false);
        } catch (const json::Error&) {}
        const std::map<long long, FaultContainmentCallable> wrong_scope{{40,
            [](const FaultEnvelope&, const FaultContainmentAuthority&) {
                return FaultContainmentResult{"contained", 51,
                    "halted_quarantined", "suppressed", "none", "wrong"};
            }}};
        try {
            (void)execute_fault_containment(authority, source, wrong_scope);
            require(false);
        } catch (const json::Error&) {}
        const std::map<long long, FaultContainmentCallable> throwing{{40,
            [](const FaultEnvelope&,
               const FaultContainmentAuthority&) -> FaultContainmentResult {
                throw std::runtime_error("containment unavailable");
            }}};
        try {
            (void)execute_fault_containment(authority, source, throwing);
            require(false);
        } catch (const json::Error&) {}
        try {
            (void)execute_fault_containment(authority, source, {});
            require(false);
        } catch (const json::Error&) {}

        auto hostile_receipt = receipt;
        hostile_receipt.scope = 51;
        require(!fault_containment_receipt_refusal(
                    hostile_receipt, authority, source).empty());
        hostile_receipt = receipt;
        hostile_receipt.normal_publication = "allowed";
        require(!fault_containment_receipt_refusal(
                    hostile_receipt, authority, source).empty());
        hostile_receipt = receipt;
        hostile_receipt.continuation = "resume";
        require(!fault_containment_receipt_refusal(
                    hostile_receipt, authority, source).empty());

        auto future_plan = json::object(plan_fact);
        future_plan["version"] = 2;
        try {
            (void)read_fault_containment_plan(future_plan, "$.plan");
            require(false);
        } catch (const json::Error&) {}
        auto future_receipt = json::object(receipt_fact);
        future_receipt["version"] = 2;
        try {
            (void)read_fault_containment_receipt(future_receipt, "$.receipt");
            require(false);
        } catch (const json::Error&) {}

        std::cout << "Canonical fault containment: exact activation, halt, quarantine, no publication, no continuation and hostile refusal PASS\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
