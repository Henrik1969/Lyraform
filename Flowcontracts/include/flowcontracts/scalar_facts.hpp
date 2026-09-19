#pragma once
#include <flowcontracts/parse_validity.hpp>
#include <flowcontracts/target_facts.hpp>

#include <flowcontracts/json.hpp>
#include <flowcontracts/scalar_semantics.hpp>
#include <map>

namespace flowcontracts {
inline json::Value scalar_fact(const lyraform::scalar::Fact& fact) {
    using namespace json;
    return Object{{"format", "lyraform.scalar_fact"}, {"version", Integer{1}},
        {"kind", fact.declaration ? "declaration" : "placement"},
        {"statement_id", fact.statement}, {"declaration_statement_id", fact.declaration_statement},
        {"expression_id", fact.expression}, {"destination_symbol_id", fact.destination},
        {"source_type", std::string(lyraform::scalar::name(fact.source_type))},
        {"destination_type", std::string(lyraform::scalar::name(fact.destination_type))},
        {"compatibility", fact.admitted() ? "admitted" : "refused"},
        {"provenance", Object{{"source", fact.origin.source}, {"ast_path", fact.origin.ast_path},
            {"line", fact.origin.line}, {"column", fact.origin.column}}}};
}

// Import checks use the same source type algebra. c_int on an integer literal
// is the existing plan encoding of a source int literal, not a new source alias.
inline lyraform::scalar::Type scalar_operand_type(const json::Value& value, const std::string& path,
    const std::map<json::Integer, lyraform::scalar::Type>& declarations, unsigned depth = 0) {
    using namespace json;
    using namespace lyraform::scalar;
    if (depth >= 256) throw Error(path, "scalar operand depth limit exceeded");
    const auto& operand = object(value, path);
    const auto kind = string(required(operand, "kind", path), path + ".kind");
    const auto spelling = string(required(operand, "type", path), path + ".type");
    if (kind == "integer_literal") {
        if (spelling != "int" && spelling != "c_int") throw Error(path, "scalar integer literal carrier changed");
        return Type::integer;
    }
    if (kind == "bool_literal") {
        if (spelling != "Bool" && spelling != "bool") throw Error(path, "scalar boolean literal carrier changed");
        const auto literal = string(required(operand, "value", path), path + ".value");
        if (literal != "true" && literal != "false") throw Error(path, "invalid boolean literal");
        return Type::boolean;
    }
    if (kind == "identifier") {
        const auto symbol = integer(required(operand, "symbol_id", path), path + ".symbol_id");
        if (symbol < 0) throw Error(path, "invalid scalar source symbol");
        const auto declaration = declarations.find(symbol);
        if (declaration != declarations.end() && declaration->second != type(spelling))
            throw Error(path, "scalar source declaration type mismatch");
        return type(spelling);
    }
    if (kind == "unary") return unary(string(required(operand, "operator", path), path + ".operator"),
        scalar_operand_type(required(operand, "operand", path), path + ".operand", declarations, depth + 1));
    if (kind == "binary") return binary(string(required(operand, "operator", path), path + ".operator"),
        scalar_operand_type(required(operand, "left", path), path + ".left", declarations, depth + 1),
        scalar_operand_type(required(operand, "right", path), path + ".right", declarations, depth + 1));
    return Type::outside_slice;
}

inline void validate_scalar_facts(const json::Value& value, std::string_view base) {
    require_plan_parse_validity(value);
    validate_target_facts(value);
    using namespace json;
    using namespace lyraform::scalar;
    const auto& plan = object(value, base);
    const auto& operations = array(required(plan, "operations", base), std::string(base) + ".operations");
    const auto string = [&](const Value& item) -> const std::string& { return json::string(item, base); };
    const auto integer = [&](const Value& item) { return json::integer(item, base); };
    std::map<Integer, std::pair<Integer, Type>> declarations;
    std::map<Integer, Type> declaration_types;
    for (const auto& value : operations) {
        const auto& operation = object(value);
        if (const auto* raw = optional(operation, "scalar_fact")) {
            const auto& fact = object(*raw);
            if (string(required(fact, "kind")) != "declaration") continue;
            const auto symbol = integer(required(fact, "destination_symbol_id"));
            declaration_types.emplace(symbol, type(string(required(fact, "destination_type"))));
            if (!declarations.emplace(symbol, std::pair{integer(required(fact, "statement_id")), type(string(required(fact, "destination_type")))}).second)
                throw Error(std::string(base), "duplicate scalar declaration fact");
        }
    }
    for (std::size_t index = 0; index < operations.size(); ++index) {
        const auto path = std::string(base) + ".operations[" + std::to_string(index) + "]";
        const auto& operation = object(operations[index], path);
        const auto* raw = optional(operation, "scalar_fact");
        if (!raw) continue; // Captured pre-Phase-2 artifacts make no scalar-proof claim.
        const auto& fact = object(*raw, path + ".scalar_fact");
        if (string(required(fact, "format")) != "lyraform.scalar_fact" || integer(required(fact, "version")) != 1)
            throw Error(path, "unsupported scalar fact contract");
        const auto kind = string(required(fact, "kind"));
        const auto operation_kind = string(required(operation, "kind"));
        if ((kind != "declaration" || operation_kind != "value_definition") &&
            (kind != "placement" || operation_kind != "assignment")) throw Error(path, "scalar fact operation kind mismatch");
        const auto source = type(string(required(fact, "source_type")));
        const auto destination = type(string(required(fact, "destination_type")));
        if (string(required(fact, "compatibility")) != "admitted" || !compatible(source, destination))
            throw Error(path, "scalar flow is not admitted");
        for (const auto key : {"statement_id", "expression_id"}) {
            const auto identity = integer(required(fact, key));
            if (identity < 0 || identity != integer(required(operation, key))) throw Error(path, "scalar fact source identity mismatch");
        }
        const auto symbol = integer(required(fact, "destination_symbol_id"));
        if (symbol < 0 || symbol != integer(required(operation, "result_symbol_id"))) throw Error(path, "scalar fact destination identity mismatch");
        const auto declaration = declarations.find(symbol);
        if (declaration == declarations.end() || declaration->second.first != integer(required(fact, "declaration_statement_id")) || declaration->second.second != destination)
            throw Error(path, "scalar destination declaration mismatch");
        const auto& operands = array(required(operation, "operands"), path + ".operands");
        if (operands.size() != 1 || scalar_operand_type(operands.front(), path + ".operands[0]", declaration_types) != source)
            throw Error(path, "scalar fact operand type mismatch");
        if (integer(required(object(operands.front()), "expression_id")) != integer(required(fact, "expression_id")))
            throw Error(path, "scalar fact operand identity mismatch");
        const auto& provenance = object(required(fact, "provenance"));
        if (string(required(provenance, "source")).empty() ||
            string(required(provenance, "ast_path")) != "/statement_pool/" + std::to_string(integer(required(fact, "statement_id"))) ||
            integer(required(provenance, "line")) < 1 || integer(required(provenance, "column")) < 1)
            throw Error(path, "invalid scalar fact provenance");
    }
}
} // namespace flowcontracts
