#pragma once
#include <flowcontracts/json.hpp>

namespace flowcontracts {
// This validates evidence, not source grammar. Missing historical evidence is
// compatible but supplies no canonical-source-completeness proof.
inline bool parse_validity_admits(const json::Value& evidence, const json::Value* ast = nullptr) {
    const auto& value = json::object(evidence, "$.parse_validity");
    const auto string = [&](const char* key) {
        return json::string(json::required(value, key), std::string("$.parse_validity.") + key);
    };
    const auto fail = [](const char* message) { throw json::Error("$.parse_validity", message); };
    if (value.size() != 7) fail("unexpected or missing parse-validity fields");
    if (string("format") != "lyraform.parse_validity" ||
        json::integer(json::required(value, "version"), "$.parse_validity.version") != 1)
        fail("unsupported parse-validity format/version");
    const auto state = string("state"), scope = string("scope"), coverage = string("coverage");
    const bool recovery = json::boolean(json::required(value, "recovery_used"), "$.parse_validity.recovery_used");
    const auto message = string("message");
    if (scope != "canonical_scalar" && scope != "scalar_with_compatibility_return" && scope != "compatibility")
        fail("unknown parse-validity scope");
    if (coverage != "complete" && coverage != "incomplete" && coverage != "unassessed") fail("unknown coverage status");
    if (state != "canonical_valid" && state != "outside_scope" && state != "recovered" && state != "incomplete" && state != "invalid")
        fail("unknown parse-validity state");
    if (state == "canonical_valid" && (scope != "canonical_scalar" || coverage != "complete" || recovery || !message.empty()))
        fail("contradictory canonical parse proof");
    if (state == "outside_scope" && (scope == "canonical_scalar" || recovery || !message.empty() ||
        (scope == "compatibility" ? coverage != "unassessed" : coverage != "complete")))
        fail("contradictory compatibility parse evidence");
    if (state == "recovered" && (!recovery || message.empty() || coverage == "complete")) fail("contradictory recovery evidence");
    if ((state == "invalid" || state == "incomplete") && (recovery || coverage != "incomplete" || message.empty()))
        fail("contradictory incomplete/invalid evidence");
    if (ast && (state == "canonical_valid" || state == "outside_scope")) {
        const auto& syntax = json::object(*ast, "$.ast");
        if (const auto* statements = json::optional(syntax, "statement_pool"))
            for (const auto& statement : json::array(*statements, "$.ast.statement_pool")) {
                const auto& entry = json::object(statement);
                if (json::string(json::required(entry, "kind"), "$.ast.statement.kind") == "unknown")
                    fail("admissible parse evidence contradicts recovery statement");
            }
        if (state == "canonical_valid") {
            const auto& declarations = json::array(json::required(syntax, "declaration_pool"), "$.ast.declaration_pool");
            if (declarations.size() != 1 ||
                json::string(json::required(json::object(declarations.front()), "kind"), "$.ast.declaration.kind") != "main_block")
                fail("canonical scalar proof contradicts declaration shape");
            for (const auto& statement : json::array(json::required(syntax, "statement_pool"), "$.ast.statement_pool")) {
                const auto& entry = json::object(statement);
                const auto kind = json::string(json::required(entry, "kind"), "$.ast.statement.kind");
                const auto& payload = json::object(json::required(entry, "payload"));
                if (kind == "placement") {
                    const auto& target = json::object(json::required(payload, "target"));
                    if (json::string(json::required(target, "kind"), "$.target.kind") != "identifier")
                        fail("canonical scalar proof contradicts target shape");
                } else if (kind == "let") {
                    const auto type = json::string(json::required(entry, "type"), "$.statement.type");
                    if ((type != "int" && type != "Bool") ||
                        std::holds_alternative<std::nullptr_t>(json::required(payload, "initializer_expression")))
                        fail("canonical scalar proof contradicts declaration initialization");
                } else if (kind != "expression") fail("canonical scalar proof contradicts statement shape");
            }
            for (const auto& expression : json::array(json::required(syntax, "expression_pool"), "$.ast.expression_pool")) {
                const auto kind = json::string(json::required(json::object(expression), "kind"), "$.expression.kind");
                if (kind != "identifier" && kind != "integer_literal" && kind != "bool_literal" && kind != "unary" && kind != "binary")
                    fail("canonical scalar proof contradicts expression shape");
            }
        }
    }
    return state == "canonical_valid" || state == "outside_scope";
}
inline void require_plan_parse_validity(const json::Value& plan) {
    if (const auto* evidence = json::optional(json::object(plan), "parse_validity"))
        if (!parse_validity_admits(*evidence))
            throw json::Error("$.lowering_plan.parse_validity", "parser refused execution admission");
}
} // namespace flowcontracts
