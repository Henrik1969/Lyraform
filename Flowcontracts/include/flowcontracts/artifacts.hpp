#pragma once

#include <flowcontracts/json.hpp>
#include <flowcontracts/scheduling.hpp>
#include <flowcontracts/source_graph.hpp>
#include <flowcontracts/graph_execution.hpp>
#include <flowcontracts/binding_evidence.hpp>
#include <flowcontracts/scalar_facts.hpp>
#include <flowcontracts/source_operation_coverage.hpp>
#include <flowcontracts/effect_scheduling.hpp>

#include <functional>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <string_view>

namespace flowcontracts {

struct Header { std::string format; json::Integer version = 0; std::string status; };

inline Header header(const json::Value& value) {
    const auto& root = json::object(value);
    return {json::string(json::required(root, "format"), "$.format"),
            json::integer(json::required(root, "version"), "$.version"),
            json::string(json::required(root, "status"), "$.status")};
}
inline Header require_header(const json::Value& value, std::string_view format, json::Integer version) {
    auto result = header(value);
    if (result.format != format) throw json::Error("$.format", "unsupported artifact format '" + result.format + "'");
    if (result.version != version) throw json::Error("$.version", "unsupported " + result.format + " version " + std::to_string(result.version));
    return result;
}

struct MatrixEntry { json::Integer row = 0; json::Integer column = 0; bool value = true; };
struct MatrixView { std::string name; json::Integer rows = 0; json::Integer columns = 0; std::string semiring; std::string storage; std::vector<MatrixEntry> entries; };
struct SemanticReport {
    Header artifact; std::string source_path; json::Array targets; json::Array external_operations;
    json::Array abi_type_contracts; json::Array aggregate_abi_layouts; json::Array effect_facts;
    json::Array provider_effect_profiles; json::Array effect_access_facts;
    json::Array parallel_candidates; json::Value lowering_plan; std::size_t proven_pure_count = 0;
    std::size_t independent_candidate_count = 0; MatrixView dependency_matrix;
};

inline void validate_effect_access_authority(const json::Object& root,
                                             std::string_view path = "$") {
    const auto* raw_profiles = json::optional(root, "provider_effect_profiles");
    const auto* raw_accesses = json::optional(root, "effect_access_facts");
    if (!raw_profiles && !raw_accesses) return;
    if (!raw_profiles || !raw_accesses)
        throw json::Error(std::string(path), "provider-effect profiles and effect-access facts must travel together");
    const auto& profiles = json::array(*raw_profiles, std::string(path) + ".provider_effect_profiles");
    const auto& accesses = json::array(*raw_accesses, std::string(path) + ".effect_access_facts");
    std::map<json::Integer, ProviderEffectProfile> by_id;
    std::set<std::string> profile_capabilities;
    for (std::size_t index = 0; index < profiles.size(); ++index) {
        const auto item_path = std::string(path) + ".provider_effect_profiles[" + std::to_string(index) + "]";
        auto profile = read_provider_effect_profile(profiles[index], item_path);
        const auto refusal = provider_effect_profile_refusal(profile);
        if (!refusal.empty()) throw json::Error(item_path, refusal);
        const auto identity = capability_identity(profile.capability, item_path + ".capability");
        if (!by_id.emplace(profile.id, profile).second || !profile_capabilities.insert(identity).second)
            throw json::Error(item_path, "duplicate provider-effect profile identity or capability");
    }
    std::set<json::Integer> operations;
    std::set<std::string> accessed_capabilities;
    const auto& plan = json::object(json::required(root, "lowering_plan", path),
                                    std::string(path) + ".lowering_plan");
    const auto& lowering_operations = json::array(
        json::required(plan, "operations", std::string(path) + ".lowering_plan"),
        std::string(path) + ".lowering_plan.operations");
    for (std::size_t index = 0; index < accesses.size(); ++index) {
        const auto item_path = std::string(path) + ".effect_access_facts[" + std::to_string(index) + "]";
        const auto access = read_effect_access(accesses[index], item_path);
        const auto found = by_id.find(access.profile);
        if (found == by_id.end()) throw json::Error(item_path, "effect-access profile is absent");
        const auto refusal = effect_access_refusal(access, found->second);
        if (!refusal.empty()) throw json::Error(item_path, refusal);
        if (!operations.insert(access.operation).second)
            throw json::Error(item_path + ".operation_id", "duplicate effect-access operation identity");
        if (access.operation >= static_cast<json::Integer>(lowering_operations.size()))
            throw json::Error(item_path + ".operation_id", "effect-access operation is absent");
        const auto operation_path = std::string(path) + ".lowering_plan.operations[" +
                                    std::to_string(access.operation) + "]";
        const auto& operation = json::object(lowering_operations[static_cast<std::size_t>(access.operation)],
                                             operation_path);
        if (json::integer(json::required(operation, "id", operation_path), operation_path + ".id") != access.operation ||
            json::string(json::required(operation, "kind", operation_path), operation_path + ".kind") != "external_call" ||
            json::integer(json::required(operation, "function_symbol_id", operation_path), operation_path + ".function_symbol_id") != access.owner_function)
            throw json::Error(item_path, "effect-access identity differs from lowering operation");
        const auto& provider = json::object(json::required(operation, "provider", operation_path),
                                            operation_path + ".provider");
        if (capability_identity(provider, operation_path + ".provider") != access.capability)
            throw json::Error(item_path, "effect-access capability differs from lowering operation provider");
        accessed_capabilities.insert(access.capability);
    }
    if (profile_capabilities != accessed_capabilities)
        throw json::Error(std::string(path) + ".provider_effect_profiles",
                          "provider-effect profiles do not exactly match proven effect accesses");
}

inline const json::Object& required_object(const json::Object& parent, std::string_view key, std::string_view path = "$") {
    return json::object(json::required(parent, key, path), std::string(path) + "." + std::string(key));
}
inline const json::Array& required_array(const json::Object& parent, std::string_view key, std::string_view path = "$") {
    return json::array(json::required(parent, key, path), std::string(path) + "." + std::string(key));
}

inline void validate_targets(const json::Object& root) {
    const auto& targets = required_array(root, "targets");
    std::set<json::Integer> symbols; std::set<std::string> names;
    for (std::size_t index = 0; index < targets.size(); ++index) {
        const auto path = "$.targets[" + std::to_string(index) + "]";
        const auto& target = json::object(targets[index], path);
        const auto symbol = json::integer(json::required(target, "symbol_id", path), path + ".symbol_id");
        const auto name = json::string(json::required(target, "name", path), path + ".name");
        const auto mains = json::integer(json::required(target, "main_count", path), path + ".main_count");
        if (symbol < 0 || name.empty() || mains < 0) throw json::Error(path, "invalid target identity");
        if (!symbols.insert(symbol).second || !names.insert(name).second) throw json::Error(path, "duplicate target identity");
    }
}

inline void validate_abi_contracts(const json::Object& root) {
    const auto& contracts = required_array(root, "abi_type_contracts");
    std::set<std::pair<std::string, std::string>> identities;
    for (std::size_t index = 0; index < contracts.size(); ++index) {
        const auto path = "$.abi_type_contracts[" + std::to_string(index) + "]";
        const auto& contract = json::object(contracts[index], path);
        const auto owner = json::string(json::required(contract, "contract", path), path + ".contract");
        const auto name = json::string(json::required(contract, "name", path), path + ".name");
        for (const auto field : {"repr", "ownership", "access", "lifetime", "nullable", "opaque", "cleanup"})
            (void)json::string(json::required(contract, field, path), path + "." + field);
        if (!identities.emplace(owner, name).second) throw json::Error(path, "duplicate ABI contract identity");
    }
}

inline void validate_aggregate_abi_layouts(const json::Array& layouts, std::string_view path = "$.aggregate_abi_layouts") {
    std::set<std::pair<std::string, std::string>> identities;
    for (std::size_t index = 0; index < layouts.size(); ++index) {
        const auto item_path = std::string(path) + "[" + std::to_string(index) + "]";
        const auto& layout = json::object(layouts[index], item_path);
        const auto contract = json::string(json::required(layout, "contract", item_path), item_path + ".contract");
        const auto name = json::string(json::required(layout, "name", item_path), item_path + ".name");
        if (contract.empty() || name.empty() || !identities.emplace(contract, name).second)
            throw json::Error(item_path, "empty or duplicate aggregate ABI layout identity");
        if (json::integer(json::required(layout, "version", item_path), item_path + ".version") != 1)
            throw json::Error(item_path + ".version", "unsupported aggregate ABI layout version");
        const auto status = json::string(json::required(layout, "status", item_path), item_path + ".status");
        if (status != "declared" && status != "verified") throw json::Error(item_path + ".status", "unsupported aggregate ABI layout status");
        const auto policy = json::string(json::required(layout, "layout_policy", item_path), item_path + ".layout_policy");
        if (policy != "provider_verified_required" && policy != "provider_verified")
            throw json::Error(item_path + ".layout_policy", "unsupported aggregate ABI layout policy");
        const auto& fields = required_array(layout, "fields", item_path);
        if (status == "verified") {
            if (json::integer(json::required(layout, "size", item_path), item_path + ".size") <= 0 ||
                json::integer(json::required(layout, "alignment", item_path), item_path + ".alignment") <= 0)
                throw json::Error(item_path, "verified aggregate ABI layout requires positive size and alignment");
        }
        if (fields.empty() || fields.size() > 16) throw json::Error(item_path + ".fields", "aggregate ABI layout must contain 1..16 fields");
        std::set<std::string> field_names;
        for (std::size_t field_index = 0; field_index < fields.size(); ++field_index) {
            const auto field_path = item_path + ".fields[" + std::to_string(field_index) + "]";
            const auto& field = json::object(fields[field_index], field_path);
            const auto field_name = json::string(json::required(field, "name", field_path), field_path + ".name");
            const auto field_type = json::string(json::required(field, "type", field_path), field_path + ".type");
            if (field_name.empty() || !field_names.insert(field_name).second || field_type.empty())
                throw json::Error(field_path, "aggregate ABI fields must have unique non-empty names and types");
            if (status == "verified" && field_type != "c_int" && field_type != "c_long" &&
                field_type != "c_ulong" && field_type != "c_size_t")
                throw json::Error(field_path + ".type", "verified aggregate ABI fields must use a supported integer carrier");
            if (const auto* offset = json::optional(field, "offset")) {
                if (json::integer(*offset, field_path + ".offset") < 0) throw json::Error(field_path + ".offset", "aggregate field offset must be non-negative");
            } else if (status == "verified") {
                throw json::Error(field_path + ".offset", "verified aggregate ABI field requires an offset");
            }
        }
    }
}

inline void validate_provider_authority(const json::Object& root,
                                        std::string_view path = "$") {
    std::set<std::string> declared;
    const auto& requirements = required_array(root, "binding_requirements", path);
    for (std::size_t index = 0; index < requirements.size(); ++index) {
        const auto item_path = std::string(path) + ".binding_requirements[" +
                               std::to_string(index) + "]";
        const auto& requirement = json::object(requirements[index], item_path);
        if (!declared.insert(capability_identity(requirement, item_path)).second)
            throw json::Error(item_path, "duplicate binding requirement identity");
    }

    std::set<std::string> used;
    const auto& plan = required_object(root, "lowering_plan", path);
    const auto& operations = required_array(plan, "operations",
                                            std::string(path) + ".lowering_plan");
    for (std::size_t index = 0; index < operations.size(); ++index) {
        const auto operation_path = std::string(path) + ".lowering_plan.operations[" +
                                    std::to_string(index) + "]";
        const auto& operation = json::object(operations[index], operation_path);
        const auto kind = json::string(json::required(operation, "kind", operation_path),
                                       operation_path + ".kind");
        if (kind != "external_call" && kind != "text_outcome") continue;
        const auto& provider = required_object(operation, "provider", operation_path);
        used.insert(capability_identity(provider, operation_path + ".provider"));
    }
    if (const auto* graph_value = json::optional(plan, "source_graph")) {
        const auto graph = source_graph(*graph_value,
                                        std::string(path) + ".lowering_plan.source_graph");
        for (std::size_t index = 0; index < graph.providers.size(); ++index) {
            const auto provider_path = std::string(path) +
                ".lowering_plan.source_graph.providers[" + std::to_string(index) + "]";
            const auto& node = json::object(graph.providers[index], provider_path);
            used.insert(capability_identity(required_object(node, "provider", provider_path),
                                            provider_path + ".provider"));
            if (const auto* count = json::optional(node, "count_provider"))
                used.insert(capability_identity(json::object(*count, provider_path + ".count_provider"),
                                                provider_path + ".count_provider"));
        }
    }

    if (declared != used)
        throw json::Error(std::string(path) + ".binding_requirements",
                          "binding requirements do not exactly match external operation providers");
}

inline void validate_call_operation_projection(const json::Object& root,
                                               std::string_view path = "$") {
    const auto& plan = required_object(root, "lowering_plan", path);
    const auto version = json::integer(json::required(plan, "version", path),
                                       std::string(path) + ".lowering_plan.version");
    if (version != 2) return; // Historical plan v1 makes no connectivity claim.

    std::map<json::Integer, const json::Object*> projected_operations;
    const auto& operations = required_array(plan, "operations",
                                            std::string(path) + ".lowering_plan");
    for (std::size_t index = 0; index < operations.size(); ++index) {
        const auto operation_path = std::string(path) + ".lowering_plan.operations[" +
                                    std::to_string(index) + "]";
        const auto& operation = json::object(operations[index], operation_path);
        const bool projected = json::boolean(
            json::required(operation, "source_call_projection", operation_path),
            operation_path + ".source_call_projection");
        if (!projected) continue;
        const auto kind = json::string(json::required(operation, "kind", operation_path),
                                       operation_path + ".kind");
        if (kind != "call" && kind != "external_call" && kind != "text_outcome")
            throw json::Error(operation_path + ".source_call_projection",
                              "non-call operation claims a source-call projection");
        const auto id = json::integer(json::required(operation, "id", operation_path),
                                      operation_path + ".id");
        if (!projected_operations.emplace(id, &operation).second)
            throw json::Error(operation_path + ".id", "duplicate projected operation identity");
    }

    std::set<json::Integer> pure_symbols;
    const auto& effect_facts = required_array(root, "effect_facts", path);
    for (std::size_t index = 0; index < effect_facts.size(); ++index) {
        const auto fact_path = std::string(path) + ".effect_facts[" +
                               std::to_string(index) + "]";
        const auto& fact = json::object(effect_facts[index], fact_path);
        const auto symbol = json::integer(json::required(fact, "symbol_id", fact_path),
                                          fact_path + ".symbol_id");
        const auto effect = json::string(json::required(fact, "effect", fact_path),
                                         fact_path + ".effect");
        const auto certainty = json::string(json::required(fact, "certainty", fact_path),
                                            fact_path + ".certainty");
        if ((effect == "pure") != (certainty == "proven") ||
            (effect != "pure" && effect != "unknown") ||
            (certainty != "proven" && certainty != "unresolved"))
            throw json::Error(fact_path, "effect fact and certainty disagree");
        if (effect == "pure" && symbol >= 0) pure_symbols.insert(symbol);
    }

    std::set<json::Integer> covered;
    const auto& projections = required_array(root, "external_operations", path);
    for (std::size_t index = 0; index < projections.size(); ++index) {
        const auto projection_path = std::string(path) + ".external_operations[" +
                                     std::to_string(index) + "]";
        const auto& projection = json::object(projections[index], projection_path);
        if (json::string(json::required(projection, "operation", projection_path),
                         projection_path + ".operation") != "call")
            throw json::Error(projection_path + ".operation", "unsupported call projection kind");
        const auto operation_id = json::integer(
            json::required(projection, "operation_id", projection_path),
            projection_path + ".operation_id");
        const auto found = projected_operations.find(operation_id);
        if (found == projected_operations.end() || !covered.insert(operation_id).second)
            throw json::Error(projection_path + ".operation_id",
                              "missing or duplicate source-call operation");
        const auto& operation = *found->second;
        for (const auto field : {"expression_id", "statement_id", "scope_id", "callee_symbol_id"})
            if (json::integer(json::required(projection, field, projection_path), projection_path + "." + field) !=
                json::integer(json::required(operation, field, projection_path), projection_path + "." + field))
                throw json::Error(projection_path + "." + field,
                                  "call projection identity differs from operation");
        if (json::string(json::required(projection, "callee", projection_path), projection_path + ".callee") !=
            json::string(json::required(operation, "callee", projection_path), projection_path + ".callee"))
            throw json::Error(projection_path + ".callee",
                              "call projection callee differs from operation");

        const auto& arguments = required_array(projection, "arguments", projection_path);
        const auto& operation_arguments = required_array(operation, "arguments", projection_path);
        if (arguments != operation_arguments)
            throw json::Error(projection_path + ".arguments",
                              "call projection arguments differ from operation");
        const auto* projection_result = json::optional(projection, "result_symbol_id");
        const auto* operation_result = json::optional(operation, "result_symbol_id");
        if ((projection_result == nullptr) != (operation_result == nullptr) ||
            (projection_result && json::integer(*projection_result, projection_path + ".result_symbol_id") !=
                                  json::integer(*operation_result, projection_path + ".result_symbol_id")))
            throw json::Error(projection_path + ".result_symbol_id",
                              "call projection result differs from operation");
        const auto callee_symbol = json::integer(
            json::required(operation, "callee_symbol_id", projection_path),
            projection_path + ".callee_symbol_id");
        const auto expected_purity = pure_symbols.count(callee_symbol) ? "pure" : "effectful";
        if (json::string(json::required(projection, "purity", projection_path),
                         projection_path + ".purity") != expected_purity)
            throw json::Error(projection_path + ".purity",
                              "call projection purity differs from effect authority");
    }
    if (covered.size() != projected_operations.size())
        throw json::Error(std::string(path) + ".external_operations",
                          "source-call operation lacks its call projection");
}

inline void validate_parallel_candidate_projection(const json::Object& root,
                                                   std::string_view path = "$") {
    const auto& plan = required_object(root, "lowering_plan", path);
    const auto version = json::integer(json::required(plan, "version", path),
                                       std::string(path) + ".lowering_plan.version");
    if (version != 2) return; // Historical plan v1 makes no connectivity claim.

    std::set<json::Integer> pure_symbols;
    const auto& effect_facts = required_array(root, "effect_facts", path);
    for (std::size_t index = 0; index < effect_facts.size(); ++index) {
        const auto fact_path = std::string(path) + ".effect_facts[" +
                               std::to_string(index) + "]";
        const auto& fact = json::object(effect_facts[index], fact_path);
        if (json::string(json::required(fact, "effect", fact_path), fact_path + ".effect") == "pure" &&
            json::string(json::required(fact, "certainty", fact_path), fact_path + ".certainty") == "proven")
            pure_symbols.insert(json::integer(json::required(fact, "symbol_id", fact_path),
                                              fact_path + ".symbol_id"));
    }

    struct OperationEvidence {
        const json::Object* operation = nullptr;
        json::Integer expression = -1;
        json::Integer statement = -1;
        json::Integer scope = -1;
        json::Integer result = -1;
        std::set<json::Integer> reads;
    };
    std::map<json::Integer, OperationEvidence> pure_calls;
    const auto& operations = required_array(plan, "operations",
                                            std::string(path) + ".lowering_plan");
    for (std::size_t index = 0; index < operations.size(); ++index) {
        const auto operation_path = std::string(path) + ".lowering_plan.operations[" +
                                    std::to_string(index) + "]";
        const auto& operation = json::object(operations[index], operation_path);
        if (!json::boolean(json::required(operation, "source_call_projection", operation_path),
                           operation_path + ".source_call_projection")) continue;
        const auto callee_symbol = json::integer(
            json::required(operation, "callee_symbol_id", operation_path),
            operation_path + ".callee_symbol_id");
        if (!pure_symbols.count(callee_symbol)) continue;

        OperationEvidence evidence;
        evidence.operation = &operation;
        evidence.expression = json::integer(json::required(operation, "expression_id", operation_path),
                                            operation_path + ".expression_id");
        evidence.statement = json::integer(json::required(operation, "statement_id", operation_path),
                                           operation_path + ".statement_id");
        evidence.scope = json::integer(json::required(operation, "scope_id", operation_path),
                                       operation_path + ".scope_id");
        if (const auto* result = json::optional(operation, "result_symbol_id"))
            evidence.result = json::integer(*result, operation_path + ".result_symbol_id");
        std::function<void(const json::Value&)> collect_reads = [&](const json::Value& value) {
            if (std::holds_alternative<json::Object>(value)) {
                const auto& object = std::get<json::Object>(value);
                const auto* kind = json::optional(object, "kind");
                if (kind && json::string(*kind, operation_path + ".operands[].kind") == "identifier")
                    evidence.reads.insert(json::integer(
                        json::required(object, "symbol_id", operation_path + ".operands[]"),
                        operation_path + ".operands[].symbol_id"));
                for (const auto& [key, child] : object) { (void)key; collect_reads(child); }
            } else if (std::holds_alternative<json::Array>(value)) {
                for (const auto& child : std::get<json::Array>(value)) collect_reads(child);
            }
        };
        collect_reads(json::required(operation, "operands", operation_path));
        const auto id = json::integer(json::required(operation, "id", operation_path),
                                      operation_path + ".id");
        pure_calls.emplace(id, std::move(evidence));
    }

    std::map<json::Integer, std::set<json::Integer>> expected;
    for (auto left = pure_calls.begin(); left != pure_calls.end(); ++left) {
        for (auto right = std::next(left); right != pure_calls.end(); ++right) {
            const auto& first = left->second;
            const auto& second = right->second;
            if (first.scope != second.scope || first.statement == second.statement) continue;
            bool shared_read = false;
            for (const auto symbol : first.reads)
                if (second.reads.count(symbol)) { shared_read = true; break; }
            const bool output_conflict = first.result >= 0 && first.result == second.result;
            const bool read_after_write =
                (first.result >= 0 && second.reads.count(first.result)) ||
                (second.result >= 0 && first.reads.count(second.result));
            if (!shared_read && !output_conflict && !read_after_write) {
                expected[left->first].insert(right->first);
                expected[right->first].insert(left->first);
            }
        }
    }

    std::map<json::Integer, std::set<json::Integer>> observed;
    const auto& candidates = required_array(root, "parallel_candidates", path);
    for (std::size_t index = 0; index < candidates.size(); ++index) {
        const auto candidate_path = std::string(path) + ".parallel_candidates[" +
                                    std::to_string(index) + "]";
        const auto& candidate = json::object(candidates[index], candidate_path);
        const auto operation_id = json::integer(
            json::required(candidate, "operation_id", candidate_path),
            candidate_path + ".operation_id");
        const auto found = pure_calls.find(operation_id);
        if (found == pure_calls.end() || observed.count(operation_id))
            throw json::Error(candidate_path + ".operation_id",
                              "parallel candidate lacks one unique proven-pure call operation");
        const auto& operation = *found->second.operation;
        if (json::integer(json::required(candidate, "call_expression", candidate_path),
                          candidate_path + ".call_expression") != found->second.expression ||
            json::integer(json::required(candidate, "statement_id", candidate_path),
                          candidate_path + ".statement_id") != found->second.statement ||
            json::string(json::required(candidate, "callee", candidate_path),
                         candidate_path + ".callee") !=
                json::string(json::required(operation, "callee", candidate_path),
                             candidate_path + ".callee"))
            throw json::Error(candidate_path, "parallel candidate identity differs from call operation");
        if (json::string(json::required(candidate, "proof", candidate_path),
                         candidate_path + ".proof") != "pure-callee-disjoint-inputs" ||
            json::string(json::required(candidate, "status", candidate_path),
                         candidate_path + ".status") != "deferred")
            throw json::Error(candidate_path, "unsupported parallel candidate proof or status");

        const auto& peer_operations = required_array(candidate, "independent_operation_ids",
                                                     candidate_path);
        const auto& peer_expressions = required_array(candidate, "independent_with",
                                                      candidate_path);
        if (peer_operations.empty() || peer_operations.size() != peer_expressions.size())
            throw json::Error(candidate_path + ".independent_operation_ids",
                              "parallel peer operation and expression identities differ");
        auto& peers = observed[operation_id];
        for (std::size_t peer_index = 0; peer_index < peer_operations.size(); ++peer_index) {
            const auto peer_id = json::integer(peer_operations[peer_index],
                                               candidate_path + ".independent_operation_ids[]");
            const auto peer = pure_calls.find(peer_id);
            if (peer == pure_calls.end() || peer_id == operation_id || !peers.insert(peer_id).second)
                throw json::Error(candidate_path + ".independent_operation_ids",
                                  "parallel peer is not one distinct proven-pure call operation");
            if (json::integer(peer_expressions[peer_index],
                              candidate_path + ".independent_with[]") != peer->second.expression)
                throw json::Error(candidate_path + ".independent_with",
                                  "parallel peer expression differs from call operation");
        }
    }
    if (observed != expected)
        throw json::Error(std::string(path) + ".parallel_candidates",
                          "parallel candidates do not exactly match canonical operation evidence");
}

inline void validate_lowering_authority(const json::Value& value, std::string_view base = "$.lowering_plan") {
    validate_scalar_facts(value, base);
    const auto& plan = json::object(value, base);
    if (const auto* coverage = json::optional(plan, "source_operation_coverage"))
        validate_source_operation_coverage(*coverage, std::string(base) + ".source_operation_coverage");
    if (const auto* graph = json::optional(plan, "source_graph")) {
        if (!source_graph(*graph, std::string(base) + ".source_graph").executable)
            throw json::Error(std::string(base) + ".source_graph", "source graph execution is not admitted");
    }
    if (json::string(json::required(plan, "format", base), std::string(base) + ".format") != "flowcore.lowering_plan") throw json::Error(std::string(base) + ".format", "unsupported lowering plan format");
    const auto version = json::integer(json::required(plan, "version", base), std::string(base) + ".version");
    if (version != 1 && version != 2) throw json::Error(std::string(base) + ".version", "unsupported lowering plan version");
    const auto& operations = required_array(plan, "operations", base);
    std::set<json::Integer> function_ids;
    if (version == 2) {
        const auto& functions = required_array(plan, "functions", base);
        std::size_t entries = 0;
        for (std::size_t index = 0; index < functions.size(); ++index) {
            const auto path = std::string(base) + ".functions[" + std::to_string(index) + "]";
            const auto& function = json::object(functions[index], path);
            const auto id = json::integer(json::required(function, "symbol_id", path), path + ".symbol_id");
            if (id < 0 || !function_ids.insert(id).second) throw json::Error(path + ".symbol_id", "invalid or duplicate function identity");
            (void)json::string(json::required(function, "name", path), path + ".name");
            (void)json::integer(json::required(function, "scope_id", path), path + ".scope_id");
            (void)json::integer(json::required(function, "body_block_id", path), path + ".body_block_id");
            (void)json::string(json::required(function, "return_type", path), path + ".return_type");
            const auto availability = json::string(json::required(function, "availability", path), path + ".availability");
            if (availability != "definition" && availability != "declaration") throw json::Error(path + ".availability", "unsupported callable availability");
            if (json::boolean(json::required(function, "entry", path), path + ".entry")) ++entries;
            const auto& parameters = required_array(function, "parameters", path);
            std::set<json::Integer> parameter_ids;
            for (std::size_t parameter = 0; parameter < parameters.size(); ++parameter) {
                const auto parameter_path = path + ".parameters[" + std::to_string(parameter) + "]";
                const auto& item = json::object(parameters[parameter], parameter_path);
                const auto parameter_id = json::integer(json::required(item, "symbol_id", parameter_path), parameter_path + ".symbol_id");
                if (parameter_id < 0 || !parameter_ids.insert(parameter_id).second) throw json::Error(parameter_path + ".symbol_id", "invalid or duplicate parameter identity");
                (void)json::string(json::required(item, "type", parameter_path), parameter_path + ".type");
            }
        }
        (void)entries;
    }
    std::set<json::Integer> ids;
    for (std::size_t index = 0; index < operations.size(); ++index) {
        const auto path = std::string(base) + ".operations[" + std::to_string(index) + "]";
        const auto& operation = json::object(operations[index], path);
        const auto id = json::integer(json::required(operation, "id", path), path + ".id");
        if (id < 0 || !ids.insert(id).second) throw json::Error(path + ".id", id < 0 ? "operation identity must be non-negative" : "duplicate operation identity");
        (void)json::string(json::required(operation, "kind", path), path + ".kind");
        if (version == 2) {
            const auto owner = json::integer(json::required(operation, "function_symbol_id", path), path + ".function_symbol_id");
            if (!function_ids.count(owner)) throw json::Error(path + ".function_symbol_id", "operation owner is not in function catalog");
            if (json::string(json::required(operation, "kind", path), path + ".kind") == "call") {
                const auto callee = json::integer(json::required(operation, "callee_symbol_id", path), path + ".callee_symbol_id");
                if (callee >= 0 && !function_ids.count(callee)) throw json::Error(path + ".callee_symbol_id", "callee is not in function catalog");
            }
        }
        if (const auto* block = json::optional(operation, "block_id")) if (json::integer(*block, path + ".block_id") < 0) throw json::Error(path + ".block_id", "operation block identity must be non-negative");
        (void)required_array(operation, "operands", path);
        const auto operation_kind = json::string(json::required(operation, "kind", path), path + ".kind");
        if (operation_kind == "external_call" || operation_kind == "text_outcome") {
            const auto& provider = required_object(operation, "provider", path);
            for (const auto field : {"contract", "library", "convention", "symbol", "effect", "parameter_types", "return_type"})
                (void)json::string(json::required(provider, field, path + ".provider"), path + ".provider." + field);
            (void)binding_evidence(provider, path + ".provider");
            const auto& effect = required_object(operation, "effect_contract", path);
            for (const auto field : {"external", "determinism", "certainty"})
                (void)json::string(json::required(effect, field, path + ".effect_contract"), path + ".effect_contract." + field);
            const auto* outcome = json::optional(operation, "result_outcome");
            if (operation_kind == "text_outcome" && outcome == nullptr)
                throw json::Error(path + ".result_outcome", "text_outcome operation must declare its typed outcome");
            if (outcome) {
                const auto& value = json::object(*outcome, path + ".result_outcome");
                const auto outcome_type = json::string(json::required(value, "type", path + ".result_outcome"), path + ".result_outcome.type");
                const auto representation = json::string(json::required(value, "representation", path + ".result_outcome"), path + ".result_outcome.representation");
                const auto success_type = json::string(json::required(value, "success_type", path + ".result_outcome"), path + ".result_outcome.success_type");
                const auto failure_type = json::string(json::required(value, "failure_type", path + ".result_outcome"), path + ".result_outcome.failure_type");
                if (operation_kind == "text_outcome" && (outcome_type != "Outcome" || representation != "tagged" || success_type != "Text" || failure_type != "TextFailure"))
                    throw json::Error(path + ".result_outcome", "Text outcome must use the tagged Outcome<Text,TextFailure> contract");
                const auto& codes = required_array(value, "failure_codes", path + ".result_outcome");
                if (codes.empty()) throw json::Error(path + ".result_outcome.failure_codes", "Text outcome must declare at least one failure code");
                for (const auto& code : codes) (void)json::string(code, path + ".result_outcome.failure_codes[]");
            }
            const auto& resources = required_array(operation, "argument_resources", path);
            for (std::size_t resource_index = 0; resource_index < resources.size(); ++resource_index) {
                const auto resource_path = path + ".argument_resources[" + std::to_string(resource_index) + "]";
                const auto& resource = json::object(resources[resource_index], resource_path);
                if (json::integer(json::required(resource, "index", resource_path), resource_path + ".index") != static_cast<json::Integer>(resource_index)) throw json::Error(resource_path + ".index", "resource index does not match position");
                for (const auto field : {"type", "memory_effect", "ownership", "access", "lifetime", "nullable", "opaque"})
                    (void)json::string(json::required(resource, field, resource_path), resource_path + "." + field);
            }
        }
    }
    if (const auto* coverage = json::optional(plan, "source_operation_coverage"))
        validate_source_operation_links(*coverage, operations,
                                        std::string(base) + ".source_operation_coverage");
    if (const auto* graph_value = json::optional(plan, "source_graph")) {
        if (version != 2) throw json::Error(std::string(base), "native graph requires callable plan version 2");
        const auto graph = source_graph(*graph_value);
        const auto& functions = required_array(plan, "functions", base);
        std::map<json::Integer, const json::Object*> catalog;
        std::size_t entries = 0;
        for (const auto& item : functions) {
            const auto& fn = json::object(item);
            catalog.emplace(json::integer(json::required(fn, "symbol_id"), "$.symbol_id"), &fn);
            if (json::boolean(json::required(fn, "entry"), "$.entry")) {
                ++entries;
                if (!required_array(fn, "parameters").empty()) throw json::Error(std::string(base), "native graph entry arguments are unsupported");
            }
        }
        if (entries != 1) throw json::Error(std::string(base), "native graph requires one entry");
        auto resolve = [&](const json::Value& item, bool receiver) {
            const auto& node = json::object(item);
            const auto id = json::integer(json::required(node, "function_symbol_id"), "$.function_symbol_id");
            if (!catalog.count(id)) throw json::Error(std::string(base), "graph function identity is absent from callable catalog");
            const auto& fn = *catalog.at(id);
            if (json::boolean(json::required(fn, "entry"), "$.entry")) throw json::Error(std::string(base), "graph node cannot invoke native entry");
            const auto& parameters = required_array(fn, "parameters");
            const auto activation = receiver ? std::string{} : json::string(json::required(node, "activation"), "$.activation");
            const auto stream = !receiver && activation == "finite_stream_once";
            const auto persistent = receiver && json::optional(node, "state_contract") != nullptr;
            if (parameters.size() != (receiver ? (persistent ? 2u : 1u) : (stream ? 1u : 0u)) ||
                json::string(json::required(fn, "return_type"), "$.return_type") != json::string(json::required(node, "output_type"), "$.output_type"))
                throw json::Error(std::string(base), "graph function signature differs from callable catalog");
            if (!receiver) {
                const auto& provider = required_object(node, "provider");
                const auto& declared = required_object(fn, "provider");
                if (capability_identity(provider, "$.graph.provider") != capability_identity(declared, "$.function.provider") ||
                    json::string(json::required(node, stream ? "item_callable" : "source_callable"), stream ? "$.item_callable" : "$.source_callable") !=
                        json::string(json::required(provider, "contract"), "$.contract") + "." + json::string(json::required(fn, "name"), "$.name"))
                    throw json::Error(std::string(base), "graph provider differs from external callable identity");
                if (stream) {
                    const auto count_id = json::integer(json::required(node, "count_function_symbol_id"), "$.count_function_symbol_id");
                    if (!catalog.count(count_id)) throw json::Error(std::string(base), "graph count function identity is absent from callable catalog");
                    const auto& count = *catalog.at(count_id);
                    const auto& count_parameters = required_array(count, "parameters");
                    const auto& count_provider = required_object(node, "count_provider");
                    const auto& declared_count = required_object(count, "provider");
                    if (json::boolean(json::required(count, "entry"), "$.entry") || !count_parameters.empty() ||
                        json::string(json::required(count, "return_type"), "$.return_type") != "c_size_t" ||
                        capability_identity(count_provider, "$.graph.count_provider") != capability_identity(declared_count, "$.function.count_provider") ||
                        json::string(json::required(node, "count_callable"), "$.count_callable") !=
                            json::string(json::required(count_provider, "contract"), "$.count_provider.contract") + "." + json::string(json::required(count, "name"), "$.count.name"))
                        throw json::Error(std::string(base), "graph stream count differs from external callable identity");
                }
            }
            if (receiver) {
                const auto& param = json::object(parameters.front());
                if (json::integer(json::required(param, "symbol_id"), "$.symbol_id") != json::integer(json::required(node, "parameter_symbol_id"), "$.parameter_symbol_id") ||
                    json::string(json::required(param, "type"), "$.type") != json::string(json::required(node, "input_type"), "$.input_type") ||
                    json::string(json::required(fn, "availability"), "$.availability") != "definition")
                    throw json::Error(std::string(base), "graph receiver definition differs from callable catalog");
                if (persistent) {
                    const auto& state = node;
                    const auto& state_param = json::object(parameters[1]);
                    const auto state_type = json::string(json::required(state, "state_type"), "$.state_type");
                    const auto expected_state_contract = state_type == "c_long" ? "persistent_scalar_v1" : "persistent_aggregate_v1";
                    if (json::string(json::required(state, "state_contract"), "$.state_contract") != expected_state_contract ||
                        state_type.empty() ||
                        json::string(json::required(state, "state_initial_value"), "$.state_initial_value").empty() ||
                        json::integer(json::required(state, "state_parameter_symbol_id"), "$.state_parameter_symbol_id") != json::integer(json::required(state_param, "symbol_id"), "$.state_parameter_symbol_id") ||
                        json::string(json::required(state_param, "type"), "$.state_parameter.type") != state_type)
                        throw json::Error(std::string(base), "graph persistent receiver state differs from callable catalog");
                }
            }
        };
        for (const auto& node : graph.receivers) resolve(node, true);
        for (const auto& node : graph.providers) resolve(node, false);
    }
    for (std::size_t index = 0; index < operations.size(); ++index) {
        const auto path = std::string(base) + ".operations[" + std::to_string(index) + "]";
        const auto& operation = json::object(operations[index], path);
        for (const auto field : {"then_block_id", "else_block_id", "body_block_id"}) if (const auto* reference = json::optional(operation, field)) {
            const auto block = json::integer(*reference, path + "." + field);
            if (block < -1) throw json::Error(path + "." + field, "control reference must be -1 or non-negative");
        }
    }
}

inline MatrixView parse_matrix(const json::Object& root) {
    const auto& graph = required_object(root, "analysis_graph");
    if (json::string(json::required(graph, "format", "$.analysis_graph"), "$.analysis_graph.format") != "flowanalyst.analysis_graph") throw json::Error("$.analysis_graph.format", "unsupported analysis graph format");
    if (json::integer(json::required(graph, "version", "$.analysis_graph"), "$.analysis_graph.version") != 1) throw json::Error("$.analysis_graph.version", "unsupported analysis graph version");
    const auto& views = required_array(graph, "matrix_views", "$.analysis_graph");
    const json::Object* selected = nullptr; std::size_t selected_index = 0;
    for (std::size_t index = 0; index < views.size(); ++index) {
        const auto path = "$.analysis_graph.matrix_views[" + std::to_string(index) + "]";
        const auto& view = json::object(views[index], path);
        if (json::string(json::required(view, "name", path), path + ".name") == "region_dependency") {
            if (selected) throw json::Error("$.analysis_graph.matrix_views", "duplicate region_dependency matrix view");
            selected = &view; selected_index = index;
        }
    }
    if (!selected) throw json::Error("$.analysis_graph.matrix_views", "required region_dependency matrix view is missing");
    const auto path = "$.analysis_graph.matrix_views[" + std::to_string(selected_index) + "]";
    MatrixView result;
    result.name = "region_dependency";
    result.rows = json::integer(json::required(*selected, "rows", path), path + ".rows");
    result.columns = json::integer(json::required(*selected, "columns", path), path + ".columns");
    if (result.rows < 0 || result.columns < 0) throw json::Error(path, "matrix dimensions must be non-negative");
    result.semiring = json::string(json::required(*selected, "semiring", path), path + ".semiring");
    result.storage = json::string(json::required(*selected, "storage", path), path + ".storage");
    const auto& entries = required_array(*selected, "entries", path);
    std::set<std::pair<json::Integer, json::Integer>> coordinates;
    for (std::size_t index = 0; index < entries.size(); ++index) {
        const auto entry_path = path + ".entries[" + std::to_string(index) + "]";
        const auto& entry = json::object(entries[index], entry_path);
        MatrixEntry parsed;
        parsed.row = json::integer(json::required(entry, "row", entry_path), entry_path + ".row");
        parsed.column = json::integer(json::required(entry, "column", entry_path), entry_path + ".column");
        if (const auto* item = json::optional(entry, "value")) parsed.value = json::boolean(*item, entry_path + ".value");
        if (parsed.row < 0 || parsed.row >= result.rows) throw json::Error(entry_path + ".row", "matrix row is outside declared dimensions");
        if (parsed.column < 0 || parsed.column >= result.columns) throw json::Error(entry_path + ".column", "matrix column is outside declared dimensions");
        if (!coordinates.emplace(parsed.row, parsed.column).second) throw json::Error(entry_path, "duplicate matrix coordinate");
        result.entries.push_back(parsed);
    }
    return result;
}

inline SemanticReport semantic_report(const json::Value& value) {
    SemanticReport result; result.artifact = require_header(value, "flowanalyst.semantic_report", 1);
    if (result.artifact.status != "ok") return result;
    const auto& root = json::object(value);
    validate_scheduling_request(root, false);
    const auto& source = required_object(root, "source");
    result.source_path = json::string(json::required(source, "path", "$.source"), "$.source.path");
    validate_targets(root); result.targets = required_array(root, "targets");
    result.external_operations = required_array(root, "external_operations");
    validate_abi_contracts(root); result.abi_type_contracts = required_array(root, "abi_type_contracts");
    if (const auto* layouts = json::optional(root, "aggregate_abi_layouts")) {
        result.aggregate_abi_layouts = json::array(*layouts, "$.aggregate_abi_layouts");
        validate_aggregate_abi_layouts(result.aggregate_abi_layouts);
    }
    result.lowering_plan = json::required(root, "lowering_plan");
    validate_lowering_authority(result.lowering_plan);
    validate_provider_authority(root);
    validate_call_operation_projection(root);
    validate_parallel_candidate_projection(root);
    const auto& plan = json::object(result.lowering_plan, "$.lowering_plan");
    if (json::string(json::required(plan, "format", "$.lowering_plan"), "$.lowering_plan.format") != "flowcore.lowering_plan") throw json::Error("$.lowering_plan.format", "unsupported lowering plan format");
    const auto plan_version = json::integer(json::required(plan, "version", "$.lowering_plan"), "$.lowering_plan.version");
    if (plan_version != 1 && plan_version != 2) throw json::Error("$.lowering_plan.version", "unsupported lowering plan version");
    result.effect_facts = required_array(root, "effect_facts");
    validate_effect_access_authority(root);
    if (const auto* profiles = json::optional(root, "provider_effect_profiles"))
        result.provider_effect_profiles = json::array(*profiles, "$.provider_effect_profiles");
    if (const auto* accesses = json::optional(root, "effect_access_facts"))
        result.effect_access_facts = json::array(*accesses, "$.effect_access_facts");
    for (const auto& item : result.effect_facts) {
        const auto& fact = json::object(item, "$.effect_facts[]");
        if (const auto* certainty = json::optional(fact, "certainty")) if (json::string(*certainty, "$.effect_facts[].certainty") == "proven") ++result.proven_pure_count;
    }
    result.parallel_candidates = required_array(root, "parallel_candidates");
    for (const auto& item : result.parallel_candidates) {
        const auto& candidate = json::object(item, "$.parallel_candidates[]");
        if (const auto* proof = json::optional(candidate, "proof")) if (json::string(*proof, "$.parallel_candidates[].proof") == "pure-callee-disjoint-inputs") ++result.independent_candidate_count;
    }
    result.dependency_matrix = parse_matrix(root);
    return result;
}

inline json::Value matrix_entries(const MatrixView& matrix) {
    json::Array entries;
    for (const auto& entry : matrix.entries) entries.emplace_back(json::Object{{"column", entry.column}, {"row", entry.row}, {"value", entry.value}});
    return entries;
}

struct ExecutionPlan {
    Header artifact; std::string source_path; json::Array targets; json::Array external_operations;
    json::Array abi_type_contracts; json::Array aggregate_abi_layouts; json::Array effect_facts;
    json::Array provider_effect_profiles; json::Array effect_access_facts;
    json::Array parallel_candidates; json::Value lowering_plan; json::Value graph_schedule; MatrixView dependency_matrix;
};

inline MatrixView execution_matrix(const json::Object& root) {
    const auto& view = required_object(root, "graph_projection");
    MatrixView result;
    result.name = json::string(json::required(view, "name", "$.graph_projection"), "$.graph_projection.name");
    if (result.name != "region_dependency") throw json::Error("$.graph_projection.name", "unsupported graph projection");
    result.rows = json::integer(json::required(view, "rows", "$.graph_projection"), "$.graph_projection.rows");
    result.columns = json::integer(json::required(view, "columns", "$.graph_projection"), "$.graph_projection.columns");
    if (result.rows < 0 || result.columns < 0) throw json::Error("$.graph_projection", "matrix dimensions must be non-negative");
    result.semiring = json::string(json::required(view, "semiring", "$.graph_projection"), "$.graph_projection.semiring");
    result.storage = json::string(json::required(view, "storage", "$.graph_projection"), "$.graph_projection.storage");
    const auto& entries = required_array(view, "entries", "$.graph_projection");
    for (std::size_t index = 0; index < entries.size(); ++index) {
        const auto path = "$.graph_projection.entries[" + std::to_string(index) + "]";
        const auto& entry = json::object(entries[index], path);
        MatrixEntry parsed;
        parsed.row = json::integer(json::required(entry, "row", path), path + ".row");
        parsed.column = json::integer(json::required(entry, "column", path), path + ".column");
        if (const auto* item = json::optional(entry, "value")) parsed.value = json::boolean(*item, path + ".value");
        if (parsed.row < 0 || parsed.row >= result.rows) throw json::Error(path + ".row", "matrix row is outside declared dimensions");
        if (parsed.column < 0 || parsed.column >= result.columns) throw json::Error(path + ".column", "matrix column is outside declared dimensions");
        result.entries.push_back(parsed);
    }
    return result;
}

inline ExecutionPlan execution_plan(const json::Value& value) {
    ExecutionPlan result; result.artifact = require_header(value, "flowparallel.execution_plan", 1);
    if (result.artifact.status != "ready") return result;
    const auto& root = json::object(value);
    validate_scheduling_request(root, true);
    const auto& source = required_object(root, "source");
    result.source_path = json::string(json::required(source, "path", "$.source"), "$.source.path");
    validate_targets(root); result.targets = required_array(root, "targets");
    result.external_operations = required_array(root, "external_operations");
    validate_abi_contracts(root); result.abi_type_contracts = required_array(root, "abi_type_contracts");
    if (const auto* layouts = json::optional(root, "aggregate_abi_layouts")) {
        result.aggregate_abi_layouts = json::array(*layouts, "$.aggregate_abi_layouts");
        validate_aggregate_abi_layouts(result.aggregate_abi_layouts);
    }
    result.lowering_plan = json::required(root, "lowering_plan");
    validate_lowering_authority(result.lowering_plan);
    validate_graph_schedule(root);
    if (const auto* schedule = json::optional(root, "graph_schedule")) result.graph_schedule = *schedule;
    const auto& plan = json::object(result.lowering_plan, "$.lowering_plan");
    if (json::string(json::required(plan, "format", "$.lowering_plan"), "$.lowering_plan.format") != "flowcore.lowering_plan") throw json::Error("$.lowering_plan.format", "unsupported lowering plan format");
    const auto plan_version = json::integer(json::required(plan, "version", "$.lowering_plan"), "$.lowering_plan.version");
    if (plan_version != 1 && plan_version != 2) throw json::Error("$.lowering_plan.version", "unsupported lowering plan version");
    if (plan_version == 2) {
        result.effect_facts = required_array(root, "effect_facts");
        result.parallel_candidates = required_array(root, "parallel_candidates");
        validate_call_operation_projection(root);
        validate_parallel_candidate_projection(root);
        const auto& summary = required_object(root, "dependency_analysis");
        if (json::string(json::required(summary, "candidate_kind", "$.dependency_analysis"),
                         "$.dependency_analysis.candidate_kind") != "pure-callee-disjoint-inputs" ||
            json::string(json::required(summary, "status", "$.dependency_analysis"),
                         "$.dependency_analysis.status") != "available")
            throw json::Error("$.dependency_analysis", "unsupported candidate summary contract");
        const auto candidate_count = json::integer(
            json::required(summary, "parallel_candidates", "$.dependency_analysis"),
            "$.dependency_analysis.parallel_candidates");
        if (candidate_count != static_cast<json::Integer>(result.parallel_candidates.size()))
            throw json::Error("$.dependency_analysis.parallel_candidates",
                              "candidate summary differs from exact evidence");
        std::size_t pure_count = 0;
        for (const auto& item : result.effect_facts) {
            const auto& fact = json::object(item, "$.effect_facts[]");
            if (json::string(json::required(fact, "effect", "$.effect_facts[]"), "$.effect_facts[].effect") == "pure" &&
                json::string(json::required(fact, "certainty", "$.effect_facts[]"), "$.effect_facts[].certainty") == "proven")
                ++pure_count;
        }
        if (json::integer(json::required(summary, "pure_callables", "$.dependency_analysis"),
                          "$.dependency_analysis.pure_callables") != static_cast<json::Integer>(pure_count))
            throw json::Error("$.dependency_analysis.pure_callables",
                              "pure-callable summary differs from effect authority");
    } else {
        if (const auto* facts = json::optional(root, "effect_facts")) result.effect_facts = json::array(*facts, "$.effect_facts");
        if (const auto* candidates = json::optional(root, "parallel_candidates")) result.parallel_candidates = json::array(*candidates, "$.parallel_candidates");
    }
    validate_effect_access_authority(root);
    if (const auto* profiles = json::optional(root, "provider_effect_profiles"))
        result.provider_effect_profiles = json::array(*profiles, "$.provider_effect_profiles");
    if (const auto* accesses = json::optional(root, "effect_access_facts"))
        result.effect_access_facts = json::array(*accesses, "$.effect_access_facts");
    result.dependency_matrix = execution_matrix(root);
    return result;
}

struct ProviderDecision { Header artifact; std::string provider; std::string representation; std::string reason; std::string fallback; };
inline ProviderDecision provider_decision(const json::Value& value) {
    ProviderDecision result; result.artifact = require_header(value, "flowparallel.graph_provider_decision", 1);
    if (result.artifact.status != "verified") throw json::Error("$.status", "provider decision is not verified");
    const auto& root = json::object(value);
    result.provider = json::string(json::required(root, "provider"), "$.provider");
    result.representation = json::string(json::required(root, "representation"), "$.representation");
    if (const auto* reason = json::optional(root, "reason")) result.reason = json::string(*reason, "$.reason");
    if (const auto* fallback = json::optional(root, "fallback")) result.fallback = json::string(*fallback, "$.fallback");
    return result;
}

} // namespace flowcontracts
