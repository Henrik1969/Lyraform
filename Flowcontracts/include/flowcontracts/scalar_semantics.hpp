#pragma once

#include <cstdint>
#include <string>
#include <string_view>

// Source-language rules shared by semantic analysis and the compatibility
// parser. No JSON, AST layout, backend carrier, or application policy lives here.
namespace lyraform::scalar {

enum class Type { integer, floating, boolean, text, other, outside_slice, invalid };

constexpr Type type(std::string_view name) noexcept {
    return name == "int" ? Type::integer :
        (name == "float" || name == "float64") ? Type::floating :
        name == "Bool" ? Type::boolean : name == "Text" ? Type::text : Type::outside_slice;
}
constexpr bool selected(Type value) noexcept {
    return value == Type::integer || value == Type::floating || value == Type::boolean;
}
constexpr bool numeric(Type value) noexcept { return value == Type::integer || value == Type::floating; }
constexpr bool record_selected(Type value) noexcept {
    return selected(value) || value == Type::text;
}
constexpr std::string_view name(Type value) noexcept {
    switch (value) {
        case Type::integer: return "int";
        case Type::floating: return "float64";
        case Type::boolean: return "Bool";
        case Type::text: return "Text";
        case Type::other: return "other";
        case Type::invalid: return "invalid";
        default: return "outside_slice";
    }
}
constexpr bool compatible(Type source, Type destination) noexcept {
    return selected(source) && source == destination;
}
constexpr Type unary(std::string_view op, Type operand) noexcept {
    if (operand == Type::outside_slice) return operand;
    if (op == "not" && operand == Type::boolean) return Type::boolean;
    if ((op == "-" || op == "+") && numeric(operand)) return operand;
    return Type::invalid;
}
constexpr Type binary(std::string_view op, Type left, Type right) noexcept {
    if (left == Type::invalid || right == Type::invalid) return Type::invalid;
    if (left == Type::outside_slice || right == Type::outside_slice) return Type::outside_slice;
    if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=")
        return numeric(left) && numeric(right) ? Type::boolean :
            (left == right && left == Type::boolean ? Type::boolean : Type::invalid);
    if (op == "div" || op == "%")
        return left == Type::integer && right == Type::integer ? Type::integer : Type::invalid;
    if (op == "+" || op == "-" || op == "*" || op == "/") {
        if (numeric(left) && numeric(right)) return left == Type::floating || right == Type::floating || op == "/" ? Type::floating : Type::integer;
    }
    return Type::invalid;
}

struct Origin {
    std::string source, ast_path;
    std::int64_t line = -1, column = -1;
};
struct Fact {
    bool declaration = false;
    std::int64_t statement = -1, declaration_statement = -1;
    std::int64_t expression = -1, destination = -1;
    Type source_type = Type::outside_slice, destination_type = Type::outside_slice;
    Origin origin;
    bool admitted() const noexcept {
        return statement >= 0 && declaration_statement >= 0 && expression >= 0 && destination >= 0 &&
               compatible(source_type, destination_type);
    }
};
} // namespace lyraform::scalar
