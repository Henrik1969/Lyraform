#pragma once

#include <flowcontracts/binding_evidence.hpp>
#include <flowcontracts/json.hpp>

#include <set>
#include <string>
#include <vector>

namespace flowcontracts {

struct ProviderEffectProfile {
    json::Integer id = -1;
    json::Object capability;
    std::string resource_domain;
    std::string access;
    std::string concurrency;
    std::string failure;
};

struct EffectAccessFact {
    json::Integer operation = -1;
    json::Integer owner_function = -1;
    std::string activation;
    json::Integer profile = -1;
    std::string capability;
    std::string effect;
    std::string resource_domain;
    std::string access;
    std::string concurrency;
    std::string origin;
};

struct EffectConflictFact {
    json::Integer left_operation = -1;
    json::Integer right_operation = -1;
    std::string relation;
    std::string proof;
    std::string required_order;
};

struct EffectScheduleFact {
    std::string graph;
    std::string strategy;
    std::string serial_reference;
    std::string publication;
    std::string worker_failure;
    std::vector<json::Integer> operations;
};

inline json::Value provider_effect_profile_fact(const ProviderEffectProfile& profile) {
    using namespace json;
    return Object{{"format", "flowcore.provider_effect_profile"},
                  {"version", 1},
                  {"status", "declarative"},
                  {"profile_id", profile.id},
                  {"capability", profile.capability},
                  {"resource_domain", profile.resource_domain},
                  {"access", profile.access},
                  {"concurrency", profile.concurrency},
                  {"failure", profile.failure}};
}

inline ProviderEffectProfile read_provider_effect_profile(const json::Value& raw,
                                                          const std::string& path) {
    using namespace json;
    const auto& fact = object(raw, path);
    const auto text = [&](std::string_view key) -> const std::string& {
        return string(required(fact, key, path), path + "." + std::string(key));
    };
    if (text("format") != "flowcore.provider_effect_profile" ||
        integer(required(fact, "version", path), path + ".version") != 1 ||
        text("status") != "declarative")
        throw Error(path, "unsupported or executable provider-effect profile");
    ProviderEffectProfile result;
    result.id = integer(required(fact, "profile_id", path), path + ".profile_id");
    result.capability = object(required(fact, "capability", path), path + ".capability");
    result.resource_domain = text("resource_domain");
    result.access = text("access");
    result.concurrency = text("concurrency");
    result.failure = text("failure");
    (void)capability_identity(result.capability, path + ".capability");
    return result;
}

inline std::string provider_effect_profile_refusal(const ProviderEffectProfile& profile) {
    if (profile.id < 0) return "provider-effect profile identity is invalid";
    try {
        const auto identity = capability_identity(profile.capability, "$.capability");
        if (identity.empty()) return "provider-effect capability identity is empty";
        const auto& effect = json::string(json::required(profile.capability, "effect", "$.capability"),
                                          "$.capability.effect");
        const auto& parameters = json::string(json::required(profile.capability, "parameter_types", "$.capability"),
                                              "$.capability.parameter_types");
        const auto& result = json::string(json::required(profile.capability, "return_type", "$.capability"),
                                          "$.capability.return_type");
        if (binding_evidence(profile.capability, "$.capability").empty())
            return "provider-effect profile requires exact generated provider evidence";
        const std::set<std::string> scalar_carriers{"int", "Bool", "c_int", "c_long", "c_ulong", "c_size_t"};
        if (effect != "readonly" || profile.access != "observe" ||
            profile.concurrency != "concurrent_observation_v1" ||
            profile.failure != "infallible_scalar_v1")
            return "provider-effect profile is outside bounded concurrent scalar observation";
        if (profile.resource_domain.empty()) return "provider-effect resource domain is empty";
        if (!parameters.empty()) {
            std::size_t start = 0;
            while (start <= parameters.size()) {
                const auto end = parameters.find(',', start);
                const auto carrier = parameters.substr(start, end == std::string::npos ? std::string::npos : end - start);
                if (!scalar_carriers.count(carrier))
                    return "provider-effect parameter is not an admitted by-value scalar";
                if (end == std::string::npos) break;
                start = end + 1;
            }
        }
        if (!scalar_carriers.count(result)) return "provider-effect result is not an admitted by-value scalar";
        return {};
    } catch (const json::Error& error) {
        return error.what();
    }
}

inline json::Value provider_effect_profile_set_fact(
    const std::vector<ProviderEffectProfile>& profiles) {
    using namespace json;
    Array facts;
    for (const auto& profile : profiles) facts.emplace_back(provider_effect_profile_fact(profile));
    return Object{{"format", "flowcore.provider_effect_profiles"}, {"version", 1},
                  {"status", "declarative"}, {"profiles", facts}};
}

inline std::vector<ProviderEffectProfile> read_provider_effect_profiles(
    const json::Value& raw, const std::string& path = "$") {
    using namespace json;
    const auto& root = object(raw, path);
    const auto text = [&](std::string_view key) -> const std::string& {
        return string(required(root, key, path), path + "." + std::string(key));
    };
    if (text("format") != "flowcore.provider_effect_profiles" ||
        integer(required(root, "version", path), path + ".version") != 1 ||
        text("status") != "declarative")
        throw Error(path, "unsupported or executable provider-effect profile set");
    const auto& values = array(required(root, "profiles", path), path + ".profiles");
    if (values.empty() || values.size() > 100000)
        throw Error(path + ".profiles", "provider-effect profile set must contain 1..100000 entries");
    std::vector<ProviderEffectProfile> result;
    std::set<Integer> ids;
    std::set<std::string> capabilities;
    for (std::size_t index = 0; index < values.size(); ++index) {
        const auto item_path = path + ".profiles[" + std::to_string(index) + "]";
        auto profile = read_provider_effect_profile(values[index], item_path);
        const auto refusal = provider_effect_profile_refusal(profile);
        if (!refusal.empty()) throw Error(item_path, refusal);
        if (!ids.insert(profile.id).second)
            throw Error(item_path + ".profile_id", "duplicate provider-effect profile identity");
        if (!capabilities.insert(capability_identity(profile.capability, item_path + ".capability")).second)
            throw Error(item_path + ".capability", "duplicate provider-effect capability identity");
        result.push_back(std::move(profile));
    }
    return result;
}

inline json::Value effect_access_fact(const EffectAccessFact& fact) {
    using namespace json;
    return Object{{"format", "lyraform.effect_access"},
                  {"version", 1},
                  {"status", "proven"},
                  {"operation_id", fact.operation},
                  {"owner_function_symbol_id", fact.owner_function},
                  {"activation_id", fact.activation},
                  {"provider_effect_profile_id", fact.profile},
                  {"capability_identity", fact.capability},
                  {"effect", fact.effect},
                  {"resource_domain", fact.resource_domain},
                  {"access", fact.access},
                  {"concurrency", fact.concurrency},
                  {"origin", fact.origin}};
}

inline EffectAccessFact read_effect_access(const json::Value& raw, const std::string& path) {
    using namespace json;
    const auto& value = object(raw, path);
    const auto text = [&](std::string_view key) -> const std::string& {
        return string(required(value, key, path), path + "." + std::string(key));
    };
    if (text("format") != "lyraform.effect_access" ||
        integer(required(value, "version", path), path + ".version") != 1 ||
        text("status") != "proven")
        throw Error(path, "unsupported or unproven effect-access fact");
    return {integer(required(value, "operation_id", path), path + ".operation_id"),
            integer(required(value, "owner_function_symbol_id", path), path + ".owner_function_symbol_id"),
            text("activation_id"),
            integer(required(value, "provider_effect_profile_id", path), path + ".provider_effect_profile_id"),
            text("capability_identity"), text("effect"), text("resource_domain"),
            text("access"), text("concurrency"), text("origin")};
}

inline std::string effect_access_refusal(const EffectAccessFact& fact,
                                         const ProviderEffectProfile& profile) {
    const auto profile_error = provider_effect_profile_refusal(profile);
    if (!profile_error.empty()) return profile_error;
    if (fact.operation < 0 || fact.owner_function < 0 || fact.activation.empty() || fact.origin.empty())
        return "effect-access identity or provenance is incomplete";
    if (fact.profile != profile.id) return "effect-access profile identity mismatch";
    if (fact.capability != capability_identity(profile.capability, "$.capability"))
        return "effect-access capability identity mismatch";
    if (fact.effect != "readonly" || fact.effect != json::string(json::required(profile.capability, "effect", "$.capability"), "$.capability.effect") ||
        fact.resource_domain != profile.resource_domain || fact.access != profile.access ||
        fact.concurrency != profile.concurrency)
        return "effect-access authority differs from provider profile";
    return {};
}

inline json::Value effect_conflict_fact(const EffectConflictFact& fact) {
    using namespace json;
    return Object{{"format", "lyraform.effect_conflict"}, {"version", 1}, {"status", "proven"},
                  {"left_operation_id", fact.left_operation}, {"right_operation_id", fact.right_operation},
                  {"relation", fact.relation}, {"proof", fact.proof}, {"required_order", fact.required_order}};
}

inline std::string effect_conflict_refusal(const EffectConflictFact& fact,
                                           const EffectAccessFact& left,
                                           const EffectAccessFact& right) {
    if (fact.left_operation != left.operation || fact.right_operation != right.operation ||
        fact.left_operation < 0 || fact.right_operation < 0 || fact.left_operation >= fact.right_operation)
        return "effect-conflict operation identity mismatch";
    if (left.effect == "readonly" && right.effect == "readonly" &&
        left.access == "observe" && right.access == "observe" &&
        left.concurrency == "concurrent_observation_v1" && right.concurrency == "concurrent_observation_v1") {
        if (fact.relation != "independent" || fact.proof != "concurrent_observation_v1" ||
            fact.required_order != "none")
            return "concurrent observations require the canonical independence proof";
        return {};
    }
    if (fact.relation != "conflict" || fact.proof != "effect_conflict" || fact.required_order != "source")
        return "effect conflict requires explicit source ordering";
    return {};
}

inline json::Value effect_schedule_fact(const EffectScheduleFact& fact) {
    using namespace json;
    Array operations;
    for (const auto operation : fact.operations) operations.emplace_back(operation);
    return Object{{"format", "lyraform.effect_schedule"}, {"version", 1}, {"status", "declarative"},
                  {"graph_identity", fact.graph}, {"strategy", fact.strategy},
                  {"serial_reference", fact.serial_reference}, {"publication", fact.publication},
                  {"worker_failure", fact.worker_failure}, {"operation_ids", operations}};
}

inline std::string effect_schedule_refusal(const EffectScheduleFact& fact,
                                           const std::vector<EffectAccessFact>& accesses,
                                           const std::vector<EffectConflictFact>& conflicts) {
    if (fact.graph.empty() || fact.strategy != "parallel_independent_v1" ||
        fact.serial_reference != "deterministic_activation_order_v1" ||
        fact.publication != "publish_after_wave_join_v1" ||
        fact.worker_failure != "no_partial_wave_publication_v1")
        return "effect schedule law is incomplete or unsupported";
    if (fact.operations.size() < 2) return "effect schedule requires at least two operations";
    std::set<json::Integer> scheduled;
    for (const auto operation : fact.operations)
        if (operation < 0 || !scheduled.insert(operation).second)
            return "effect schedule contains invalid or duplicate operation identity";
    std::set<json::Integer> available;
    for (const auto& access : accesses) available.insert(access.operation);
    if (scheduled != available) return "effect schedule operation set differs from access authority";
    const auto expected_pairs = scheduled.size() * (scheduled.size() - 1) / 2;
    if (conflicts.size() != expected_pairs) return "effect schedule lacks complete pairwise conflict evidence";
    std::set<std::pair<json::Integer, json::Integer>> pairs;
    for (const auto& conflict : conflicts) {
        if (!pairs.emplace(conflict.left_operation, conflict.right_operation).second ||
            !scheduled.count(conflict.left_operation) || !scheduled.count(conflict.right_operation) ||
            conflict.relation != "independent")
            return "effect schedule contains duplicate, foreign, or conflicting pair evidence";
    }
    return {};
}

} // namespace flowcontracts
