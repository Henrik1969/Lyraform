#pragma once

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace flowcontracts {

// A projection of existing operation identities, not a new execution order.
// Both structured backends order statements within each block by statement_id.
struct OutcomeExecutionOperation {
    long long id = -1, statement = -1, block = -1, function = -1, result = -1;
    long long then_block = -1, else_block = -1, body_block = -1;
    std::string kind;
    bool uses_value = false, disposes_value = false, borrows_value = false;
};

inline std::string outcome_execution_refusal(
    const std::vector<OutcomeExecutionOperation>& operations,
    long long producer_id, long long projection_id, long long success_id,
    long long failure_id, long long dispose_id, const std::set<long long>& zero_symbols) {
    std::map<long long, const OutcomeExecutionOperation*> by_id;
    for (const auto& op : operations)
        if (!by_id.emplace(op.id, &op).second) return "duplicate outcome operation identity";
    for (auto id : {producer_id, projection_id, success_id, failure_id, dispose_id})
        if (!by_id.count(id)) return "missing outcome execution operation";
    const auto& producer = *by_id.at(producer_id);
    const auto& projection = *by_id.at(projection_id);
    const auto& success = *by_id.at(success_id);
    const auto& failure = *by_id.at(failure_id);
    const auto& dispose = *by_id.at(dispose_id);
    const auto end = std::max(success.statement, failure.statement);
    if (producer.block < 0 || producer.result < 0 || projection.result < 0 ||
        producer.statement < 0 || projection.statement <= producer.statement ||
        success.statement <= projection.statement || failure.statement <= projection.statement ||
        success.statement == failure.statement)
        return "outcome production, projection and handling are not ordered";
    for (const auto* op : {&projection, &success, &failure})
        if (op->block != producer.block || op->function != producer.function)
            return "outcome accounting must use sibling statements in the producer function";
    if (success.else_block >= 0 || failure.else_block >= 0 ||
        success.then_block < 0 || failure.then_block < 0 ||
        success.then_block == failure.then_block ||
        success.then_block == producer.block || failure.then_block == producer.block)
        return "outcome accounting requires two distinct simple branch blocks";

    std::map<long long, int> zero_definitions;
    int disposals = 0;
    for (const auto& op : operations) {
        // No other edge may enter the producer or either accounting block.
        for (auto child : {op.then_block, op.else_block, op.body_block}) {
            if (child < 0) continue;
            if (child == producer.block ||
                (child == success.then_block && !(op.id == success_id && child == op.then_block)) ||
                (child == failure.then_block && !(op.id == failure_id && child == op.then_block)))
                return "outcome accounting is nested or has an additional control-flow entry";
        }
        const bool handling = op.block == success.then_block || op.block == failure.then_block;
        if (handling && op.function != producer.function)
            return "outcome accounting block crosses a function boundary";
        if (op.function != producer.function) continue;
        if (handling && (op.kind == "return_value" || op.kind == "branch" || op.kind == "loop"))
            return "outcome handling contains an unsupported exit or nested control flow";
        if (op.block == producer.block && op.statement <= end &&
            ((op.kind == "return_value") ||
             (op.statement >= producer.statement && (op.kind == "branch" || op.kind == "loop") &&
              op.id != success_id && op.id != failure_id)))
            return "outcome handling can be bypassed by an exit or control flow";
        if (op.result == projection.result && op.id != projection_id)
            return "outcome code projection is overwritten";
        if (zero_symbols.count(op.result)) {
            if (op.kind != "value_definition" || op.block != producer.block ||
                op.statement >= std::min(success.statement, failure.statement) ||
                ++zero_definitions[op.result] != 1)
                return "outcome zero comparison value is overwritten or unavailable";
        }
        if (op.result == producer.result && op.id != producer_id &&
            !(op.kind == "value_definition" && op.statement == producer.statement && op.block == producer.block))
            return "tagged outcome owner is overwritten";
        if (op.uses_value) {
            if (op.block != success.then_block)
                return "outcome value is outside its success block";
            if (op.disposes_value) {
                ++disposals;
                if (op.id != dispose_id) return "outcome has an additional disposal";
            } else {
                if (!op.borrows_value) return "outcome value use is not a direct read-only call borrow";
                if (op.statement >= dispose.statement)
                    return "outcome value is used at or after disposal";
            }
        }
    }
    if (!dispose.disposes_value || disposals != 1)
        return "outcome must have exactly one canonical disposal";
    for (auto symbol : zero_symbols)
        if (zero_definitions[symbol] != 1) return "outcome zero definition is missing";
    return {};
}

} // namespace flowcontracts
