#pragma once

#include <flowcontracts/json.hpp>

#include <map>
#include <set>
#include <string>
#include <tuple>

namespace flowcontracts {

inline void validate_source_disposition_topology(const json::Value& raw,
                                                 std::string path = "$") {
    using namespace json;
    const auto& root = object(raw, path);
    auto text = [&](const Object& value, std::string_view key, const std::string& owner) -> const std::string& {
        return string(required(value, key, owner), owner + "." + std::string(key));
    };
    auto number = [&](const Object& value, std::string_view key, const std::string& owner) {
        return integer(required(value, key, owner), owner + "." + std::string(key));
    };
    std::set<std::string> observed_origins;
    auto provenance = [&](const json::Value& value, const std::string& owner) {
        const auto& p = object(value, owner);
        const auto& source = text(p, "source", owner);
        if (source.empty() || number(p, "line", owner) < 1 ||
            number(p, "column", owner) < 1)
            throw Error(owner, "verified source provenance is incomplete");
        observed_origins.insert(source);
    };
    auto type = [&](const json::Value& value, const std::string& owner) {
        const auto& t = object(value, owner);
        const auto spelling = text(t, "spelling", owner);
        const auto identity = text(t, "identity", owner);
        if (spelling.empty() || identity != "type:" + spelling)
            throw Error(owner, "type spelling and semantic identity disagree");
        provenance(required(t, "provenance", owner), owner + ".provenance");
        return spelling;
    };

    if (text(root, "format", path) != "lyraform.source_disposition_topology" ||
        number(root, "version", path) != 1 || text(root, "status", path) != "semantic" ||
        text(root, "execution", path) != "unsupported")
        throw Error(path, "unsupported or executable source-disposition topology");

    const auto& module = object(required(root, "module", path), path + ".module");
    const auto module_identity = text(module, "identity", path + ".module");
    const auto module_revision = text(module, "revision", path + ".module");
    if (module_identity.empty() || module_revision.empty())
        throw Error(path + ".module", "module identity and revision are required");
    provenance(required(module, "provenance", path + ".module"), path + ".module.provenance");
    std::set<std::string> allowed_origins;
    const auto& raw_origins = array(required(module, "source_origins", path + ".module"),
                                    path + ".module.source_origins");
    if (raw_origins.empty() || raw_origins.size() > 64)
        throw Error(path + ".module.source_origins", "bounded source-origin authority is required");
    for (std::size_t i = 0; i < raw_origins.size(); ++i) {
        const auto& origin = string(raw_origins[i], path + ".module.source_origins[" + std::to_string(i) + "]");
        if (origin.empty() || !allowed_origins.insert(origin).second)
            throw Error(path + ".module.source_origins", "empty or duplicate source origin");
    }

    const auto& producer = object(required(root, "producer", path), path + ".producer");
    const auto operation = number(producer, "operation_id", path + ".producer");
    const auto disposition = number(producer, "disposition_id", path + ".producer");
    const auto function = number(producer, "function_symbol_id", path + ".producer");
    const auto owner = number(producer, "owner_symbol_id", path + ".producer");
    const auto return_operation = number(producer, "return_operation_id", path + ".producer");
    if (operation < 0 || disposition < 0 || function < 0 || owner < 0 || return_operation < 0 ||
        text(producer, "graph_node_id", path + ".producer").empty() ||
        text(producer, "completion", path + ".producer") != "exactly_one" ||
        text(producer, "commit_law", path + ".producer") != "atomic_tagged_result" ||
        text(producer, "obligation_id", path + ".producer") !=
            "operation:" + std::to_string(operation) + ":outcome")
        throw Error(path + ".producer", "invalid producer disposition authority");
    const auto success_type = type(required(producer, "success_type", path + ".producer"),
                                   path + ".producer.success_type");
    provenance(required(producer, "provenance", path + ".producer"), path + ".producer.provenance");

    std::set<std::string> failures, faults;
    const auto& raw_failures = array(required(producer, "failure_types", path + ".producer"),
                                     path + ".producer.failure_types");
    const auto& raw_faults = array(required(producer, "fault_types", path + ".producer"),
                                   path + ".producer.fault_types");
    if (raw_failures.empty() || raw_failures.size() > 64 || raw_faults.size() > 64)
        throw Error(path + ".producer", "bounded closed disposition sets are required");
    for (std::size_t i = 0; i < raw_failures.size(); ++i) {
        const auto value = type(raw_failures[i], path + ".producer.failure_types[" + std::to_string(i) + "]");
        if (!failures.insert(value).second) throw Error(path + ".producer.failure_types", "duplicate failure type");
    }
    for (std::size_t i = 0; i < raw_faults.size(); ++i) {
        const auto value = type(raw_faults[i], path + ".producer.fault_types[" + std::to_string(i) + "]");
        if (!faults.insert(value).second) throw Error(path + ".producer.fault_types", "duplicate fault type");
    }
    if (success_type != "Text" || failures != std::set<std::string>{"TextFailure"})
        throw Error(path + ".producer", "bounded TextOutcome carrier types changed");

    std::map<Integer, std::tuple<std::string, std::string, std::string>> responses;
    const auto& response_values = array(required(root, "response_functions", path), path + ".response_functions");
    if (response_values.empty() || response_values.size() > 64)
        throw Error(path + ".response_functions", "bounded response set is required");
    for (std::size_t i = 0; i < response_values.size(); ++i) {
        const auto owner_path = path + ".response_functions[" + std::to_string(i) + "]";
        const auto& response = object(response_values[i], owner_path);
        const auto id = number(response, "function_symbol_id", owner_path);
        const auto incoming = type(required(response, "incoming_failure_type", owner_path), owner_path + ".incoming_failure_type");
        const auto outgoing = type(required(response, "outgoing_type", owner_path), owner_path + ".outgoing_type");
        const auto response_class = text(response, "response_class", owner_path);
        if (id < 0 || !failures.count(incoming) ||
            text(response, "input_projection", owner_path) != "immutable_failure_envelope" ||
            (response_class != "recover" && response_class != "transform") ||
            !responses.emplace(id, std::tuple{incoming, response_class, outgoing}).second)
            throw Error(owner_path, "invalid or duplicate response function");
        if (response_class == "recover" && outgoing != success_type)
            throw Error(owner_path, "recovery result differs from producer success type");
        if (text(response, "obligation_transition", owner_path) !=
            (response_class == "recover" ? "close_original" : "linked_successor"))
            throw Error(owner_path, "response transition disagrees with its declared class");
        provenance(required(response, "provenance", owner_path), owner_path + ".provenance");
    }

    const auto& consumer = object(required(root, "consumer", path), path + ".consumer");
    if (number(consumer, "declaration_id", path + ".consumer") < 0 ||
        number(consumer, "instance_structural_id", path + ".consumer") < 0 ||
        text(consumer, "semantic_id", path + ".consumer").empty() ||
        text(consumer, "instance_node_id", path + ".consumer").empty() ||
        !boolean(required(consumer, "closed_set", path + ".consumer"), path + ".consumer.closed_set"))
        throw Error(path + ".consumer", "invalid closed consumer identity");
    provenance(required(consumer, "provenance", path + ".consumer"), path + ".consumer.provenance");
    std::set<std::string> accepted;
    for (const auto& value : array(required(consumer, "accepted_failure_types", path + ".consumer"), path))
        accepted.insert(type(value, path + ".consumer.accepted_failure_types[]"));
    if (accepted != failures) throw Error(path + ".consumer", "consumer closed set differs from producer failure set");

    std::set<Integer> route_ids, covered_responses;
    const auto& routes = array(required(consumer, "routes", path + ".consumer"), path + ".consumer.routes");
    for (std::size_t i = 0; i < routes.size(); ++i) {
        const auto owner_path = path + ".consumer.routes[" + std::to_string(i) + "]";
        const auto& route = object(routes[i], owner_path);
        const auto route_id = number(route, "route_id", owner_path);
        const auto response_id = number(route, "function_symbol_id", owner_path);
        const auto failure = text(route, "failure_type", owner_path);
        const auto found = responses.find(response_id);
        if (route_id < 0 || !route_ids.insert(route_id).second || found == responses.end() ||
            !covered_responses.insert(response_id).second || failure != std::get<0>(found->second))
            throw Error(owner_path, "consumer route disagrees with response authority");
    }
    if (covered_responses.size() != responses.size())
        throw Error(path + ".consumer.routes", "response function lacks an authorized route");
    const auto selection = text(consumer, "selection_requirement", path + ".consumer");
    if (selection != (routes.size() == 1 ? "fixed_single_route_gate_3" : "explicit_policy_gate_3"))
        throw Error(path + ".consumer.selection_requirement", "route selection requirement is inconsistent");

    const auto& junction = object(required(root, "junction", path), path + ".junction");
    const auto success_wire = text(junction, "success_wire_id", path + ".junction");
    const auto failure_wire = text(junction, "failure_wire_id", path + ".junction");
    const auto rejoin_wire = text(junction, "rejoin_wire_id", path + ".junction");
    if (text(junction, "pairing", path + ".junction") != "same_producer_attempt_exclusive" ||
        text(junction, "success_type", path + ".junction") != success_type ||
        text(junction, "rejoin_type", path + ".junction") != success_type ||
        text(junction, "producer_node_id", path + ".junction") != text(producer, "graph_node_id", path + ".producer") ||
        text(junction, "consumer_node_id", path + ".junction") != text(consumer, "instance_node_id", path + ".consumer") ||
        success_wire.empty() || failure_wire.empty() || rejoin_wire.empty() ||
        success_wire == failure_wire || success_wire == rejoin_wire || failure_wire == rejoin_wire ||
        number(junction, "producer_function_symbol_id", path + ".junction") != function ||
        number(junction, "producer_operation_id", path + ".junction") != operation)
        throw Error(path + ".junction", "junction does not preserve the exact producer attempt and type");
    provenance(required(junction, "provenance", path + ".junction"), path + ".junction.provenance");

    const auto& containment = object(required(root, "containment", path), path + ".containment");
    const auto containment_wire = text(containment, "wire_id", path + ".containment");
    if (text(containment, "node_id", path + ".containment").empty() || containment_wire.empty() ||
        containment_wire == success_wire || containment_wire == failure_wire || containment_wire == rejoin_wire ||
        text(containment, "scope_kind", path + ".containment") != "activation" ||
        text(containment, "action", path + ".containment") != "halt_and_quarantine" ||
        text(containment, "execution", path + ".containment") != "unsupported")
        throw Error(path + ".containment", "fault containment meaning changed");
    provenance(required(containment, "provenance", path + ".containment"), path + ".containment.provenance");
    std::set<std::string> contained;
    for (const auto& value : array(required(containment, "accepted_fault_types", path + ".containment"), path))
        contained.insert(type(value, path + ".containment.accepted_fault_types[]"));
    if (contained != faults) throw Error(path + ".containment", "containment set differs from producer fault set");

    const auto& bridge = object(required(root, "bridge", path), path + ".bridge");
    if (text(bridge, "format", path + ".bridge") != "lyraform.source_disposition_bridge" ||
        number(bridge, "version", path + ".bridge") != 1 ||
        text(bridge, "status", path + ".bridge") != "declarative" ||
        text(bridge, "execution", path + ".bridge") != "unsupported" ||
        text(bridge, "module_identity", path + ".bridge") != module_identity ||
        text(bridge, "module_revision", path + ".bridge") != module_revision ||
        text(bridge, "policy_selection", path + ".bridge") != "not_materialized_gate_3")
        throw Error(path + ".bridge", "source-to-bridge authority mapping is inconsistent");
    const auto& mapping = object(required(bridge, "producer_mapping", path + ".bridge"), path + ".bridge.producer_mapping");
    if (number(mapping, "operation_id", path) != operation || number(mapping, "disposition_id", path) != disposition ||
        number(mapping, "function_symbol_id", path) != function || number(mapping, "owner_symbol_id", path) != owner)
        throw Error(path + ".bridge.producer_mapping", "bridge producer identity was forged");
    if (text(mapping, "graph_node_id", path + ".bridge.producer_mapping") !=
        text(producer, "graph_node_id", path + ".producer"))
        throw Error(path + ".bridge.producer_mapping", "bridge producer endpoint was forged");

    const std::map<std::string, std::string> expected_wires{
        {"success_input", success_wire}, {"failure_input", failure_wire},
        {"fault_containment", containment_wire}, {"typed_rejoin", rejoin_wire}};
    std::map<std::string, std::string> mapped_wires;
    const auto& wire_mapping = array(required(bridge, "wire_mapping", path + ".bridge"),
                                     path + ".bridge.wire_mapping");
    for (std::size_t i = 0; i < wire_mapping.size(); ++i) {
        const auto owner_path = path + ".bridge.wire_mapping[" + std::to_string(i) + "]";
        const auto& item = object(wire_mapping[i], owner_path);
        if (!mapped_wires.emplace(text(item, "semantic", owner_path),
                                  text(item, "graph_wire_id", owner_path)).second)
            throw Error(owner_path, "duplicate semantic wire mapping");
    }
    if (mapped_wires != expected_wires)
        throw Error(path + ".bridge.wire_mapping", "bridge wire identity mapping is inconsistent");

    const auto& claims = object(required(root, "execution_claims", path), path + ".execution_claims");
    for (const auto* key : {"policy_selected", "graph_ir_ready", "llvm_ready", "tinyvm_ready", "runtime_route_ready"})
        if (boolean(required(claims, key, path + ".execution_claims"), path + ".execution_claims." + key))
            throw Error(path + ".execution_claims", "Gate 2 artifact makes an executable claim");
    for (const auto& origin : observed_origins)
        if (!allowed_origins.count(origin))
            throw Error(path + ".module.source_origins", "provenance names a foreign source origin");
}

} // namespace flowcontracts
