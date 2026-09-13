#pragma once

#include <flowcontracts/json.hpp>
#include <flowcontracts/binding_evidence.hpp>
#include <flowcontracts/graph_provider_map.hpp>
#include <set>

namespace flowcontracts {

struct SourceGraphNode {
    std::string id, role, implementation_kind, implementation_name;
};
struct SourceGraphEndpoint { std::string node, port; };
struct SourceGraphWire { std::string id; SourceGraphEndpoint from, to; };
struct SourceGraph {
    bool executable = false;
    std::vector<SourceGraphNode> nodes;
    std::vector<SourceGraphWire> wires;
    json::Array policies, receivers, providers;
};

// This contract preserves analysis evidence. It grants no provider authority and
// cannot be executed merely by changing an outer artifact's status to ready.
inline SourceGraph source_graph(const json::Value& value, std::string path = "$") {
    using namespace json;
    const auto& root = object(value, path);
    auto str = [](const Object& object, const char* key, const std::string& path) {
        return string(required(object, key, path), path + "." + key);
    };
    auto nonempty = [&](const Object& object, const char* key, const std::string& path) {
        auto result = str(object, key, path);
        if (result.empty()) throw Error(path + "." + key, "graph identity must not be empty");
        return result;
    };
    auto provenance = [&](const Object& owner, const std::string& path) {
        const auto where = path + ".provenance";
        const auto& p = object(required(owner, "provenance", path), where);
        (void)nonempty(p, "source", where);
        for (const auto* key : {"line", "column"})
            if (integer(required(p, key, where), where + "." + key) < 1)
                throw Error(where + "." + key, "graph provenance must be positive");
    };
    const auto version = integer(required(root, "version", path), path + ".version");
    if (str(root, "format", path) != "flowcore.source_graph" || (version != 1 && version != 2))
        throw Error(path, "unsupported source graph contract");
    if (str(root, "status", path) != (version == 2 ? "ready" : "non_executable"))
        throw Error(path + ".status", "source graph execution is not admitted");
    const auto syntax_path = path + ".syntax";
    const auto& syntax = object(required(root, "syntax", path), syntax_path);
    if (str(syntax, "format", syntax_path) != "flowmini.graph_syntax" || integer(required(syntax, "version", syntax_path), syntax_path + ".version") != 1)
        throw Error(syntax_path, "unsupported graph syntax contract");
    SourceGraph result;
    result.executable = version == 2;
    std::map<std::string, SourceGraphNode> nodes;
    for (const auto& value : array(required(syntax, "nodes", syntax_path), syntax_path + ".nodes")) {
        const auto p = syntax_path + ".nodes[" + std::to_string(result.nodes.size()) + "]";
        const auto& item = object(value, p);
        SourceGraphNode node{nonempty(item, "node_id", p), str(item, "role", p), str(item, "implementation_kind", p), nonempty(item, "implementation_name", p)};
        if (node.role != "producer" && node.role != "node" && node.role != "sink") throw Error(p + ".role", "unknown graph node role");
        if (node.implementation_kind != "source_function" && node.implementation_kind != "provider_atom") throw Error(p, "unknown graph implementation kind");
        if (!nodes.emplace(node.id, node).second) throw Error(p, "duplicate graph node identity");
        provenance(item, p);
        result.nodes.push_back(std::move(node));
    }
    std::set<std::string> wires;
    for (const auto& value : array(required(syntax, "wires", syntax_path), syntax_path + ".wires")) {
        const auto p = syntax_path + ".wires[" + std::to_string(result.wires.size()) + "]";
        const auto& item = object(value, p);
        SourceGraphWire wire;
        wire.id = nonempty(item, "wire_id", p);
        if (!wires.insert(wire.id).second) throw Error(p, "duplicate graph wire identity");
        auto endpoint = [&](const char* key) {
            const auto where = p + "." + key;
            const auto& e = object(required(item, key, p), where);
            SourceGraphEndpoint endpoint{nonempty(e, "node_id", where), nonempty(e, "port_id", where)};
            if (!nodes.count(endpoint.node)) throw Error(where, "graph endpoint node is absent");
            provenance(e, where);
            return endpoint;
        };
        wire.from = endpoint("from"); wire.to = endpoint("to"); provenance(item, p);
        result.wires.push_back(std::move(wire));
    }
    result.policies = array(required(syntax, "policies", syntax_path), syntax_path + ".policies");
    std::set<std::pair<std::string, std::string>> policies;
    for (std::size_t i = 0; i < result.policies.size(); ++i) {
        const auto p = syntax_path + ".policies[" + std::to_string(i) + "]";
        const auto& item = object(result.policies[i], p);
        const auto node = nonempty(item, "node_id", p), key = nonempty(item, "key", p);
        if (!nodes.count(node) || !policies.emplace(node, key).second) throw Error(p, "unknown policy node or duplicate policy identity");
        if (nodes.at(node).implementation_kind == "source_function") throw Error(p, "source receiver policies are unsupported");
        const auto kind = str(item, "value_kind", p), text = str(item, "value_text", p);
        if (kind == "integer") {
            Integer number = 0;
            const auto parsed = std::from_chars(text.data(), text.data() + text.size(), number);
            if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size()) throw Error(p, "invalid integer graph policy");
        } else if (kind == "boolean") {
            if (text != "true" && text != "false") throw Error(p, "invalid boolean graph policy");
        } else if (kind != "string") throw Error(p, "unknown graph policy value kind");
        provenance(item, p);
    }
    result.receivers = array(required(root, "receivers", path), path + ".receivers");
    std::set<std::string> receivers;
    std::map<std::string, std::pair<std::string, std::string>> types;
    for (std::size_t i = 0; i < result.receivers.size(); ++i) {
        const auto p = path + ".receivers[" + std::to_string(i) + "]";
        const auto& item = object(result.receivers[i], p);
        const auto node = nonempty(item, "node_id", p);
        if (!nodes.count(node) || nodes.at(node).implementation_kind != "source_function" || nodes.at(node).role != "node" || !receivers.insert(node).second)
            throw Error(p, "receiver identity does not match one source node");
        for (const auto* key : {"function_symbol_id", "parameter_symbol_id"})
            if (integer(required(item, key, p), p + "." + key) < 0) throw Error(p, "invalid receiver function identity");
        if (str(item, "activation_contract", p) != "fresh_single_input_v1" || str(item, "input_port", p) != "in" || str(item, "output_port", p) != "out")
            throw Error(p, "unsupported receiver activation contract");
        types[node] = {nonempty(item, "input_type", p), nonempty(item, "output_type", p)};
        provenance(item, p);
    }
    std::map<std::string, std::string> provider_types;
    if (const auto* providers = optional(root, "providers")) {
        result.providers = array(*providers, path + ".providers");
        const auto selections = graph_provider_map(required(root, "provider_selection", path));
        for (std::size_t i = 0; i < result.providers.size(); ++i) {
            const auto p = path + ".providers[" + std::to_string(i) + "]";
            const auto& item = object(result.providers[i], p);
            const auto node = nonempty(item, "node_id", p);
            if (!nodes.count(node) || nodes.at(node).role != "producer" ||
                nodes.at(node).implementation_kind != "provider_atom" || provider_types.count(node))
                throw Error(p, "provider resolution does not match one producer node");
            const auto implementation = nonempty(item, "implementation", p);
            const GraphProviderSelection* selected = nullptr;
            for (const auto& selection : selections)
                if (selection.implementation == implementation) selected = &selection;
            if (!selected || implementation != nodes.at(node).implementation_name)
                throw Error(p, "provider resolution differs from explicit selection");
            const auto activation = str(item, "activation", p);
            const auto type = nonempty(item, "output_type", p);
            const auto& provider = object(required(item, "provider", p), p + ".provider");
            for (const auto* key : {"contract", "library", "symbol", "convention", "effect", "return_type"})
                (void)nonempty(provider, key, p + ".provider");
            (void)binding_evidence(provider, p + ".provider");
            if (activation == "startup_once" && selected->activation == "startup_once") {
                if (nonempty(item, "source_callable", p) != selected->source_callable ||
                    integer(required(item, "function_symbol_id", p), p + ".function_symbol_id") < 0 ||
                    str(item, "output_port", p) != "out" ||
                    !str(provider, "parameter_types", p + ".provider").empty() ||
                    str(provider, "return_type", p + ".provider") != type || type == "void")
                    throw Error(p, "unsupported startup producer contract");
            } else if (activation == "finite_stream_once" && selected->activation == "finite_stream_once") {
                const auto count_callable = nonempty(item, "count_callable", p);
                const auto item_callable = nonempty(item, "item_callable", p);
                const auto max_items = integer(required(item, "max_items", p), p + ".max_items");
                if (count_callable != selected->count_callable || item_callable != selected->item_callable ||
                    max_items != selected->max_items || str(item, "output_port", p) != "out" ||
                    integer(required(item, "function_symbol_id", p), p + ".function_symbol_id") < 0 ||
                    integer(required(item, "count_function_symbol_id", p), p + ".count_function_symbol_id") < 0 ||
                    str(provider, "parameter_types", p + ".provider") != "c_size_t" ||
                    str(provider, "return_type", p + ".provider") != type || type == "void")
                    throw Error(p, "unsupported finite stream item provider contract");
                const auto& count_provider = object(required(item, "count_provider", p), p + ".count_provider");
                for (const auto* key : {"contract", "library", "symbol", "convention", "effect", "return_type"})
                    (void)nonempty(count_provider, key, p + ".count_provider");
                (void)binding_evidence(count_provider, p + ".count_provider");
                if (str(count_provider, "parameter_types", p + ".count_provider") != "" ||
                    str(count_provider, "return_type", p + ".count_provider") != "c_size_t")
                    throw Error(p, "finite stream count provider must return c_size_t without arguments");
            } else {
                throw Error(p, "provider resolution differs from selected activation");
            }
            provider_types.emplace(node, type);
            provenance(item, p);
        }
    }
    std::set<std::string> connected;
    for (const auto& wire : result.wires) {
        if (provider_types.count(wire.to.node)) throw Error(path + ".syntax.wires", "startup producer has no input port");
        if (provider_types.count(wire.from.node)) {
            if (wire.from.port != "out") throw Error(path + ".syntax.wires", "invalid startup producer output port");
            if (receivers.count(wire.to.node) && provider_types.at(wire.from.node) != types.at(wire.to.node).first)
                throw Error(path + ".syntax.wires", "incompatible producer and receiver port types");
        }
        if (receivers.count(wire.from.node) && wire.from.port != "out") throw Error(path + ".syntax.wires", "invalid receiver output port");
        if (receivers.count(wire.to.node)) {
            if (wire.to.port != "in") throw Error(path + ".syntax.wires", "invalid receiver input port");
            connected.insert(wire.to.node);
        }
        if (receivers.count(wire.from.node) && receivers.count(wire.to.node) && types.at(wire.from.node).second != types.at(wire.to.node).first)
            throw Error(path + ".syntax.wires", "incompatible receiver port types");
    }
    for (const auto& node : result.nodes) if (node.implementation_kind == "source_function" && (!receivers.count(node.id) || !connected.count(node.id)))
        throw Error(path + ".receivers", "missing receiver resolution or input connection");
    if (result.executable) {
        if (!result.policies.empty() || result.providers.empty() ||
            result.providers.size() + result.receivers.size() != result.nodes.size())
            throw Error(path, "native graph requires fully resolved startup producers and source receivers without policies");
        for (const auto& [node, type] : provider_types)
            if (type != "c_int" && type != "c_long" && type != "c_ulong" && type != "c_size_t" && type != "c_string")
                throw Error(path + ".providers", "unsupported native producer result carrier");
        std::map<std::string, std::size_t> indegree;
        for (const auto& node : result.nodes) indegree[node.id] = 0;
        for (const auto& wire : result.wires) ++indegree.at(wire.to.node);
        std::vector<std::string> pending;
        for (const auto& node : result.nodes) if (!indegree.at(node.id)) pending.push_back(node.id);
        for (std::size_t i = 0; i < pending.size(); ++i)
            for (const auto& wire : result.wires) if (wire.from.node == pending[i] && --indegree.at(wire.to.node) == 0)
                pending.push_back(wire.to.node);
        if (pending.size() != result.nodes.size()) throw Error(path, "cyclic native graph is unsupported");
    }
    return result;
}

} // namespace flowcontracts
