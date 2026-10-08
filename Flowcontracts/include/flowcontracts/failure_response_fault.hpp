#pragma once

#include <flowcontracts/failure_flow_closed_set.hpp>
#include <flowcontracts/fault_containment.hpp>

namespace flowcontracts {

struct FailureResponseFaultLink {
    long long id = -1;
    long long original_plan = -1;
    std::string original_failure_type;
    long long original_route = -1;
    long long original_function = -1;
    long long original_transition = -1;
    long long containment_plan = -1;
    long long fault_disposition = -1;
    long long fault_operation = -1;
    std::string fault_type;
    std::string original_obligation_law;
    std::string fault_obligation_law;
    std::string correlation_law;
    std::string commit_law;
    std::string provenance_law;
    std::string containment_law;
};

struct FailureResponseFaultPlan {
    long long id = -1;
    std::string schedule;
    FailureFlowClosedSetPlan original;
    FailureResponseFaultLink link;
    FaultContainmentPlan containment;
};

struct FailureResponseFaultReceipt {
    long long plan = -1;
    long long link = -1;
    long long original_obligation = -1;
    long long fault_obligation = -1;
    long long original_function = -1;
    long long original_transition = -1;
    std::string response_attempt;
    std::string correlation;
    FaultEnvelope fault_envelope;
    FaultContainmentReceipt containment_receipt;
    std::string original_obligation_state;
    std::string final_state;
    std::string normal_publication;
    std::string continuation;
    std::string rejoin;
};

inline json::Value failure_response_fault_plan_fact(
    const FailureResponseFaultPlan& plan) {
    using namespace json;
    return Object{
        {"format", "lyraform.failure_response_fault_plan"},
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
            {"containment_plan_id", Integer(plan.link.containment_plan)},
            {"fault_disposition_id", Integer(plan.link.fault_disposition)},
            {"fault_operation_id", Integer(plan.link.fault_operation)},
            {"fault_type", plan.link.fault_type},
            {"original_obligation_law", plan.link.original_obligation_law},
            {"fault_obligation_law", plan.link.fault_obligation_law},
            {"correlation_law", plan.link.correlation_law},
            {"commit_law", plan.link.commit_law},
            {"provenance_law", plan.link.provenance_law},
            {"containment_law", plan.link.containment_law}}},
        {"containment_plan", fault_containment_plan_fact(plan.containment)}};
}

inline FailureResponseFaultPlan read_failure_response_fault_plan(
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
    if (text(root, "format", path) != "lyraform.failure_response_fault_plan" ||
        number(root, "version", path) != 1 || text(root, "status", path) != "ready")
        throw Error(path, "unsupported response-fault containment plan");
    FailureResponseFaultPlan plan;
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
        number(link, "containment_plan_id", link_path),
        number(link, "fault_disposition_id", link_path),
        number(link, "fault_operation_id", link_path),
        text(link, "fault_type", link_path),
        text(link, "original_obligation_law", link_path),
        text(link, "fault_obligation_law", link_path),
        text(link, "correlation_law", link_path),
        text(link, "commit_law", link_path),
        text(link, "provenance_law", link_path),
        text(link, "containment_law", link_path)};
    plan.containment = read_fault_containment_plan(
        required(root, "containment_plan", path), path + ".containment_plan");
    return plan;
}

inline std::string failure_response_fault_plan_refusal(
    const FailureResponseFaultPlan& plan) {
    if (plan.id < 0 || plan.schedule != "serial_response_fault_containment_v1")
        return "invalid response-fault plan identity or schedule";
    const auto original_error = failure_flow_closed_set_plan_refusal(plan.original);
    if (!original_error.empty()) return original_error;
    const auto containment_error = fault_containment_plan_refusal(plan.containment);
    if (!containment_error.empty()) return containment_error;
    try {
        const auto& route = selected_failure_route(
            plan.original, plan.link.original_failure_type);
        const auto& transition = selected_failure_transition(plan.original, route);
        if (plan.link.id < 0 || plan.link.original_plan != plan.original.id ||
            plan.link.original_route != route.id ||
            plan.link.original_function != route.function ||
            plan.link.original_transition != transition.id ||
            transition.response_class != "recover" ||
            transition.outgoing_disposition != "success")
            return "response-fault origin is not the exact selected recovery";
        if (plan.link.containment_plan != plan.containment.id ||
            plan.link.fault_disposition != plan.containment.producer.id ||
            plan.link.fault_operation != plan.containment.producer.operation ||
            plan.link.fault_type != plan.containment.producer.fault_type ||
            plan.containment.producer.function != route.function)
            return "response fault is not bound to the selected response function";
    } catch (const json::Error&) {
        return "response-fault route is incomplete";
    }
    if (plan.link.original_obligation_law !=
            "preserve_unresolved_in_contained_scope" ||
        plan.link.fault_obligation_law != "distinct_linked_obligation" ||
        plan.link.correlation_law != "preserve_correlation" ||
        plan.link.commit_law != "response_fault_publishes_no_transition" ||
        plan.link.provenance_law != "response_fault_provenance_to_fault_origin" ||
        plan.link.containment_law != "halt_quarantine_no_rejoin")
        return "response-fault obligation, evidence, or containment law changed";
    return {};
}

