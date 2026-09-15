#pragma once

#include <flowcontracts/json.hpp>
#include <flowcontracts/scheduling.hpp>
#include <set>

namespace flowcontracts {

struct GraphProviderSelection {
    std::string implementation, source_callable, count_callable, item_callable, activation = "startup_once", schedule_policy = "serial";
    json::Integer max_items = 0;
};

// Explicit provider selection, never capability authorization. The bounded
// adapter invokes a zero-argument provider once at startup and emits one value.
inline std::vector<GraphProviderSelection> graph_provider_map(const json::Value& value) {
    using namespace json;
    const auto& root = object(value);
    const auto version = integer(required(root, "version"), "$.version");
    if (string(required(root, "format"), "$.format") != "flowcore.graph_provider_map" ||
        (version != 1 && version != 2 && version != 3))
        throw Error("$", "unsupported graph provider map contract");
    std::vector<GraphProviderSelection> result;
    std::set<std::string> names;
    for (const auto& value : array(required(root, "providers"), "$.providers")) {
        const auto path = "$.providers[" + std::to_string(result.size()) + "]";
        const auto& item = object(value, path);
        validate_scheduling_request(item, true, path);
        GraphProviderSelection selection;
        selection.implementation = string(required(item, "implementation", path), path + ".implementation");
        selection.activation = string(required(item, "activation", path), path + ".activation");
        if (version == 3) {
            selection.schedule_policy = string(required(item, "schedule_policy", path), path + ".schedule_policy");
            if (selection.schedule_policy != "serial" && selection.schedule_policy != "parallel_independent_v1")
                throw Error(path + ".schedule_policy", "unsupported graph schedule policy");
        } else if (optional(item, "schedule_policy")) {
            throw Error(path + ".schedule_policy", "schedule policy requires graph provider map version 3");
        }
        if (selection.implementation.empty() || !names.insert(selection.implementation).second)
            throw Error(path, "empty or duplicate graph provider selection");
        if (string(required(item, "output_port", path), path + ".output_port") != "out")
            throw Error(path, "unsupported graph provider output port");
        if (selection.activation == "startup_once") {
            selection.source_callable = string(required(item, "source_callable", path), path + ".source_callable");
            if (selection.source_callable.empty()) throw Error(path, "empty startup graph provider callable");
        } else if ((version == 2 || version == 3) && selection.activation == "finite_stream_once") {
            selection.count_callable = string(required(item, "count_callable", path), path + ".count_callable");
            selection.item_callable = string(required(item, "item_callable", path), path + ".item_callable");
            selection.max_items = integer(required(item, "max_items", path), path + ".max_items");
            if (selection.count_callable.empty() || selection.item_callable.empty() ||
                selection.max_items < 1 || selection.max_items > 4096)
                throw Error(path, "invalid finite stream callable or item bound");
        } else {
            throw Error(path, "unsupported graph provider activation");
        }
        result.push_back(std::move(selection));
    }
    return result;
}

} // namespace flowcontracts
