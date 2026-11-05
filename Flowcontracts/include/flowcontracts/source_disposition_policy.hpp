#pragma once

#include <flowcontracts/source_disposition.hpp>

#include <initializer_list>
#include <string>

namespace flowcontracts {

namespace source_disposition_policy_detail {

inline void exact_fields(const json::Object& value,
                         std::initializer_list<std::string_view> expected,
                         const std::string& path) {
    if (value.size() != expected.size())
        throw json::Error(path, "unexpected field set");
    for (const auto field : expected)
        if (!value.contains(field))
            throw json::Error(path + "." + std::string(field),
                              "required field is missing");
}

inline const std::string& text(const json::Object& value, std::string_view key,
                               const std::string& path) {
    return json::string(json::required(value, key, path), path + "." + std::string(key));
}

inline json::Integer number(const json::Object& value, std::string_view key,
                            const std::string& path) {
    return json::integer(json::required(value, key, path), path + "." + std::string(key));
}

inline void policy_provenance(const json::Value& raw, const std::string& path) {
    const auto& value = json::object(raw, path);
    exact_fields(value, {"source", "line", "column"}, path);
    if (text(value, "source", path).empty() || number(value, "line", path) < 1 ||
        number(value, "column", path) < 1)
        throw json::Error(path, "policy provenance is incomplete");
}

inline const json::Object& topology_module(const json::Object& topology,
                                            const std::string& path) {
    return json::object(json::required(topology, "module", path), path + ".module");
}

inline const json::Object& topology_consumer(const json::Object& topology,
                                              const std::string& path) {
    return json::object(json::required(topology, "consumer", path), path + ".consumer");
}

inline const json::Object& only_selection(const json::Object& policy,
                                           const std::string& path) {
    const auto& selections = json::array(json::required(policy, "selections", path),
                                         path + ".selections");
    if (selections.size() != 1)
        throw json::Error(path + ".selections",
                          "bounded policy requires exactly one selection");
    return json::object(selections.front(), path + ".selections[0]");
}

inline const json::Object& policy_metadata(const json::Object& root,
                                            const std::string& path) {
    const auto& policy = json::object(json::required(root, "policy", path), path + ".policy");
    exact_fields(policy, {"profile", "revision", "authority", "provenance"},
                 path + ".policy");
    if (text(policy, "profile", path + ".policy").empty() ||
        text(policy, "revision", path + ".policy").empty() ||
        text(policy, "authority", path + ".policy").empty())
        throw json::Error(path + ".policy", "policy identity is incomplete");
    policy_provenance(json::required(policy, "provenance", path + ".policy"),
                      path + ".policy.provenance");
    return policy;
}

inline bool same_type_identity(const json::Value& selected, const json::Value& canonical,
                               const std::string& selected_path,
                               const std::string& canonical_path) {
    const auto& selected_type = json::object(selected, selected_path);
    const auto& canonical_type = json::object(canonical, canonical_path);
    exact_fields(selected_type, {"identity", "spelling"}, selected_path);
    return text(selected_type, "spelling", selected_path) ==
               text(canonical_type, "spelling", canonical_path) &&
           text(selected_type, "identity", selected_path) ==
               text(canonical_type, "identity", canonical_path);
}

} // namespace source_disposition_policy_detail

inline void validate_source_disposition_policy_selection(
    const json::Value& raw_policy, const json::Value& raw_topology,
    std::string policy_path = "$", std::string topology_path = "$.topology") {
    using namespace json;
    using namespace source_disposition_policy_detail;

    validate_source_disposition_topology(raw_topology, topology_path);
    const auto& topology = object(raw_topology, topology_path);
    const auto& topology_module_value = topology_module(topology, topology_path);
    const auto& topology_consumer_value = topology_consumer(topology, topology_path);
    const auto& policy = object(raw_policy, policy_path);
    exact_fields(policy,
                 {"format", "version", "status", "execution", "module",
                  "selection_kind", "selections", "policy"},
                 policy_path);

    if (text(policy, "format", policy_path) !=
            "lyraform.source_disposition_policy_selection" ||
        number(policy, "version", policy_path) != 1 ||
        text(policy, "status", policy_path) != "selected" ||
        text(policy, "execution", policy_path) != "unsupported")
        throw Error(policy_path, "unsupported or executable source-disposition policy");

    const auto& module = object(required(policy, "module", policy_path),
                                policy_path + ".module");
    exact_fields(module, {"identity", "revision"}, policy_path + ".module");
    if (text(module, "identity", policy_path + ".module") !=
            text(topology_module_value, "identity", topology_path + ".module") ||
        text(module, "revision", policy_path + ".module") !=
            text(topology_module_value, "revision", topology_path + ".module"))
        throw Error(policy_path + ".module",
                    "policy module identity or revision is stale or foreign");

    const auto& routes = array(required(topology_consumer_value, "routes",
                                        topology_path + ".consumer"),
                               topology_path + ".consumer.routes");
    if (routes.empty())
        throw Error(topology_path + ".consumer.routes", "source topology has no legal route");
    const auto selection_kind = text(policy, "selection_kind", policy_path);
    const auto required_kind = routes.size() == 1 ? "fixed_single_route" : "supplied";
    if (selection_kind != required_kind)
        throw Error(policy_path + ".selection_kind",
                    "selection kind disagrees with the source-authorized route set");

    const auto& metadata = policy_metadata(policy, policy_path);
    if (selection_kind == "fixed_single_route" &&
        (text(metadata, "profile", policy_path + ".policy") !=
             "compiler.fixed_single_route" ||
         text(metadata, "authority", policy_path + ".policy") !=
             "lyraform.semantic-policy-selector" ||
         text(metadata, "revision", policy_path + ".policy") !=
             text(topology_module_value, "revision", topology_path + ".module")))
        throw Error(policy_path + ".policy",
                    "fixed selection does not have deterministic compiler authority");

    const auto& selection = only_selection(policy, policy_path);
    exact_fields(selection,
                 {"declaration_id", "semantic_id", "instance_structural_id",
                  "instance_node_id", "incoming_failure_type", "route_id",
                  "response_function_symbol_id", "response_transition"},
                 policy_path + ".selections[0]");
    for (const auto key : {"declaration_id", "instance_structural_id"})
        if (number(selection, key, policy_path + ".selections[0]") !=
            number(topology_consumer_value, key, topology_path + ".consumer"))
            throw Error(policy_path + ".selections[0]." + key,
                        "selected consumer identity is foreign");
    for (const auto key : {"semantic_id", "instance_node_id"})
        if (text(selection, key, policy_path + ".selections[0]") !=
            text(topology_consumer_value, key, topology_path + ".consumer"))
            throw Error(policy_path + ".selections[0]." + key,
                        "selected consumer identity is foreign");

    const auto selected_route_id = number(selection, "route_id",
                                          policy_path + ".selections[0]");
    const Object* selected_route = nullptr;
    for (std::size_t index = 0; index < routes.size(); ++index) {
        const auto route_path = topology_path + ".consumer.routes[" +
                                std::to_string(index) + "]";
        const auto& route = object(routes[index], route_path);
        if (number(route, "route_id", route_path) == selected_route_id) {
            if (selected_route)
                throw Error(topology_path + ".consumer.routes",
                            "source topology has an ambiguous route identity");
            selected_route = &route;
        }
    }
    if (!selected_route)
        throw Error(policy_path + ".selections[0].route_id",
                    "policy selected an unauthorized route");

    const auto failure_type = text(*selected_route, "failure_type",
                                   topology_path + ".consumer.routes[]");
    const auto response_function = number(*selected_route, "function_symbol_id",
                                          topology_path + ".consumer.routes[]");
    if (number(selection, "response_function_symbol_id",
               policy_path + ".selections[0]") != response_function)
        throw Error(policy_path + ".selections[0].response_function_symbol_id",
                    "policy changed the selected response-function identity");

    const Value* canonical_failure_type = nullptr;
    const auto& producer = object(required(topology, "producer", topology_path),
                                  topology_path + ".producer");
    for (const auto& type : array(required(producer, "failure_types",
                                           topology_path + ".producer"),
                                  topology_path + ".producer.failure_types")) {
        const auto& candidate = object(type, topology_path + ".producer.failure_types[]");
        if (text(candidate, "spelling", topology_path + ".producer.failure_types[]") ==
            failure_type)
            canonical_failure_type = &type;
    }
    if (!canonical_failure_type ||
        !same_type_identity(required(selection, "incoming_failure_type",
                                     policy_path + ".selections[0]"),
                            *canonical_failure_type,
                            policy_path + ".selections[0].incoming_failure_type",
                            topology_path + ".producer.failure_types[]"))
        throw Error(policy_path + ".selections[0].incoming_failure_type",
                    "policy changed the incoming failure type identity");

    const Object* response = nullptr;
    for (const auto& raw_response : array(required(topology, "response_functions",
                                                  topology_path),
                                           topology_path + ".response_functions")) {
        const auto& candidate = object(raw_response,
                                       topology_path + ".response_functions[]");
        if (number(candidate, "function_symbol_id",
                   topology_path + ".response_functions[]") == response_function)
            response = &candidate;
    }
    if (!response)
        throw Error(policy_path + ".selections[0].response_function_symbol_id",
                    "selected response function is absent from source authority");
    const auto& transition = object(required(selection, "response_transition",
                                             policy_path + ".selections[0]"),
                                    policy_path + ".selections[0].response_transition");
    exact_fields(transition,
                 {"response_class", "outgoing_type", "result_resource",
                  "obligation_transition"},
                 policy_path + ".selections[0].response_transition");
    if (text(transition, "response_class",
             policy_path + ".selections[0].response_transition") !=
            text(*response, "response_class", topology_path + ".response_functions[]") ||
        text(transition, "obligation_transition",
             policy_path + ".selections[0].response_transition") !=
            text(*response, "obligation_transition",
                 topology_path + ".response_functions[]") ||
        !same_type_identity(required(transition, "outgoing_type",
                                     policy_path + ".selections[0].response_transition"),
                            required(*response, "outgoing_type",
                                     topology_path + ".response_functions[]"),
                            policy_path + ".selections[0].response_transition.outgoing_type",
                            topology_path + ".response_functions[].outgoing_type") ||
        serialize(required(transition, "result_resource",
                          policy_path + ".selections[0].response_transition")) !=
            serialize(required(*response, "result_resource",
                              topology_path + ".response_functions[]")))
        throw Error(policy_path + ".selections[0].response_transition",
                    "policy changed the source-declared response transition");
}

inline json::Value fixed_source_disposition_policy_selection(
    const json::Value& raw_topology) {
    using namespace json;
    using namespace source_disposition_policy_detail;

    validate_source_disposition_topology(raw_topology);
    const auto& topology = object(raw_topology);
    const std::string topology_root = "$topology";
    const auto& module = topology_module(topology, topology_root);
    const auto& consumer = topology_consumer(topology, topology_root);
    const auto& routes = array(required(consumer, "routes", "$topology.consumer"),
                               "$topology.consumer.routes");
    if (routes.size() != 1)
        throw Error("$topology.consumer.routes",
                    "fixed selection requires exactly one source-authorized route");
    const auto& route = object(routes.front(), "$topology.consumer.routes[0]");
    const auto response_id = number(route, "function_symbol_id",
                                    "$topology.consumer.routes[0]");
    const Object* response = nullptr;
    for (const auto& raw_response : array(required(topology, "response_functions", "$topology"),
                                           "$topology.response_functions")) {
        const auto& candidate = object(raw_response, "$topology.response_functions[]");
        if (number(candidate, "function_symbol_id", "$topology.response_functions[]") ==
            response_id)
            response = &candidate;
    }
    if (!response)
        throw Error("$topology.response_functions",
                    "fixed route has no source-authorized response function");
    const auto failure_spelling = text(route, "failure_type",
                                       "$topology.consumer.routes[0]");
    const Value* failure_type = nullptr;
    const auto& producer = object(required(topology, "producer", "$topology"),
                                  "$topology.producer");
    for (const auto& candidate : array(required(producer, "failure_types",
                                                "$topology.producer"),
                                       "$topology.producer.failure_types"))
        if (text(object(candidate, "$topology.producer.failure_types[]"), "spelling",
                 "$topology.producer.failure_types[]") == failure_spelling)
            failure_type = &candidate;
    if (!failure_type)
        throw Error("$topology.consumer.routes[0].failure_type",
                    "fixed route has no canonical failure type");

    Value policy = Object{
        {"format", "lyraform.source_disposition_policy_selection"},
        {"version", 1}, {"status", "selected"}, {"execution", "unsupported"},
        {"module", Object{{"identity", text(module, "identity", "$topology.module")},
                          {"revision", text(module, "revision", "$topology.module")}}},
        {"selection_kind", "fixed_single_route"},
        {"selections", Array{Object{
            {"declaration_id", number(consumer, "declaration_id", "$topology.consumer")},
            {"semantic_id", text(consumer, "semantic_id", "$topology.consumer")},
            {"instance_structural_id", number(consumer, "instance_structural_id",
                                               "$topology.consumer")},
            {"instance_node_id", text(consumer, "instance_node_id", "$topology.consumer")},
            {"incoming_failure_type", Object{
                {"identity", text(object(*failure_type), "identity",
                                  "$topology.producer.failure_types[]")},
                {"spelling", text(object(*failure_type), "spelling",
                                  "$topology.producer.failure_types[]")}}},
            {"route_id", number(route, "route_id", "$topology.consumer.routes[0]")},
            {"response_function_symbol_id", response_id},
            {"response_transition", Object{
                {"response_class", text(*response, "response_class",
                                        "$topology.response_functions[]")},
                {"outgoing_type", Object{
                    {"identity", text(object(required(*response, "outgoing_type",
                                                     "$topology.response_functions[]")),
                                      "identity", "$topology.response_functions[].outgoing_type")},
                    {"spelling", text(object(required(*response, "outgoing_type",
                                                     "$topology.response_functions[]")),
                                      "spelling", "$topology.response_functions[].outgoing_type")}}},
                {"result_resource", required(*response, "result_resource",
                                                "$topology.response_functions[]")},
                {"obligation_transition", text(*response, "obligation_transition",
                                                "$topology.response_functions[]")}}}}}},
        {"policy", Object{
            {"profile", "compiler.fixed_single_route"},
            {"revision", text(module, "revision", "$topology.module")},
            {"authority", "lyraform.semantic-policy-selector"},
            {"provenance", required(consumer, "provenance", "$topology.consumer")}}}};
    validate_source_disposition_policy_selection(policy, raw_topology);
    return policy;
}

inline void validate_host_disposition_policy_selection(
    const json::Value& raw_policy, const json::Value& raw_topology,
    std::string policy_path = "$", std::string topology_path = "$.topology") {
    using namespace json;
    using namespace source_disposition_policy_detail;

    validate_source_disposition_topology(raw_topology, topology_path);
    const auto& topology = object(raw_topology, topology_path);
    const auto& topology_module_value = topology_module(topology, topology_path);
    const auto& containment = object(required(topology, "containment", topology_path),
                                     topology_path + ".containment");
    const auto& policy = object(raw_policy, policy_path);
    exact_fields(policy,
                 {"format", "version", "status", "execution", "module",
                  "containment", "action", "policy"},
                 policy_path);
    if (text(policy, "format", policy_path) !=
            "lyraform.host_disposition_policy_selection" ||
        number(policy, "version", policy_path) != 1 ||
        text(policy, "status", policy_path) != "selected" ||
        text(policy, "execution", policy_path) != "unsupported")
        throw Error(policy_path, "unsupported or executable host disposition policy");
    const auto& module = object(required(policy, "module", policy_path),
                                policy_path + ".module");
    exact_fields(module, {"identity", "revision"}, policy_path + ".module");
    if (text(module, "identity", policy_path + ".module") !=
            text(topology_module_value, "identity", topology_path + ".module") ||
        text(module, "revision", policy_path + ".module") !=
            text(topology_module_value, "revision", topology_path + ".module"))
        throw Error(policy_path + ".module",
                    "host policy module identity or revision is stale or foreign");
    const auto& selected_containment = object(required(policy, "containment", policy_path),
                                              policy_path + ".containment");
    exact_fields(selected_containment, {"node_id", "scope_kind"},
                 policy_path + ".containment");
    if (text(selected_containment, "node_id", policy_path + ".containment") !=
            text(containment, "node_id", topology_path + ".containment") ||
        text(selected_containment, "scope_kind", policy_path + ".containment") !=
            text(containment, "scope_kind", topology_path + ".containment"))
        throw Error(policy_path + ".containment",
                    "host policy changed the containment identity or scope");
    if (text(policy, "action", policy_path) != "report_and_fail")
        throw Error(policy_path + ".action",
                    "only report_and_fail is admitted by the bounded host policy");
    (void)policy_metadata(policy, policy_path);
}

} // namespace flowcontracts
