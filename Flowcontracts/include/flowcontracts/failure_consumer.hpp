#pragma once

#include <flowcontracts/json.hpp>

#include <map>
#include <set>
#include <string>
#include <vector>

namespace flowcontracts {

// Declarative expected-failure routing authority. It deliberately stops before
// executable response semantics: an ordinary return type is not proof that a
// function recovered, propagated, or requested a safe retry.
struct FailureConsumerFunction {
    long long id = -1;
    std::string parameter_projection;
    std::string failure_payload_type;
    std::string result_type;
    std::string availability;
};

struct FailureConsumerRoute {
    long long id = -1;
    std::string failure_type;
    long long function = -1;
};

struct FailureConsumer {
    long long id = -1;
    long long scope = -1;
    std::vector<std::string> accepted_failure_types;
    std::vector<FailureConsumerRoute> routes;
};

struct FailurePolicySelection {
    long long consumer = -1;
    std::string failure_type;
    long long route = -1;
    std::string policy;
    std::string policy_revision;
};

inline json::Value failure_consumer_fact(const FailureConsumer& consumer) {
    using namespace json;
    Array accepted;
    for (const auto& type : consumer.accepted_failure_types) accepted.emplace_back(type);
    Array routes;
    for (const auto& route : consumer.routes) {
        routes.emplace_back(Object{{"route_id", Integer(route.id)},
                                   {"failure_type", route.failure_type},
                                   {"function_symbol_id", Integer(route.function)}});
    }
    return Object{{"format", "lyraform.failure_consumer"}, {"version", 1},
                  {"status", "declarative"}, {"consumer_id", Integer(consumer.id)},
                  {"scope_id", Integer(consumer.scope)}, {"closed_set", true},
                  {"accepted_failure_types", std::move(accepted)}, {"routes", std::move(routes)}};
}

inline FailureConsumer read_failure_consumer(const json::Value& raw, const std::string& path) {
    using namespace json;
    const auto& fact = object(raw, path);
    auto number = [&](std::string_view key) {
        return integer(required(fact, key, path), path + "." + std::string(key));
    };
    auto text = [&](std::string_view key) -> const std::string& {
        return string(required(fact, key, path), path + "." + std::string(key));
    };
    if (text("format") != "lyraform.failure_consumer" || number("version") != 1 ||
        text("status") != "declarative" ||
        !boolean(required(fact, "closed_set", path), path + ".closed_set"))
        throw Error(path, "unsupported or executable failure-consumer contract");

    FailureConsumer consumer;
    consumer.id = number("consumer_id");
    consumer.scope = number("scope_id");
    const auto& accepted = array(required(fact, "accepted_failure_types", path),
                                 path + ".accepted_failure_types");
    for (std::size_t index = 0; index < accepted.size(); ++index)
        consumer.accepted_failure_types.push_back(
            string(accepted[index], path + ".accepted_failure_types[" + std::to_string(index) + "]"));
    const auto& routes = array(required(fact, "routes", path), path + ".routes");
    for (std::size_t index = 0; index < routes.size(); ++index) {
        const auto route_path = path + ".routes[" + std::to_string(index) + "]";
        const auto& route = object(routes[index], route_path);
        consumer.routes.push_back({integer(required(route, "route_id", route_path), route_path + ".route_id"),
                                   string(required(route, "failure_type", route_path), route_path + ".failure_type"),
                                   integer(required(route, "function_symbol_id", route_path),
                                           route_path + ".function_symbol_id")});
    }
    return consumer;
}

inline std::string failure_consumer_refusal(
    const FailureConsumer& consumer,
    const std::vector<FailureConsumerFunction>& functions) {
    if (consumer.id < 0 || consumer.scope < 0 || consumer.accepted_failure_types.empty() ||
        consumer.routes.empty())
        return "invalid failure-consumer identity or empty closed set";

    std::set<std::string> accepted;
    for (const auto& type : consumer.accepted_failure_types)
        if (type.empty() || !accepted.insert(type).second)
            return "invalid or duplicate accepted failure type";

    std::map<long long, const FailureConsumerFunction*> function_by_id;
    for (const auto& function : functions)
        if (function.id < 0 || !function_by_id.emplace(function.id, &function).second)
            return "invalid or duplicate failure-response function identity";

    std::set<long long> route_ids;
    std::set<std::pair<std::string, long long>> route_functions;
    std::map<std::string, std::size_t> route_count;
    for (const auto& route : consumer.routes) {
        if (route.id < 0 || !route_ids.insert(route.id).second)
            return "invalid or duplicate failure route identity";
        if (!accepted.count(route.failure_type))
            return "failure route is outside the consumer closed set";
        if (!route_functions.emplace(route.failure_type, route.function).second)
            return "failure route duplicates a response function";
        const auto found = function_by_id.find(route.function);
        if (found == function_by_id.end()) return "failure route function is missing";
        const auto& function = *found->second;
        if (function.availability != "definition" ||
            function.parameter_projection != "failure_envelope" ||
            function.failure_payload_type != route.failure_type || function.result_type.empty() ||
            function.result_type == "void")
            return "failure route function contract is incompatible";
        ++route_count[route.failure_type];
    }
    for (const auto& type : accepted)
        if (route_count[type] == 0) return "accepted failure type has no response route";
    return {};
}

inline json::Value failure_policy_selection_fact(const FailurePolicySelection& selection) {
    using namespace json;
    return Object{{"format", "lyraform.failure_policy_selection"}, {"version", 1},
                  {"status", "declarative"}, {"consumer_id", Integer(selection.consumer)},
                  {"failure_type", selection.failure_type}, {"route_id", Integer(selection.route)},
                  {"policy", selection.policy}, {"policy_revision", selection.policy_revision}};
}

inline FailurePolicySelection read_failure_policy_selection(const json::Value& raw,
                                                             const std::string& path) {
    using namespace json;
    const auto& fact = object(raw, path);
    auto number = [&](std::string_view key) {
        return integer(required(fact, key, path), path + "." + std::string(key));
    };
    auto text = [&](std::string_view key) -> const std::string& {
        return string(required(fact, key, path), path + "." + std::string(key));
    };
    if (text("format") != "lyraform.failure_policy_selection" || number("version") != 1 ||
        text("status") != "declarative")
        throw Error(path, "unsupported or executable failure-policy selection contract");
    return {number("consumer_id"), text("failure_type"), number("route_id"),
            text("policy"), text("policy_revision")};
}

inline std::string failure_policy_selection_refusal(const FailurePolicySelection& selection,
                                                    const FailureConsumer& consumer,
                                                    const std::vector<FailureConsumerFunction>& functions) {
    const auto consumer_error = failure_consumer_refusal(consumer, functions);
    if (!consumer_error.empty()) return consumer_error;
    if (selection.consumer != consumer.id || selection.failure_type.empty() || selection.route < 0 ||
        selection.policy.empty() || selection.policy_revision.empty())
        return "invalid failure-policy selection identity";
    for (const auto& route : consumer.routes)
        if (route.id == selection.route && route.failure_type == selection.failure_type) return {};
    return "policy selected a route not authorized for this failure";
}

} // namespace flowcontracts
