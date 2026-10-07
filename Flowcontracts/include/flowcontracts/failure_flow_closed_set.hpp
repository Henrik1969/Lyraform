#pragma once

#include <flowcontracts/failure_flow_execution.hpp>

#include <algorithm>
#include <set>

namespace flowcontracts {

struct FailureProducerDispositionSet {
    long long id = -1;
    long long operation = -1;
    long long function = -1;
    std::string success_type;
    std::vector<std::string> failure_types;
    std::string failure_port;
    std::string completion;
    std::string failure_commit_law;
    std::string provenance;
};

struct FailureFlowClosedSetPlan {
    long long id = -1;
    std::string schedule;
    FailureProducerDispositionSet producer;
    std::vector<FailureFlowWire> wires;
    FailureConsumer consumer;
    std::vector<FailureConsumerFunction> functions;
    std::vector<FailurePolicySelection> selections;
    std::vector<FailureResponseTransition> transitions;
};

inline json::Value failure_flow_closed_set_plan_fact(
    const FailureFlowClosedSetPlan& plan) {
    using namespace json;
    Array failure_types;
    for (const auto& type : plan.producer.failure_types)
        failure_types.emplace_back(type);
    Array wires;
    for (const auto& wire : plan.wires)
        wires.emplace_back(Object{{"wire_id", Integer(wire.id)},
                                  {"producer_disposition_id", Integer(wire.producer_disposition)},
                                  {"producer_operation_id", Integer(wire.producer_operation)},
                                  {"from_port", wire.from_port},
                                  {"consumer_id", Integer(wire.consumer)},
                                  {"route_id", Integer(wire.route)},
                                  {"to_port", wire.to_port},
                                  {"failure_type", wire.failure_type}});
    Array functions;
    for (const auto& function : plan.functions)
        functions.emplace_back(Object{{"function_symbol_id", Integer(function.id)},
                                      {"parameter_projection", function.parameter_projection},
                                      {"failure_payload_type", function.failure_payload_type},
                                      {"result_type", function.result_type},
                                      {"availability", function.availability}});
    Array selections;
    for (const auto& selection : plan.selections)
        selections.emplace_back(failure_policy_selection_fact(selection));
    Array transitions;
    for (const auto& transition : plan.transitions)
        transitions.emplace_back(failure_response_transition_fact(transition));
    return Object{
        {"format", "lyraform.failure_flow_plan"}, {"version", Integer{2}},
        {"status", "ready"}, {"plan_id", Integer(plan.id)},
        {"schedule", plan.schedule},
        {"producer", Object{{"disposition_id", Integer(plan.producer.id)},
                            {"operation_id", Integer(plan.producer.operation)},
                            {"function_symbol_id", Integer(plan.producer.function)},
                            {"success_type", plan.producer.success_type},
                            {"failure_types", std::move(failure_types)},
                            {"failure_port", plan.producer.failure_port},
                            {"completion", plan.producer.completion},
                            {"failure_commit_law", plan.producer.failure_commit_law},
                            {"provenance", plan.producer.provenance}}},
        {"wires", std::move(wires)},
        {"consumer", failure_consumer_fact(plan.consumer)},
        {"functions", std::move(functions)},
        {"policy_selections", std::move(selections)},
        {"transitions", std::move(transitions)}};
}

inline FailureFlowClosedSetPlan read_failure_flow_closed_set_plan(
    const json::Value& raw, const std::string& path) {
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
        number(root, "version", path) != 2 || text(root, "status", path) != "ready")
        throw Error(path, "unsupported or non-executable closed-set failure-flow plan");

    FailureFlowClosedSetPlan plan;
    plan.id = number(root, "plan_id", path);
    plan.schedule = text(root, "schedule", path);
    const auto producer_path = path + ".producer";
    const auto& producer = object(required(root, "producer", path), producer_path);
    plan.producer.id = number(producer, "disposition_id", producer_path);
    plan.producer.operation = number(producer, "operation_id", producer_path);
    plan.producer.function = number(producer, "function_symbol_id", producer_path);
    plan.producer.success_type = text(producer, "success_type", producer_path);
    const auto& failure_types = array(required(producer, "failure_types", producer_path),
                                      producer_path + ".failure_types");
    for (std::size_t index = 0; index < failure_types.size(); ++index)
        plan.producer.failure_types.push_back(string(
            failure_types[index], producer_path + ".failure_types[" +
                                      std::to_string(index) + "]"));
    plan.producer.failure_port = text(producer, "failure_port", producer_path);
    plan.producer.completion = text(producer, "completion", producer_path);
    plan.producer.failure_commit_law = text(producer, "failure_commit_law", producer_path);
    plan.producer.provenance = text(producer, "provenance", producer_path);

