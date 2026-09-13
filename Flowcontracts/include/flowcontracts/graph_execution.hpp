#pragma once

#include <flowcontracts/source_graph.hpp>
#include <algorithm>
#include <deque>

namespace flowcontracts {

// The bounded graph contract permits a static FIFO schedule. Fan-out references
// the producer activation's value; a linear stream pipeline carries each
// receiver result into the next activation without invoking a node twice.
inline json::Value graph_schedule(const json::Value& graph_value) {
    using namespace json;
    const auto graph = source_graph(graph_value);
    if (!graph.executable) throw Error("$.source_graph", "source graph execution is not admitted");
    const auto schedule_policy = optional(object(graph_value), "schedule_policy")
        ? string(*optional(object(graph_value), "schedule_policy"), "$.schedule_policy") : std::string{"serial"};
    std::vector<const json::Value*> stream_providers;
    for (const auto& provider : graph.providers)
        if (string(required(object(provider, "$.providers[]"), "activation", "$.providers[]"), "$.providers[].activation") == "finite_stream_once")
            stream_providers.push_back(&provider);
    if (!stream_providers.empty()) {
        if (schedule_policy == "parallel_independent_v1")
            throw Error("$.graph_schedule", "parallel scheduling is not admitted for finite streams");
        if (stream_providers.size() != 1 || graph.providers.size() != 1)
            throw Error("$.graph_schedule", "finite stream template requires exactly one stream root");
        const auto& provider = object(*stream_providers.front(), "$.providers[0]");
        const auto root = string(required(provider, "node_id", "$.providers[0]"), "$.providers[0].node_id");
        const auto count_callable = string(required(provider, "count_callable", "$.providers[0]"), "$.providers[0].count_callable");
        const auto item_callable = string(required(provider, "item_callable", "$.providers[0]"), "$.providers[0].item_callable");
        const auto count_symbol = integer(required(provider, "count_function_symbol_id", "$.providers[0]"), "$.providers[0].count_function_symbol_id");
        const auto item_symbol = integer(required(provider, "function_symbol_id", "$.providers[0]"), "$.providers[0].function_symbol_id");
        const auto max_items = integer(required(provider, "max_items", "$.providers[0]"), "$.providers[0].max_items");
        const auto item_type = string(required(provider, "output_type", "$.providers[0]"), "$.providers[0].output_type");
        bool aggregate_item = false;
        if (const auto* layouts = optional(object(graph_value), "aggregate_abi_layouts"))
            for (const auto& value : array(*layouts, "$.aggregate_abi_layouts"))
                if (string(required(object(value, "$.aggregate_abi_layouts[]"), "name", "$.aggregate_abi_layouts[]"), "$.aggregate_abi_layouts[].name") == item_type)
                    aggregate_item = true;
        std::map<std::string, std::vector<const SourceGraphWire*>> outgoing;
        for (const auto& wire : graph.wires) outgoing[wire.from.node].push_back(&wire);
        const auto found = outgoing.find(root);
        if (found == outgoing.end() || found->second.empty())
            throw Error("$.graph_schedule", "finite stream root requires at least one output delivery");
        Array deliveries, steps;
        Integer activation = 0;
        steps.push_back(Object{
            {"activation_id", activation}, {"node_id", root}, {"kind", "stream_root"},
            {"input_activation_id", Integer{-1}}, {"input_signal_id", Integer{0}},
            {"output_signal_id", Integer{1}}, {"delivery_id", Integer{0}}, {"wire_id", ""},
            {"source_node", ""}, {"source_port", ""}, {"input_port", ""},
            {"output_port", "out"}, {"output_connected", true}});
        const auto receiver_node = [&](const SourceGraphWire* wire) {
            if (wire->from.port != "out") throw Error("$.graph_schedule", "finite stream root exposes only out");
            const auto receiver = std::find_if(graph.nodes.begin(), graph.nodes.end(), [&](const auto& node) { return node.id == wire->to.node; });
            if (receiver == graph.nodes.end() || receiver->implementation_kind != "source_function" || receiver->role != "node" || receiver->persistent || wire->to.port != "in")
                throw Error("$.graph_schedule", "finite stream deliveries must target source receivers");
            return &*receiver;
        };
        bool pipeline = false;
        for (const auto* wire : found->second) {
            receiver_node(wire);
            const auto receiver_outgoing = outgoing.find(wire->to.node);
            if (receiver_outgoing != outgoing.end() && !receiver_outgoing->second.empty()) pipeline = true;
        }
        if (pipeline) {
            if (found->second.size() != 1)
                throw Error("$.graph_schedule", "finite stream pipeline requires exactly one root delivery");
            std::set<std::string> visited;
            const SourceGraphWire* wire = found->second.front();
            Integer input_activation = 0, input_signal = 1;
            std::string input_type = item_type;
            while (wire) {
                const auto* receiver = receiver_node(wire);
                if (!visited.insert(receiver->id).second)
                    throw Error("$.graph_schedule", "finite stream pipeline must be acyclic");
                const auto receiver_info = std::find_if(graph.receivers.begin(), graph.receivers.end(), [&](const auto& value) {
                    return string(required(object(value, "$.graph_schedule.receivers[]"), "node_id", "$.graph_schedule.receivers[]"), "$.graph_schedule.receivers[].node_id") == receiver->id;
                });
                if (receiver_info == graph.receivers.end())
                    throw Error("$.graph_schedule", "finite stream pipeline receiver metadata is absent");
                const auto& info = object(*receiver_info, "$.graph_schedule.receivers[]");
                const auto receiver_input = string(required(info, "input_type", "$.graph_schedule.receivers[]"), "$.graph_schedule.receivers[].input_type");
                const auto receiver_output = string(required(info, "output_type", "$.graph_schedule.receivers[]"), "$.graph_schedule.receivers[].output_type");
                if (receiver_input != input_type)
                    throw Error("$.graph_schedule", "finite stream pipeline delivery carrier mismatch");
                const auto receiver_outgoing = outgoing.find(receiver->id);
                const bool connected = receiver_outgoing != outgoing.end() && !receiver_outgoing->second.empty();
                if (connected && receiver_outgoing->second.size() != 1)
                    throw Error("$.graph_schedule", "finite stream pipeline cannot fan out or merge");
                ++activation;
                steps.push_back(Object{
                    {"activation_id", activation}, {"node_id", receiver->id}, {"kind", "stream_receiver"},
                    {"input_activation_id", input_activation}, {"input_signal_id", input_signal},
                    {"output_signal_id", activation + 1}, {"delivery_id", activation},
                    {"wire_id", wire->id}, {"source_node", wire->from.node}, {"source_port", wire->from.port},
                    {"input_port", wire->to.port}, {"output_port", "out"}, {"output_connected", connected},
                    {"stream_index", "$index"}});
                deliveries.push_back(Object{{"node_id", receiver->id}, {"wire_id", wire->id},
                    {"input_port", wire->to.port}, {"output_port", "out"}, {"receiver_output_type", receiver_output},
                    {"input_activation_id", input_activation}, {"input_signal_id", input_signal}});
                input_activation = activation;
                input_signal = activation + 1;
                input_type = receiver_output;
                wire = connected ? receiver_outgoing->second.front() : nullptr;
            }
            return Object{{"format", "flowcore.graph_schedule"}, {"version", Integer{5}},
                {"policy", "fifo_per_root_source_order_v1"}, {"activation_contract", "fresh_single_input_v1"},
                {"stream_contract", aggregate_item ? "finite_aggregate_stream_pipeline_v1" : "finite_scalar_stream_pipeline_v1"},
                {"streams", Array{Object{{"root_node", root}, {"count_callable", count_callable},
                    {"count_function_symbol_id", count_symbol}, {"item_callable", item_callable},
                    {"item_function_symbol_id", item_symbol}, {"max_items", max_items},
                    {"item_output_type", item_type}, {"deliveries", deliveries}}}}, {"steps", steps}};
        }
        for (const auto* wire : found->second) {
            const auto* receiver = receiver_node(wire);
            ++activation;
            steps.push_back(Object{
                {"activation_id", activation}, {"node_id", receiver->id}, {"kind", "stream_receiver"},
                {"input_activation_id", Integer{0}}, {"input_signal_id", Integer{1}},
                {"output_signal_id", activation + 1}, {"delivery_id", activation},
                {"wire_id", wire->id}, {"source_node", wire->from.node}, {"source_port", wire->from.port},
                {"input_port", wire->to.port}, {"output_port", "out"}, {"output_connected", false},
                {"stream_index", "$index"}});
            deliveries.push_back(Object{{"node_id", receiver->id}, {"wire_id", wire->id},
                {"input_port", wire->to.port}, {"output_port", "out"}, {"receiver_output_type", item_type}});
        }
        return Object{{"format", "flowcore.graph_schedule"}, {"version", Integer{2}},
            {"policy", "fifo_per_root_source_order_v1"}, {"activation_contract", "fresh_single_input_v1"},
            {"stream_contract", aggregate_item ? "finite_aggregate_stream_v1" : "finite_scalar_stream_v1"},
            {"streams", Array{Object{{"root_node", root}, {"count_callable", count_callable},
                {"count_function_symbol_id", count_symbol}, {"item_callable", item_callable},
                {"item_function_symbol_id", item_symbol}, {"max_items", max_items},
                {"item_output_type", item_type}, {"deliveries", deliveries}}}}, {"steps", steps}};
    }
    bool persistent = false;
    std::set<std::string> persistent_state_types;
    for (const auto& receiver : graph.receivers)
        if (optional(object(receiver, "$.receivers[]"), "state_contract")) persistent = true;
    if (persistent) {
        if (schedule_policy == "parallel_independent_v1")
            throw Error("$.graph_schedule", "parallel scheduling cannot share persistent receiver state");
        if (graph.providers.size() != 1 || graph.receivers.empty())
            throw Error("$.graph_schedule", "persistent template requires one startup root and at least one receiver");
        const auto& root_provider = object(graph.providers.front(), "$.providers[0]");
        if (string(required(root_provider, "activation", "$.providers[0]"), "$.providers[0].activation") != "startup_once")
            throw Error("$.graph_schedule", "persistent template requires a startup root");
        const auto root = string(required(root_provider, "node_id", "$.providers[0]"), "$.providers[0].node_id");
        for (const auto& receiver : graph.receivers)
            if (!optional(object(receiver, "$.receivers[]"), "state_contract"))
                throw Error("$.graph_schedule", "persistent template cannot mix persistent and fresh receivers");
        for (const auto& receiver : graph.receivers)
            persistent_state_types.insert(string(required(object(receiver, "$.receivers[]"), "state_type", "$.receivers[].state_type"), "$.receivers[].state_type"));
        if (persistent_state_types.size() != 1) throw Error("$.graph_schedule", "persistent template requires one shared state carrier");
        for (const auto& wire : graph.wires) {
            if (wire.from.node != root || wire.from.port != "out" || wire.to.port != "in")
                throw Error("$.graph_schedule", "persistent template admits only direct startup-to-receiver deliveries");
            const auto receiver = std::find_if(graph.receivers.begin(), graph.receivers.end(), [&](const auto& item) {
                return string(required(object(item, "$.receivers[]"), "node_id", "$.receivers[]"), "$.receivers[].node_id") == wire.to.node;
            });
            if (receiver == graph.receivers.end()) throw Error("$.graph_schedule", "persistent delivery target is not a receiver");
        }
    }
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
    if (schedule_policy == "parallel_independent_v1") {
        std::map<Integer, Array> waves;
        std::map<Integer, Integer> levels;
        for (const auto& step : steps) {
            const auto item = object(step, "$.graph_schedule.steps[]");
            const auto id = integer(required(item, "activation_id", "$.graph_schedule.steps[]"), "$.graph_schedule.steps[].activation_id");
            const auto input = integer(required(item, "input_activation_id", "$.graph_schedule.steps[]"), "$.graph_schedule.steps[].input_activation_id");
            levels[id] = input < 0 ? 0 : levels.at(input) + 1;
            waves[levels[id]].emplace_back(id);
        }
        Array parallel_waves;
        for (const auto& [level, activations] : waves)
            parallel_waves.emplace_back(Object{{"activation_ids", activations}, {"level", level}, {"status", "independent"}});
        return Object{{"format", "flowcore.graph_schedule"}, {"version", Integer{4}},
            {"policy", "parallel_independent_v1"}, {"activation_contract", "fresh_single_input_v1"},
            {"parallel_contract", "dependency_waves_v1"}, {"parallel_waves", parallel_waves}, {"steps", steps}};
    }
    if (persistent) {
        Array state_steps;
        for (const auto& step : steps) {
            auto item = object(step, "$.graph_schedule.steps[]");
            if (string(required(item, "kind", "$.graph_schedule.steps[]"), "$.graph_schedule.steps[].kind") == "receiver") {
                const auto node = string(required(item, "node_id", "$.graph_schedule.steps[]"), "$.graph_schedule.steps[].node_id");
                for (const auto& receiver : graph.receivers) {
                    const auto& value = object(receiver, "$.receivers[]");
                    if (string(required(value, "node_id", "$.receivers[]"), "$.receivers[].node_id") == node) {
                        item["kind"] = "persistent_receiver";
                        item.emplace("state_contract", string(required(value, "state_contract", "$.receivers[]"), "$.receivers[].state_contract"));
                        item.emplace("state_type", string(required(value, "state_type", "$.receivers[]"), "$.receivers[].state_type"));
                        item.emplace("state_initial_value", string(required(value, "state_initial_value", "$.receivers[]"), "$.receivers[].state_initial_value"));
                        item.emplace("state_parameter_symbol_id", integer(required(value, "state_parameter_symbol_id", "$.receivers[]"), "$.receivers[].state_parameter_symbol_id"));
                    }
                }
            }
            state_steps.emplace_back(std::move(item));
        }
        const auto state_contract = *persistent_state_types.begin() == "c_long" ? "persistent_scalar_v1" : "persistent_aggregate_v1";
        return Object{{"format", "flowcore.graph_schedule"}, {"version", Integer{3}},
            {"policy", "fifo_per_root_source_order_v1"}, {"activation_contract", "persistent_single_input_v1"},
            {"state_contract", state_contract}, {"state_type", *persistent_state_types.begin()}, {"steps", state_steps}};
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