inline std::string failure_response_fault_receipt_refusal(
    const FailureResponseFaultReceipt& receipt,
    const FailureResponseFaultPlan& plan, const FailureEnvelope& origin) {
    const auto plan_error = failure_response_fault_plan_refusal(plan);
    if (!plan_error.empty()) return plan_error;
    const auto origin_error = failure_envelope_refusal(origin, plan.original);
    if (!origin_error.empty()) return origin_error;
    const auto& route = selected_failure_route(
        plan.original, plan.link.original_failure_type);
    const auto& transition = selected_failure_transition(plan.original, route);
    if (receipt.plan != plan.id || receipt.link != plan.link.id ||
        receipt.original_obligation != origin.obligation ||
        receipt.fault_obligation < 0 ||
        receipt.fault_obligation == origin.obligation ||
        receipt.original_function != route.function ||
        receipt.original_transition != transition.id ||
        receipt.response_attempt.empty() ||
        receipt.correlation != origin.correlation ||
        receipt.original_obligation_state != "unresolved_contained" ||
        receipt.final_state != "contained_fault_no_rejoin" ||
        receipt.normal_publication != "suppressed" ||
        receipt.continuation != "none" || receipt.rejoin != "none")
        return "response-fault receipt contradicts its origin or containment";
    const auto& fault = receipt.fault_envelope;
    if (fault.disposition != plan.containment.producer.id ||
        fault.obligation != receipt.fault_obligation ||
        fault.producer_operation != plan.containment.producer.operation ||
        fault.attempt != receipt.response_attempt ||
        fault.correlation != origin.correlation ||
        fault.fault_type != plan.link.fault_type ||
        fault.origin_commit != "no_commit" || fault.provenance.empty())
        return "response fault envelope changed identity or evidence";
    const auto containment_error = fault_containment_receipt_refusal(
        receipt.containment_receipt, plan.containment, fault);
    if (!containment_error.empty()) return containment_error;
    if (receipt.containment_receipt.obligation != receipt.fault_obligation ||
        receipt.containment_receipt.normal_publication != "suppressed" ||
        receipt.containment_receipt.continuation != "none")
        return "response fault was not contained without continuation";
    return {};
}

inline FailureResponseFaultReceipt execute_failure_response_fault_containment(
    const FailureResponseFaultPlan& plan, const FailureEnvelope& origin,
    const std::string& response_attempt, long long fault_obligation,
    const std::map<long long, FailureResponseCallable>& response_callables,
    const std::map<long long, FaultContainmentCallable>& authorities) {
    const auto plan_error = failure_response_fault_plan_refusal(plan);
    if (!plan_error.empty())
        throw json::Error("$.failure_response_fault", plan_error);
    const auto origin_error = failure_envelope_refusal(origin, plan.original);
    if (!origin_error.empty())
        throw json::Error("$.failure_response_fault.origin", origin_error);
    if (origin.failure_type != plan.link.original_failure_type ||
        response_attempt.empty() || fault_obligation < 0 ||
        fault_obligation == origin.obligation)
        throw json::Error("$.failure_response_fault",
                          "invalid response attempt or fault obligation identity");
    const auto callable = response_callables.find(plan.link.original_function);
    if (callable == response_callables.end())
        throw json::Error("$.failure_response_fault.function_symbol_id",
                          "selected response function is unavailable");
    FailureResponseResult result;
    try {
        result = callable->second(origin);
    } catch (...) {
        throw json::Error("$.failure_response_fault.host_fault",
                          "response escaped without a declared disposition");
    }
    if (result.disposition != "fault" ||
        result.payload_type != plan.link.fault_type || result.provenance.empty())
        throw json::Error("$.failure_response_fault.result",
                          "response attempt did not produce its declared fault");
    FaultEnvelope fault{
        plan.containment.producer.id, fault_obligation,
        plan.containment.producer.operation, response_attempt,
        origin.correlation, plan.link.fault_type, std::move(result.payload),
        "no_commit", result.provenance};
    auto contained = execute_fault_containment(plan.containment, fault, authorities);
    FailureResponseFaultReceipt receipt{
        plan.id, plan.link.id, origin.obligation, fault_obligation,
        plan.link.original_function, plan.link.original_transition,
        response_attempt, origin.correlation, fault, contained,
        "unresolved_contained", "contained_fault_no_rejoin",
        "suppressed", "none", "none"};
    const auto receipt_error = failure_response_fault_receipt_refusal(
        receipt, plan, origin);
    if (!receipt_error.empty())
        throw json::Error("$.failure_response_fault.receipt", receipt_error);
    return receipt;
}

} // namespace flowcontracts
