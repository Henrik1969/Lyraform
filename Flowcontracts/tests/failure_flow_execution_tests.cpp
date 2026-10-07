#include <flowcontracts/failure_flow_execution.hpp>

#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition) {
    if (!condition) throw std::runtime_error("failure-flow execution regression");
}

flowcontracts::FailureFlowPlan plan(std::string response_class) {
    using namespace flowcontracts;
    const bool recover = response_class == "recover";
    FailureFlowPlan result;
    result.id = recover ? 100 : 101;
    result.schedule = "serial_explicit_failure_route_v1";
    result.producer = {10, 20, 30, "SensorReading", "ReadFailure", "failure",
                       "exactly_one", "failure_publishes_no_normal_state",
                       "sensor.flow:4:5"};
    result.wire = {40, 10, 20, "failure", 50, 60, "failure_envelope", "ReadFailure"};
    result.consumer = {50, 70, {"ReadFailure"}, {{60, "ReadFailure", 80}}};
    result.functions = {{80, "failure_envelope", "ReadFailure",
                         recover ? "SensorReading" : "SensorUnavailable", "definition"}};
    result.selection = {50, "ReadFailure", 60, "laboratory_policy", "v1"};
    result.transitions = {{90, 50, 60, 80, "ReadFailure", "failure_envelope",
                           response_class, recover ? "success" : "failure",
                           recover ? "SensorReading" : "SensorUnavailable",
                           recover ? "close_original" : "linked_successor",
                           "preserve_origin_commit", "link_response_to_origin",
                           "separate_obligations"}};
    return result;
}

flowcontracts::FailureEnvelope envelope() {
    using namespace flowcontracts;
    return {10, 200, 20, "attempt-1", "correlation-1", "ReadFailure",
            json::Object{{"code", "provider_unavailable"}}, "no_commit",
            "sensor.flow:4:5/provider:thermometer"};
}

template <typename Mutation>
void require_plan_refusal(Mutation mutation) {
    auto hostile = plan("recover");
    mutation(hostile);
    require(!flowcontracts::failure_flow_plan_refusal(hostile).empty());
}

template <typename Mutation>
void require_envelope_refusal(Mutation mutation) {
    const auto authority = plan("recover");
    auto hostile = envelope();
    mutation(hostile);
    require(!flowcontracts::failure_envelope_refusal(hostile, authority).empty());
}
}

