#pragma once

#include <flowcontracts/json.hpp>
#include <flowcontracts/outcome_execution.hpp>
#include <flowcontracts/ownership_transfer.hpp>

#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace flowcontracts {

inline bool disposition_contains_field(const json::Value& value, json::Integer owner,
                                       std::string_view field_name) {
    using namespace json;
    if (const auto* object_value = std::get_if<Object>(&value)) {
        const auto* kind = optional(*object_value, "kind");
        const auto* field = optional(*object_value, "field");
        const auto* base = optional(*object_value, "base");
        if (kind && field && base && string(*kind, "$.kind") == "field_access" &&
            string(*field, "$.field") == field_name) {
            const auto& base_object = object(*base, "$.base");
            if (const auto* symbol = optional(base_object, "symbol_id");
                symbol && integer(*symbol, "$.base.symbol_id") == owner) return true;
        }
        for (const auto& [key, child] : *object_value) {
            (void)key;
            if (disposition_contains_field(child, owner, field_name)) return true;
        }
    } else if (const auto* array_value = std::get_if<Array>(&value)) {
        for (const auto& child : *array_value)
            if (disposition_contains_field(child, owner, field_name)) return true;
    }
    return false;
}

inline bool disposition_direct_field(const json::Value& value, json::Integer owner,
                                     std::string_view field_name) {
    using namespace json;
    const auto integer = [](const Value& v, std::string_view p = "$") { return json::integer(v, p); };
    const auto string = [](const Value& v, std::string_view p = "$") -> const std::string& { return json::string(v, p); };
    const auto& operand = object(value);
    const auto* kind = optional(operand, "kind");
    const auto* field = optional(operand, "field");
    const auto* base = optional(operand, "base");
    if (!kind || !field || !base || string(*kind) != "field_access" || string(*field) != field_name)
        return false;
    const auto& identity = object(*base);
    const auto* symbol = optional(identity, "symbol_id");
    const auto* base_kind = optional(identity, "kind");
    return symbol && base_kind && string(*base_kind) == "identifier" && integer(*symbol) == owner;
}

inline bool disposition_is_symbol(const json::Value& value, json::Integer symbol_id) {
    using namespace json;
    const auto* object_value = std::get_if<Object>(&value);
    if (!object_value) return false;
    const auto* kind = optional(*object_value, "kind");
    const auto* symbol = optional(*object_value, "symbol_id");
    return kind && symbol && string(*kind, "$.kind") == "identifier" &&
           integer(*symbol, "$.symbol_id") == symbol_id;
}

inline bool disposition_requires_text_outcome_authority(const json::Object& operation) {
    using namespace json;
    const auto* raw_kind = optional(operation, "kind");
    const auto* raw_provider = optional(operation, "provider");
    if (!raw_kind || !raw_provider || string(*raw_kind, "$.operation.kind") != "text_outcome") return false;
    const auto& provider = object(*raw_provider, "$.operation.provider");
    const auto* return_type = optional(provider, "return_type");
    return return_type && string(*return_type, "$.operation.provider.return_type") == "TextOutcome";
}

