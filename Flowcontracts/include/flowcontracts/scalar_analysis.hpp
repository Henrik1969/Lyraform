#pragma once

#include <flowcontracts/json.hpp>
#include <flowcontracts/scalar_semantics.hpp>
#include <functional>
#include <map>
#include <set>

namespace lyraform::scalar::frontend {
using namespace lyraform::scalar;
using Json = flowcontracts::json::Value;
using Index = std::map<int, const Json*>;

inline const Json* get(const Json* value, std::string_view key) {
    if (value) if (const auto* object = std::get_if<flowcontracts::json::Object>(value)) {
        const auto found = object->find(std::string(key));
        if (found != object->end()) return &found->second;
    }
    return nullptr;
}
inline std::string text(const Json* value) {
    if (value) if (const auto* string = std::get_if<std::string>(value)) return *string;
    return {};
}
inline int id(const Json* value) {
    if (!value || std::holds_alternative<std::nullptr_t>(*value)) return -1;
    const auto number = flowcontracts::json::integer(*value, "$.scalar.identity");
    if (number < 0 || number > INT32_MAX) return -1;
    return static_cast<int>(number);
}
inline std::pair<int, int> position(const Json* statement) {
    return {id(get(get(statement, "location"), "line")), id(get(get(statement, "location"), "column"))};
}

inline const flowcontracts::json::Array& items(const Json* value) {
    static const flowcontracts::json::Array empty;
    return value ? flowcontracts::json::array(*value, "$.scalar.projection") : empty;
}

// Lexical lookup is shared by the staged analyzer and the direct-source model.
inline int symbol_in_scope(const Index& scopes, const Index& symbols,
                           int scope, const std::string& name) {
    if (!scopes.count(scope)) return -1;
    for (const auto& candidate : items(get(scopes.at(scope), "symbol_ids"))) {
        const int key = id(&candidate);
        if (symbols.count(key) && text(get(symbols.at(key), "name")) == name) return key;
    }
    return -1;
}

inline int visible_symbol(const Index& scopes, const Index& symbols,
                          int scope, const std::string& name) {
    std::set<int> visited;
    while (scopes.count(scope) && visited.insert(scope).second) {
        const int found = symbol_in_scope(scopes, symbols, scope, name);
        if (found >= 0) return found;
        scope = id(get(scopes.at(scope), "parent_id"));
    }
    return -1;
}

inline Type infer_expression_type(
    int statement, int expression, const Index& statements, const Index& expressions,
    const std::map<int, int>& resolved_symbols,
    const std::map<int, std::string>& symbol_types,
    const std::map<int, int>& initialized_declarations,
    const std::set<int>& uninitialized_declarations,
    std::map<int, Type>* projected_types = nullptr) {
    std::set<int> active;
    std::map<int, Type> inferred;
    std::function<Type(int)> infer;
    const auto symbol_type = [&](int symbol) {
        const auto found = symbol_types.find(symbol);
        return found == symbol_types.end() ? Type::outside_slice : type(found->second);
    };
    const auto compute = [&](int expression_id) -> Type {
        const auto found = expressions.find(expression_id);
        if (found == expressions.end() || active.size() >= 256 || !active.insert(expression_id).second) return Type::invalid;
        struct Erase { std::set<int>& ids; int id; ~Erase() { ids.erase(id); } } erase{active, expression_id};
        const auto* value = found->second;
        const auto kind = text(get(value, "kind"));
        const auto* payload = get(value, "payload");
        if (kind == "integer_literal") return Type::integer;
        if (kind == "bool_literal") return Type::boolean;
        if (kind == "string_literal" || kind == "float_literal" || kind == "list_literal" || kind == "record_literal") return Type::other;
        if (kind == "identifier") {
            const auto symbol = resolved_symbols.find(expression_id);
            if (symbol == resolved_symbols.end()) return Type::invalid;
            if (uninitialized_declarations.count(symbol->second)) return Type::outside_slice;
            if (const auto local = initialized_declarations.find(symbol->second); local != initialized_declarations.end()) {
                if (!statements.count(local->second) || !statements.count(statement) ||
                    position(statements.at(local->second)) >= position(statements.at(statement))) return Type::invalid;
            }
            return symbol_type(symbol->second);
        }
        if (kind == "unary") return unary(text(get(payload, "operator")), infer(id(get(payload, "operand"))));
        if (kind == "binary") return binary(text(get(payload, "operator")),
            infer(id(get(payload, "left"))), infer(id(get(payload, "right"))));
        return Type::outside_slice;
    };
    infer = [&](int expression_id) -> Type {
        if (const auto cached = inferred.find(expression_id); cached != inferred.end()) return cached->second;
        const auto result = compute(expression_id);
        inferred.emplace(expression_id, result);
        if (projected_types) (*projected_types)[expression_id] = result;
        return result;
    };
    return infer(expression);
}

// This adapter reads structural facts only. Compatibility and operator result
// rules live in the shared scalar component, not in this JSON projection.
inline std::map<int, Fact> analyze(
    const Json& bundle, const Index& statements, const Index& expressions,
    const std::map<int, int>& statement_scopes,
    const std::map<int, int>& resolved_symbols,
    const std::map<int, std::string>& symbol_types,
    const std::function<int(int, const std::string&)>& visible_symbol,
    std::map<int, Type>* expression_types = nullptr) {
    auto symbol_type = [&](int symbol) {
        const auto found = symbol_types.find(symbol);
        return found == symbol_types.end() ? Type::outside_slice : type(found->second);
    };
    auto destination = [&](int statement, const std::string& name) {
        const auto scope = statement_scopes.find(statement);
        return scope == statement_scopes.end() ? -1 : visible_symbol(scope->second, name);
    };
    std::map<int, int> initialized_declarations;
    std::set<int> uninitialized_declarations;
    std::set<int> conflicting_declarations;
    for (const auto& [statement_id, statement] : statements) {
        if (text(get(statement, "kind")) != "let") continue;
        const auto symbol = destination(statement_id, text(get(statement, "name")));
        if (id(get(get(statement, "payload"), "initializer_expression")) < 0) {
            uninitialized_declarations.insert(symbol);
            continue;
        }
        const auto* reference = get(get(statement, "payload"), "type_ref");
        Type declared = Type::outside_slice;
        if (text(get(reference, "kind")) == "named") {
            if (const auto* names = get(get(reference, "payload"), "name_segments")) {
                const auto& segments = flowcontracts::json::array(*names, "$.scalar.type_ref.name_segments");
                if (segments.size() == 1) declared = type(text(&segments.front()));
            }
        }
        if (selected(declared) || selected(symbol_type(symbol))) {
            initialized_declarations.emplace(symbol, statement_id);
            if (declared != symbol_type(symbol)) conflicting_declarations.insert(statement_id);
        }
    }

    std::map<int, Fact> facts;
    for (const auto& [statement_id, statement] : statements) {
        const auto kind = text(get(statement, "kind"));
        const auto* payload = get(statement, "payload");
        if (kind != "let" && kind != "placement") continue;
        if (kind == "placement" && text(get(get(payload, "target"), "kind")) != "identifier") continue;
        Fact fact;
        fact.declaration = kind == "let";
        fact.statement = statement_id;
        fact.expression = id(get(payload, fact.declaration ? "initializer_expression" : "value_expression"));
        if (fact.expression < 0) continue; // No definite-initialization policy in this phase.
        const auto name = fact.declaration ? text(get(statement, "name")) : text(get(get(payload, "target"), "name"));
        fact.destination = destination(statement_id, name);
        fact.destination_type = symbol_type(static_cast<int>(fact.destination));
        const auto declaration = initialized_declarations.find(static_cast<int>(fact.destination));
        if (declaration != initialized_declarations.end()) fact.declaration_statement = declaration->second;
        // Parameters, uninitialized locals, ABI carriers and aggregate targets
        // retain their existing contracts; none receives a Phase 2 proof.
        if (fact.destination >= 0 && (declaration == initialized_declarations.end() ||
            (!selected(fact.destination_type) && !conflicting_declarations.count(statement_id)))) continue;

        fact.source_type = infer_expression_type(statement_id, static_cast<int>(fact.expression),
            statements, expressions, resolved_symbols, symbol_types, initialized_declarations,
            uninitialized_declarations, expression_types);
        if (conflicting_declarations.count(statement_id)) fact.source_type = Type::invalid;
        if (fact.source_type == Type::outside_slice) continue;
        if (!fact.declaration && declaration != initialized_declarations.end() &&
            position(statements.at(declaration->second)) >= position(statement)) fact.source_type = Type::invalid;
        fact.origin.source = text(get(get(&bundle, "source"), "path"));
        fact.origin.ast_path = "/statement_pool/" + std::to_string(statement_id);
        const auto [line, column] = position(statement);
        fact.origin.line = line;
        fact.origin.column = column;
        // Map expanded-line coordinates back to the originating imported file.
        const auto* map = get(&bundle, "source_map");
        if (const auto* lines = get(map, "lines")) for (const auto& entry : flowcontracts::json::array(*lines, "$.source_map.lines")) {
            if (id(get(&entry, "expanded_line")) != line) continue;
            fact.origin.line = id(get(&entry, "source_line"));
            const auto source_id = id(get(&entry, "source_id"));
            if (const auto* files = get(map, "files")) for (const auto& file : flowcontracts::json::array(*files, "$.source_map.files"))
                if (id(get(&file, "id")) == source_id) fact.origin.source = text(get(&file, "path"));
            break;
        }
        facts.emplace(statement_id, std::move(fact));
    }
    for (auto it = facts.begin(); it != facts.end();) {
        if (!it->second.declaration && it->second.destination >= 0 &&
            !facts.count(static_cast<int>(it->second.declaration_statement))) it = facts.erase(it);
        else ++it;
    }
    return facts;
}
} // namespace lyraform::scalar::frontend
