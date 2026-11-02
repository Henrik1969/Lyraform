#pragma once

#include <flowcontracts/scalar_analysis.hpp>

namespace flowanalyst::record {
using namespace lyraform::scalar::frontend;

// Source type identity and field identity only. This is deliberately separate
// from provider-verified aggregate ABI layout, byte offsets and foreign names.
inline Json type_catalog(const Index& declarations, const Index& symbols,
                         const Index& scopes, const Index& origins) {
    using flowcontracts::json::Array;
    using flowcontracts::json::Object;
    using flowcontracts::json::Integer;
    std::map<int, std::pair<int, const Json*>> records;
    for (const auto& [symbol, origin] : origins)
        for (const auto& [declaration, value] : declarations)
            if (text(get(value, "kind")) == "record" &&
                text(get(origin, "ast_path")) == "/declaration_pool/" + std::to_string(declaration))
                records.emplace(symbol, std::make_pair(declaration, value));
    std::map<int, Json> projected;
    std::set<int> active;
    std::function<bool(int)> project = [&](int symbol) {
        if (projected.count(symbol)) return true;
        if (!records.count(symbol) || !symbols.count(symbol) || active.size() >= 256 ||
            !active.insert(symbol).second) return false;
        struct Erase { std::set<int>& ids; int value; ~Erase() { ids.erase(value); } } erase{active, symbol};
        const int scope = id(get(symbols.at(symbol), "introduced_scope_id"));
        Array fields;
        std::set<std::string> names;
        for (const auto& field : items(get(records.at(symbol).second, "fields"))) {
            const auto name = text(get(&field, "name"));
            const auto spelling = text(get(&field, "type"));
            if (name.empty() || !names.insert(name).second) return false;
            Json type;
            if (lyraform::scalar::record_selected(lyraform::scalar::type(spelling)))
                type = Object{{"kind", std::string("scalar")}, {"name", spelling}};
            else {
                const int child = visible_symbol(scopes, symbols, scope, spelling);
                if (!project(child)) return false;
                type = Object{{"kind", std::string("record")}, {"symbol_id", Integer{child}}};
            }
            fields.emplace_back(Object{{"name", name}, {"type", type}});
        }
        projected.emplace(symbol, Object{{"symbol_id", Integer{symbol}},
            {"declaration_id", Integer{records.at(symbol).first}},
            {"name", text(get(records.at(symbol).second, "name"))}, {"fields", fields}});
        return true;
    };
    for (const auto& [symbol, declaration] : records) { (void)declaration; project(symbol); }
    if (projected.empty()) return nullptr;
    Array types;
    for (const auto& [symbol, value] : projected) { (void)symbol; types.push_back(value); }
    return Object{{"format", std::string("lyraform.ordinary_record_types")},
        {"version", Integer{1}}, {"status", std::string("non_executable")},
        {"layout", std::string("private")}, {"semantics", std::string("independent_value")},
        {"types", types}};
}
}
