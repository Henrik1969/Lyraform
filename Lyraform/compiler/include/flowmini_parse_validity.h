#pragma once
#include "flowmini_ast.h"
#include "flowmini_lexer.h"
#include "flow_common.h"
#include <flowcontracts/scalar_semantics.hpp>
#include <functional>
#include <set>

// Parser-owned completeness policy. Runtime and staged consumers inspect its
// result; they do not reconstruct source completeness independently.
namespace flowmini::parse_validity {
namespace semantic = lyraform::scalar;
enum class Ownership { canonical, compatibility };

// Whole-source ownership is chosen by constructs, never by admission/lowering
// success. Unknown syntax in an otherwise scalar unit belongs to refusal.
inline Ownership classify(const ast::AstModule& module, bool compatibility_return = false) {
    if (module.source_unit.kind != ast::SourceUnitKind::Program ||
        !module.graph_nodes.empty() || !module.graph_wires.empty() ||
        !module.graph_policies.empty() || !module.graph_states.empty() ||
        !module.unsupported_graph_locations.empty()) return Ownership::compatibility;
    for (const auto& declaration : module.declaration_pool)
        if (!std::holds_alternative<ast::MainBlock>(declaration)) return Ownership::compatibility;
    for (const auto& statement : module.statement_pool) {
        if (const auto* declaration = std::get_if<ast::LetStatement>(&statement.payload)) {
            if (!semantic::selected(semantic::type(ast::type_ref_text(declaration->type))))
                return Ownership::compatibility;
        } else if (const auto* placement = std::get_if<ast::PlacementStatement>(&statement.payload)) {
            if (!std::holds_alternative<ast::IdentifierTarget>(placement->target)) return Ownership::compatibility;
        } else if (const auto* expression = std::get_if<ast::ExpressionStatement>(&statement.payload)) {
            if (!expression->print || expression->expression >= module.expression_pool.size() ||
                !std::holds_alternative<ast::IdentifierExpr>(module.expression_pool[expression->expression].payload))
                return Ownership::compatibility;
        } else if (compatibility_return && std::holds_alternative<ast::ReturnStatement>(statement.payload)) {
            continue;
        } else if (!std::holds_alternative<ast::UnknownStatement>(statement.payload)) {
            return Ownership::compatibility;
        }
    }
    for (const auto& expression : module.expression_pool) {
        if (std::holds_alternative<ast::CallExpr>(expression.payload) ||
            std::holds_alternative<ast::FieldAccessExpr>(expression.payload) ||
            std::holds_alternative<ast::IndexExpr>(expression.payload) ||
            std::holds_alternative<ast::ListLiteralExpr>(expression.payload) ||
            std::holds_alternative<ast::RecordLiteralExpr>(expression.payload)) return Ownership::compatibility;
    }
    return Ownership::canonical;
}

[[noreturn]] inline void refuse(const std::string& message) {
    throw flow::DiagnosticError{"canonical-scalar", message};
}

// The structural frontend currently has recovery/shell behavior. Before using
// a scalar AST for execution, compare its token projection with the input.
// This is a losslessness check, not another parser: it neither creates AST
// nodes nor interprets names/types. Parentheses are checked for balance; the
// canonical AST owns their expression grouping. No projected text is reparsed.
inline void require_source_coverage(const ast::AstModule& module, const std::vector<Token>& tokens, bool compatibility_return = false) {
    std::vector<std::string> actual, expected;
    std::vector<TokenKind> delimiters;
    std::vector<std::size_t> paren_starts;
    std::set<std::pair<std::size_t, std::size_t>> parentheses, expression_spans;
    unsigned main_parentheses = 0;
    for (const auto& token : tokens) {
        if (token.kind == TokenKind::LeftParen) paren_starts.push_back(actual.size());
        if (token.kind == TokenKind::LeftParen || token.kind == TokenKind::LeftBrace)
            delimiters.push_back(token.kind);
        if (token.kind == TokenKind::RightParen || token.kind == TokenKind::RightBrace) {
            const auto required = token.kind == TokenKind::RightParen ? TokenKind::LeftParen : TokenKind::LeftBrace;
            if (delimiters.empty() || delimiters.back() != required) refuse("unbalanced source delimiters");
            delimiters.pop_back();
            if (token.kind == TokenKind::RightParen) {
                if (paren_starts.back() == 3 && actual.size() == 3 && ++main_parentheses > 1)
                    refuse("multiple main signature delimiters are outside the scalar source slice");
                parentheses.emplace(paren_starts.back(), actual.size());
                paren_starts.pop_back();
            }
        }
        if (token.kind != TokenKind::Newline && token.kind != TokenKind::End &&
            token.kind != TokenKind::LeftParen && token.kind != TokenKind::RightParen)
            actual.push_back(token.text);
    }
    if (!delimiters.empty()) refuse("unclosed source delimiter");
    if (module.declaration_pool.size() != 1) refuse("canonical scalar source requires exactly one main block");
    const auto* main = std::get_if<ast::MainBlock>(&module.declaration_pool.front());
    if (!main || !main->body || !main->parameters.empty()) refuse("canonical scalar source requires a parameterless main block");
    if (module.block_pool.at(*main->body).statements.empty()) refuse("empty main is outside the migrated scalar subset");
    expected = {"program", module.source_unit.name, "main", "{"};
    std::function<void(std::size_t, unsigned)> expression;
    expression = [&](std::size_t id, unsigned depth) {
        const auto begin = expected.size();
        if (id >= module.expression_pool.size() || depth >= 256) refuse("invalid scalar expression identity/depth");
        const auto& value = module.expression_pool[id].payload;
        if (const auto* v = std::get_if<ast::IdentifierExpr>(&value)) expected.push_back(v->name);
        else if (const auto* v = std::get_if<ast::IntegerLiteralExpr>(&value)) expected.push_back(v->text);
        else if (const auto* v = std::get_if<ast::BoolLiteralExpr>(&value)) expected.push_back(v->text);
        else if (const auto* v = std::get_if<ast::UnaryExpr>(&value)) {
            expected.push_back(v->op);
            if (!v->operand) refuse("missing unary operand");
            expression(*v->operand, depth + 1);
        } else if (const auto* v = std::get_if<ast::BinaryExpr>(&value)) {
            if (!v->left || !v->right) refuse("missing binary operand");
            expression(*v->left, depth + 1); expected.push_back(v->op); expression(*v->right, depth + 1);
        } else refuse("unsupported scalar expression; no compatibility retry");
        expression_spans.emplace(begin, expected.size());
    };
    std::size_t previous_line = 0;
    for (const auto id : module.block_pool.at(*main->body).statements) {
        const auto& statement = module.statement_pool.at(id);
        if (statement.location.line == previous_line) refuse("multiple scalar statements on one line are outside this source slice");
        previous_line = statement.location.line;
        if (const auto* value = std::get_if<ast::LetStatement>(&statement.payload)) {
            if (!value->initializer_expression) refuse("uninitialized scalar declarations are outside the migrated subset");
            expected.push_back(value->name); expected.push_back(":"); expected.push_back(ast::type_ref_text(value->type));
            const auto begin = expected.size();
            expression(*value->initializer_expression, 0);
            if (!parentheses.count({begin, expected.size()})) refuse("scalar initializer delimiters are not represented completely");
        } else if (const auto* value = std::get_if<ast::PlacementStatement>(&statement.payload)) {
            expression(value->value_expression, 0); expected.push_back("->");
            expected.push_back(std::get<ast::IdentifierTarget>(value->target).name);
        } else if (const auto* value = std::get_if<ast::ExpressionStatement>(&statement.payload); value && value->print) {
            expected.push_back("print"); expression(value->expression, 0);
        } else if (const auto* value = std::get_if<ast::ReturnStatement>(&statement.payload); compatibility_return && value && value->value_expression) {
            if (value->source_form == ast::StatementSourceForm::KeywordReturn) expected.push_back("return");
            expression(*value->value_expression, 0);
            if (value->source_form == ast::StatementSourceForm::ArrowPlacement) {
                expected.push_back("->"); expected.push_back("return");
            }
        } else refuse("unrepresented scalar statement; no compatibility retry");
    }
    expected.push_back("}");
    if (expected != actual) refuse("source tokens are not completely represented by the canonical scalar AST");
    for (const auto& span : parentheses)
        if (span != std::pair<std::size_t, std::size_t>{3, 3} && !expression_spans.count(span))
            refuse("source parentheses are not represented by a scalar expression or main signature");
}


inline void finalize(ast::AstModule& module, const std::vector<Token>& tokens) {
    auto& result = module.parse_validity;
    for (const auto& statement : module.statement_pool) {
        if (const auto* unknown = std::get_if<ast::UnknownStatement>(&statement.payload)) {
            result.state = "recovered"; result.recovery = true;
            result.message = unknown->text.empty() ? "structural recovery statement" : unknown->text;
            return;
        }
    }
    const bool canonical = classify(module) == Ownership::canonical;
    const bool scalar_observation = classify(module, true) == Ownership::canonical;
    if (!canonical && !scalar_observation) return;
    // Empty main has an existing staged compatibility lowering, but was never
    // admitted by the Mission 03 scalar guard. Preserve that distinction:
    // no canonical proof, and direct canonical execution still refuses it.
    if (module.declaration_pool.size() == 1)
        if (const auto* main = std::get_if<ast::MainBlock>(&module.declaration_pool.front());
            main && main->body && module.block_pool.at(*main->body).statements.empty()) return;
    result.scope = canonical ? "canonical_scalar" : "scalar_with_compatibility_return";
    try {
        require_source_coverage(module, tokens, !canonical);
        result.coverage = "complete";
        result.state = canonical ? "canonical_valid" : "outside_scope";
    } catch (const flow::DiagnosticError& error) {
        result.coverage = "incomplete";
        result.state = "invalid";
        result.message = error.what();
        // Missing structure is distinct from unexpected additional structure.
        if (result.message.find("unclosed") != std::string::npos ||
            result.message.find("missing") != std::string::npos ||
            result.message.find("requires") != std::string::npos ||
            result.message.find("uninitialized") != std::string::npos ||
            result.message.find("empty main") != std::string::npos)
            result.state = "incomplete";
    }
}
inline void require_execution_validity(const ast::AstModule& module) {
    const auto& validity = module.parse_validity;
    if (validity.state != "canonical_valid" && validity.state != "outside_scope")
        refuse("FLOWMINI_PARSE_NOT_EXECUTABLE: " + validity.state + ": " + validity.message);
    if (classify(module) == Ownership::canonical && validity.state != "canonical_valid")
        refuse("canonical scalar source lacks parser validity");
}
} // namespace flowmini::parse_validity
