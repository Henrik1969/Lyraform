#pragma once

#include <flowcontracts/source_graph.hpp>
#include <flowcontracts/disposition_diagnostic.hpp>
#include <flowcontracts/failure_flow_execution.hpp>
#include <flowcontracts/fault_containment.hpp>

namespace flowcontracts {

// Declarative seam only. These inputs prove consistency, not source admission.
// The independently supplied graph-side node bindings are a semantic producer's
// responsibility. No parser, backend, or runtime synthesizes them here.
inline json::Value disposition_route_semantics(const json::Value& raw) {
    using namespace json;
    const auto& root = object(raw, "$.plan");
    const auto format = string(required(root, "format", "$.plan"), "$.plan.format");
    if (format == "lyraform.failure_flow_plan") {
        const auto plan = read_failure_flow_plan(raw, "$.plan");
        if (serialize(raw) != serialize(failure_flow_plan_fact(plan)))
            throw Error("$.plan", "noncanonical failure plan fields");
        const auto refusal = failure_flow_plan_refusal(plan);
        if (!refusal.empty()) throw Error("$.plan", refusal);
        const auto& transition = plan.transitions.front();
        if (transition.response_class != "recover")
            throw Error("$.plan", "this projection admits only a declared recovery transition");
        return Object{
            {"lane", "failure"}, {"plan_id", Integer(plan.id)},
            {"route_id", "failure:plan:" + std::to_string(plan.id) + ":route:" + std::to_string(plan.wire.route)},
            {"plan_wire_id", Integer(plan.wire.id)},
            {"producer_operation_id", Integer(plan.producer.operation)},
            {"producer_disposition_id", Integer(plan.producer.id)},
            {"producer_function_id", Integer(plan.producer.function)},
            {"from_port", plan.wire.from_port}, {"to_port", plan.wire.to_port},
            {"payload_type", plan.producer.failure_type},
            {"completion", plan.producer.completion},
            {"commit_law", plan.producer.failure_commit_law},
            {"provenance", plan.producer.provenance},
            {"destination_kind", "response_function"},
            {"destination_id", Integer(transition.function)},
            {"scope_id", Integer(plan.consumer.scope)}, {"scope_kind", "consumer_declaration"},
            {"obligation_law", transition.obligation_transition},
            {"consumer", failure_consumer_fact(plan.consumer)},
            {"transition", failure_response_transition_fact(transition)},
            {"policy_selection", failure_policy_selection_fact(plan.selection)},
            {"correlation_law", "preserve_signal_and_delivery_fresh_activation_attempt_unique_obligation"}};
    }
    if (format == "lyraform.fault_containment_plan") {
        const auto plan = read_fault_containment_plan(raw, "$.plan");
        if (serialize(raw) != serialize(fault_containment_plan_fact(plan)))
            throw Error("$.plan", "noncanonical fault plan fields");
        const auto refusal = fault_containment_plan_refusal(plan);
        if (!refusal.empty()) throw Error("$.plan", refusal);
        const auto serialized = fault_containment_plan_fact(plan);
        const auto& fact = object(serialized, "$.plan");
        return Object{
            {"lane", "fault"}, {"plan_id", Integer(plan.id)},
            {"route_id", "fault:plan:" + std::to_string(plan.id) + ":wire:" + std::to_string(plan.wire.id)},
            {"plan_wire_id", Integer(plan.wire.id)},
            {"producer_operation_id", Integer(plan.producer.operation)},
            {"producer_disposition_id", Integer(plan.producer.id)},
            {"producer_function_id", Integer(plan.producer.function)},
            {"from_port", plan.wire.from_port}, {"to_port", plan.wire.to_port},
            {"payload_type", plan.producer.fault_type},
            {"completion", plan.producer.completion},
            {"commit_law", plan.producer.fault_commit_law},
            {"provenance", plan.producer.provenance},
            {"destination_kind", "containment_authority"},
            {"destination_id", Integer(plan.authority.id)},
            {"scope_id", Integer(plan.authority.scope)}, {"scope_kind", plan.authority.scope_kind},
            {"obligation_law", "contained_no_continuation"},
            {"authority", required(fact, "authority", "$.plan")},
            {"policy_selection", required(fact, "policy_selection", "$.plan")},
            {"correlation_law", "preserve_signal_and_delivery_fresh_activation_attempt_unique_obligation"}};
    }
    throw Error("$.plan.format", "unsupported disposition plan");
}

inline void disposition_projection_equal(const json::Value& actual,
                                         const json::Value& expected,
                                         const std::string& path) {
    if (json::serialize(actual) != json::serialize(expected))
        throw json::Error(path, "disposition projection differs from independently validated authority");
}

// The graph side and plan side are independently parsed, then compared. Wire
// strings are never converted into numeric route, operation, or obligation IDs.
inline json::Value make_disposition_route_projection(const json::Value& graph_side,
                                                      const json::Value& plan_side) {
    using namespace json;
    const auto& side = object(graph_side, "$.graph_side");
    if (string(required(side, "format", "$.graph_side"), "$.graph_side.format") !=
            "lyraform.disposition_graph_projection" ||
        integer(required(side, "version", "$.graph_side"), "$.graph_side.version") != 1 ||
        string(required(side, "status", "$.graph_side"), "$.graph_side.status") != "declarative")
        throw Error("$.graph_side", "unsupported or executable disposition graph projection");
    const auto& graph_id = string(required(side, "graph_id", "$.graph_side"), "$.graph_side.graph_id");
    if (graph_id.empty()) throw Error("$.graph_side.graph_id", "missing graph identity");
    const auto& origin = required(side, "provenance", "$.graph_side");
    const auto& source = object(origin, "$.graph_side.provenance");
    if (string(required(source, "source", "$.graph_side.provenance"), "$.source").empty() ||
        integer(required(source, "line", "$.graph_side.provenance"), "$.line") < 1 ||
        integer(required(source, "column", "$.graph_side.provenance"), "$.column") < 1)
        throw Error("$.graph_side.provenance", "original source location is required");
    const auto& raw_graph = required(side, "source_graph", "$.graph_side");
    const auto graph = source_graph(raw_graph, "$.graph_side.source_graph");
    if (graph.executable) throw Error("$.graph_side.source_graph", "disposition graph execution is not admitted");
    const auto& plans = array(plan_side, "$.plans");
    const auto& lanes = array(required(side, "lanes", "$.graph_side"), "$.graph_side.lanes");
    if (plans.empty() || plans.size() > 2 || lanes.size() != plans.size())
        throw DispositionProjectionError("$.graph_side.lanes", "one or two exact declarative lanes required",
            lanes.size() > plans.size() ? DispositionDiagnosticKind::ambiguous_route : DispositionDiagnosticKind::missing_route);
    const auto& bindings = array(required(side, "node_bindings", "$.graph_side"), "$.graph_side.node_bindings");
    std::map<std::string, Value> node_bindings;
    for (const auto& raw : bindings) {
        const auto& binding = object(raw, "$.graph_side.node_bindings[]");
        const auto& node = string(required(binding, "node_id", "$.binding"), "$.binding.node_id");
        bool found = false;
        for (const auto& candidate : graph.nodes) if (candidate.id == node) found = true;
        if (!found || !node_bindings.emplace(node, raw).second)
            throw Error("$.graph_side.node_bindings", "foreign or duplicate node binding");
    }
    std::set<std::string> wires, routes, used_bindings, kinds;
    std::set<Integer> dispositions;
    Array projected;
    for (std::size_t i = 0; i < plans.size(); ++i) {
        const auto path = "$.graph_side.lanes[" + std::to_string(i) + "]";
        const auto semantics = disposition_route_semantics(plans[i]);
        const auto& facts = object(semantics, path);
        const auto& lane = object(lanes[i], path);
        const auto& claimed = object(required(lane, "semantics", path), path + ".semantics");
        if (serialize(required(claimed, "payload_type", path)) != serialize(required(facts, "payload_type", path)))
            throw DispositionProjectionError(path, "destination type differs from producer", DispositionDiagnosticKind::wrong_type);
        if (string(required(facts, "lane", path), path) == "fault" &&
            string(required(claimed, "destination_kind", path), path) == "response_function")
            throw DispositionProjectionError(path, "fault cannot enter ordinary recovery", DispositionDiagnosticKind::fault_to_recovery);
        disposition_projection_equal(required(lane, "semantics", path), semantics, path + ".semantics");
        const auto& wire_id = string(required(lane, "graph_wire_id", path), path + ".graph_wire_id");
        if (!wires.insert(wire_id).second ||
            !routes.insert(string(required(facts, "route_id", path), path)).second ||
            !kinds.insert(string(required(facts, "lane", path), path)).second ||
            !dispositions.insert(integer(required(facts, "producer_disposition_id", path), path)).second)
            throw DispositionProjectionError(path, "duplicate wire, route, lane, or disposition identity", DispositionDiagnosticKind::ambiguous_route);
        const SourceGraphWire* wire = nullptr;
        for (const auto& candidate : graph.wires) if (candidate.id == wire_id) wire = &candidate;
        if (!wire) throw DispositionProjectionError(path, "dangling disposition graph wire", DispositionDiagnosticKind::missing_route);
        if (wire->from.port != string(required(facts, "from_port", path), path) ||
            wire->to.port != string(required(facts, "to_port", path), path))
            throw Error(path, "wrong disposition graph port or lane");
        const auto& graph_root = object(raw_graph, "$.graph_side.source_graph");
        const auto& syntax = object(required(graph_root, "syntax", path), path);
        for (const auto& raw_wire : array(required(syntax, "wires", path), path)) {
            const auto& item = object(raw_wire, path);
            if (string(required(item, "wire_id", path), path) != wire_id) continue;
            const auto& endpoint = object(required(item, "from", path), path);
            const auto& location = object(required(endpoint, "provenance", path), path);
            const auto span = string(required(location, "source", path), path) + ":" +
                std::to_string(integer(required(location, "line", path), path)) + ":" +
                std::to_string(integer(required(location, "column", path), path));
            if (span != string(required(facts, "provenance", path), path))
                throw Error(path, "producer origin differs from original graph endpoint source span");
        }
        const Object producer{{"node_id", wire->from.node}, {"kind", "producer"},
            {"operation_id", required(facts, "producer_operation_id", path)},
            {"disposition_id", required(facts, "producer_disposition_id", path)},
            {"function_id", required(facts, "producer_function_id", path)},
            {"payload_type", required(facts, "payload_type", path)},
            {"provenance", required(facts, "provenance", path)}};
        const Object destination{{"node_id", wire->to.node},
            {"kind", required(facts, "destination_kind", path)},
            {"identity", required(facts, "destination_id", path)},
            {"scope_id", required(facts, "scope_id", path)},
            {"scope_kind", required(facts, "scope_kind", path)},
            {"payload_type", required(facts, "payload_type", path)}};
        for (const auto& [node, expected] : std::vector<std::pair<std::string, Value>>{
                 {wire->from.node, producer}, {wire->to.node, destination}}) {
            const auto found = node_bindings.find(node);
            if (found == node_bindings.end()) throw Error(path, "missing semantic graph node binding");
            disposition_projection_equal(found->second, expected, path + ".node_binding");
            used_bindings.insert(node);
        }
        Object projection{{"graph_wire_id", wire_id}, {"from_node", wire->from.node},
                          {"to_node", wire->to.node}, {"semantics", semantics}};
        disposition_projection_equal(lanes[i], projection, path);
        projected.emplace_back(std::move(projection));
    }
    if (used_bindings.size() != node_bindings.size())
        throw Error("$.graph_side.node_bindings", "unreferenced disposition binding");
    for (const auto& wire : graph.wires)
        if (!wires.count(wire.id) && (wire.from.port != "out" || wire.to.port != "in"))
            throw Error("$.graph_side.source_graph", "unaccounted unsuccessful graph wire");
    return Object{{"format", "lyraform.disposition_route_projection"}, {"version", Integer{1}},
                  {"status", "declarative"}, {"graph_id", graph_id}, {"provenance", origin},
                  {"schedule", "serial"}, {"routes", std::move(projected)}};
}

inline void validate_disposition_route_projection(const json::Value& projection,
                                                  const json::Value& graph_side,
                                                  const json::Value& plan_side) {
    disposition_projection_equal(projection,
        make_disposition_route_projection(graph_side, plan_side), "$.projection");
}

inline void validate_disposition_route_bundle(const json::Value& raw) {
    using namespace json;
    const auto& root = object(raw, "$");
    if (string(required(root, "format", "$"), "$.format") != "lyraform.disposition_route_bundle" ||
        integer(required(root, "version", "$"), "$.version") != 1 ||
        string(required(root, "status", "$"), "$.status") != "declarative")
        throw Error("$", "unsupported or executable disposition route bundle");
    validate_disposition_route_projection(required(root, "projection", "$"),
        required(root, "graph_side", "$"), required(root, "plans", "$"));
}

// Diagnostic origins from a supplied artifact are visibly unverified. This
// projection does not claim that a parser resolved imports or admitted source.
inline json::Value disposition_bundle_refusal(const json::Value& raw,
                                              const json::Error& error) {
    DispositionDiagnosticContext c;
    c.source = "<artifact>";
    c.boundary = "disposition_route_bundle";
    c.route = error.path();
    c.cause = error.reason();
    try {
        const auto& root = json::object(raw, "$");
        const auto& side = json::object(json::required(root, "graph_side", "$"), "$.graph_side");
        const auto& p = json::object(json::required(side, "provenance", "$"), "$.provenance");
        c.source = json::string(json::required(p, "source", "$"), "$.source");
        c.line = json::integer(json::required(p, "line", "$"), "$.line");
        c.column = json::integer(json::required(p, "column", "$"), "$.column");
        const auto& lanes = json::array(json::required(side, "lanes", "$"), "$.lanes");
        std::size_t index = 0;
        const auto offset = error.path().find("lanes[");
        if (offset != std::string::npos) {
            const auto begin = error.path().data() + offset + 6;
            const auto end = error.path().data() + error.path().size();
            const auto parsed = std::from_chars(begin, end, index);
            if (parsed.ec != std::errc{}) index = lanes.size();
        } else index = lanes.size();
        if (index < lanes.size()) {
            const auto& lane = json::object(lanes[index], "$.lane");
            const auto& facts = json::object(json::required(lane, "semantics", "$"), "$.semantics");
            c.operation = json::integer(json::required(facts, "producer_operation_id", "$"), "$");
            c.disposition = json::integer(json::required(facts, "producer_disposition_id", "$"), "$");
            c.lane = json::string(json::required(facts, "lane", "$"), "$");
            c.payload_type = json::string(json::required(facts, "payload_type", "$"), "$");
        }
    } catch (const json::Error&) { /* Absent facts stay explicitly unknown. */ }
    const auto* typed = dynamic_cast<const DispositionProjectionError*>(&error);
    return disposition_diagnostic(typed ? typed->kind : DispositionDiagnosticKind::forged_artifact, c);
}

} // namespace flowcontracts
