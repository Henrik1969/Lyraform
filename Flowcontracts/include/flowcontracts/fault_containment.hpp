#pragma once

#include <flowcontracts/json.hpp>

#include <functional>
#include <map>
#include <string>

namespace flowcontracts {

struct FaultProducerDisposition {
    long long id = -1;
    long long operation = -1;
    long long function = -1;
    std::string fault_type;
    std::string fault_port;
    std::string completion;
    std::string fault_commit_law;
    std::string provenance;
};

struct FaultContainmentWire {
    long long id = -1;
    long long producer_disposition = -1;
    long long producer_operation = -1;
    std::string from_port;
    long long authority = -1;
    std::string to_port;
    std::string fault_type;
};

struct FaultContainmentAuthority {
    long long id = -1;
    long long scope = -1;
    std::string scope_kind;
    std::string accepted_fault_type;
    std::string action;
    std::string scope_state;
    std::string normal_publication;
    std::string continuation;
    std::string evidence_law;
};

struct FaultContainmentPolicySelection {
    long long authority = -1;
    std::string fault_type;
    std::string action;
    std::string profile;
    std::string version;
};

struct FaultContainmentPlan {
    long long id = -1;
    std::string schedule;
    FaultProducerDisposition producer;
    FaultContainmentWire wire;
    FaultContainmentAuthority authority;
    FaultContainmentPolicySelection selection;
};

struct FaultEnvelope {
    long long disposition = -1;
    long long obligation = -1;
    long long producer_operation = -1;
    std::string attempt;
    std::string correlation;
    std::string fault_type;
    json::Value payload;
    std::string origin_commit;
    std::string provenance;
};

struct FaultContainmentResult {
    std::string status;
    long long scope = -1;
    std::string scope_state;
    std::string normal_publication;
    std::string continuation;
    std::string provenance;
};

struct FaultContainmentReceipt {
    long long plan = -1;
    long long wire = -1;
    long long authority = -1;
    long long obligation = -1;
    long long scope = -1;
    std::string scope_kind;
    std::string action;
    std::string scope_state;
    std::string normal_publication;
    std::string continuation;
    std::string correlation;
    std::string fault_type;
    std::string origin_commit;
    std::string origin_provenance;
    std::string containment_provenance;
    std::string obligation_transition;
};

using FaultContainmentCallable =
    std::function<FaultContainmentResult(const FaultEnvelope&,
                                         const FaultContainmentAuthority&)>;

inline json::Value fault_containment_plan_fact(const FaultContainmentPlan& plan) {
    using namespace json;
    return Object{
        {"format", "lyraform.fault_containment_plan"}, {"version", Integer{1}},
        {"status", "ready"}, {"plan_id", Integer(plan.id)},
        {"schedule", plan.schedule},
        {"producer", Object{
            {"disposition_id", Integer(plan.producer.id)},
            {"operation_id", Integer(plan.producer.operation)},
            {"function_symbol_id", Integer(plan.producer.function)},
            {"fault_type", plan.producer.fault_type},
            {"fault_port", plan.producer.fault_port},
            {"completion", plan.producer.completion},
            {"fault_commit_law", plan.producer.fault_commit_law},
            {"provenance", plan.producer.provenance}}},
        {"wire", Object{
            {"wire_id", Integer(plan.wire.id)},
            {"producer_disposition_id", Integer(plan.wire.producer_disposition)},
            {"producer_operation_id", Integer(plan.wire.producer_operation)},
            {"from_port", plan.wire.from_port},
            {"authority_id", Integer(plan.wire.authority)},
            {"to_port", plan.wire.to_port},
            {"fault_type", plan.wire.fault_type}}},
        {"authority", Object{
            {"authority_id", Integer(plan.authority.id)},
            {"scope_id", Integer(plan.authority.scope)},
            {"scope_kind", plan.authority.scope_kind},
            {"accepted_fault_type", plan.authority.accepted_fault_type},
            {"action", plan.authority.action},
            {"scope_state", plan.authority.scope_state},
            {"normal_publication", plan.authority.normal_publication},
            {"continuation", plan.authority.continuation},
            {"evidence_law", plan.authority.evidence_law}}},
        {"policy_selection", Object{
            {"authority_id", Integer(plan.selection.authority)},
            {"fault_type", plan.selection.fault_type},
            {"action", plan.selection.action},
            {"profile", plan.selection.profile},
            {"version", plan.selection.version}}}};
}

inline FaultContainmentPlan read_fault_containment_plan(
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
    if (text(root, "format", path) != "lyraform.fault_containment_plan" ||
        number(root, "version", path) != 1 || text(root, "status", path) != "ready")
        throw Error(path, "unsupported or non-executable fault-containment plan");
    FaultContainmentPlan plan;
    plan.id = number(root, "plan_id", path);
    plan.schedule = text(root, "schedule", path);
    const auto producer_path = path + ".producer";
    const auto& producer = object(required(root, "producer", path), producer_path);
    plan.producer = {number(producer, "disposition_id", producer_path),
                     number(producer, "operation_id", producer_path),
                     number(producer, "function_symbol_id", producer_path),
                     text(producer, "fault_type", producer_path),
                     text(producer, "fault_port", producer_path),
                     text(producer, "completion", producer_path),
                     text(producer, "fault_commit_law", producer_path),
                     text(producer, "provenance", producer_path)};
    const auto wire_path = path + ".wire";
    const auto& wire = object(required(root, "wire", path), wire_path);
    plan.wire = {number(wire, "wire_id", wire_path),
                 number(wire, "producer_disposition_id", wire_path),
                 number(wire, "producer_operation_id", wire_path),
                 text(wire, "from_port", wire_path),
                 number(wire, "authority_id", wire_path),
                 text(wire, "to_port", wire_path),
                 text(wire, "fault_type", wire_path)};
    const auto authority_path = path + ".authority";
    const auto& authority = object(required(root, "authority", path), authority_path);
    plan.authority = {number(authority, "authority_id", authority_path),
                      number(authority, "scope_id", authority_path),
                      text(authority, "scope_kind", authority_path),
                      text(authority, "accepted_fault_type", authority_path),
                      text(authority, "action", authority_path),
                      text(authority, "scope_state", authority_path),
                      text(authority, "normal_publication", authority_path),
                      text(authority, "continuation", authority_path),
                      text(authority, "evidence_law", authority_path)};
    const auto selection_path = path + ".policy_selection";
    const auto& selection = object(required(root, "policy_selection", path),
                                   selection_path);
    plan.selection = {number(selection, "authority_id", selection_path),
                      text(selection, "fault_type", selection_path),
                      text(selection, "action", selection_path),
                      text(selection, "profile", selection_path),
                      text(selection, "version", selection_path)};
    return plan;
}

inline std::string fault_containment_plan_refusal(
    const FaultContainmentPlan& plan) {
    if (plan.id < 0 || plan.schedule != "serial_halt_quarantine_v1")
        return "invalid fault-containment plan identity or schedule";
    if (plan.producer.id < 0 || plan.producer.operation < 0 ||
        plan.producer.function < 0 || plan.producer.fault_type.empty() ||
        plan.producer.fault_port != "fault" ||
        plan.producer.completion != "exactly_one" ||
        plan.producer.fault_commit_law != "fault_publishes_no_normal_state" ||
        plan.producer.provenance.empty())
        return "invalid fault producer disposition";
    if (plan.wire.id < 0 ||
        plan.wire.producer_disposition != plan.producer.id ||
        plan.wire.producer_operation != plan.producer.operation ||
        plan.wire.from_port != "fault" ||
        plan.wire.authority != plan.authority.id ||
        plan.wire.to_port != "fault_envelope" ||
        plan.wire.fault_type != plan.producer.fault_type)
        return "fault wire does not bind the producer to the authority";
    if (plan.authority.id < 0 || plan.authority.scope < 0 ||
        plan.authority.scope_kind != "activation" ||
        plan.authority.accepted_fault_type != plan.producer.fault_type ||
        plan.authority.action != "halt_and_quarantine" ||
        plan.authority.scope_state != "halted_quarantined" ||
        plan.authority.normal_publication != "suppressed" ||
        plan.authority.continuation != "none" ||
        plan.authority.evidence_law != "preserve_fault_envelope")
        return "fault-containment authority weakens bounded quarantine";
    if (plan.selection.authority != plan.authority.id ||
        plan.selection.fault_type != plan.producer.fault_type ||
        plan.selection.action != plan.authority.action ||
        plan.selection.profile.empty() || plan.selection.version.empty())
        return "fault-containment policy selection is not exact";
    return {};
}

inline json::Value fault_envelope_fact(const FaultEnvelope& envelope) {
    using namespace json;
    return Object{{"format", "lyraform.fault_envelope"}, {"version", Integer{1}},
                  {"disposition_id", Integer(envelope.disposition)},
                  {"obligation_id", Integer(envelope.obligation)},
                  {"producer_operation_id", Integer(envelope.producer_operation)},
                  {"attempt_id", envelope.attempt},
                  {"correlation_id", envelope.correlation},
                  {"fault_type", envelope.fault_type}, {"payload", envelope.payload},
                  {"origin_commit", envelope.origin_commit},
                  {"provenance", envelope.provenance}};
}

inline FaultEnvelope read_fault_envelope(const json::Value& raw,
                                         const std::string& path) {
    using namespace json;
    const auto& value = object(raw, path);
    auto number = [&](std::string_view key) {
        return integer(required(value, key, path), path + "." + std::string(key));
    };
    auto text = [&](std::string_view key) -> const std::string& {
        return string(required(value, key, path), path + "." + std::string(key));
    };
    if (text("format") != "lyraform.fault_envelope" || number("version") != 1)
        throw Error(path, "unsupported fault envelope");
    return {number("disposition_id"), number("obligation_id"),
            number("producer_operation_id"), text("attempt_id"),
            text("correlation_id"), text("fault_type"),
            required(value, "payload", path), text("origin_commit"),
            text("provenance")};
}

inline std::string fault_envelope_refusal(const FaultEnvelope& envelope,
                                          const FaultContainmentPlan& plan) {
    const auto plan_error = fault_containment_plan_refusal(plan);
    if (!plan_error.empty()) return plan_error;
    if (envelope.disposition != plan.producer.id || envelope.obligation < 0 ||
        envelope.producer_operation != plan.producer.operation ||
        envelope.attempt.empty() || envelope.correlation.empty() ||
        envelope.fault_type != plan.producer.fault_type ||
        envelope.origin_commit != "no_commit" || envelope.provenance.empty())
        return "fault envelope does not prove the selected producer fault";
    return {};
}

inline json::Value fault_containment_receipt_fact(
    const FaultContainmentReceipt& receipt) {
    using namespace json;
    return Object{
        {"format", "lyraform.fault_containment_receipt"}, {"version", Integer{1}},
        {"status", "contained"}, {"plan_id", Integer(receipt.plan)},
        {"wire_id", Integer(receipt.wire)},
        {"authority_id", Integer(receipt.authority)},
        {"obligation_id", Integer(receipt.obligation)},
        {"scope_id", Integer(receipt.scope)}, {"scope_kind", receipt.scope_kind},
        {"action", receipt.action}, {"scope_state", receipt.scope_state},
        {"normal_publication", receipt.normal_publication},
        {"continuation", receipt.continuation},
        {"correlation_id", receipt.correlation},
        {"fault_type", receipt.fault_type},
        {"origin_commit", receipt.origin_commit},
        {"origin_provenance", receipt.origin_provenance},
        {"containment_provenance", receipt.containment_provenance},
        {"obligation_transition", receipt.obligation_transition}};
}

inline FaultContainmentReceipt read_fault_containment_receipt(
    const json::Value& raw, const std::string& path) {
    using namespace json;
    const auto& value = object(raw, path);
    auto number = [&](std::string_view key) {
        return integer(required(value, key, path), path + "." + std::string(key));
    };
    auto text = [&](std::string_view key) -> const std::string& {
        return string(required(value, key, path), path + "." + std::string(key));
    };
    if (text("format") != "lyraform.fault_containment_receipt" ||
        number("version") != 1 || text("status") != "contained")
        throw Error(path, "unsupported or incomplete fault-containment receipt");
    return {number("plan_id"), number("wire_id"), number("authority_id"),
            number("obligation_id"), number("scope_id"), text("scope_kind"),
            text("action"), text("scope_state"), text("normal_publication"),
            text("continuation"), text("correlation_id"), text("fault_type"),
            text("origin_commit"), text("origin_provenance"),
            text("containment_provenance"), text("obligation_transition")};
}

inline std::string fault_containment_receipt_refusal(
    const FaultContainmentReceipt& receipt, const FaultContainmentPlan& plan,
    const FaultEnvelope& envelope) {
    const auto envelope_error = fault_envelope_refusal(envelope, plan);
    if (!envelope_error.empty()) return envelope_error;
    if (receipt.plan != plan.id || receipt.wire != plan.wire.id ||
        receipt.authority != plan.authority.id ||
        receipt.obligation != envelope.obligation ||
        receipt.scope != plan.authority.scope ||
        receipt.scope_kind != plan.authority.scope_kind ||
        receipt.action != plan.authority.action ||
        receipt.scope_state != "halted_quarantined" ||
        receipt.normal_publication != "suppressed" ||
        receipt.continuation != "none" ||
        receipt.correlation != envelope.correlation ||
        receipt.fault_type != envelope.fault_type ||
        receipt.origin_commit != envelope.origin_commit ||
        receipt.origin_provenance != envelope.provenance ||
        receipt.containment_provenance.empty() ||
        receipt.obligation_transition != "contained_no_continuation")
        return "fault-containment receipt contradicts its plan or fault envelope";
    return {};
}

inline FaultContainmentReceipt execute_fault_containment(
    const FaultContainmentPlan& plan, const FaultEnvelope& envelope,
    const std::map<long long, FaultContainmentCallable>& authorities) {
    const auto envelope_error = fault_envelope_refusal(envelope, plan);
    if (!envelope_error.empty())
        throw json::Error("$.fault_containment", envelope_error);
    const auto callable = authorities.find(plan.authority.id);
    if (callable == authorities.end())
        throw json::Error("$.fault_containment.authority_id",
                          "selected containment authority is unavailable");
    FaultContainmentResult result;
    try {
        result = callable->second(envelope, plan.authority);
    } catch (...) {
        throw json::Error("$.fault_containment.authority",
                          "containment authority did not produce a receipt");
    }
    if (result.status != "contained" || result.scope != plan.authority.scope ||
        result.scope_state != "halted_quarantined" ||
        result.normal_publication != "suppressed" ||
        result.continuation != "none" || result.provenance.empty())
        throw json::Error("$.fault_containment.result",
                          "containment result weakens halt or quarantine");
    FaultContainmentReceipt receipt{
        plan.id, plan.wire.id, plan.authority.id, envelope.obligation,
        plan.authority.scope, plan.authority.scope_kind, plan.authority.action,
        result.scope_state, result.normal_publication, result.continuation,
        envelope.correlation, envelope.fault_type, envelope.origin_commit,
        envelope.provenance, result.provenance,
        "contained_no_continuation"};
    const auto receipt_error = fault_containment_receipt_refusal(
        receipt, plan, envelope);
    if (!receipt_error.empty())
        throw json::Error("$.fault_containment.receipt", receipt_error);
    return receipt;
}

} // namespace flowcontracts
