#pragma once

#include <flowcontracts/source_disposition_policy.hpp>

namespace flowcontracts {

// Canonical control/data projection of already resolved disposition authority.
// This does not select routes, resolve source names, or grant execution.
inline json::Value project_disposition_graph(const json::Value& topology_value,
                                            const json::Value& policy_value,
                                            const json::Value& host_value,
                                            int projection_version = 3) {
    using namespace json;
    if (projection_version < 1 || projection_version > 3)
        throw Error("$.disposition_graph.version", "unsupported disposition projection version");
    validate_source_disposition_policy_selection(policy_value, topology_value);
    validate_host_disposition_policy_selection(host_value, topology_value);
    const auto& topology = object(topology_value);
    if (projection_version >= 2)
        for (const auto& raw : array(required(topology, "response_functions"), "$.response_functions")) {
            const auto& response = object(raw);
            if (integer(required(response, "parameter_symbol_id"), "$.response.parameter_symbol_id") < 0)
                throw Error("$.response.parameter_symbol_id", "response envelope requires a resolved parameter identity");
        }
    const auto& policy = object(policy_value);
    const auto& producer = object(required(topology, "producer"));
    const auto& consumer = object(required(topology, "consumer"));
    const auto& junction = object(required(topology, "junction"));
    const auto& rejoin = object(required(topology, "rejoin"));
    const auto& containment = object(required(topology, "containment"));
    auto str = [](const Object& owner, const char* key) {
        return string(required(owner, key), key);
    };
    const auto producer_id = str(producer, "graph_node_id");
    const auto consumer_id = str(consumer, "instance_node_id");
    const auto dispatch_id = consumer_id + ":failure-dispatch";
    const auto join_id = consumer_id + ":success-junction";
    const auto destination_id = str(rejoin, "node_id");
    const auto containment_id = str(containment, "node_id");
    Array nodes{
        Object{{"id", producer_id}, {"role", "producer"},
            {"function_symbol_id", required(producer, "function_symbol_id")},
            {"operation_id", required(producer, "operation_id")}},
        Object{{"id", dispatch_id}, {"role", "failure_dispatch"},
            {"consumer_identity", required(consumer, "semantic_id")}},
        Object{{"id", join_id}, {"role", "exclusive_success_junction"},
            {"consumer_identity", required(consumer, "semantic_id")},
            {"pairing", required(junction, "pairing")}},
        Object{{"id", destination_id}, {"role", "rejoin_destination"},
            {"function_symbol_id", required(rejoin, "function_symbol_id")},
            {"parameter_symbol_id", required(rejoin, "parameter_symbol_id")}},
        Object{{"id", containment_id}, {"role", "activation_containment"},
            {"scope_kind", required(containment, "scope_kind")},
            {"action", required(containment, "action")}}
    };
    if (projection_version >= 3)
        std::get<Object>(nodes.front()).emplace("result_projection", required(producer, "result_projection"));
    auto edge = [](Value id, std::string from, std::string to,
                   std::string lane, Value type) -> Value {
        return Object{{"id", std::move(id)}, {"from", std::move(from)},
            {"to", std::move(to)}, {"lane", std::move(lane)}, {"payload", std::move(type)}};
    };
    Array edges{
        edge(required(junction, "success_wire_id"), producer_id, join_id,
             "success", required(producer, "success_type")),
        edge(required(junction, "failure_wire_id"), producer_id, dispatch_id,
             "failure", required(producer, "failure_types")),
        edge(required(containment, "wire_id"), producer_id, containment_id,
             "fault", required(producer, "fault_types")),
        edge(required(junction, "rejoin_wire_id"), join_id, destination_id,
             "success", required(producer, "success_type"))
    };
    for (const auto& raw : array(required(policy, "selections"), "$.selections")) {
        const auto& selection = object(raw);
        const auto route_id = integer(required(selection, "route_id"), "$.route_id");
        const auto& transition = object(required(selection, "response_transition"));
        if (str(transition, "response_class") != "recover")
            throw Error("$.disposition_graph", "transformation projection requires resolved successor authority");
        const auto response_id = consumer_id + ":response:" + std::to_string(route_id);
        nodes.emplace_back(Object{{"id", response_id}, {"role", "ordinary_response"},
            {"route_id", required(selection, "route_id")},
            {"function_symbol_id", required(selection, "response_function_symbol_id")},
            {"transition", required(selection, "response_transition")}});
        if (projection_version >= 2) {
            const Object* response = nullptr;
            for (const auto& candidate : array(required(topology, "response_functions"), "$.response_functions")) {
                const auto& fact = object(candidate);
                if (integer(required(fact, "function_symbol_id"), "$.response.function_symbol_id") ==
                    integer(required(selection, "response_function_symbol_id"), "$.selection.response_function_symbol_id")) response = &fact;
            }
            if (!response) throw Error("$.disposition_graph", "selected response lacks its source witness");
            auto& node = std::get<Object>(nodes.back());
            node.emplace("parameter_symbol_id", required(*response, "parameter_symbol_id"));
            node.emplace("input_projection", required(*response, "input_projection"));
            node.emplace("incoming_failure_type", required(*response, "incoming_failure_type"));
        }
        edges.push_back(edge("selected-route:" + std::to_string(route_id), dispatch_id, response_id,
            "failure", required(selection, "incoming_failure_type")));
        edges.push_back(edge("response-result:" + std::to_string(route_id), response_id, join_id,
            "recovered_success", required(transition, "outgoing_type")));
    }
    std::set<std::string> identities;
    for (const auto& node : nodes)
        if (!identities.insert(str(object(node), "id")).second)
            throw Error("$.disposition_graph.nodes", "projected node identities collide");
    std::set<std::string> wires;
    for (const auto& value : edges) {
        const auto& wire = object(value);
        if (!wires.insert(str(wire, "id")).second ||
            !identities.count(str(wire, "from")) || !identities.count(str(wire, "to")))
            throw Error("$.disposition_graph.edges", "duplicate edge or missing endpoint");
    }
    return Object{{"format", "lyraform.disposition_graph"}, {"version", projection_version},
        {"status", "validated_projection"}, {"execution", "unsupported"},
        {"module", required(topology, "module")},
        {"policy", required(policy, "policy")},
        {"host_policy", host_value},
        {"schedule", "serial"}, {"boundary_transforms", "forbidden_without_equivalence_proof"},
        {"obligation_id", required(producer, "obligation_id")},
        {"commit_law", required(producer, "commit_law")},
        {"nodes", std::move(nodes)}, {"edges", std::move(edges)}};
}

inline void validate_disposition_graph(const json::Object& plan, std::string path) {
    using namespace json;
    const auto* graph = optional(plan, "disposition_graph");
    const auto* version = optional(plan, "disposition_graph_version");
    if (!graph && !version) return;
    if (!graph || !version || (integer(*version, path + ".disposition_graph_version") != 1 &&
        integer(*version, path + ".disposition_graph_version") != 2 &&
        integer(*version, path + ".disposition_graph_version") != 3))
        throw Error(path, "disposition graph and admitted projection version must both be present");
    const auto expected = project_disposition_graph(required(plan, "source_disposition", path),
        required(plan, "source_disposition_policy", path), required(plan, "host_disposition_policy", path),
        static_cast<int>(integer(*version, path + ".disposition_graph_version")));
    if (serialize(*graph) != serialize(expected))
        throw Error(path + ".disposition_graph", "graph differs from its exact source and selected policy projection");
}

} // namespace flowcontracts