inline void validate_disposition_facts(const json::Value& value, std::string_view base) {
    using namespace json;
    const auto integer = [](const Value& v, std::string_view p = "$") { return json::integer(v, p); };
    const auto string = [](const Value& v, std::string_view p = "$") -> const std::string& { return json::string(v, p); };
    const auto& plan = object(value, base);
    const auto* raw_facts = optional(plan, "disposition_facts");
    if (!raw_facts) {
        // Artifacts predating guard/disposition authority make no claim. Once
        // guard facts exist, removing disposition authority is a downgrade.
        if (const auto* raw_guards = optional(plan, "guard_facts");
            raw_guards && !array(*raw_guards, std::string(base) + ".guard_facts").empty())
            throw Error(std::string(base) + ".disposition_facts", "guard authority lacks disposition authority");
        if (const auto* raw_operations = optional(plan, "operations")) {
            for (const auto& raw_operation : array(*raw_operations, std::string(base) + ".operations")) {
                const auto& operation = object(raw_operation, std::string(base) + ".operations[]");
                if (disposition_requires_text_outcome_authority(operation))
                    throw Error(std::string(base) + ".disposition_facts", "tagged outcome authority lacks disposition authority");
            }
        }
        return;
    }
    const auto& facts = array(*raw_facts, std::string(base) + ".disposition_facts");
    const auto& operations = array(required(plan, "operations", base), std::string(base) + ".operations");
    const auto& guards = array(required(plan, "guard_facts", base), std::string(base) + ".guard_facts");
    const auto status = string(required(plan, "status", base), std::string(base) + ".status");
    if (status != "ready" && !facts.empty())
        throw Error(std::string(base) + ".disposition_facts", "blocked plan carries executable disposition authority");

    std::map<Integer, const Object*> operations_by_id;
    std::set<Integer> text_outcome_operations;
    std::set<Integer> zero_symbols;
    for (const auto& raw_operation : operations) {
        const auto& operation = object(raw_operation);
        const auto operation_id = integer(required(operation, "id"), "$.operation.id");
        operations_by_id.emplace(operation_id, &operation);
        const auto kind = string(required(operation, "kind"), "$.operation.kind");
        if (disposition_requires_text_outcome_authority(operation)) text_outcome_operations.insert(operation_id);
        if (kind == "value_definition") {
            const auto* raw_operands = optional(operation, "operands");
            const auto* raw_result = optional(operation, "result_symbol_id");
            if (raw_operands && raw_result) {
                const auto& operands = array(*raw_operands, "$.operation.operands");
                if (operands.size() == 1) {
                    const auto& operand = object(operands.front(), "$.operation.operands[0]");
                    const auto* operand_kind = optional(operand, "kind");
                    const auto* operand_value = optional(operand, "value");
                    if (operand_kind && operand_value && string(*operand_kind, "$.operand.kind") == "integer_literal" &&
                        string(*operand_value, "$.operand.value") == "0")
                        zero_symbols.insert(integer(*raw_result, "$.operation.result_symbol_id"));
                }
            }
        }
    }
    std::map<Integer, const Object*> guards_by_id;
    std::map<Integer, std::vector<Integer>> safe_guards_by_operation;
    for (const auto& raw_guard : guards) {
        const auto& guard = object(raw_guard);
        const auto id = integer(required(guard, "fact_id"), "$.guard_fact.fact_id");
        guards_by_id.emplace(id, &guard);
        if (string(required(guard, "event"), "$.guard_fact.event") != "transition" ||
            string(required(guard, "classification"), "$.guard_fact.classification") != "proven_safe") continue;
        const auto* affected = optional(guard, "affected_operation_id");
        if (affected && !std::holds_alternative<std::nullptr_t>(*affected))
            safe_guards_by_operation[integer(*affected, "$.guard_fact.affected_operation_id")].push_back(id);
    }

    std::set<Integer> fact_ids;
    std::set<Integer> covered_operations;
    for (std::size_t index = 0; index < facts.size(); ++index) {
        const auto path = std::string(base) + ".disposition_facts[" + std::to_string(index) + "]";
        const auto& fact = object(facts[index], path);
        if (string(required(fact, "format", path), path + ".format") != "lyraform.disposition_fact" ||
            integer(required(fact, "version", path), path + ".version") != 1)
            throw Error(path, "unsupported disposition fact contract");
        const auto fact_id = integer(required(fact, "fact_id", path), path + ".fact_id");
        if (fact_id < 0 || !fact_ids.insert(fact_id).second)
            throw Error(path + ".fact_id", "invalid or duplicate disposition fact identity");
        const auto operation_id = integer(required(fact, "operation_id", path), path + ".operation_id");
        const auto found = operations_by_id.find(operation_id);
        if (found == operations_by_id.end() || !covered_operations.insert(operation_id).second)
            throw Error(path + ".operation_id", "missing or duplicate disposition operation");
        const auto& operation = *found->second;
        const auto operation_kind = string(required(operation, "kind"), path + ".operation.kind");
        for (const auto field : {"statement_id", "expression_id", "scope_id"}) {
            const auto identity = integer(required(fact, field, path), path + "." + field);
            if (identity < 0 || identity != integer(required(operation, field), path + ".operation." + field))
                throw Error(path + "." + field, "disposition source identity mismatch");
        }
        if (const auto* owner = optional(fact, "function_symbol_id")) {
            const auto* operation_owner = optional(operation, "function_symbol_id");
            if (!operation_owner || integer(*owner, path + ".function_symbol_id") < 0 ||
                integer(*owner, path + ".function_symbol_id") != integer(*operation_owner, path + ".operation.function_symbol_id"))
                throw Error(path + ".function_symbol_id", "disposition owner mismatch");
        }
        if (string(required(fact, "completion", path), path + ".completion") != "exactly_one")
            throw Error(path + ".completion", "disposition completion must be exclusive");

        auto validate_provenance = [&]() {
            const auto& provenance = object(required(fact, "provenance", path), path + ".provenance");
            const auto statement = integer(required(fact, "statement_id", path), path + ".statement_id");
            if (string(required(provenance, "source", path), path + ".provenance.source").empty() ||
                string(required(provenance, "ast_path", path), path + ".provenance.ast_path") !=
                    "/statement_pool/" + std::to_string(statement) ||
                integer(required(provenance, "line", path), path + ".provenance.line") < 1 ||
                integer(required(provenance, "column", path), path + ".provenance.column") < 1)
                throw Error(path + ".provenance", "invalid disposition provenance");
        };

        if (operation_kind == "text_outcome") {
            const auto initial_owner = integer(required(operation, "result_symbol_id", path), path + ".operation.result_symbol_id");
            auto owner = initial_owner;
            auto accounting_origin = operation_id;
            const auto& projection = object(required(operation, "result_outcome", path), path + ".operation.result_outcome");
            if (string(required(projection, "type", path), path + ".operation.result_outcome.type") != "Outcome" ||
                string(required(projection, "representation", path), path + ".operation.result_outcome.representation") != "tagged" ||
                string(required(projection, "success_type", path), path + ".operation.result_outcome.success_type") != "Text" ||
                string(required(projection, "failure_type", path), path + ".operation.result_outcome.failure_type") != "TextFailure")
                throw Error(path, "Text outcome projection disagrees with disposition authority");
            const auto& projection_codes = array(required(projection, "failure_codes", path), path + ".operation.result_outcome.failure_codes");

            const auto& possible = array(required(fact, "possible_dispositions", path), path + ".possible_dispositions");
            if (possible.size() != 2) throw Error(path + ".possible_dispositions", "Text outcome requires success and failure dispositions");
            const auto& success = object(possible[0], path + ".possible_dispositions[0]");
            const auto& failure = object(possible[1], path + ".possible_dispositions[1]");
            auto validate_route = [&](const Object& disposition, std::string_view kind,
                                      std::string_view payload, std::string_view item_path) {
                if (string(required(disposition, "kind", item_path), std::string(item_path) + ".kind") != kind ||
                    string(required(disposition, "payload_type", item_path), std::string(item_path) + ".payload_type") != payload ||
                    string(required(disposition, "commit", item_path), std::string(item_path) + ".commit") != "atomic_tagged_result")
                    throw Error(std::string(item_path), "Text outcome disposition changed");
                const auto& route = object(required(disposition, "route", item_path), std::string(item_path) + ".route");
                if (string(required(route, "kind", item_path), std::string(item_path) + ".route.kind") != "tagged_owner" ||
                    integer(required(route, "symbol_id", item_path), std::string(item_path) + ".route.symbol_id") != owner)
                    throw Error(std::string(item_path) + ".route", "Text outcome owner route mismatch");
            };
            validate_route(success, "success", "Text", path + ".possible_dispositions[0]");
            validate_route(failure, "failure", "TextFailure", path + ".possible_dispositions[1]");
            const auto& fact_codes = array(required(failure, "failure_codes", path), path + ".possible_dispositions[1].failure_codes");
            if (fact_codes != projection_codes)
                throw Error(path + ".possible_dispositions[1].failure_codes", "Text failure codes disagree with tagged projection");
            if (!array(required(fact, "eliminated_dispositions", path), path + ".eliminated_dispositions").empty())
                throw Error(path + ".eliminated_dispositions", "Text outcome cannot eliminate a declared completion");

            const auto& obligation = object(required(fact, "obligation", path), path + ".obligation");
            const auto* raw_transfer=optional(obligation,"ownership_transfer");
            const auto* raw_transfers=optional(obligation,"ownership_transfers");
            if(raw_transfer && raw_transfers)
                throw Error(path+".obligation","ownership transfer authority is ambiguous");
            if (raw_transfer) {
                const auto transfer=read_ownership_transfer(*raw_transfer,path+".obligation.ownership_transfer");
                const auto& provider=object(required(operation,"provider",path));
                if(transfer.producer!=operation_id || transfer.source_owner!=initial_owner ||
                    transfer.value_type!=string(required(provider,"return_type")) ||
                    transfer.obligation!="operation:"+std::to_string(operation_id)+":outcome")
                    throw Error(path+".obligation.ownership_transfer","ownership transfer changes the producer obligation");
                const auto error=validate_ownership_transfer_projection(transfer,plan);
                if(!error.empty()) throw Error(path+".obligation.ownership_transfer",error);
                owner=transfer.destination_owner;
                accounting_origin=transfer.call;
            } else if(raw_transfers) {
                const auto& items=array(*raw_transfers,path+".obligation.ownership_transfers");
                std::vector<OwnershipTransfer> transfers;
                for(std::size_t hop=0;hop<items.size();++hop)
                    transfers.push_back(read_ownership_transfer(items[hop],path+".obligation.ownership_transfers["+std::to_string(hop)+"]"));
                const auto& provider=object(required(operation,"provider",path));
                if(transfers.empty() || transfers.front().producer!=operation_id ||
                    transfers.front().source_owner!=initial_owner ||
                    transfers.front().value_type!=string(required(provider,"return_type")) ||
                    transfers.front().obligation!="operation:"+std::to_string(operation_id)+":outcome")
                    throw Error(path+".obligation.ownership_transfers","ownership forwarding changes the producer obligation");
                const auto error=validate_ownership_transfer_chain_projection(transfers,plan);
                if(!error.empty()) throw Error(path+".obligation.ownership_transfers",error);
                owner=transfers.back().destination_owner;
                accounting_origin=transfers.back().call;
            }

            if (string(required(obligation, "kind", path), path + ".obligation.kind") != "must_account" ||
                string(required(obligation, "status", path), path + ".obligation.status") != "accounted" ||
                string(required(obligation, "identity", path), path + ".obligation.identity") !=
                    "operation:" + std::to_string(operation_id) + ":outcome" ||
                integer(required(obligation, "owner_symbol_id", path), path + ".obligation.owner_symbol_id") != owner ||
                string(required(obligation, "proof", path), path + ".obligation.proof") != "complementary_zero_code_branches")
                throw Error(path + ".obligation", "Text outcome accounting authority changed");

            const auto operation_for = [&](std::string_view key) -> const Object& {
                const auto id = integer(required(obligation, key, path), path + ".obligation." + std::string(key));
                const auto selected = operations_by_id.find(id);
                if (selected == operations_by_id.end()) throw Error(path + ".obligation." + std::string(key), "accounting operation is absent");
                return *selected->second;
            };
            const auto code_symbol = integer(required(obligation, "code_symbol_id", path), path + ".obligation.code_symbol_id");
            const auto& code_projection = operation_for("code_projection_operation_id");
            const auto& code_operands = array(required(code_projection, "operands", path), path + ".code_projection.operands");
            if (string(required(code_projection, "kind", path), path + ".code_projection.kind") != "value_definition" ||
                integer(required(code_projection, "result_symbol_id", path), path + ".code_projection.result_symbol_id") != code_symbol ||
                code_operands.size() != 1 || !disposition_direct_field(code_operands.front(), owner, "code"))
                throw Error(path + ".obligation.code_projection_operation_id", "outcome code projection mismatch");

            const auto success_block = integer(required(obligation, "success_block_id", path), path + ".obligation.success_block_id");
            const auto failure_block = integer(required(obligation, "failure_block_id", path), path + ".obligation.failure_block_id");
            const auto& success_branch = operation_for("success_branch_operation_id");
            const auto& failure_branch = operation_for("failure_branch_operation_id");
            std::set<long long> comparison_zeros;
            auto validate_branch = [&](const Object& branch, std::string_view relation, Integer block, std::string_view branch_path) {
                if (string(required(branch, "kind", branch_path), std::string(branch_path) + ".kind") != "branch" ||
                    integer(required(branch, "then_block_id", branch_path), std::string(branch_path) + ".then_block_id") != block)
                    throw Error(std::string(branch_path), "outcome accounting branch linkage mismatch");
                const auto& operands = array(required(branch, "operands", branch_path), std::string(branch_path) + ".operands");
                if (operands.size() != 1) throw Error(std::string(branch_path), "outcome branch operand is missing");
                const auto& condition = object(operands.front(), branch_path);
                if (string(required(condition, "kind", branch_path), std::string(branch_path) + ".kind") != "binary" ||
                    string(required(condition, "operator", branch_path), std::string(branch_path) + ".operator") != relation)
                    throw Error(std::string(branch_path), "outcome branches are not complementary");
                const auto& left = required(condition, "left", branch_path);
                const auto& right = required(condition, "right", branch_path);
                const auto is_zero = [&](const Value& operand) {
                    const auto* operand_object = std::get_if<Object>(&operand);
                    if (!operand_object) return false;
                    const auto* kind = optional(*operand_object, "kind");
                    if (!kind) return false;
                    if (string(*kind, "$.branch.operand.kind") == "integer_literal") {
                        const auto* literal = optional(*operand_object, "value");
                        return literal && string(*literal, "$.branch.operand.value") == "0";
                    }
                    const auto* symbol = optional(*operand_object, "symbol_id");
                    if (string(*kind, "$.branch.operand.kind") == "identifier" && symbol &&
                        zero_symbols.count(integer(*symbol, "$.branch.operand.symbol_id"))) {
                        comparison_zeros.insert(integer(*symbol));
                        return true;
                    }
                    return false;
                };
                if (!((disposition_is_symbol(left, code_symbol) && is_zero(right)) ||
                      (disposition_is_symbol(right, code_symbol) && is_zero(left))))
                    throw Error(std::string(branch_path), "outcome branch does not compare its code projection with zero");
            };
            validate_branch(success_branch, "==", success_block, path + ".success_branch");
            validate_branch(failure_branch, "!=", failure_block, path + ".failure_branch");
            if (success_block < 0 || failure_block < 0 || success_block == failure_block)
                throw Error(path + ".obligation", "outcome accounting blocks are invalid");

            const auto& success_uses = array(required(obligation, "success_value_operation_ids", path), path + ".obligation.success_value_operation_ids");
            if (success_uses.size() < 2) throw Error(path + ".obligation.success_value_operation_ids", "success value is not consumed and disposed");
            std::set<Integer> success_ids;
            for (const auto& raw_id : success_uses) {
                const auto id = integer(raw_id, path + ".obligation.success_value_operation_ids[]");
                const auto selected = operations_by_id.find(id);
                if (!success_ids.insert(id).second || selected == operations_by_id.end() ||
                    integer(required(*selected->second, "block_id", path), path + ".success_value.block_id") != success_block ||
                    !disposition_contains_field(*selected->second, owner, "value"))
                    throw Error(path + ".obligation.success_value_operation_ids", "invalid success-value accounting operation");
            }
            const auto dispose_id = integer(required(obligation, "dispose_operation_id", path), path + ".obligation.dispose_operation_id");
            const auto dispose = operations_by_id.find(dispose_id);
            if (!success_ids.count(dispose_id) || dispose == operations_by_id.end() ||
                string(required(*dispose->second, "kind", path), path + ".dispose.kind") != "external_call")
                throw Error(path + ".obligation.dispose_operation_id", "success value disposal mismatch");

            const auto& recovery = array(required(obligation, "failure_recovery_operation_ids", path), path + ".obligation.failure_recovery_operation_ids");
            if (recovery.empty()) throw Error(path + ".obligation.failure_recovery_operation_ids", "failure branch is empty");
            std::set<Integer> recovery_ids;
            for (const auto& raw_id : recovery) {
                const auto id = integer(raw_id, path + ".obligation.failure_recovery_operation_ids[]");
                const auto selected = operations_by_id.find(id);
                if (!recovery_ids.insert(id).second || selected == operations_by_id.end() ||
                    integer(required(*selected->second, "block_id", path), path + ".failure_recovery.block_id") != failure_block ||
                    disposition_contains_field(*selected->second, owner, "value"))
                    throw Error(path + ".obligation.failure_recovery_operation_ids", "invalid failure recovery operation");
            }
            std::size_t code_projection_count = 0;
            std::set<Integer> actual_success_uses;
            std::set<Integer> actual_recovery;
            for (const auto& [candidate_id, candidate] : operations_by_id) {
                const auto candidate_kind = string(required(*candidate, "kind"), path + ".operation.kind");
                const auto candidate_block = integer(required(*candidate, "block_id"), path + ".operation.block_id");
                if (disposition_contains_field(*candidate, owner, "code")) ++code_projection_count;
                if (disposition_contains_field(*candidate, owner, "value")) {
                    if (candidate_block != success_block)
                        throw Error(path + ".obligation", "outcome success value escapes its admitted branch");
                    actual_success_uses.insert(candidate_id);
                }
                if (candidate_block == success_block || candidate_block == failure_block) {
                    if (candidate_kind == "branch" || candidate_kind == "loop")
                        throw Error(path + ".obligation", "nested control flow is outside bounded outcome accounting");
                }
                if (candidate_block == failure_block) actual_recovery.insert(candidate_id);
            }
            if (code_projection_count != 1)
                throw Error(path + ".obligation.code_projection_operation_id", "outcome code must be projected exactly once");
            if (actual_success_uses != success_ids)
                throw Error(path + ".obligation.success_value_operation_ids", "success-value accounting set is not exact");
            if (actual_recovery != recovery_ids)
                throw Error(path + ".obligation.failure_recovery_operation_ids", "failure recovery accounting set is not exact");
            std::vector<OutcomeExecutionOperation> execution;
            for (const auto& [id, candidate] : operations_by_id) {
                auto number = [&](std::string_view key) -> long long {
                    const auto* raw = optional(*candidate, key);
                    return raw ? integer(*raw) : -1;
                };
                bool canonical_dispose = false;
                const auto* raw_provider = optional(*candidate, "provider");
                const auto* raw_operands = optional(*candidate, "operands");
                if (raw_provider && raw_operands && string(required(*candidate, "kind")) == "external_call") {
                    const auto& provider = object(*raw_provider);
                    auto named = [&](std::string_view key, std::string_view expected) {
                        const auto* raw = optional(provider, key);
                        return raw && string(*raw) == expected;
                    };
                    const auto& operands = array(*raw_operands, path + ".operation.operands");
                    canonical_dispose = named("contract", "text_runtime") && named("symbol", "flow_text_dispose") &&
                        named("parameter_types", "Text") && named("return_type", "c_int") &&
                        operands.size() == 1 && disposition_direct_field(operands.front(), owner, "value");
                }
                bool borrows = raw_operands && string(required(*candidate, "kind")) == "external_call";
                if (raw_operands) {
                    const auto& operands = array(*raw_operands, path + ".operands");
                    const auto* raw_resources = optional(*candidate, "argument_resources");
                    for (std::size_t arg = 0; arg < operands.size(); ++arg) {
                        if (!disposition_contains_field(operands[arg], owner, "value")) continue;
                        int borrowed_contracts = 0;
                        if (raw_resources) for (const auto& raw : array(*raw_resources, path + ".argument_resources")) {
                            const auto& resource = object(raw);
                            if (integer(required(resource, "index")) == static_cast<long long>(arg) &&
                                string(required(resource, "type")) == "Text" &&
                                string(required(resource, "ownership")) == "borrowed" &&
                                string(required(resource, "access")) == "read" &&
                                string(required(resource, "lifetime")) == "call") ++borrowed_contracts;
                        }
                        borrows = borrows && borrowed_contracts == 1 &&
                            disposition_direct_field(operands[arg], owner, "value");
                    }
                }
                execution.push_back({id, number("statement_id"), number("block_id"), number("function_symbol_id"),
                    number("result_symbol_id"), number("then_block_id"), number("else_block_id"), number("body_block_id"),
                    string(required(*candidate, "kind")), disposition_contains_field(*candidate, owner, "value"), canonical_dispose, borrows});
            }
            const auto execution_error = outcome_execution_refusal(execution, accounting_origin,
                integer(required(obligation, "code_projection_operation_id")),
                integer(required(obligation, "success_branch_operation_id")),
                integer(required(obligation, "failure_branch_operation_id")), dispose_id, comparison_zeros);
            if (!execution_error.empty()) throw Error(path + ".obligation", execution_error);
            validate_provenance();
            continue;
        }

        if (operation_kind != "assignment")
            throw Error(path, "unsupported bounded disposition operation");

        const auto& possible = array(required(fact, "possible_dispositions", path), path + ".possible_dispositions");
        if (possible.size() != 1) throw Error(path + ".possible_dispositions", "bounded disposition must have one possible completion");
        const auto& success = object(possible.front(), path + ".possible_dispositions[0]");
        if (string(required(success, "kind", path), path + ".possible_dispositions[0].kind") != "success" ||
            string(required(success, "commit", path), path + ".possible_dispositions[0].commit") != "atomic_destination")
            throw Error(path + ".possible_dispositions[0]", "bounded disposition success contract changed");
        const auto payload_type = string(required(success, "payload_type", path), path + ".possible_dispositions[0].payload_type");
        const auto& route = object(required(success, "route", path), path + ".possible_dispositions[0].route");
        if (string(required(route, "kind", path), path + ".possible_dispositions[0].route.kind") != "destination" ||
            integer(required(route, "symbol_id", path), path + ".possible_dispositions[0].route.symbol_id") !=
                integer(required(operation, "result_symbol_id"), path + ".operation.result_symbol_id"))
            throw Error(path + ".possible_dispositions[0].route", "disposition success route mismatch");
        const auto& scalar = object(required(operation, "scalar_fact", path), path + ".operation.scalar_fact");
        if (payload_type != string(required(scalar, "destination_type", path), path + ".operation.scalar_fact.destination_type") ||
            string(required(scalar, "compatibility", path), path + ".operation.scalar_fact.compatibility") != "admitted")
            throw Error(path + ".possible_dispositions[0].payload_type", "disposition success type is not canonical");

        const auto& eliminated = array(required(fact, "eliminated_dispositions", path), path + ".eliminated_dispositions");
        if (eliminated.size() != 1) throw Error(path + ".eliminated_dispositions", "bounded guard failure proof is incomplete");
        const auto& failure = object(eliminated.front(), path + ".eliminated_dispositions[0]");
        if (string(required(failure, "kind", path), path + ".eliminated_dispositions[0].kind") != "failure" ||
            string(required(failure, "payload_type", path), path + ".eliminated_dispositions[0].payload_type") != "GuardViolation" ||
            string(required(failure, "reason", path), path + ".eliminated_dispositions[0].reason") != "proven_guard_preservation")
            throw Error(path + ".eliminated_dispositions[0]", "eliminated guard disposition changed");
        const auto& proofs = array(required(failure, "proof_guard_fact_ids", path), path + ".eliminated_dispositions[0].proof_guard_fact_ids");
        if (proofs.empty()) throw Error(path + ".eliminated_dispositions[0].proof_guard_fact_ids", "guard proof set is empty");
        std::set<Integer> proof_ids;
        for (const auto& raw_proof : proofs) {
            const auto proof_id = integer(raw_proof, path + ".eliminated_dispositions[0].proof_guard_fact_ids[]");
            const auto guard = guards_by_id.find(proof_id);
            if (!proof_ids.insert(proof_id).second || guard == guards_by_id.end())
                throw Error(path, "invalid or duplicate guard proof identity");
            const auto& proof = *guard->second;
            if (string(required(proof, "event"), path + ".guard.event") != "transition" ||
                string(required(proof, "classification"), path + ".guard.classification") != "proven_safe" ||
                integer(required(proof, "affected_operation_id"), path + ".guard.affected_operation_id") != operation_id)
                throw Error(path, "guard proof does not establish this disposition");
        }
        const auto expected = safe_guards_by_operation.find(operation_id);
        if (expected == safe_guards_by_operation.end() ||
            proof_ids != std::set<Integer>(expected->second.begin(), expected->second.end()))
            throw Error(path, "disposition guard proof set is not exact");

        validate_provenance();
    }
    for (const auto& [operation_id, proof_ids] : safe_guards_by_operation) {
        (void)proof_ids;
        if (!covered_operations.count(operation_id))
            throw Error(std::string(base) + ".disposition_facts", "proven guarded operation lacks disposition authority");
    }
    for (const auto operation_id : text_outcome_operations) {
        if (!covered_operations.count(operation_id))
            throw Error(std::string(base) + ".disposition_facts", "tagged outcome operation lacks disposition authority");
    }
}

} // namespace flowcontracts
