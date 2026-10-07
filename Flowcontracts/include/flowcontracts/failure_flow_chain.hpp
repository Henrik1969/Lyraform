#pragma once

#include <flowcontracts/failure_flow_closed_set.hpp>

namespace flowcontracts {

struct FailureTransformationLink {
    long long id = -1;
    long long from_plan = -1;
    std::string from_failure_type;
    long long from_transition = -1;
    long long to_plan = -1;
    long long successor_disposition = -1;
    long long successor_operation = -1;
    std::string successor_failure_type;
    std::string correlation_law;
    std::string payload_law;
    std::string commit_law;
    std::string provenance_law;
};

struct FailureFlowTwoStageChain {
    long long id = -1;
    std::string schedule;
    FailureFlowClosedSetPlan first;
    FailureTransformationLink link;
    FailureFlowClosedSetPlan second;
};

struct FailureFlowChainReceipt {
    long long chain = -1;
    long long link = -1;
    FailureFlowReceipt transformed;
    FailureFlowReceipt recovered;
    std::string final_disposition;
    std::string correlation;
    std::string closure_law;
};

inline json::Value failure_flow_chain_fact(const FailureFlowTwoStageChain& chain) {
    using namespace json;
    return Object{
        {"format", "lyraform.failure_flow_chain"}, {"version", Integer{1}},
        {"status", "ready"}, {"chain_id", Integer(chain.id)},
        {"schedule", chain.schedule},
        {"first_plan", failure_flow_closed_set_plan_fact(chain.first)},
        {"link", Object{{"link_id", Integer(chain.link.id)},
                        {"from_plan_id", Integer(chain.link.from_plan)},
                        {"from_failure_type", chain.link.from_failure_type},
                        {"from_transition_id", Integer(chain.link.from_transition)},
                        {"to_plan_id", Integer(chain.link.to_plan)},
                        {"successor_disposition_id", Integer(chain.link.successor_disposition)},
                        {"successor_operation_id", Integer(chain.link.successor_operation)},
                        {"successor_failure_type", chain.link.successor_failure_type},
                        {"correlation_law", chain.link.correlation_law},
                        {"payload_law", chain.link.payload_law},
                        {"commit_law", chain.link.commit_law},
                        {"provenance_law", chain.link.provenance_law}}},
        {"second_plan", failure_flow_closed_set_plan_fact(chain.second)}};
}

inline FailureFlowTwoStageChain read_failure_flow_chain(
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
    if (text(root, "format", path) != "lyraform.failure_flow_chain" ||
        number(root, "version", path) != 1 || text(root, "status", path) != "ready")
        throw Error(path, "unsupported or non-executable failure-flow chain");
    FailureFlowTwoStageChain chain;
    chain.id = number(root, "chain_id", path);
    chain.schedule = text(root, "schedule", path);
    chain.first = read_failure_flow_closed_set_plan(
        required(root, "first_plan", path), path + ".first_plan");
    const auto link_path = path + ".link";
    const auto& link = object(required(root, "link", path), link_path);
    chain.link = {number(link, "link_id", link_path),
                  number(link, "from_plan_id", link_path),
                  text(link, "from_failure_type", link_path),
                  number(link, "from_transition_id", link_path),
                  number(link, "to_plan_id", link_path),
                  number(link, "successor_disposition_id", link_path),
                  number(link, "successor_operation_id", link_path),
                  text(link, "successor_failure_type", link_path),
                  text(link, "correlation_law", link_path),
                  text(link, "payload_law", link_path),
                  text(link, "commit_law", link_path),
                  text(link, "provenance_law", link_path)};
    chain.second = read_failure_flow_closed_set_plan(
        required(root, "second_plan", path), path + ".second_plan");
    return chain;
}

inline std::string failure_flow_chain_refusal(const FailureFlowTwoStageChain& chain) {
    if (chain.id < 0 || chain.schedule != "serial_transform_then_recover_v1")
        return "invalid two-stage failure-flow chain identity or schedule";
    const auto first_error = failure_flow_closed_set_plan_refusal(chain.first);
    if (!first_error.empty()) return first_error;
    const auto second_error = failure_flow_closed_set_plan_refusal(chain.second);
    if (!second_error.empty()) return second_error;
    if (chain.first.id == chain.second.id || chain.first.producer.id == chain.second.producer.id)
        return "two-stage failure-flow chain is cyclic or identity-colliding";
    if (chain.link.id < 0 || chain.link.from_plan != chain.first.id ||
        chain.link.to_plan != chain.second.id ||
        chain.link.successor_disposition != chain.second.producer.id ||
        chain.link.successor_operation != chain.second.producer.operation ||
        chain.link.correlation_law != "preserve_correlation" ||
        chain.link.payload_law != "response_payload_to_failure_payload" ||
        chain.link.commit_law != "preserve_no_commit" ||
        chain.link.provenance_law != "response_provenance_to_successor_origin")
        return "transformation handoff identity or evidence law changed";
    try {
        const auto& first_route = selected_failure_route(
            chain.first, chain.link.from_failure_type);
        const auto& first_transition = selected_failure_transition(chain.first, first_route);
        if (first_transition.id != chain.link.from_transition ||
            first_transition.response_class != "transform" ||
            first_transition.outgoing_disposition != "failure" ||
            first_transition.outgoing_payload_type != chain.link.successor_failure_type)
            return "handoff source is not the exact selected transformation";
        if (!std::count(chain.second.producer.failure_types.begin(),
                        chain.second.producer.failure_types.end(),
                        chain.link.successor_failure_type))
            return "handoff failure type is outside the successor producer set";
        const auto& second_route = selected_failure_route(
            chain.second, chain.link.successor_failure_type);
        const auto& second_transition = selected_failure_transition(
            chain.second, second_route);
        if (second_transition.response_class != "recover" ||
            second_transition.outgoing_disposition != "success" ||
            second_transition.obligation_transition != "close_original")
            return "two-stage failure-flow chain must finish in recovery";
    } catch (const json::Error&) {
        return "transformation handoff route is missing";
    }
    return {};
}

inline FailureEnvelope successor_failure_envelope(
    const FailureFlowTwoStageChain& chain, const FailureFlowReceipt& receipt) {
    const auto chain_error = failure_flow_chain_refusal(chain);
    if (!chain_error.empty()) throw json::Error("$.failure_flow_chain", chain_error);
    if (receipt.plan != chain.first.id ||
        receipt.transition != chain.link.from_transition ||
        receipt.incoming_failure_type != chain.link.from_failure_type ||
        receipt.outgoing_disposition != "failure" ||
        receipt.outgoing_payload_type != chain.link.successor_failure_type ||
        receipt.obligation_transition != "linked_successor" ||
        receipt.successor_obligation < 0 || receipt.correlation.empty() ||
        receipt.origin_commit != "no_commit" || receipt.response_provenance.empty())
        throw json::Error("$.failure_flow_chain.link",
                          "transform receipt does not prove the successor handoff");
    return {chain.link.successor_disposition, receipt.successor_obligation,
            chain.link.successor_operation, receipt.response_attempt,
            receipt.correlation, chain.link.successor_failure_type,
            receipt.outgoing_payload, receipt.origin_commit,
            receipt.response_provenance};
}

inline std::string failure_flow_chain_receipt_refusal(
    const FailureFlowChainReceipt& receipt, const FailureFlowTwoStageChain& chain,
    const FailureEnvelope& origin) {
    const auto chain_error = failure_flow_chain_refusal(chain);
    if (!chain_error.empty()) return chain_error;
    if (receipt.chain != chain.id || receipt.link != chain.link.id ||
        receipt.final_disposition != "success" ||
        receipt.correlation != origin.correlation ||
        receipt.closure_law != "linked_successor_recovered")
        return "failure-flow chain receipt summary changed";
    const auto first_error = failure_flow_receipt_refusal(
        receipt.transformed, chain.first, origin);
    if (!first_error.empty()) return first_error;
    try {
        const auto successor = successor_failure_envelope(chain, receipt.transformed);
        const auto second_error = failure_flow_receipt_refusal(
            receipt.recovered, chain.second, successor);
        if (!second_error.empty()) return second_error;
        if (receipt.recovered.original_obligation !=
                receipt.transformed.successor_obligation ||
            receipt.recovered.successor_obligation != -1)
            return "successor obligation was not closed by recovery";
    } catch (const json::Error&) {
        return "failure-flow chain successor handoff is invalid";
    }
    return {};
}

inline FailureFlowChainReceipt execute_failure_flow_chain(
    const FailureFlowTwoStageChain& chain, const FailureEnvelope& origin,
    const std::string& transform_attempt, long long successor_obligation,
    const std::string& recovery_attempt,
    const std::map<long long, FailureResponseCallable>& first_callables,
    const std::map<long long, FailureResponseCallable>& second_callables) {
    const auto chain_error = failure_flow_chain_refusal(chain);
    if (!chain_error.empty()) throw json::Error("$.failure_flow_chain", chain_error);
    if (origin.failure_type != chain.link.from_failure_type)
        throw json::Error("$.failure_flow_chain.origin",
                          "origin failure type does not select the chain link");
    auto transformed = execute_failure_flow(chain.first, origin, transform_attempt,
                                            successor_obligation, first_callables);
    const auto successor = successor_failure_envelope(chain, transformed);
    auto recovered = execute_failure_flow(chain.second, successor, recovery_attempt,
                                          -1, second_callables);
    FailureFlowChainReceipt receipt{chain.id, chain.link.id, std::move(transformed),
                                    std::move(recovered), "success",
                                    origin.correlation,
                                    "linked_successor_recovered"};
    const auto receipt_error = failure_flow_chain_receipt_refusal(receipt, chain, origin);
    if (!receipt_error.empty())
        throw json::Error("$.failure_flow_chain.receipt", receipt_error);
    return receipt;
}

} // namespace flowcontracts
