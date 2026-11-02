#pragma once

#include "record_types.hpp"
#include <cctype>
#include <map>

namespace flowanalyst::record {
inline Json read_operand(int expression, const Index& expressions, const Json& catalog,
                         const std::map<int, int>& resolved_symbols,
                         const std::map<int, int>& local_types, std::set<int>& active);
}

namespace flowanalyst::record {
// Initial typed operand projection: literal/scalar-expression construction only.
// Other expressions keep their explicit unsupported representation until typed.
inline Json construction_operand(int expression, const Json& expected,
                                 const Index& expressions, const Json& catalog,
                                 std::set<int>& active,
                                 const std::map<int, int>& resolved_symbols = {},
                                 const std::map<int, int>& local_types = {},
                                 const std::map<int, std::string>& symbol_types = {}) {
    using flowcontracts::json::Object;
    using flowcontracts::json::Array;
    using flowcontracts::json::Integer;
    if (!expressions.count(expression) || active.size() >= 256 ||
        !active.insert(expression).second) return nullptr;
    struct Erase { std::set<int>& ids; int value; ~Erase() { ids.erase(value); } } erase{active, expression};
    const auto* source = expressions.at(expression);
    const auto kind = text(get(source, "kind"));
    const auto* payload = get(source, "payload");
    if (text(get(&expected, "kind")) == "record") {
        if (kind != "record_literal") return nullptr;
        const int symbol = id(get(&expected, "symbol_id"));
        const Json* definition = nullptr;
        for (const auto& type : items(get(&catalog, "types")))
            if (id(get(&type, "symbol_id")) == symbol) definition = &type;
        if (!definition) return nullptr;
        std::map<std::string, int> supplied;
        for (const auto& field : items(get(payload, "fields")))
            if (!supplied.emplace(text(get(&field, "name")), id(get(&field, "value"))).second) return nullptr;
        if (supplied.size() != items(get(definition, "fields")).size()) return nullptr;
        Array fields;
        for (const auto& field : items(get(definition, "fields"))) {
            const auto name = text(get(&field, "name"));
            if (!supplied.count(name)) return nullptr;
            auto operand = construction_operand(supplied.at(name), *get(&field, "type"), expressions, catalog, active,
                resolved_symbols, local_types, symbol_types);
            if (std::holds_alternative<std::nullptr_t>(operand)) return nullptr;
            fields.emplace_back(Object{{"name", name}, {"operand", operand}});
        }
        return Object{{"kind", std::string("record_construct")}, {"expression_id", Integer{expression}},
            {"type", std::string("ordinary_record")}, {"record_symbol_id", Integer{symbol}}, {"fields", fields}};
    }
    const auto type = text(get(&expected, "name"));
    const auto scalar = lyraform::scalar::type(type);
    if (!lyraform::scalar::record_selected(scalar)) return nullptr;
    if (kind == "identifier" && resolved_symbols.count(expression) &&
        symbol_types.count(resolved_symbols.at(expression)) &&
        symbol_types.at(resolved_symbols.at(expression)) == type) {
        return Object{{"kind", std::string("identifier")}, {"expression_id", Integer{expression}},
            {"type", type}, {"symbol_id", Integer{resolved_symbols.at(expression)}}};
    }
    if (kind == "field_access") {
        auto projected_active = active;
        projected_active.erase(expression);
        const auto projected = read_operand(expression, expressions, catalog, resolved_symbols, local_types, projected_active);
        if (!std::holds_alternative<std::nullptr_t>(projected))
            return projected;
    }
    if ((kind == "integer_literal" && scalar == lyraform::scalar::Type::integer) ||
        (kind == "bool_literal" && scalar == lyraform::scalar::Type::boolean) ||
        (kind == "string_literal" && scalar == lyraform::scalar::Type::text)) {
        const auto literal = text(get(payload, "value_text"));
        if (literal.empty() && kind != "string_literal") return nullptr;
        if (kind == "integer_literal") {
            for (const unsigned char character : literal)
                if (!std::isdigit(character)) return nullptr;
        } else if (kind == "bool_literal" && literal != "true" && literal != "false") return nullptr;
        return Object{{"kind", kind}, {"expression_id", Integer{expression}},
            {"type", type}, {"value", literal}};
    }
    // The established operator rules select input and result types, not LLVM
    // carrier spelling. Probe only the two already-admitted scalar identities.
    if (kind == "unary" || kind == "binary") {
        const auto operation = text(get(payload, "operator"));
        for (const auto* input : {"int", "Bool"}) {
            const auto input_type = lyraform::scalar::type(input);
            const auto result = kind == "unary" ? lyraform::scalar::unary(operation, input_type)
                : lyraform::scalar::binary(operation, input_type, input_type);
            if (result != scalar) continue;
            const Json reference = Object{{"kind", std::string("scalar")}, {"name", std::string(input)}};
            auto left = construction_operand(id(get(payload, kind == "unary" ? "operand" : "left")),
                reference, expressions, catalog, active, resolved_symbols, local_types, symbol_types);
            if (std::holds_alternative<std::nullptr_t>(left)) continue;
            Object operand{{"kind", kind}, {"expression_id", Integer{expression}}, {"type", type}, {"operator", operation}};
            if (kind == "unary") operand.emplace("operand", left);
            else {
                auto right = construction_operand(id(get(payload, "right")), reference, expressions, catalog, active,
                    resolved_symbols, local_types, symbol_types);
                if (std::holds_alternative<std::nullptr_t>(right)) continue;
                operand.emplace("left", left); operand.emplace("right", right);
            }
            return operand;
        }
    }
    return nullptr;
}
inline Json read_operand(int expression, const Index& expressions, const Json& catalog,
                         const std::map<int, int>& resolved_symbols,
                         const std::map<int, int>& local_types, std::set<int>& active) {
    using flowcontracts::json::Object;
    using flowcontracts::json::Integer;
    if (!expressions.count(expression) || active.size() >= 256 ||
        !active.insert(expression).second) return nullptr;
    struct Erase { std::set<int>& ids; int value; ~Erase() { ids.erase(value); } } erase{active, expression};
    const auto* source = expressions.at(expression);
    const auto kind = text(get(source, "kind"));
    if (kind == "identifier") {
        const auto found = resolved_symbols.find(expression);
        if (found == resolved_symbols.end() || !local_types.count(found->second)) return nullptr;
        return Object{{"kind", std::string("record_value")}, {"expression_id", Integer{expression}},
            {"type", std::string("ordinary_record")}, {"symbol_id", Integer{found->second}},
            {"record_symbol_id", Integer{local_types.at(found->second)}}};
    }
    if (kind != "field_access") return nullptr;
    const auto* payload = get(source, "payload");
    auto base = read_operand(id(get(payload, "base")), expressions, catalog, resolved_symbols, local_types, active);
    if (std::holds_alternative<std::nullptr_t>(base)) return nullptr;
    int type = -1;
    if (text(get(&base, "kind")) == "record_value") type = id(get(&base, "record_symbol_id"));
    else if (text(get(get(&base, "result_type"), "kind")) == "record")
        type = id(get(get(&base, "result_type"), "symbol_id"));
    const auto field = text(get(payload, "field"));
    for (const auto& record : items(get(&catalog, "types"))) {
        if (id(get(&record, "symbol_id")) != type) continue;
        for (const auto& member : items(get(&record, "fields"))) {
            if (text(get(&member, "name")) != field) continue;
            const auto& result = *get(&member, "type");
            const auto carrier = text(get(&result, "kind")) == "record"
                ? "ordinary_record" : text(get(&result, "name"));
            return Object{{"kind", std::string("record_project")}, {"expression_id", Integer{expression}},
                {"type", std::string(carrier)}, {"record_symbol_id", Integer{type}},
                {"result_type", result}, {"field", field}, {"base", base}};
        }
    }
    return nullptr;
}

}
