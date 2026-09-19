#pragma once

#include "flowmini_ast.h"
#include "flowmini_parse_validity.h"
#include "flowmini_frontend_bundle.h"
#include "flowmini_lexer.h"
#include "flowmini_schema.h"
#include <flowcontracts/scalar_analysis.hpp>
#include <charconv>
#include <functional>
#include <sstream>

namespace flowmini::scalar_source {
namespace semantic = lyraform::scalar;
namespace frontend = lyraform::scalar::frontend;
using Type = semantic::Type;

using parse_validity::Ownership;
using parse_validity::classify;
using parse_validity::refuse;

struct Model {
    std::map<int, semantic::Fact> facts;
    std::map<int, int> references;
    std::map<int, Type> expression_types;
};

// Consume the existing frontend bundle projection, including its canonical
// scope/symbol IDs. The same analyzer and lexical lookup serve Flowanalyst.
inline Model analyze(const ast::AstModule& ast, const frontend::Json& bundle) {
    using namespace frontend;
    Index statements, expressions, scopes, symbols;
    const auto index = [](const Json* array, Index& out) {
        for (const auto& entry : items(array)) out.emplace(id(get(&entry, "id")), &entry);
    };
    index(get(get(&bundle, "ast"), "statement_pool"), statements);
    index(get(get(&bundle, "ast"), "expression_pool"), expressions);
    index(get(get(&bundle, "symbol_table"), "scopes"), scopes);
    index(get(get(&bundle, "symbol_table"), "symbols"), symbols);
    int main_scope = -1;
    for (const auto& origin : items(get(&bundle, "scope_origins")))
        if (text(get(&origin, "ast_path")) == "/declaration_pool/0") main_scope = id(get(&origin, "scope_id"));
    if (main_scope < 0) refuse("missing canonical main scope identity");
    std::map<int, int> statement_scopes;
    for (const auto& [key, statement] : statements) { (void)statement; statement_scopes[key] = main_scope; }
    std::map<int, std::string> types;
    for (const auto& [key, symbol] : symbols)
        for (const auto& fact : items(get(symbol, "facts")))
            if (text(get(&fact, "key")) == "declared_type_spelling") types[key] = text(get(get(&fact, "value"), "value"));
    Model model;
    for (const auto& [key, expression] : expressions)
        if (text(get(expression, "kind")) == "identifier") {
            const int symbol = visible_symbol(scopes, symbols, main_scope, text(get(get(expression, "payload"), "name")));
            if (symbol >= 0) model.references[key] = symbol;
        }
    model.facts = frontend::analyze(bundle, statements, expressions, statement_scopes, model.references, types,
        [&](int scope, const std::string& name) { return visible_symbol(scopes, symbols, scope, name); },
        &model.expression_types);
    for (const auto& [key, fact] : model.facts) {
        (void)key;
        if (!fact.admitted()) refuse("FLOWANALYST_SCALAR_FLOW_REFUSED at " + fact.origin.source + ":" +
            std::to_string(fact.origin.line) + ":" + std::to_string(fact.origin.column) +
            " (" + std::string(semantic::name(fact.source_type)) + " -> " + std::string(semantic::name(fact.destination_type)) + ")");
    }
    std::set<int> initialized;
    const auto& main = std::get<ast::MainBlock>(ast.declaration_pool.front());
    for (const auto key : ast.block_pool.at(*main.body).statements) {
        const auto& statement = ast.statement_pool.at(key).payload;
        if (const auto* print = std::get_if<ast::ExpressionStatement>(&statement)) {
            const auto reference = model.references.find(static_cast<int>(print->expression));
            if (reference == model.references.end() || !initialized.count(reference->second))
                refuse("print requires an established canonical scalar identifier");
            model.expression_types[static_cast<int>(print->expression)] = semantic::type(types.at(reference->second));
            continue;
        }
        const auto fact = model.facts.find(static_cast<int>(key));
        if (fact == model.facts.end()) refuse("missing canonical scalar fact; no compatibility retry");
        if (fact->second.declaration && !initialized.insert(static_cast<int>(fact->second.destination)).second)
            refuse("duplicate scalar destination identity");
    }
    return model;
}

struct RuntimeOrigin {
    std::string node;
    std::int64_t statement = -1, declaration = -1, expression = -1, destination = -1;
    semantic::Origin source;
};
struct Adapted {
    ModuleSpec module;
    std::vector<RuntimeOrigin> origins;
};

// A representation adapter only: all names and scalar types have been resolved
// and admitted before entry. Existing atoms retain arithmetic/runtime policy.
inline Adapted adapt(const ast::AstModule& ast, const Model& model,
                     const std::string& source, const std::vector<ast::FrontendSourceLineOrigin>& lines) {
    Adapted result;
    result.module.name = ast.source_unit.name;
    result.module.nodes.push_back({"producer", "scalar_start", "start.record"});
    Endpoint tail{"scalar_start", "out"};
    RuntimeOrigin origin;
    const auto node = [&](const std::string& kind, const std::vector<std::pair<std::string, flow::PolicyValue>>& policies) {
        const std::string name = "scalar_node_" + std::to_string(result.module.nodes.size());
        result.module.nodes.push_back({"node", name, kind});
        result.module.wires.push_back({tail, {name, "in"}});
        tail = {name, "out"};
        for (const auto& [key, value] : policies) result.module.policies.push_back({name, key, value});
        auto provenance = origin; provenance.node = name; result.origins.push_back(std::move(provenance));
    };
    const auto path = [](std::int64_t symbol) { return "scalar_symbol_" + std::to_string(symbol); };
    std::function<std::string(std::size_t)> expression = [&](std::size_t key) -> std::string {
        const auto& payload = ast.expression_pool.at(key).payload;
        const std::string out = "scalar_expression_" + std::to_string(key);
        if (std::holds_alternative<ast::IdentifierExpr>(payload)) return path(model.references.at(static_cast<int>(key)));
        if (const auto* value = std::get_if<ast::IntegerLiteralExpr>(&payload)) {
            int number = 0;
            const auto parsed = std::from_chars(value->text.data(), value->text.data() + value->text.size(), number);
            if (parsed.ec != std::errc{} || parsed.ptr != value->text.data() + value->text.size()) refuse("integer literal outside runtime representation");
            origin.expression = static_cast<std::int64_t>(key);
            node("const.int", {{"out", out}, {"value", number}});
        } else if (const auto* value = std::get_if<ast::BoolLiteralExpr>(&payload)) {
            origin.expression = static_cast<std::int64_t>(key);
            node("const.bool", {{"out", out}, {"value", value->text == "true"}});
        } else if (const auto* value = std::get_if<ast::UnaryExpr>(&payload)) {
            const auto input = expression(*value->operand);
            origin.expression = static_cast<std::int64_t>(key);
            if (value->op == "+") return input;
            if (value->op == "not") node("bool.not", {{"in_path", input}, {"out", out}});
            else {
                const auto zero = out + "_zero";
                node("const.int", {{"out", zero}, {"value", 0}});
                node("int.sub", {{"lhs", zero}, {"rhs", input}, {"out", out}});
            }
        } else if (const auto* value = std::get_if<ast::BinaryExpr>(&payload)) {
            const auto left = expression(*value->left), right = expression(*value->right);
            const std::map<std::string, std::string> atoms = {{"+","int.add"},{"-","int.sub"},{"*","int.mul"},
                {"/","int.div"},{"%","int.mod"},{"<","int.lt"},{">","int.gt"},{"==","int.eq"}};
            const auto atom = atoms.find(value->op);
            if (atom == atoms.end()) refuse("runtime backend does not implement admitted operator " + value->op);
            const auto kind = value->op == "==" && model.expression_types.at(static_cast<int>(*value->left)) == Type::boolean ? "bool.eq" : atom->second;
            origin.expression = static_cast<std::int64_t>(key);
            node(kind, {{"lhs", left}, {"rhs", right}, {"out", out}});
        } else refuse("runtime adapter received unsupported expression");
        return out;
    };
    const auto& main = std::get<ast::MainBlock>(ast.declaration_pool.front());
    for (const auto key : ast.block_pool.at(*main.body).statements) {
        const auto& statement = ast.statement_pool.at(key);
        origin = {}; origin.statement = static_cast<std::int64_t>(key);
        origin.source = {source, "/statement_pool/" + std::to_string(key),
            static_cast<std::int64_t>(statement.location.line), static_cast<std::int64_t>(statement.location.column)};
        if (statement.location.line > 0 && statement.location.line <= lines.size()) {
            const auto& line = lines[statement.location.line - 1];
            origin.source.source = line.source_path; origin.source.line = static_cast<std::int64_t>(line.source_line);
        }
        if (const auto* print = std::get_if<ast::ExpressionStatement>(&statement.payload)) {
            const auto value = expression(print->expression);
            origin.expression = static_cast<std::int64_t>(print->expression);
            node(model.expression_types.at(static_cast<int>(print->expression)) == Type::integer ? "stdout.int_line" : "stdout.bool_line", {{"path", value}});
        } else {
            const auto& fact = model.facts.at(static_cast<int>(key));
            if (!fact.admitted()) refuse("runtime adapter requires admitted scalar facts");
            origin.declaration = fact.declaration_statement; origin.destination = fact.destination; origin.source = fact.origin;
            const auto value = expression(static_cast<std::size_t>(fact.expression));
            origin.expression = fact.expression;
            // Always copy into the established destination, including identifier
            // initializers. This bypasses the legacy ignored-return-path defect.
            node("record.copy", {{"from", value}, {"to", path(fact.destination)}});
        }
    }
    result.module.nodes.push_back({"sink", "scalar_halt", "halt.record"});
    result.module.wires.push_back({tail, {"scalar_halt", "in"}});
    return result;
}
} // namespace flowmini::scalar_source
