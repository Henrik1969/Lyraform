#pragma once

#include <flowcontracts/failure_response_transition.hpp>

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace flowcontracts {

struct FailureProducerDisposition {
    long long id = -1;
    long long operation = -1;
    long long function = -1;
    std::string success_type;
    std::string failure_type;
    std::string failure_port;
    std::string completion;
    std::string failure_commit_law;
    std::string provenance;
};

struct FailureFlowWire {
    long long id = -1;
    long long producer_disposition = -1;
    long long producer_operation = -1;
    std::string from_port;
    long long consumer = -1;
    long long route = -1;
    std::string to_port;
    std::string failure_type;
};

struct FailureFlowPlan {
    long long id = -1;
    std::string schedule;
    FailureProducerDisposition producer;
    FailureFlowWire wire;
    FailureConsumer consumer;
    std::vector<FailureConsumerFunction> functions;
    FailurePolicySelection selection;
    std::vector<FailureResponseTransition> transitions;
};

struct FailureEnvelope {
    long long disposition = -1;
    long long obligation = -1;
    long long producer_operation = -1;
    std::string attempt;
    std::string correlation;
    std::string failure_type;
    json::Value payload;
    std::string origin_commit;
    std::string provenance;
};

struct FailureResponseResult {
    std::string disposition;
    std::string payload_type;
    json::Value payload;
    std::string provenance;
};

struct FailureFlowReceipt {
    long long plan = -1;
    long long wire = -1;
    long long consumer = -1;
    long long route = -1;
    long long function = -1;
    long long transition = -1;
    long long original_obligation = -1;
    long long successor_obligation = -1;
    std::string response_attempt;
    std::string correlation;
    std::string incoming_failure_type;
    std::string outgoing_disposition;
    std::string outgoing_payload_type;
    json::Value outgoing_payload;
    std::string obligation_transition;
    std::string origin_commit;
    std::string origin_provenance;
    std::string response_provenance;
};

using FailureResponseCallable =
    std::function<FailureResponseResult(const FailureEnvelope&)>;

inline json::Value failure_flow_plan_fact(const FailureFlowPlan& plan) {
    using namespace json;
    Array functions;
    for (const auto& function : plan.functions)
        functions.emplace_back(Object{{"function_symbol_id", Integer(function.id)},
                                      {"parameter_projection", function.parameter_projection},
                                      {"failure_payload_type", function.failure_payload_type},
                                      {"result_type", function.result_type},
                                      {"availability", function.availability}});
    Array transitions;
    for (const auto& transition : plan.transitions)
        transitions.emplace_back(failure_response_transition_fact(transition));
    return Object{
        {"format", "lyraform.failure_flow_plan"}, {"version", Integer{1}},
        {"status", "ready"}, {"plan_id", Integer(plan.id)},
        {"schedule", plan.schedule},
        {"producer", Object{{"disposition_id", Integer(plan.producer.id)},
                            {"operation_id", Integer(plan.producer.operation)},
                            {"function_symbol_id", Integer(plan.producer.function)},
                            {"success_type", plan.producer.success_type},
                            {"failure_type", plan.producer.failure_type},
                            {"failure_port", plan.producer.failure_port},
                            {"completion", plan.producer.completion},
                            {"failure_commit_law", plan.producer.failure_commit_law},
                            {"provenance", plan.producer.provenance}}},
        {"wire", Object{{"wire_id", Integer(plan.wire.id)},
                        {"producer_disposition_id", Integer(plan.wire.producer_disposition)},
                        {"producer_operation_id", Integer(plan.wire.producer_operation)},
                        {"from_port", plan.wire.from_port},
                        {"consumer_id", Integer(plan.wire.consumer)},
                        {"route_id", Integer(plan.wire.route)},
                        {"to_port", plan.wire.to_port},
                        {"failure_type", plan.wire.failure_type}}},
        {"consumer", failure_consumer_fact(plan.consumer)},
        {"functions", std::move(functions)},
        {"policy_selection", failure_policy_selection_fact(plan.selection)},
        {"transitions", std::move(transitions)}};
}

inline FailureFlowPlan read_failure_flow_plan(const json::Value& raw,
                                              const std::string& path) {
    using namespace json;
    const auto& root = object(raw, path);
    auto number = [&](const Object& value, std::string_view key,
                      const std::string& owner) {
        return integer(required(value, key, owner), owner + "." + std::string(key));
    };
    auto text = [&](const Object& value, std::string_view key,
                    const std::string& owner) -> const std::string& {
        return string(required(value, key, owner), owner + "." + std::string(key));
    };
    if (text(root, "format", path) != "lyraform.failure_flow_plan" ||
        number(root, "version", path) != 1 || text(root, "status", path) != "ready")
        throw Error(path, "unsupported or non-executable failure-flow plan");

    FailureFlowPlan plan;
    plan.id = number(root, "plan_id", path);
    plan.schedule = text(root, "schedule", path);
    const auto producer_path = path + ".producer";
    const auto& producer = object(required(root, "producer", path), producer_path);
    plan.producer = {number(producer, "disposition_id", producer_path),
                     number(producer, "operation_id", producer_path),
                     number(producer, "function_symbol_id", producer_path),
                     text(producer, "success_type", producer_path),
                     text(producer, "failure_type", producer_path),
                     text(producer, "failure_port", producer_path),
                     text(producer, "completion", producer_path),
                     text(producer, "failure_commit_law", producer_path),
                     text(producer, "provenance", producer_path)};
    const auto wire_path = path + ".wire";
    const auto& wire = object(required(root, "wire", path), wire_path);
    plan.wire = {number(wire, "wire_id", wire_path),
                 number(wire, "producer_disposition_id", wire_path),
                 number(wire, "producer_operation_id", wire_path),
                 text(wire, "from_port", wire_path),
                 number(wire, "consumer_id", wire_path),
                 number(wire, "route_id", wire_path),
                 text(wire, "to_port", wire_path),
                 text(wire, "failure_type", wire_path)};
    plan.consumer = read_failure_consumer(required(root, "consumer", path),
                                          path + ".consumer");
    const auto& functions = array(required(root, "functions", path),
                                  path + ".functions");
    for (std::size_t index = 0; index < functions.size(); ++index) {
        const auto item_path = path + ".functions[" + std::to_string(index) + "]";
        const auto& item = object(functions[index], item_path);
        plan.functions.push_back({number(item, "function_symbol_id", item_path),
                                  text(item, "parameter_projection", item_path),
                                  text(item, "failure_payload_type", item_path),
                                  text(item, "result_type", item_path),
                                  text(item, "availability", item_path)});
    }
    plan.selection = read_failure_policy_selection(
        required(root, "policy_selection", path), path + ".policy_selection");
    const auto& transitions = array(required(root, "transitions", path),
                                    path + ".transitions");
    for (std::size_t index = 0; index < transitions.size(); ++index)
        plan.transitions.push_back(read_failure_response_transition(
            transitions[index], path + ".transitions[" + std::to_string(index) + "]"));
    return plan;
}

inline std::string failure_flow_plan_refusal(const FailureFlowPlan& plan) {
    if (plan.id < 0 || plan.schedule != "serial_explicit_failure_route_v1")
        return "invalid failure-flow plan identity or schedule";
    if (plan.producer.id < 0 || plan.producer.operation < 0 ||
        plan.producer.function < 0 || plan.producer.success_type.empty() ||
        plan.producer.failure_type.empty() || plan.producer.failure_port != "failure" ||
        plan.producer.completion != "exactly_one" ||
        plan.producer.failure_commit_law != "failure_publishes_no_normal_state" ||
        plan.producer.provenance.empty())
        return "invalid failure producer disposition";
    if (plan.consumer.accepted_failure_types.size() != 1 ||
        plan.consumer.routes.size() != 1 || plan.functions.size() != 1 ||
        plan.transitions.size() != 1)
        return "bounded failure-flow plan requires exactly one consumer route";
    const auto consumer_error = failure_consumer_refusal(plan.consumer, plan.functions);
    if (!consumer_error.empty()) return consumer_error;
    const auto policy_error = failure_policy_selection_refusal(
        plan.selection, plan.consumer, plan.functions);
    if (!policy_error.empty()) return policy_error;
    const auto transition_error = failure_response_transitions_refusal(
        plan.transitions, plan.consumer, plan.functions);
    if (!transition_error.empty()) return transition_error;
    const auto& route = plan.consumer.routes.front();
    const auto& transition = plan.transitions.front();
    if (plan.wire.id < 0 ||
        plan.wire.producer_disposition != plan.producer.id ||
        plan.wire.producer_operation != plan.producer.operation ||
        plan.wire.from_port != plan.producer.failure_port ||
        plan.wire.consumer != plan.consumer.id || plan.wire.route != route.id ||
        plan.wire.to_port != "failure_envelope" ||
        plan.wire.failure_type != plan.producer.failure_type ||
        route.failure_type != plan.producer.failure_type ||
        plan.selection.consumer != plan.consumer.id ||
        plan.selection.failure_type != plan.producer.failure_type ||
        plan.selection.route != route.id || transition.consumer != plan.consumer.id ||
        transition.route != route.id || transition.function != route.function ||
        transition.incoming_failure_type != plan.producer.failure_type)
        return "failure wire, policy, and response authority do not form one exact route";
    return {};
}

inline json::Value failure_envelope_fact(const FailureEnvelope& envelope) {
    using namespace json;
    return Object{{"format", "lyraform.failure_envelope"}, {"version", Integer{1}},
                  {"disposition_id", Integer(envelope.disposition)},
                  {"obligation_id", Integer(envelope.obligation)},
                  {"producer_operation_id", Integer(envelope.producer_operation)},
                  {"attempt_id", envelope.attempt}, {"correlation_id", envelope.correlation},
                  {"failure_type", envelope.failure_type}, {"payload", envelope.payload},
                  {"origin_commit", envelope.origin_commit},
                  {"provenance", envelope.provenance}};
}

inline FailureEnvelope read_failure_envelope(const json::Value& raw,
                                             const std::string& path) {
    using namespace json;
    const auto& value = object(raw, path);
    auto number = [&](std::string_view key) {
        return integer(required(value, key, path), path + "." + std::string(key));
    };
    auto text = [&](std::string_view key) -> const std::string& {
        return string(required(value, key, path), path + "." + std::string(key));
    };
    if (text("format") != "lyraform.failure_envelope" || number("version") != 1)
        throw Error(path, "unsupported failure envelope");
    return {number("disposition_id"), number("obligation_id"),
            number("producer_operation_id"), text("attempt_id"),
            text("correlation_id"), text("failure_type"),
            required(value, "payload", path), text("origin_commit"),
            text("provenance")};
}

inline std::string failure_envelope_refusal(const FailureEnvelope& envelope,
                                            const FailureFlowPlan& plan) {
    const auto plan_error = failure_flow_plan_refusal(plan);
    if (!plan_error.empty()) return plan_error;
    if (envelope.disposition != plan.producer.id || envelope.obligation < 0 ||
        envelope.producer_operation != plan.producer.operation ||
        envelope.attempt.empty() || envelope.correlation.empty() ||
        envelope.failure_type != plan.producer.failure_type ||
        envelope.origin_commit != "no_commit" || envelope.provenance.empty())
        return "failure envelope does not prove the selected producer failure";
    return {};
}

inline json::Value failure_flow_receipt_fact(const FailureFlowReceipt& receipt) {
    using namespace json;
    return Object{{"format", "lyraform.failure_flow_receipt"}, {"version", Integer{1}},
                  {"status", "completed"}, {"plan_id", Integer(receipt.plan)},
                  {"wire_id", Integer(receipt.wire)},
                  {"consumer_id", Integer(receipt.consumer)},
                  {"route_id", Integer(receipt.route)},
                  {"function_symbol_id", Integer(receipt.function)},
                  {"transition_id", Integer(receipt.transition)},
                  {"original_obligation_id", Integer(receipt.original_obligation)},
                  {"successor_obligation_id", Integer(receipt.successor_obligation)},
                  {"response_attempt_id", receipt.response_attempt},
                  {"correlation_id", receipt.correlation},
                  {"incoming_failure_type", receipt.incoming_failure_type},
                  {"outgoing_disposition", receipt.outgoing_disposition},
                  {"outgoing_payload_type", receipt.outgoing_payload_type},
                  {"outgoing_payload", receipt.outgoing_payload},
                  {"obligation_transition", receipt.obligation_transition},
                  {"origin_commit", receipt.origin_commit},
                  {"origin_provenance", receipt.origin_provenance},
                  {"response_provenance", receipt.response_provenance}};
}

inline FailureFlowReceipt read_failure_flow_receipt(const json::Value& raw,
                                                    const std::string& path) {
    using namespace json;
    const auto& value = object(raw, path);
    auto number = [&](std::string_view key) {
        return integer(required(value, key, path), path + "." + std::string(key));
    };
    auto text = [&](std::string_view key) -> const std::string& {
        return string(required(value, key, path), path + "." + std::string(key));
    };
    if (text("format") != "lyraform.failure_flow_receipt" || number("version") != 1 ||
        text("status") != "completed")
        throw Error(path, "unsupported or incomplete failure-flow receipt");
    return {number("plan_id"), number("wire_id"), number("consumer_id"),
            number("route_id"), number("function_symbol_id"),
            number("transition_id"), number("original_obligation_id"),
            number("successor_obligation_id"), text("response_attempt_id"),
            text("correlation_id"), text("incoming_failure_type"),
            text("outgoing_disposition"), text("outgoing_payload_type"),
            required(value, "outgoing_payload", path), text("obligation_transition"),
            text("origin_commit"), text("origin_provenance"),
            text("response_provenance")};
}

inline std::string failure_flow_receipt_refusal(const FailureFlowReceipt& receipt,
                                                const FailureFlowPlan& plan,
                                                const FailureEnvelope& envelope) {
    const auto envelope_error = failure_envelope_refusal(envelope, plan);
    if (!envelope_error.empty()) return envelope_error;
    const auto& route = plan.consumer.routes.front();
    const auto& transition = plan.transitions.front();
    if (receipt.plan != plan.id || receipt.wire != plan.wire.id ||
        receipt.consumer != plan.consumer.id || receipt.route != route.id ||
        receipt.function != route.function || receipt.transition != transition.id ||
        receipt.original_obligation != envelope.obligation ||
        receipt.response_attempt.empty() || receipt.correlation != envelope.correlation ||
        receipt.incoming_failure_type != envelope.failure_type ||
        receipt.outgoing_disposition != transition.outgoing_disposition ||
        receipt.outgoing_payload_type != transition.outgoing_payload_type ||
        receipt.obligation_transition != transition.obligation_transition ||
        receipt.origin_commit != envelope.origin_commit ||
        receipt.origin_provenance != envelope.provenance ||
        receipt.response_provenance.empty())
        return "failure-flow receipt contradicts its plan or origin envelope";
    if (transition.response_class == "recover") {
        if (receipt.successor_obligation != -1)
            return "recovery receipt created a successor failure obligation";
    } else if (receipt.successor_obligation < 0 ||
               receipt.successor_obligation == receipt.original_obligation) {
        return "transformation receipt lacks a distinct successor failure obligation";
    }
    return {};
}

inline FailureFlowReceipt execute_failure_flow(
    const FailureFlowPlan& plan, const FailureEnvelope& envelope,
    const std::string& response_attempt, long long successor_obligation,
    const std::map<long long, FailureResponseCallable>& callables) {
    const auto envelope_error = failure_envelope_refusal(envelope, plan);
    if (!envelope_error.empty()) throw json::Error("$.failure_flow", envelope_error);
    if (response_attempt.empty())
        throw json::Error("$.failure_flow.response_attempt_id", "response attempt identity is empty");
    const auto& route = plan.consumer.routes.front();
    const auto& transition = plan.transitions.front();
    const auto callable = callables.find(route.function);
    if (callable == callables.end())
        throw json::Error("$.failure_flow.function_symbol_id",
                          "selected response function is unavailable");

    FailureResponseResult result;
    try {
        result = callable->second(envelope);
    } catch (...) {
        throw json::Error("$.failure_flow.response_attempt",
                          "response function did not complete under the bounded contract");
    }
    if (result.disposition != transition.outgoing_disposition ||
        result.payload_type != transition.outgoing_payload_type ||
        result.provenance.empty())
        throw json::Error("$.failure_flow.response_result",
                          "response result contradicts the canonical transition");
    if (transition.response_class == "recover") {
        if (successor_obligation != -1)
            throw json::Error("$.failure_flow.successor_obligation_id",
                              "recovery cannot create a successor failure obligation");
    } else if (successor_obligation < 0 || successor_obligation == envelope.obligation) {
        throw json::Error("$.failure_flow.successor_obligation_id",
                          "transformation requires a distinct successor failure obligation");
    }

    FailureFlowReceipt receipt{
            plan.id, plan.wire.id, plan.consumer.id, route.id, route.function,
            transition.id, envelope.obligation, successor_obligation,
            response_attempt, envelope.correlation, envelope.failure_type,
            result.disposition, result.payload_type, std::move(result.payload),
            transition.obligation_transition, envelope.origin_commit,
            envelope.provenance, result.provenance};
    const auto receipt_error = failure_flow_receipt_refusal(receipt, plan, envelope);
    if (!receipt_error.empty()) throw json::Error("$.failure_flow.receipt", receipt_error);
    return receipt;
}

} // namespace flowcontracts
