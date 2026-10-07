#pragma once

#include <flowcontracts/failure_consumer.hpp>
#include <flowcontracts/json.hpp>

#include <map>
#include <set>
#include <string>
#include <vector>

namespace flowcontracts {

// Declarative meaning of successful completion of one already-authorized
// failure-response function. This is semantic authority only: it neither
// invokes the function nor makes the route executable.
struct FailureResponseTransition {
    long long id = -1;
    long long consumer = -1;
    long long route = -1;
    long long function = -1;
    std::string incoming_failure_type;
    std::string input_projection;
    std::string response_class;
    std::string outgoing_disposition;
    std::string outgoing_payload_type;
    std::string obligation_transition;
    std::string origin_commit_law;
    std::string provenance_law;
    std::string response_attempt_failure_law;
};

inline json::Value failure_response_transition_fact(
    const FailureResponseTransition& transition) {
    using namespace json;
    return Object{{"format", "lyraform.failure_response_transition"},
                  {"version", 1},
                  {"status", "declarative"},
                  {"transition_id", Integer(transition.id)},
                  {"consumer_id", Integer(transition.consumer)},
                  {"route_id", Integer(transition.route)},
                  {"function_symbol_id", Integer(transition.function)},
                  {"incoming_failure_type", transition.incoming_failure_type},
                  {"input_projection", transition.input_projection},
                  {"response_class", transition.response_class},
                  {"outgoing_disposition", transition.outgoing_disposition},
                  {"outgoing_payload_type", transition.outgoing_payload_type},
                  {"obligation_transition", transition.obligation_transition},
                  {"origin_commit_law", transition.origin_commit_law},
                  {"provenance_law", transition.provenance_law},
                  {"response_attempt_failure_law",
                   transition.response_attempt_failure_law}};
}

inline FailureResponseTransition read_failure_response_transition(
    const json::Value& raw, const std::string& path) {
    using namespace json;
    const auto& fact = object(raw, path);
    auto number = [&](std::string_view key) {
        return integer(required(fact, key, path), path + "." + std::string(key));
    };
    auto text = [&](std::string_view key) -> const std::string& {
        return string(required(fact, key, path), path + "." + std::string(key));
    };
    if (text("format") != "lyraform.failure_response_transition" ||
        number("version") != 1 || text("status") != "declarative")
        throw Error(path, "unsupported or executable failure-response transition contract");

    return {number("transition_id"),
            number("consumer_id"),
            number("route_id"),
            number("function_symbol_id"),
            text("incoming_failure_type"),
            text("input_projection"),
            text("response_class"),
            text("outgoing_disposition"),
            text("outgoing_payload_type"),
            text("obligation_transition"),
            text("origin_commit_law"),
            text("provenance_law"),
            text("response_attempt_failure_law")};
}

inline std::string failure_response_transitions_refusal(
    const std::vector<FailureResponseTransition>& transitions,
    const FailureConsumer& consumer,
    const std::vector<FailureConsumerFunction>& functions) {
    const auto consumer_error = failure_consumer_refusal(consumer, functions);
    if (!consumer_error.empty()) return consumer_error;
    if (transitions.empty()) return "failure-response transition set is empty";

    std::map<long long, const FailureConsumerRoute*> routes;
    for (const auto& route : consumer.routes) routes.emplace(route.id, &route);
    std::map<long long, const FailureConsumerFunction*> functions_by_id;
    for (const auto& function : functions) functions_by_id.emplace(function.id, &function);

    std::set<long long> transition_ids;
    std::set<long long> covered_routes;
    for (const auto& transition : transitions) {
        if (transition.id < 0 || !transition_ids.insert(transition.id).second)
            return "invalid or duplicate failure-response transition identity";
        if (transition.consumer != consumer.id)
            return "failure-response transition consumer identity mismatch";
        const auto route = routes.find(transition.route);
        if (route == routes.end() || !covered_routes.insert(transition.route).second)
            return "missing or duplicate failure-response route transition";
        if (transition.function != route->second->function ||
            transition.incoming_failure_type != route->second->failure_type)
            return "failure-response transition disagrees with its authorized route";
        const auto function = functions_by_id.find(transition.function);
        if (function == functions_by_id.end())
            return "failure-response transition function is missing";
        if (transition.input_projection != "failure_envelope" ||
            transition.input_projection != function->second->parameter_projection ||
            transition.incoming_failure_type != function->second->failure_payload_type)
            return "failure-response transition input projection is incompatible";
        if (transition.outgoing_payload_type.empty() ||
            transition.outgoing_payload_type == "void" ||
            transition.outgoing_payload_type != function->second->result_type)
            return "failure-response transition result type is incompatible";

        if (transition.response_class == "recover") {
            if (transition.outgoing_disposition != "success" ||
                transition.obligation_transition != "close_original")
                return "recovery transition must produce success and close the original obligation";
        } else if (transition.response_class == "transform") {
            if (transition.outgoing_disposition != "failure" ||
                transition.obligation_transition != "linked_successor")
                return "transformation transition must produce a linked successor failure";
        } else {
            return "unsupported failure-response class";
        }

        if (transition.origin_commit_law != "preserve_origin_commit" ||
            transition.provenance_law != "link_response_to_origin" ||
            transition.response_attempt_failure_law != "separate_obligations")
            return "failure-response accounting law changed";
    }
    if (covered_routes.size() != routes.size())
        return "authorized failure route lacks a response transition";
    return {};
}

} // namespace flowcontracts
