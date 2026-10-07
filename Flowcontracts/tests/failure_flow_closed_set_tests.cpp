#include <flowcontracts/failure_flow_closed_set.hpp>

#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition) {
    if (!condition) throw std::runtime_error("closed-set failure-flow regression");
}

flowcontracts::FailureFlowClosedSetPlan plan() {
    using namespace flowcontracts;
    FailureFlowClosedSetPlan result;
    result.id = 100;
    result.schedule = "serial_closed_failure_set_v1";
    result.producer = {10, 20, 30, "SensorReading",
                       {"ReadFailure", "CredentialFailure"}, "failure",
                       "exactly_one", "failure_publishes_no_normal_state",
                       "sensor.flow:4:5"};
    result.wires = {
        {40, 10, 20, "failure", 50, 60, "failure_envelope", "ReadFailure"},
        {41, 10, 20, "failure", 50, 62, "failure_envelope", "CredentialFailure"}};
    result.consumer = {
        50, 70, {"ReadFailure", "CredentialFailure"},
        {{60, "ReadFailure", 80}, {61, "ReadFailure", 81},
         {62, "CredentialFailure", 82}}};
    result.functions = {
        {80, "failure_envelope", "ReadFailure", "SensorReading", "definition"},
        {81, "failure_envelope", "ReadFailure", "SensorReading", "definition"},
        {82, "failure_envelope", "CredentialFailure", "AuthenticationUnavailable",
         "definition"}};
    result.selections = {
        {50, "ReadFailure", 60, "laboratory_policy", "v1"},
        {50, "CredentialFailure", 62, "laboratory_policy", "v1"}};
    result.transitions = {
        {90, 50, 60, 80, "ReadFailure", "failure_envelope", "recover", "success",
         "SensorReading", "close_original", "preserve_origin_commit",
         "link_response_to_origin", "separate_obligations"},
        {91, 50, 61, 81, "ReadFailure", "failure_envelope", "recover", "success",
         "SensorReading", "close_original", "preserve_origin_commit",
         "link_response_to_origin", "separate_obligations"},
        {92, 50, 62, 82, "CredentialFailure", "failure_envelope", "transform",
         "failure", "AuthenticationUnavailable", "linked_successor",
         "preserve_origin_commit", "link_response_to_origin",
         "separate_obligations"}};
    return result;
}

flowcontracts::FailureEnvelope envelope(std::string type) {
    using namespace flowcontracts;
    return {10, type == "ReadFailure" ? 200 : 201, 20,
            type == "ReadFailure" ? "attempt-read" : "attempt-credential",
            "correlation-1", std::move(type),
            json::Object{{"code", "unavailable"}}, "no_commit",
            "sensor.flow:4:5/provider:thermometer"};
}

template <typename Mutation>
void require_plan_refusal(Mutation mutation) {
    auto hostile = plan();
    mutation(hostile);
    require(!flowcontracts::failure_flow_closed_set_plan_refusal(hostile).empty());
}
}

int main() {
    using namespace flowcontracts;
    try {
        const auto authority = plan();
        require(failure_flow_closed_set_plan_refusal(authority).empty());
        const auto encoded = failure_flow_closed_set_plan_fact(authority);
        const auto decoded = read_failure_flow_closed_set_plan(encoded, "$.plan");
        require(failure_flow_closed_set_plan_refusal(decoded).empty());
        require(json::serialize(failure_flow_closed_set_plan_fact(decoded)) ==
                json::serialize(encoded));

        bool unselected_called = false;
        const std::map<long long, FailureResponseCallable> callables{
            {80, [](const FailureEnvelope& input) {
                require(input.failure_type == "ReadFailure");
                return FailureResponseResult{"success", "SensorReading", 21,
                                             "use_safe_reading:response-1"};
            }},
            {81, [&](const FailureEnvelope&) {
                unselected_called = true;
                return FailureResponseResult{"success", "SensorReading", 22,
                                             "alternate:must-not-run"};
            }},
            {82, [](const FailureEnvelope& input) {
                require(input.failure_type == "CredentialFailure");
                return FailureResponseResult{
                    "failure", "AuthenticationUnavailable",
                    json::Object{{"source", "credential-provider"}},
                    "reject_credential:response-2"};
            }}};

        const auto read_failure = envelope("ReadFailure");
        const auto recovered = execute_failure_flow(
            authority, read_failure, "response-attempt-1", -1, callables);
        require(recovered.wire == 40 && recovered.route == 60 && recovered.function == 80);
        require(recovered.transition == 90 && recovered.outgoing_disposition == "success");
        require(recovered.successor_obligation == -1 && !unselected_called);
        require(failure_flow_receipt_refusal(recovered, authority, read_failure).empty());

        const auto credential_failure = envelope("CredentialFailure");
        const auto transformed = execute_failure_flow(
            authority, credential_failure, "response-attempt-2", 202, callables);
        require(transformed.wire == 41 && transformed.route == 62 &&
                transformed.function == 82 && transformed.transition == 92);
        require(transformed.outgoing_disposition == "failure");
        require(transformed.successor_obligation == 202 && !unselected_called);
        require(failure_flow_receipt_refusal(
                    transformed, authority, credential_failure).empty());

        require_plan_refusal([](auto& value) { value.producer.failure_types.clear(); });
        require_plan_refusal([](auto& value) {
            value.producer.failure_types.push_back("ReadFailure");
        });
        require_plan_refusal([](auto& value) {
            std::swap(value.producer.failure_types[0], value.producer.failure_types[1]);
        });
        require_plan_refusal([](auto& value) { value.consumer.accepted_failure_types.pop_back(); });
        require_plan_refusal([](auto& value) { value.wires.pop_back(); });
        require_plan_refusal([](auto& value) { value.selections.pop_back(); });
        require_plan_refusal([](auto& value) { value.wires[1].id = value.wires[0].id; });
        require_plan_refusal([](auto& value) { value.wires[0].failure_type = "OtherFailure"; });
        require_plan_refusal([](auto& value) { value.wires[0].route = 61; });
        require_plan_refusal([](auto& value) { value.selections[0].route = 61; });
        require_plan_refusal([](auto& value) { value.transitions.pop_back(); });
        require_plan_refusal([](auto& value) { value.functions.pop_back(); });
        require_plan_refusal([](auto& value) {
            value.functions.push_back({99, "failure_envelope", "ReadFailure",
                                       "SensorReading", "definition"});
        });

        auto unknown = envelope("OtherFailure");
        require(!failure_envelope_refusal(unknown, authority).empty());
        try {
            (void)execute_failure_flow(authority, unknown, "response-attempt-3", -1,
                                       callables);
            require(false);
        } catch (const json::Error&) {}
        auto missing_callable = callables;
        missing_callable.erase(82);
        try {
            (void)execute_failure_flow(authority, credential_failure,
                                       "response-attempt-4", 203, missing_callable);
            require(false);
        } catch (const json::Error&) {}

        auto v1 = json::object(encoded);
        v1["version"] = 1;
        try {
            (void)read_failure_flow_closed_set_plan(v1, "$.plan");
            require(false);
        } catch (const json::Error&) {}
        auto missing = json::object(encoded);
        missing.erase("policy_selections");
        try {
            (void)read_failure_flow_closed_set_plan(missing, "$.plan");
            require(false);
        } catch (const json::Error&) {}

        std::cout << "Canonical closed-set serial failure flow: complete typed coverage, exact policy selection, recover/transform dispatch and hostile refusal PASS\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
