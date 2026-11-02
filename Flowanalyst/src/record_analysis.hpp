#pragma once

#include <flowcontracts/scalar_analysis.hpp>
#include <vector>

namespace flowanalyst::record {
using namespace lyraform::scalar::frontend;

struct Issue {
    int statement;
    int expression;
    std::string code;
    std::string message;
};

// This pass checks named-field completeness and established scalar types.
// It does not grant executable
// admission, infer resource lifetimes, or define backend aggregate layout.
inline std::vector<Issue> construction_shape(
    const Index& declarations, const Index& statements, const Index& expressions,
    const Index& symbols, const Index& scopes, const Index& origins,
    const std::map<int, int>& statement_scopes,
    const std::map<int, int>& resolved_symbols,
    const std::map<int, std::string>& symbol_types,
    const std::map<int, bool>& pure_symbols, bool executable) {
    std::vector<Issue> issues;
    std::map<int, const Json*> records;
    for (const auto& [symbol, origin] : origins) {
        const auto path = text(get(origin, "ast_path"));
        for (const auto& [declaration, value] : declarations) {
            if (path == "/declaration_pool/" + std::to_string(declaration) &&
                text(get(value, "kind")) == "record") records.emplace(symbol, value);
        }
    }
    // A repeated record on the active type path is recursive by value. Shared
    // subrecords on separate paths are finite and must not be mistaken for cycles.
    std::map<int, bool> finite_records;
    std::set<int> active_types;
    std::function<bool(int)> finite = [&](int symbol) {
        if (const auto known = finite_records.find(symbol); known != finite_records.end())
            return known->second;
        if (active_types.size() >= 256 || !active_types.insert(symbol).second) return false;
        const int scope = symbols.count(symbol)
            ? id(get(symbols.at(symbol), "introduced_scope_id")) : -1;
        bool result = true;
        for (const auto& value : items(get(records.at(symbol), "fields"))) {
            const int child = visible_symbol(scopes, symbols, scope, text(get(&value, "type")));
            if (records.count(child) && !finite(child)) result = false;
        }
        active_types.erase(symbol);
        finite_records.emplace(symbol, result);
        return result;
    };
    std::set<int> active_purity;
    std::function<bool(int)> pure = [&](int expression) {
        const auto found = expressions.find(expression);
        if (found == expressions.end() || active_purity.size() >= 256 ||
            !active_purity.insert(expression).second) return false;
        struct Erase { std::set<int>& active; int id; ~Erase() { active.erase(id); } } erase{active_purity, expression};
        const auto kind = text(get(found->second, "kind"));
        if (kind == "call") {
            const int base = id(get(get(found->second, "payload"), "base"));
            const auto resolved = resolved_symbols.find(base);
            if (resolved == resolved_symbols.end()) return false;
            const auto effect = pure_symbols.find(resolved->second);
            if (effect == pure_symbols.end() || !effect->second) return false;
        } else if (kind != "integer_literal" && kind != "bool_literal" &&
                   kind != "float_literal" && kind != "string_literal" &&
                   kind != "identifier" && kind != "unary" && kind != "binary" &&
                   kind != "field_access" && kind != "record_literal") return false;
        for (const auto& child : items(get(found->second, "child_expressions")))
            if (!pure(id(&child))) return false;
        return true;
    };
    // Reuse the scalar authority, including declaration-before-use checks.
    std::map<int, int> initialized;
    std::set<int> uninitialized;
    for (const auto& [statement, value] : statements) {
        if (text(get(value, "kind")) != "let" || !statement_scopes.count(statement)) continue;
        const int symbol = visible_symbol(scopes, symbols, statement_scopes.at(statement),
            text(get(value, "name")));
        if (symbol < 0) continue;
        if (id(get(get(value, "payload"), "initializer_expression")) >= 0)
            initialized.emplace(symbol, statement);
        else uninitialized.insert(symbol);
    }
    std::function<void(int, int, int, const std::string&, std::set<int>&)> check;
    check = [&](int statement, int expression, int scope, const std::string& type_name,
                std::set<int>& active) {
        const auto found = expressions.find(expression);
        if (found == expressions.end() || text(get(found->second, "kind")) != "record_literal") return;
        if (active.size() >= 256 || !active.insert(expression).second) {
            issues.push_back({statement, expression, "FLOWANALYST_RECORD_STRUCTURE_INVALID",
                "record initializer structure is cyclic or exceeds the analysis depth bound"});
            return;
        }
        struct Erase { std::set<int>& active; int expression; ~Erase() { active.erase(expression); } } erase{active, expression};
        const int symbol = visible_symbol(scopes, symbols, scope, type_name);
        if (!records.count(symbol)) return; // Type resolution owns unknown/non-record types.
        std::map<std::string, std::string> fields;
        for (const auto& value : items(get(records.at(symbol), "fields"))) {
            const auto name = text(get(&value, "name"));
            if (!fields.emplace(name, text(get(&value, "type"))).second)
                issues.push_back({statement, expression, "FLOWANALYST_RECORD_DUPLICATE_DECLARATION",
                    "record '" + type_name + "' declares field '" + name + "' more than once"});
        }
        std::set<std::string> supplied;
        for (const auto& value : items(get(get(found->second, "payload"), "fields"))) {
            const auto name = text(get(&value, "name"));
            if (!supplied.insert(name).second)
                issues.push_back({statement, expression, "FLOWANALYST_RECORD_DUPLICATE_FIELD",
                    "record field '" + name + "' must be constructed exactly once"});
            const auto declared = fields.find(name);
            if (declared == fields.end()) {
                issues.push_back({statement, expression, "FLOWANALYST_RECORD_UNKNOWN_FIELD",
                    "record '" + type_name + "' has no field '" + name + "'"});
                continue;
            }
            // A nested field's type is resolved in its declaring record's scope,
            // not in the scope of the caller constructing the outer value.
            const int nested_scope = symbols.count(symbol)
                ? id(get(symbols.at(symbol), "introduced_scope_id")) : -1;
            const int initializer = id(get(&value, "value"));
            const auto destination = lyraform::scalar::type(declared->second);
            const auto source = infer_expression_type(statement, initializer, statements,
                expressions, resolved_symbols, symbol_types, initialized, uninitialized);
            if (lyraform::scalar::selected(destination) &&
                source != lyraform::scalar::Type::outside_slice &&
                !lyraform::scalar::compatible(source, destination)) {
                issues.push_back({statement, initializer, "FLOWANALYST_RECORD_FIELD_TYPE",
                    "record field '" + name + "' requires '" + declared->second +
                    "', but its initializer has scalar classification '" +
                    std::string(lyraform::scalar::name(source)) + "'"});
            }
            const int field_record = visible_symbol(scopes, symbols, nested_scope, declared->second);
            if (records.count(field_record) && lyraform::scalar::selected(source)) {
                issues.push_back({statement, initializer, "FLOWANALYST_RECORD_FIELD_TYPE",
                    "record field '" + name + "' requires record '" + declared->second +
                    "', not a scalar value"});
            }
            // Outside-slice expressions need the pending general value analysis;
            // they are not certified by absence of a diagnostic in this pass.
            check(statement, initializer, nested_scope, declared->second, active);
        }
        for (const auto& [name, field_type] : fields) {
            (void)field_type;
            if (!supplied.count(name))
                issues.push_back({statement, expression, "FLOWANALYST_RECORD_MISSING_FIELD",
                    "record '" + type_name + "' requires field '" + name + "'"});
        }
    };
    for (const auto& [statement, value] : statements) {
        if (text(get(value, "kind")) != "let") continue;
        const auto scope = statement_scopes.find(statement);
        if (scope == statement_scopes.end()) continue;
        const int initializer = id(get(get(value, "payload"), "initializer_expression"));
        const int symbol = visible_symbol(scopes, symbols, scope->second, text(get(value, "type")));
        const bool literal_construction = expressions.count(initializer) &&
            text(get(expressions.at(initializer), "kind")) == "record_literal";
        if (executable && literal_construction && records.count(symbol)) {
            if (!finite(symbol))
                issues.push_back({statement, initializer, "FLOWANALYST_RECORD_TYPE_NOT_FINITE",
                    "ordinary record construction requires finite value fields within the analysis depth bound"});
            if (!pure(initializer))
                issues.push_back({statement, initializer, "FLOWANALYST_RECORD_INITIALIZER_PURITY_UNPROVEN",
                    "ordinary record initializer purity is not established; effectful field evaluation order is not admitted"});
        }
        std::set<int> active;
        check(statement, id(get(get(value, "payload"), "initializer_expression")),
            scope->second, text(get(value, "type")), active);
    }
    return issues;
}
}
