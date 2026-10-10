#pragma once
#include <flowcontracts/failure_flow_execution.hpp>
#include <optional>

namespace flowcontracts {
// One reference recovery epoch. This is not a source/runtime budget policy.
// Admission reserves 64 KiB of live evidence and 16 KiB for a closure receipt;
// there is one live obligation, one response transition, and no recursion.
inline constexpr std::size_t disposition_epoch_live_bytes = 65536;
inline constexpr std::size_t disposition_epoch_receipt_bytes = 16384;

inline void validate_disposition_correlation(const json::Value& correlation,
                                              const FailureEnvelope& envelope) {
    using namespace json;
    const auto& c = object(correlation, "$.correlation");
    if (c.size() != 5) throw Error("$.correlation", "exact correlation fields required");
    for (const auto* field : {"graph_id", "signal_id", "delivery_id", "activation_id", "attempt_id"})
        if (string(required(c, field, "$.correlation"), "$.correlation." + std::string(field)).empty())
            throw Error("$.correlation", "missing correlation identity");
    if (string(required(c, "attempt_id", "$"), "$") != envelope.attempt ||
        serialize(correlation) != envelope.correlation)
        throw Error("$.correlation", "envelope lost signal/delivery/activation/attempt correlation");
}

inline json::Value disposition_epoch_closure(long long epoch, const std::string& boundary,
                                             const FailureFlowPlan& plan,
                                             const FailureEnvelope& envelope,
                                             const json::Value& correlation,
                                             const FailureFlowReceipt& receipt) {
    using namespace json;
    const auto refusal = failure_flow_receipt_refusal(receipt, plan, envelope);
    if (!refusal.empty()) throw Error("$.receipt", refusal);
    if (epoch < 0 || boundary.empty() || plan.transitions.front().response_class != "recover" ||
        receipt.response_attempt == envelope.attempt)
        throw Error("$.epoch", "one distinct recovering response attempt required");
    validate_disposition_correlation(correlation, envelope);
    Value result = Object{{"format", "lyraform.disposition_epoch_closure"}, {"version", 1},
        {"epoch_id", Integer(epoch)}, {"boundary", boundary},
        {"historical_disposition", "failure"}, {"disposition_id", Integer(envelope.disposition)},
        {"obligation_id", Integer(envelope.obligation)}, {"producer_operation_id", Integer(envelope.producer_operation)},
        {"origin_commit", envelope.origin_commit}, {"origin_provenance", envelope.provenance},
        {"correlation", correlation}, {"policy_selection", failure_policy_selection_fact(plan.selection)},
        {"receipt", failure_flow_receipt_fact(receipt)}, {"final_disposition", "success"},
        {"obligation_state", "closed"}};
    if (serialize(result).size() > disposition_epoch_receipt_bytes)
        throw Error("$.epoch", "closure exceeds reserved evidence budget; retain live evidence");
    return result;
}

inline void validate_disposition_epoch_closure(const json::Value& closure,
                                               long long epoch, const std::string& boundary,
                                               const FailureFlowPlan& plan,
                                               const FailureEnvelope& envelope,
                                               const json::Value& correlation) {
    using namespace json;
    const auto& raw = object(closure, "$.closure");
    const auto receipt = read_failure_flow_receipt(required(raw, "receipt", "$.closure"), "$.closure.receipt");
    if (serialize(closure) != serialize(disposition_epoch_closure(epoch, boundary, plan, envelope, correlation, receipt)))
        throw Error("$.closure", "forged or truncated closure evidence");
}

class DispositionEvidenceEpoch {
public:
    DispositionEvidenceEpoch(long long epoch, std::string boundary, FailureFlowPlan plan,
                            FailureEnvelope envelope, json::Value correlation,
                            bool required_observer)
        : epoch_(epoch), boundary_(std::move(boundary)), observer_(required_observer) {
        const auto refusal = failure_envelope_refusal(envelope, plan);
        if (!refusal.empty()) throw json::Error("$.epoch", refusal);
        if (epoch < 0 || boundary_.empty() || plan.transitions.front().response_class != "recover")
            throw json::Error("$.epoch", "only one recovering reference epoch is admitted");
        validate_disposition_correlation(correlation, envelope);
        const auto bytes = json::serialize(failure_flow_plan_fact(plan)).size() +
            json::serialize(failure_envelope_fact(envelope)).size() + json::serialize(correlation).size() +
            boundary_.size() + std::to_string(epoch).size();
        if (bytes > disposition_epoch_live_bytes)
            throw json::Error("$.epoch", "evidence budget admission refusal; no runtime attempt created");
        evidence_.emplace(Evidence{std::move(plan), std::move(envelope), std::move(correlation)});
    }
    // No implicit copying of an owned live obligation or its release authority.
    DispositionEvidenceEpoch(const DispositionEvidenceEpoch&) = delete;
    DispositionEvidenceEpoch& operator=(const DispositionEvidenceEpoch&) = delete;
    bool live() const { return evidence_.has_value(); }
    bool closed() const { return closure_.has_value(); }
    void accept(const json::Value& closure) {
        if (!evidence_ || closure_) throw json::Error("$.epoch", "duplicate or post-release closure");
        validate_disposition_epoch_closure(closure, epoch_, boundary_, evidence_->plan,
            evidence_->envelope, evidence_->correlation);
        closure_ = closure;
    }
    void observer_published() { observer_ = false; }
    void release() {
        if (!closure_ || observer_) throw json::Error("$.epoch", "live obligation or required observer prevents release");
        evidence_.reset();
    }
    const json::Value& closure() const {
        if (!closure_) throw json::Error("$.epoch", "open obligation has no closure");
        return *closure_;
    }
private:
    struct Evidence { FailureFlowPlan plan; FailureEnvelope envelope; json::Value correlation; };
    long long epoch_;
    std::string boundary_;
    bool observer_;
    std::optional<Evidence> evidence_;
    std::optional<json::Value> closure_;
};
} // namespace flowcontracts
