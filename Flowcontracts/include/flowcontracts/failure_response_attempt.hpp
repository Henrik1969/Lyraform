#pragma once

#include <flowcontracts/failure_flow_closed_set.hpp>

namespace flowcontracts {

struct FailureResponseAttemptLink {
    long long id = -1;
    long long original_plan = -1;
    std::string original_failure_type;
    long long original_route = -1;
    long long original_function = -1;
    long long original_transition = -1;
    long long response_failure_plan = -1;
    long long response_failure_disposition = -1;
    long long response_failure_operation = -1;
    std::string response_failure_type;
    std::string original_obligation_law;
    std::string response_failure_obligation_law;
    std::string correlation_law;
    std::string commit_law;
    std::string provenance_law;
    std::string rejoin_law;
};

struct FailureResponseAttemptPlan {
    long long id = -1;
    std::string schedule;
    FailureFlowClosedSetPlan original;
    FailureResponseAttemptLink link;
    FailureFlowClosedSetPlan response_failure;
};

struct FailureResponseAttemptReceipt {
    long long plan = -1;
    long long link = -1;
    long long original_obligation = -1;
    long long response_failure_obligation = -1;
    long long original_function = -1;
    long long original_transition = -1;
    std::string response_attempt;
    std::string correlation;
    FailureEnvelope response_failure_envelope;
    FailureFlowReceipt response_failure_recovery;
    std::string final_disposition;
    std::string final_payload_type;
    json::Value final_payload;
    std::string original_obligation_transition;
    std::string provenance;
};

inline json::Value failure_response_attempt_plan_fact(
    const FailureResponseAttemptPlan& plan) {
    using namespace json;
    return Object{
        {"format", "lyraform.failure_response_attempt_plan"},
        {"version", Integer{1}}, {"status", "ready"},
        {"plan_id", Integer(plan.id)}, {"schedule", plan.schedule},
        {"original_plan", failure_flow_closed_set_plan_fact(plan.original)},
        {"link", Object{
            {"link_id", Integer(plan.link.id)},
            {"original_plan_id", Integer(plan.link.original_plan)},
            {"original_failure_type", plan.link.original_failure_type},
            {"original_route_id", Integer(plan.link.original_route)},
            {"original_function_symbol_id", Integer(plan.link.original_function)},
            {"original_transition_id", Integer(plan.link.original_transition)},
            {"response_failure_plan_id", Integer(plan.link.response_failure_plan)},
            {"response_failure_disposition_id", Integer(plan.link.response_failure_disposition)},
            {"response_failure_operation_id", Integer(plan.link.response_failure_operation)},
            {"response_failure_type", plan.link.response_failure_type},
            {"original_obligation_law", plan.link.original_obligation_law},
            {"response_failure_obligation_law", plan.link.response_failure_obligation_law},
            {"correlation_law", plan.link.correlation_law},
            {"commit_law", plan.link.commit_law},
            {"provenance_law", plan.link.provenance_law},
            {"rejoin_law", plan.link.rejoin_law}}},
        {"response_failure_plan",
         failure_flow_closed_set_plan_fact(plan.response_failure)}};
}

inline FailureResponseAttemptPlan read_failure_response_attempt_plan(
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
    if (text(root, "format", path) != "lyraform.failure_response_attempt_plan" ||
        number(root, "version", path) != 1 || text(root, "status", path) != "ready")
        throw Error(path, "unsupported response-attempt failure plan");
    FailureResponseAttemptPlan plan;
    plan.id = number(root, "plan_id", path);
    plan.schedule = text(root, "schedule", path);
    plan.original = read_failure_flow_closed_set_plan(
        required(root, "original_plan", path), path + ".original_plan");
    const auto link_path = path + ".link";
    const auto& link = object(required(root, "link", path), link_path);
    plan.link = {
        number(link, "link_id", link_path),
        number(link, "original_plan_id", link_path),
        text(link, "original_failure_type", link_path),
        number(link, "original_route_id", link_path),
        number(link, "original_function_symbol_id", link_path),
        number(link, "original_transition_id", link_path),
        number(link, "response_failure_plan_id", link_path),
        number(link, "response_failure_disposition_id", link_path),
        number(link, "response_failure_operation_id", link_path),
        text(link, "response_failure_type", link_path),
        text(link, "original_obligation_law", link_path),
        text(link, "response_failure_obligation_law", link_path),
        text(link, "correlation_law", link_path),
        text(link, "commit_law", link_path),
        text(link, "provenance_law", link_path),
        text(link, "rejoin_law", link_path)};
    plan.response_failure = read_failure_flow_closed_set_plan(
        required(root, "response_failure_plan", path),
        path + ".response_failure_plan");
    return plan;
}

inline std::string failure_response_attempt_plan_refusal(
    const FailureResponseAttemptPlan& plan) {
    if (plan.id < 0 || plan.schedule != "serial_response_failure_recovery_v1")
        return "invalid response-attempt failure plan identity or schedule";
    const auto original_error = failure_flow_closed_set_plan_refusal(plan.original);
    if (!original_error.empty()) return original_error;
    const auto response_error =
        failure_flow_closed_set_plan_refusal(plan.response_failure);
    if (!response_error.empty()) return response_error;
    if (plan.original.id == plan.response_failure.id ||
        plan.original.producer.id == plan.response_failure.producer.id)
        return "response-attempt plans or dispositions collide";
    if (plan.response_failure.producer.failure_types.size() != 1)
        return "bounded response attempt declares exactly one expected failure type";
    try {
        const auto& route = selected_failure_route(
            plan.original, plan.link.original_failure_type);
        const auto& transition = selected_failure_transition(plan.original, route);
        const auto& response_route = selected_failure_route(
            plan.response_failure, plan.link.response_failure_type);
        const auto& response_transition = selected_failure_transition(
            plan.response_failure, response_route);
        if (plan.link.id < 0 || plan.link.original_plan != plan.original.id ||
            plan.link.original_route != route.id ||
            plan.link.original_function != route.function ||
            plan.link.original_transition != transition.id ||
            transition.response_class != "recover" ||
            transition.outgoing_disposition != "success" ||
            transition.obligation_transition != "close_original")
            return "response-attempt origin is not the exact selected recovery";
        if (plan.link.response_failure_plan != plan.response_failure.id ||
            plan.link.response_failure_disposition !=
                plan.response_failure.producer.id ||
            plan.link.response_failure_operation !=
                plan.response_failure.producer.operation ||
            plan.link.response_failure_type !=
                plan.response_failure.producer.failure_types.front() ||
            plan.response_failure.producer.function != route.function)
            return "response failure is not bound to the selected response function";
        if (response_route.function == route.function ||
            response_transition.response_class != "recover" ||
            response_transition.outgoing_disposition != "success" ||
            response_transition.obligation_transition != "close_original" ||
            response_transition.outgoing_payload_type !=
                transition.outgoing_payload_type)
            return "response failure recovery cannot rejoin the original transition";
    } catch (const json::Error&) {
        return "response-attempt failure route is incomplete";
    }
    if (plan.link.original_obligation_law != "remain_open_until_rejoin" ||
        plan.link.response_failure_obligation_law !=
            "distinct_linked_obligation" ||
        plan.link.correlation_law != "preserve_correlation" ||
        plan.link.commit_law != "response_failure_publishes_no_transition" ||
        plan.link.provenance_law !=
            "response_failure_provenance_to_successor_origin" ||
        plan.link.rejoin_law !=
            "recovered_payload_resumes_original_transition")
        return "response-attempt obligation, evidence, or rejoin law changed";
    return {};
}

inline std::string failure_response_attempt_receipt_refusal(
    const FailureResponseAttemptReceipt& receipt,
    const FailureResponseAttemptPlan& plan, const FailureEnvelope& origin) {
    const auto plan_error = failure_response_attempt_plan_refusal(plan);
    if (!plan_error.empty()) return plan_error;
    const auto origin_error = failure_envelope_refusal(origin, plan.original);
    if (!origin_error.empty()) return origin_error;
    const auto& route = selected_failure_route(
        plan.original, plan.link.original_failure_type);
    const auto& transition = selected_failure_transition(plan.original, route);
    if (receipt.plan != plan.id || receipt.link != plan.link.id ||
        receipt.original_obligation != origin.obligation ||
        receipt.response_failure_obligation < 0 ||
        receipt.response_failure_obligation == origin.obligation ||
        receipt.original_function != route.function ||
        receipt.original_transition != transition.id ||
        receipt.response_attempt.empty() ||
        receipt.correlation != origin.correlation ||
        receipt.final_disposition != "success" ||
        receipt.final_payload_type != transition.outgoing_payload_type ||
        receipt.original_obligation_transition !=
            "close_original_after_response_failure_recovery" ||
        receipt.provenance.empty())
        return "response-attempt receipt contradicts its origin or rejoin";
    const auto& response_envelope = receipt.response_failure_envelope;
    if (response_envelope.disposition !=
            plan.response_failure.producer.id ||
        response_envelope.obligation != receipt.response_failure_obligation ||
        response_envelope.producer_operation !=
            plan.response_failure.producer.operation ||
        response_envelope.attempt != receipt.response_attempt ||
        response_envelope.correlation != origin.correlation ||
        response_envelope.failure_type != plan.link.response_failure_type ||
        response_envelope.origin_commit != "no_commit" ||
        response_envelope.provenance.empty())
        return "response failure envelope changed identity or evidence";
    const auto recovery_error = failure_flow_receipt_refusal(
        receipt.response_failure_recovery, plan.response_failure,
        response_envelope);
    if (!recovery_error.empty()) return recovery_error;
    if (receipt.response_failure_recovery.original_obligation !=
            receipt.response_failure_obligation ||
        receipt.response_failure_recovery.successor_obligation != -1 ||
        receipt.response_failure_recovery.outgoing_disposition != "success" ||
        receipt.response_failure_recovery.outgoing_payload_type !=
            receipt.final_payload_type ||
        json::serialize(receipt.response_failure_recovery.outgoing_payload) !=
            json::serialize(receipt.final_payload))
        return "response failure was not recovered into the exact rejoin value";
    return {};
}

inline FailureResponseAttemptReceipt execute_failure_response_attempt_recovery(
    const FailureResponseAttemptPlan& plan, const FailureEnvelope& origin,
    const std::string& response_attempt, long long response_failure_obligation,
    const std::string& recovery_attempt,
    const std::map<long long, FailureResponseCallable>& response_callables,
    const std::map<long long, FailureResponseCallable>& recovery_callables) {
    const auto plan_error = failure_response_attempt_plan_refusal(plan);
    if (!plan_error.empty())
        throw json::Error("$.failure_response_attempt", plan_error);
    const auto origin_error = failure_envelope_refusal(origin, plan.original);
    if (!origin_error.empty())
        throw json::Error("$.failure_response_attempt.origin", origin_error);
    if (origin.failure_type != plan.link.original_failure_type ||
        response_attempt.empty() || response_failure_obligation < 0 ||
        response_failure_obligation == origin.obligation)
        throw json::Error("$.failure_response_attempt",
                          "invalid response attempt or obligation identity");
    const auto callable = response_callables.find(plan.link.original_function);
    if (callable == response_callables.end())
        throw json::Error("$.failure_response_attempt.function_symbol_id",
                          "selected response function is unavailable");
    FailureResponseResult failure;
    try {
        failure = callable->second(origin);
    } catch (...) {
        throw json::Error("$.failure_response_attempt.host_fault",
                          "response escaped without a declared disposition");
    }
    if (failure.disposition == "fault")
        throw json::Error("$.failure_response_attempt.fault",
                          "fault containment is not admitted by this plan");
    if (failure.disposition != "failure" ||
        failure.payload_type != plan.link.response_failure_type ||
        failure.provenance.empty())
        throw json::Error("$.failure_response_attempt.result",
                          "response attempt did not produce its declared expected failure");

    FailureEnvelope response_envelope{
        plan.response_failure.producer.id, response_failure_obligation,
        plan.response_failure.producer.operation, response_attempt,
        origin.correlation, plan.link.response_failure_type,
        std::move(failure.payload), "no_commit", failure.provenance};
    const auto response_envelope_error = failure_envelope_refusal(
        response_envelope, plan.response_failure);
    if (!response_envelope_error.empty())
        throw json::Error("$.failure_response_attempt.envelope",
                          response_envelope_error);
    auto recovery = execute_failure_flow(
        plan.response_failure, response_envelope, recovery_attempt, -1,
        recovery_callables);
    FailureResponseAttemptReceipt receipt{
        plan.id, plan.link.id, origin.obligation, response_failure_obligation,
        plan.link.original_function, plan.link.original_transition,
        response_attempt, origin.correlation, response_envelope, recovery,
        "success", recovery.outgoing_payload_type, recovery.outgoing_payload,
        "close_original_after_response_failure_recovery",
        recovery.response_provenance};
    const auto receipt_error = failure_response_attempt_receipt_refusal(
        receipt, plan, origin);
    if (!receipt_error.empty())
        throw json::Error("$.failure_response_attempt.receipt", receipt_error);
    return receipt;
}

} // namespace flowcontracts