int main() {
    using namespace flowcontracts;
    try {
        const auto recover_plan = plan("recover");
        require(failure_flow_plan_refusal(recover_plan).empty());
        const auto encoded_plan = failure_flow_plan_fact(recover_plan);
        const auto decoded_plan = read_failure_flow_plan(encoded_plan, "$.plan");
        require(failure_flow_plan_refusal(decoded_plan).empty());
        require(json::serialize(failure_flow_plan_fact(decoded_plan)) ==
                json::serialize(encoded_plan));

        const auto failure = envelope();
        const auto encoded_envelope = failure_envelope_fact(failure);
        const auto decoded_envelope = read_failure_envelope(encoded_envelope, "$.envelope");
        require(failure_envelope_refusal(decoded_envelope, recover_plan).empty());
        require(json::serialize(failure_envelope_fact(decoded_envelope)) ==
                json::serialize(encoded_envelope));

        const std::map<long long, FailureResponseCallable> recover_callables{{80,
            [](const FailureEnvelope& input) {
                require(input.failure_type == "ReadFailure");
                return FailureResponseResult{"success", "SensorReading", 21,
                                             "use_safe_reading:response-1"};
            }}};
        const auto recovered = execute_failure_flow(
            recover_plan, failure, "response-attempt-1", -1, recover_callables);
        require(recovered.outgoing_disposition == "success");
        require(recovered.outgoing_payload_type == "SensorReading");
        require(recovered.successor_obligation == -1);
        require(recovered.obligation_transition == "close_original");
        require(recovered.original_obligation == failure.obligation);
        require(recovered.origin_commit == failure.origin_commit);
        require(recovered.origin_provenance == failure.provenance);
        const auto recovery_receipt_fact = failure_flow_receipt_fact(recovered);
        const auto recovery_receipt = read_failure_flow_receipt(
            recovery_receipt_fact, "$.receipt");
        require(failure_flow_receipt_refusal(
                    recovery_receipt, recover_plan, failure).empty());
        require(json::serialize(failure_flow_receipt_fact(recovery_receipt)) ==
                json::serialize(recovery_receipt_fact));

        const auto transform_plan = plan("transform");
        require(failure_flow_plan_refusal(transform_plan).empty());
        const std::map<long long, FailureResponseCallable> transform_callables{{80,
            [](const FailureEnvelope&) {
                return FailureResponseResult{
                    "failure", "SensorUnavailable",
                    json::Object{{"source", "thermometer"}},
                    "poll_sensor:response-2"};
            }}};
        const auto transformed = execute_failure_flow(
            transform_plan, failure, "response-attempt-2", 201, transform_callables);
        require(transformed.outgoing_disposition == "failure");
        require(transformed.outgoing_payload_type == "SensorUnavailable");
        require(transformed.successor_obligation == 201);
        require(transformed.obligation_transition == "linked_successor");
        require(transformed.original_obligation == failure.obligation);
        require(failure_flow_receipt_refusal(
                    transformed, transform_plan, failure).empty());

        auto hostile_receipt = recovered;
        hostile_receipt.plan = 999;
        require(!failure_flow_receipt_refusal(
                    hostile_receipt, recover_plan, failure).empty());
        hostile_receipt = recovered;
        hostile_receipt.original_obligation = 999;
        require(!failure_flow_receipt_refusal(
                    hostile_receipt, recover_plan, failure).empty());
        hostile_receipt = recovered;
        hostile_receipt.outgoing_disposition = "failure";
        require(!failure_flow_receipt_refusal(
                    hostile_receipt, recover_plan, failure).empty());
        hostile_receipt = recovered;
        hostile_receipt.successor_obligation = 201;
        require(!failure_flow_receipt_refusal(
                    hostile_receipt, recover_plan, failure).empty());
        hostile_receipt = transformed;
        hostile_receipt.successor_obligation = hostile_receipt.original_obligation;
        require(!failure_flow_receipt_refusal(
                    hostile_receipt, transform_plan, failure).empty());

        auto future_receipt = json::object(recovery_receipt_fact);
        future_receipt["version"] = 2;
        try {
            (void)read_failure_flow_receipt(future_receipt, "$.receipt");
            require(false);
        } catch (const json::Error&) {}

        require_plan_refusal([](auto& value) { value.id = -1; });
        require_plan_refusal([](auto& value) { value.schedule = "parallel"; });
        require_plan_refusal([](auto& value) { value.producer.failure_type = "OtherFailure"; });
        require_plan_refusal([](auto& value) { value.producer.failure_commit_law = "partial"; });
        require_plan_refusal([](auto& value) { value.wire.from_port = "diagnostic"; });
        require_plan_refusal([](auto& value) { value.wire.to_port = "payload"; });
        require_plan_refusal([](auto& value) { value.wire.consumer = 51; });
        require_plan_refusal([](auto& value) { value.wire.route = 61; });
        require_plan_refusal([](auto& value) { value.wire.failure_type = "OtherFailure"; });
        require_plan_refusal([](auto& value) { value.selection.route = 61; });
        require_plan_refusal([](auto& value) { value.transitions.front().function = 81; });
        require_plan_refusal([](auto& value) { value.consumer.routes.clear(); });
        require_plan_refusal([](auto& value) { value.functions.clear(); });
        require_plan_refusal([](auto& value) { value.transitions.clear(); });
        require_plan_refusal([](auto& value) {
            value.consumer.routes.push_back(value.consumer.routes.front());
        });

        require_envelope_refusal([](auto& value) { value.disposition = 11; });
        require_envelope_refusal([](auto& value) { value.obligation = -1; });
        require_envelope_refusal([](auto& value) { value.producer_operation = 21; });
        require_envelope_refusal([](auto& value) { value.attempt.clear(); });
        require_envelope_refusal([](auto& value) { value.correlation.clear(); });
        require_envelope_refusal([](auto& value) { value.failure_type = "OtherFailure"; });
        require_envelope_refusal([](auto& value) { value.origin_commit = "partial"; });
        require_envelope_refusal([](auto& value) { value.provenance.clear(); });

        try {
            (void)execute_failure_flow(recover_plan, failure, "response-attempt-3", -1, {});
            require(false);
        } catch (const json::Error&) {}
        try {
            const std::map<long long, FailureResponseCallable> wrong{{80,
                [](const FailureEnvelope&) {
                    return FailureResponseResult{"failure", "SensorReading", 0, "response"};
                }}};
            (void)execute_failure_flow(recover_plan, failure, "response-attempt-4", -1, wrong);
            require(false);
        } catch (const json::Error&) {}
        try {
            (void)execute_failure_flow(recover_plan, failure, "response-attempt-5", 201,
                                       recover_callables);
            require(false);
        } catch (const json::Error&) {}
        try {
            (void)execute_failure_flow(transform_plan, failure, "response-attempt-6", -1,
                                       transform_callables);
            require(false);
        } catch (const json::Error&) {}
        try {
            const std::map<long long, FailureResponseCallable> throwing{{80,
                [](const FailureEnvelope&) -> FailureResponseResult {
                    throw std::runtime_error("not a Lyraform failure");
                }}};
            (void)execute_failure_flow(recover_plan, failure, "response-attempt-7", -1,
                                       throwing);
            require(false);
        } catch (const json::Error&) {}

        auto non_executable = json::object(encoded_plan);
        non_executable["status"] = "declarative";
        try {
            (void)read_failure_flow_plan(non_executable, "$.plan");
            require(false);
        } catch (const json::Error&) {}
        auto future = json::object(encoded_plan);
        future["version"] = 2;
        try {
            (void)read_failure_flow_plan(future, "$.plan");
            require(false);
        } catch (const json::Error&) {}
        auto missing_wire = json::object(encoded_plan);
        missing_wire.erase("wire");
        try {
            (void)read_failure_flow_plan(missing_wire, "$.plan");
            require(false);
        } catch (const json::Error&) {}

        std::cout << "Canonical serial failure flow: recover, transform, explicit wire, exact identity dispatch and hostile refusal PASS\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
