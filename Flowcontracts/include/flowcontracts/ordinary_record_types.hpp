#pragma once

#include <flowcontracts/json.hpp>
#include <flowcontracts/scalar_semantics.hpp>
#include <functional>
#include <map>
#include <set>

namespace flowcontracts {
inline void validate_ordinary_record_types(const json::Value& value) {
    using namespace json;
    const std::string path = "$.lowering_plan.ordinary_record_types";
    const auto fail = [&](const std::string& reason) { throw Error(path, reason); };
    const auto exact = [&](const Object& object, const std::set<std::string>& keys) {
        if (object.size() != keys.size()) fail("unexpected record type contract fields");
        for (const auto& [key, entry] : object) {
            (void)entry;
            if (!keys.count(key)) fail("unexpected record type contract field: " + key);
        }
    };
    const auto& root = object(value, path);
    exact(root, {"format", "version", "status", "layout", "semantics", "ownership", "lifetime", "types"});
    if (string(required(root, "format", path), path) != "lyraform.ordinary_record_types" ||
        integer(required(root, "version", path), path) != 1 ||
        string(required(root, "status", path), path) != "non_executable" ||
        string(required(root, "layout", path), path) != "private" ||
        string(required(root, "semantics", path), path) != "independent_value" ||
        string(required(root, "ownership", path), path) != "value" ||
        string(required(root, "lifetime", path), path) != "enclosing_value") fail("unsupported ordinary record type contract");
    std::map<Integer, std::vector<Integer>> dependencies;
    std::set<Integer> declarations;
    for (const auto& entry : array(required(root, "types", path), path)) {
        const auto& type = object(entry);
        exact(type, {"symbol_id", "declaration_id", "name", "fields", "ownership", "lifetime"});
        const auto symbol = integer(required(type, "symbol_id", path), path);
        const auto declaration = integer(required(type, "declaration_id", path), path);
        if (symbol < 0 || declaration < 0 ||
            string(required(type, "ownership", path), path) != "value" ||
            string(required(type, "lifetime", path), path) != "enclosing_value" ||
            !dependencies.emplace(symbol, std::vector<Integer>{}).second ||
            !declarations.insert(declaration).second || string(required(type, "name", path), path).empty())
            fail("invalid or duplicate record declaration identity");
        std::set<std::string> names;
        for (const auto& item : array(required(type, "fields", path), path)) {
            const auto& field = object(item);
            exact(field, {"name", "type"});
            const auto name = string(required(field, "name", path), path);
            if (name.empty() || !names.insert(name).second) fail("invalid or duplicate record field identity");
            const auto& reference = object(required(field, "type"));
            const auto kind = string(required(reference, "kind", path), path);
            if (kind == "scalar") {
                exact(reference, {"kind", "name"});
                const auto name = string(required(reference, "name", path), path);
                if (!lyraform::scalar::record_selected(lyraform::scalar::type(name)) &&
                    name != "int32" && name != "int64")
                    fail("record scalar type is outside the projected bound");
            } else if (kind == "record") {
                exact(reference, {"kind", "symbol_id"});
                dependencies.at(symbol).push_back(integer(required(reference, "symbol_id", path), path));
            } else fail("unsupported record field type");
        }
    }
    if (dependencies.empty()) fail("record type catalog must not be empty");
    std::set<Integer> active, visited;
    std::function<void(Integer)> visit = [&](Integer symbol) {
        if (visited.count(symbol)) return;
        if (!dependencies.count(symbol)) fail("record field references an absent type identity");
        if (active.size() >= 256 || !active.insert(symbol).second)
            fail("record type dependencies are recursive or exceed the depth bound");
        for (auto child : dependencies.at(symbol)) visit(child);
        active.erase(symbol);
        visited.insert(symbol);
    };
    for (const auto& [symbol, children] : dependencies) { (void)children; visit(symbol); }
}
}