    const auto& wires = array(required(root, "wires", path), path + ".wires");
    for (std::size_t index = 0; index < wires.size(); ++index) {
        const auto item_path = path + ".wires[" + std::to_string(index) + "]";
        const auto& wire = object(wires[index], item_path);
        plan.wires.push_back({number(wire, "wire_id", item_path),
                              number(wire, "producer_disposition_id", item_path),
                              number(wire, "producer_operation_id", item_path),
                              text(wire, "from_port", item_path),
                              number(wire, "consumer_id", item_path),
                              number(wire, "route_id", item_path),
                              text(wire, "to_port", item_path),
                              text(wire, "failure_type", item_path)});
    }
    plan.consumer = read_failure_consumer(required(root, "consumer", path),
                                          path + ".consumer");
    const auto& functions = array(required(root, "functions", path), path + ".functions");
    for (std::size_t index = 0; index < functions.size(); ++index) {
        const auto item_path = path + ".functions[" + std::to_string(index) + "]";
        const auto& item = object(functions[index], item_path);
        plan.functions.push_back({number(item, "function_symbol_id", item_path),
                                  text(item, "parameter_projection", item_path),
                                  text(item, "failure_payload_type", item_path),
                                  text(item, "result_type", item_path),
                                  text(item, "availability", item_path)});
    }
    const auto& selections = array(required(root, "policy_selections", path),
                                   path + ".policy_selections");
    for (std::size_t index = 0; index < selections.size(); ++index)
        plan.selections.push_back(read_failure_policy_selection(
            selections[index], path + ".policy_selections[" +
                                   std::to_string(index) + "]"));
    const auto& transitions = array(required(root, "transitions", path),
                                    path + ".transitions");
    for (std::size_t index = 0; index < transitions.size(); ++index)
        plan.transitions.push_back(read_failure_response_transition(
            transitions[index], path + ".transitions[" +
                                    std::to_string(index) + "]"));
    return plan;
}

inline std::string failure_flow_closed_set_plan_refusal(
    const FailureFlowClosedSetPlan& plan) {
    if (plan.id < 0 || plan.schedule != "serial_closed_failure_set_v1")
        return "invalid closed-set failure-flow plan identity or schedule";
    if (plan.producer.id < 0 || plan.producer.operation < 0 ||
        plan.producer.function < 0 || plan.producer.success_type.empty() ||
        plan.producer.failure_types.empty() || plan.producer.failure_port != "failure" ||
        plan.producer.completion != "exactly_one" ||
        plan.producer.failure_commit_law != "failure_publishes_no_normal_state" ||
        plan.producer.provenance.empty())
        return "invalid closed-set failure producer disposition";

    std::set<std::string> failure_types;
    for (const auto& type : plan.producer.failure_types)
        if (type.empty() || !failure_types.insert(type).second)
            return "invalid or duplicate producer failure type";
    const std::set<std::string> consumer_types(
        plan.consumer.accepted_failure_types.begin(),
        plan.consumer.accepted_failure_types.end());
    if (consumer_types != failure_types)
        return "producer and consumer closed failure sets differ";
    if (plan.wires.size() != plan.producer.failure_types.size() ||
        plan.selections.size() != plan.producer.failure_types.size())
        return "every producer failure type requires one wire and one policy selection";

    const auto consumer_error = failure_consumer_refusal(plan.consumer, plan.functions);
    if (!consumer_error.empty()) return consumer_error;
    const auto transition_error = failure_response_transitions_refusal(
        plan.transitions, plan.consumer, plan.functions);
    if (!transition_error.empty()) return transition_error;

    std::set<long long> used_functions;
    for (const auto& route : plan.consumer.routes) used_functions.insert(route.function);
    if (used_functions.size() != plan.functions.size())
        return "closed-set plan contains missing or unreferenced response functions";
    for (const auto& function : plan.functions)
        if (!used_functions.count(function.id))
            return "closed-set plan contains an unreferenced response function";

    std::set<long long> wire_ids;
    for (std::size_t index = 0; index < plan.producer.failure_types.size(); ++index) {
        const auto& type = plan.producer.failure_types[index];
        const auto& wire = plan.wires[index];
        const auto& selection = plan.selections[index];
        if (wire.id < 0 || !wire_ids.insert(wire.id).second ||
            wire.producer_disposition != plan.producer.id ||
            wire.producer_operation != plan.producer.operation ||
            wire.from_port != plan.producer.failure_port ||
            wire.consumer != plan.consumer.id || wire.to_port != "failure_envelope" ||
            wire.failure_type != type || selection.consumer != plan.consumer.id ||
            selection.failure_type != type || selection.route != wire.route)
            return "closed-set wire and policy selection do not match producer order";
        const auto policy_error = failure_policy_selection_refusal(
            selection, plan.consumer, plan.functions);
        if (!policy_error.empty()) return policy_error;
    }
    return {};
}

