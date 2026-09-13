#pragma once

#include <flowcontracts/source_graph.hpp>
#include <deque>

namespace flowcontracts {

// The bounded one-output contract permits a static FIFO schedule. Fan-out
// references the producer activation's value; it never invokes that node again.
inline json::Value graph_schedule(const json::Value& graph_value) {
    using namespace json;
    const auto graph = source_graph(graph_value);
    if (!graph.executable) throw Error("$.source_graph", "source graph execution is not admitted");
    Array steps;
    struct Pending { std::string node; Integer from; const SourceGraphWire* wire; };
    std::map<std::string, std::vector<const SourceGraphWire*>> outgoing;
    for (const auto& wire : graph.wires) outgoing[wire.from.node].push_back(&wire);
    for (const auto& root : graph.nodes) {
        if (root.role != "producer") continue;
        std::deque<Pending> pending{{root.id, -1, nullptr}};
        while (!pending.empty()) {
            const auto current = pending.front(); pending.pop_front();
            if (steps.size() >= 65536) throw Error("$.graph_schedule", "native graph exceeds 65536 activation bound");
            const auto identity = static_cast<Integer>(steps.size());
            bool connected = false;
            if (const auto found = outgoing.find(current.node); found != outgoing.end()) {
                connected = !found->second.empty();
                for (const auto* wire : found->second)
                    pending.push_back({wire->to.node, identity, wire});
            }
            steps.push_back(Object{
                {"activation_id", identity}, {"node_id", current.node},
                {"kind", current.wire ? "receiver" : "startup"},
                {"input_activation_id", current.from},
                {"input_signal_id", current.from < 0 ? Integer{0} : current.from + 1},
                {"output_signal_id", identity + 1},
                {"delivery_id", current.wire ? identity + 1 : Integer{0}},
                {"wire_id", current.wire ? current.wire->id : ""},
                {"source_node", current.wire ? current.wire->from.node : ""},
                {"source_port", current.wire ? current.wire->from.port : ""},
                {"input_port", current.wire ? current.wire->to.port : ""},
                {"output_port", "out"}, {"output_connected", connected}});
        }
    }
    return Object{{"format", "flowcore.graph_schedule"}, {"version", Integer{1}},
        {"policy", "fifo_per_root_source_order_v1"},
        {"activation_contract", "fresh_single_input_v1"}, {"steps", steps}};
}

inline void validate_graph_schedule(const json::Object& artifact) {
    using namespace json;
    const auto& plan = object(required(artifact, "lowering_plan"), "$.lowering_plan");
    const auto* graph = optional(plan, "source_graph");
    const auto* schedule = optional(artifact, "graph_schedule");
    if (graph && source_graph(*graph).executable) {
        if (!schedule || serialize(*schedule) != serialize(graph_schedule(*graph)))
            throw Error("$.graph_schedule", "schedule differs from graph port, wire, signal or activation contract");
    } else if (schedule) throw Error("$.graph_schedule", "schedule has no executable source graph");
}

} // namespace flowcontracts
