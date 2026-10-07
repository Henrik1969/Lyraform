#include <flowcontracts/failure_flow_chain.hpp>

#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition) {
    if (!condition) throw std::runtime_error("failure-flow chain regression");
}

flowcontracts::FailureFlowClosedSetPlan stage(long long plan_id, long long disposition,
                                              long long operation, long long consumer,
                                              long long route, long long function,
                                              long long transition, std::string failure_type,
                                              std::string response_class,
                                              std::string output_type) {
    using namespace flowcontracts;
    const bool recover = response_class == "recover";
    FailureFlowClosedSetPlan result;
    result.id = plan_id;
    result.schedule = "serial_closed_failure_set_v1";
    result.producer = {disposition, operation, operation + 1, "Reading",
                       {failure_type}, "failure", "exactly_one",
                       "failure_publishes_no_normal_state", "sensor.flow:4:5"};
    result.wires = {{disposition + 100, disposition, operation, "failure",
                     consumer, route, "failure_envelope", failure_type}};
    result.consumer = {consumer, consumer + 1, {failure_type},
                       {{route, failure_type, function}}};
    result.functions = {{function, "failure_envelope", failure_type,
                         output_type, "definition"}};
    result.selections = {{consumer, failure_type, route, "chain_policy", "v1"}};
    result.transitions = {{transition, consumer, route, function, failure_type,
                           "failure_envelope", response_class,
                           recover ? "success" : "failure", output_type,
                           recover ? "close_original" : "linked_successor",
                           "preserve_origin_commit", "link_response_to_origin",
                           "separate_obligations"}};
    return result;
}

flowcontracts::FailureFlowTwoStageChain chain() {
    using namespace flowcontracts;
    FailureFlowTwoStageChain result;
    result.id = 500;
    result.schedule = "serial_transform_then_recover_v1";
    result.first = stage(100, 10, 20, 30, 40, 50, 60,
                         "ReadFailure", "transform", "SensorUnavailable");
    result.second = stage(101, 11, 21, 31, 41, 51, 61,
                          "SensorUnavailable", "recover", "Reading");
    result.link = {70, 100, "ReadFailure", 60, 101, 11, 21,
                   "SensorUnavailable", "preserve_correlation",
                   "response_payload_to_failure_payload", "preserve_no_commit",
                   "response_provenance_to_successor_origin"};
    return result;
}

flowcontracts::FailureEnvelope origin() {
    using namespace flowcontracts;
    return {10, 200, 20, "origin-attempt", "correlation-1", "ReadFailure",
            json::Object{{"code", "provider_unavailable"}}, "no_commit",
            "sensor.flow:4:5/provider:thermometer"};
}

template <typename Mutation>
void require_refusal(Mutation mutation) {
    auto hostile = chain();
    mutation(hostile);
    require(!flowcontracts::failure_flow_chain_refusal(hostile).empty());
}
}

int main() {
    using namespace flowcontracts;
    try {
        const auto authority = chain();
        require(failure_flow_chain_refusal(authority).empty());
        const auto fact = failure_flow_chain_fact(authority);
        const auto decoded = read_failure_flow_chain(fact, "$.chain");
        require(failure_flow_chain_refusal(decoded).empty());
        require(json::serialize(failure_flow_chain_fact(decoded)) == json::serialize(fact));

        const std::map<long long, FailureResponseCallable> first{{50,
            [](const FailureEnvelope&) {
                return FailureResponseResult{
                    "failure", "SensorUnavailable",
                    json::Object{{"source", "thermometer"}},
                    "poll_sensor:transform"};
            }}};
        const std::map<long long, FailureResponseCallable> second{{51,
            [](const FailureEnvelope& input) {
                require(input.disposition == 11 && input.obligation == 201);
                require(input.producer_operation == 21);
                require(input.attempt == "transform-attempt");
                require(input.correlation == "correlation-1");
                require(input.failure_type == "SensorUnavailable");
                require(input.origin_commit == "no_commit");
                require(input.provenance == "poll_sensor:transform");
                return FailureResponseResult{"success", "Reading", 21,
                                             "use_cached_reading:recover"};
            }}};
        const auto source = origin();
        const auto receipt = execute_failure_flow_chain(
            authority, source, "transform-attempt", 201,
            "recovery-attempt", first, second);
        require(receipt.transformed.successor_obligation == 201);
        require(receipt.recovered.original_obligation == 201);
        require(receipt.recovered.outgoing_disposition == "success");
        require(receipt.recovered.successor_obligation == -1);
        require(failure_flow_chain_receipt_refusal(receipt, authority, source).empty());

        require_refusal([](auto& value) { value.id = -1; });
        require_refusal([](auto& value) { value.schedule = "unbounded"; });
        require_refusal([](auto& value) { value.second.id = value.first.id; });
        require_refusal([](auto& value) { value.link.from_plan = 999; });
        require_refusal([](auto& value) { value.link.from_transition = 999; });
        require_refusal([](auto& value) { value.link.to_plan = 999; });
        require_refusal([](auto& value) { value.link.successor_disposition = 999; });
        require_refusal([](auto& value) { value.link.successor_operation = 999; });
        require_refusal([](auto& value) { value.link.successor_failure_type = "Other"; });
        require_refusal([](auto& value) { value.link.correlation_law = "replace"; });
        require_refusal([](auto& value) { value.link.payload_law = "drop"; });
        require_refusal([](auto& value) { value.link.commit_law = "commit"; });
        require_refusal([](auto& value) { value.link.provenance_law = "erase"; });
        require_refusal([](auto& value) {
            value.first.transitions.front().response_class = "recover";
        });
        require_refusal([](auto& value) {
            value.second.transitions.front().response_class = "transform";
            value.second.transitions.front().outgoing_disposition = "failure";
            value.second.transitions.front().obligation_transition = "linked_successor";
        });

        auto hostile_receipt = receipt;
        hostile_receipt.recovered.original_obligation = 999;
        require(!failure_flow_chain_receipt_refusal(
                    hostile_receipt, authority, source).empty());
        hostile_receipt = receipt;
        hostile_receipt.correlation = "other";
        require(!failure_flow_chain_receipt_refusal(
                    hostile_receipt, authority, source).empty());
        auto wrong_origin = source;
        wrong_origin.failure_type = "Other";
        try {
            (void)execute_failure_flow_chain(authority, wrong_origin,
                                             "transform-attempt", 201,
                                             "recovery-attempt", first, second);
            require(false);
        } catch (const json::Error&) {}

        auto future = json::object(fact);
        future["version"] = 2;
        try {
            (void)read_failure_flow_chain(future, "$.chain");
            require(false);
        } catch (const json::Error&) {}

        std::cout << "Canonical two-stage failure flow: explicit transform handoff, successor evidence, bounded recovery closure and hostile refusal PASS\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