inline const FailureFlowWire& selected_failure_wire(
    const FailureFlowClosedSetPlan& plan, const std::string& failure_type) {
    for (std::size_t index = 0; index < plan.producer.failure_types.size(); ++index)
        if (plan.producer.failure_types[index] == failure_type) return plan.wires[index];
    throw json::Error("$.failure_flow.failure_type",
                      "failure type is outside the producer closed set");
}

inline const FailureConsumerRoute& selected_failure_route(
    const FailureFlowClosedSetPlan& plan, const std::string& failure_type) {
    const auto& wire = selected_failure_wire(plan, failure_type);
    for (const auto& route : plan.consumer.routes)
        if (route.id == wire.route && route.failure_type == failure_type) return route;
    throw json::Error("$.failure_flow.route_id",
                      "selected failure route is absent from the consumer authority");
}

inline const FailureResponseTransition& selected_failure_transition(
    const FailureFlowClosedSetPlan& plan, const FailureConsumerRoute& route) {
    for (const auto& transition : plan.transitions)
        if (transition.route == route.id && transition.function == route.function)
            return transition;
    throw json::Error("$.failure_flow.transition_id",
                      "selected failure route lacks transition authority");
}

inline std::string failure_envelope_refusal(
    const FailureEnvelope& envelope, const FailureFlowClosedSetPlan& plan) {
    const auto plan_error = failure_flow_closed_set_plan_refusal(plan);
    if (!plan_error.empty()) return plan_error;
    if (envelope.disposition != plan.producer.id || envelope.obligation < 0 ||
        envelope.producer_operation != plan.producer.operation ||
        envelope.attempt.empty() || envelope.correlation.empty() ||
        !std::count(plan.producer.failure_types.begin(), plan.producer.failure_types.end(),
                    envelope.failure_type) ||
        envelope.origin_commit != "no_commit" || envelope.provenance.empty())
        return "failure envelope does not prove a member of the producer closed set";
    return {};
}

inline std::string failure_flow_receipt_refusal(
    const FailureFlowReceipt& receipt, const FailureFlowClosedSetPlan& plan,
    const FailureEnvelope& envelope) {
    const auto envelope_error = failure_envelope_refusal(envelope, plan);
    if (!envelope_error.empty()) return envelope_error;
    const auto& wire = selected_failure_wire(plan, envelope.failure_type);
    const auto& route = selected_failure_route(plan, envelope.failure_type);
    const auto& transition = selected_failure_transition(plan, route);
    if (receipt.plan != plan.id || receipt.wire != wire.id ||
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
        return "closed-set failure-flow receipt contradicts its route or origin";
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
    const FailureFlowClosedSetPlan& plan, const FailureEnvelope& envelope,
    const std::string& response_attempt, long long successor_obligation,
    const std::map<long long, FailureResponseCallable>& callables) {
    const auto envelope_error = failure_envelope_refusal(envelope, plan);
    if (!envelope_error.empty()) throw json::Error("$.failure_flow", envelope_error);
    if (response_attempt.empty())
        throw json::Error("$.failure_flow.response_attempt_id",
                          "response attempt identity is empty");
    const auto& wire = selected_failure_wire(plan, envelope.failure_type);
    const auto& route = selected_failure_route(plan, envelope.failure_type);
    const auto& transition = selected_failure_transition(plan, route);
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
        plan.id, wire.id, plan.consumer.id, route.id, route.function,
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
