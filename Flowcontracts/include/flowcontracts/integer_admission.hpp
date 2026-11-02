#pragma once

#include <flowcontracts/json.hpp>
#include <charconv>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <string>

namespace flowcontracts {

// Shared integer range, conversion-intent and division admission under the
// owner-approved numeric rules. Unknown values remain unproved; no optional
// serialized flag can waive a required check.
inline void validate_integer_admission(const json::Value& value,
                                       std::string_view path = "$.lowering_plan") {
    using namespace json;
    const auto& plan = object(value, path);
    // Readiness is not proof: imported envelopes can contradict or omit it.
    // Every carried operation must satisfy the same execution safety checks.
    const auto str = [](const Object& item, std::string_view key) {
        const auto* value = optional(item, key);
        return value ? string(*value, key) : std::string{};
    };
    const auto number = [](const Object& item, std::string_view key) -> Integer {
        const auto* value = optional(item, key);
        return value ? integer(*value, key) : -1;
    };
    const auto minimum = [](const std::string& type) -> std::optional<std::int64_t> {
        if (type == "int" || type == "int32" || type == "c_int") return INT32_MIN;
        if (type == "int64" || type == "c_long" || type == "c_size_t" || type == "c_ulong") return INT64_MIN;
        return {};
    };
    struct Definition {
        const Value* operand = nullptr;
        Integer function = -1, block = -1, statement = -1;
        unsigned writes = 0;
    };
    std::map<Integer, Definition> definitions;
    const auto& operations = array(required(plan, "operations", path), path);
    for (const auto& raw : operations) {
        const auto& op = object(raw, path);
        const auto symbol = number(op, "result_symbol_id");
        if (symbol < 0) continue;
        auto& definition = definitions[symbol];
        ++definition.writes;
        const auto& operands = array(required(op, "operands", path), path);
        if (str(op, "kind") == "value_definition" && operands.size() == 1) {
            definition.operand = &operands.front();
            definition.function = number(op, "function_symbol_id");
            definition.block = number(op, "block_id");
            definition.statement = number(op, "statement_id");
        }
    }
    using Constant = std::optional<std::int64_t>;
    std::function<Constant(const Value&, const Object&, std::set<Integer>&, unsigned)> constant;
    constant = [&](const Value& raw, const Object& operation,
                   std::set<Integer>& active, unsigned depth) -> Constant {
        if (depth >= 256) return {};
        const auto& operand = object(raw, path);
        const auto kind = str(operand, "kind");
        const auto type = str(operand, "type");
        const auto low = minimum(type);
        if (!low) return {};
        const auto in_range = [&](std::int64_t n) {
            return *low == INT64_MIN || (n >= INT32_MIN && n <= INT32_MAX);
        };
        if (kind == "integer_literal") {
            const auto spelling = str(operand, "value");
            std::int64_t n = 0;
            const auto parsed = std::from_chars(spelling.data(), spelling.data() + spelling.size(), n);
            if (parsed.ec == std::errc{} && parsed.ptr == spelling.data() + spelling.size() && in_range(n)) return n;
            return {};
        }
        if (kind == "identifier") {
            const auto symbol = number(operand, "symbol_id");
            const auto found = definitions.find(symbol);
            if (found == definitions.end() || !active.insert(symbol).second) return {};
            const auto& d = found->second;
            Constant result;
            if (d.writes == 1 && d.operand && d.function >= 0 && d.block >= 0 &&
                d.function == number(operation, "function_symbol_id") &&
                d.block == number(operation, "block_id") &&
                d.statement < number(operation, "statement_id")) {
                // Evaluate at the definition's own position, never use a later
                // declaration to justify an earlier initializer.
                Object origin{{"function_symbol_id", d.function}, {"block_id", d.block},
                              {"statement_id", d.statement}};
                result = constant(*d.operand, origin, active, depth + 1);
            }
            active.erase(symbol);
            return result && in_range(*result) ? result : Constant{};
        }
        if (kind == "unary") {
            const auto n = constant(required(operand, "operand", path), operation, active, depth + 1);
            if (!n) return {};
            const auto op = str(operand, "operator");
            if (op == "+") return n;
            if (op == "-" && *n != INT64_MIN && in_range(-*n)) return -*n;
        }
        if (kind == "binary") {
            const auto left = constant(required(operand, "left", path), operation, active, depth + 1);
            const auto right = constant(required(operand, "right", path), operation, active, depth + 1);
            if (!left || !right) return {};
            const auto op = str(operand, "operator");
            std::int64_t result = 0;
            bool overflow = true;
            if (op == "+") overflow = __builtin_add_overflow(*left, *right, &result);
            else if (op == "-") overflow = __builtin_sub_overflow(*left, *right, &result);
            else if (op == "*") overflow = __builtin_mul_overflow(*left, *right, &result);
            else if (op == "div") {
                if (*right == 0 || (*left == INT64_MIN && *right == -1)) return {};
                result = *left / *right;
                overflow = false;
            }
            if (!overflow && in_range(result)) return result;
        }
        // Avoid inferring wrap, narrowing or overflow meaning. Only directly
        // established values and unary signs participate in this bounded proof.
        return {};
    };
    // Type identity, not equal machine width, determines compatibility.
    // Reconstruct expected boundaries from the carried declaration/catalog.
    std::map<Integer, std::string> symbol_types;
    std::map<Integer, const Object*> functions;
    if (const auto* catalog = optional(plan, "functions")) {
        for (const auto& raw : array(*catalog, path)) {
            const auto& function = object(raw, path);
            functions.emplace(number(function, "symbol_id"), &function);
            for (const auto& raw_parameter : array(required(function, "parameters", path), path)) {
                const auto& parameter = object(raw_parameter, path);
                symbol_types.emplace(number(parameter, "symbol_id"), str(parameter, "type"));
            }
        }
    }
    for (const auto& raw : operations) {
        const auto& op = object(raw, path);
        const auto& operands = array(required(op, "operands", path), path);
        if (str(op, "kind") == "value_definition" && operands.size() == 1)
            symbol_types.emplace(number(op, "result_symbol_id"), str(object(operands.front(), path), "type"));
    }
    const auto numeric = [&](const std::string& type) { return minimum(type).has_value(); };
    const auto require_identity = [&](const std::string& source, const std::string& destination,
                                      const std::string& where) {
        if ((!source.empty() && numeric(source)) || (!destination.empty() && numeric(destination)))
            if (source != destination)
                throw Error(where, "implicit integer conversion is forbidden; use an explicit typed initializer or an admitted explicit policy route");
    };
    // A direct true branch can prove a signed source nonnegative. Reuse the
    // actual condition and block identities; do not trust a serialized claim.
    const auto positive_branch = [&](const Value& input, const Object& operation) {
        const auto& operand = object(input, path);
        if (str(operand, "kind") != "identifier") return false;
        const auto symbol = number(operand, "symbol_id"), block = number(operation, "block_id");
        const bool single_entry_compatibility = number(plan, "version") == 1 && !optional(plan, "functions");
        if (symbol < 0 || block < 0 ||
            (number(operation, "function_symbol_id") < 0 && !single_entry_compatibility)) return false;
        const Object* branch = nullptr;
        unsigned incoming = 0;
        for (const auto& raw : operations) {
            const auto& candidate = object(raw, path);
            if (number(candidate, "result_symbol_id") == symbol &&
                number(candidate, "block_id") == block &&
                number(candidate, "statement_id") <= number(operation, "statement_id")) return false;
            if (str(candidate, "kind") == "branch") {
                if (number(candidate, "then_block_id") == block) { ++incoming; branch = &candidate; }
                if (number(candidate, "else_block_id") == block) ++incoming;
            }
            if (str(candidate, "kind") == "loop" && number(candidate, "body_block_id") == block) ++incoming;
        }
        if (incoming != 1 || !branch || number(*branch, "function_symbol_id") !=
            number(operation, "function_symbol_id")) return false;
        // A nested branch/loop before the conversion must not mutate the
        // guarded source. Conservatively reject writes anywhere below this
        // direct branch block; only later writes in the same block are safe.
        std::set<Integer> nested_blocks{block};
        bool expanded = true;
        while (expanded) {
            expanded = false;
            for (const auto& raw : operations) {
                const auto& candidate = object(raw, path);
                if (!nested_blocks.count(number(candidate, "block_id"))) continue;
                for (const auto* key : {"then_block_id", "else_block_id", "body_block_id"}) {
                    const auto child = number(candidate, key);
                    if (child >= 0 && nested_blocks.insert(child).second) expanded = true;
                }
            }
        }
        for (const auto& raw : operations) {
            const auto& candidate = object(raw, path);
            if (number(candidate, "result_symbol_id") == symbol &&
                number(candidate, "block_id") != block &&
                nested_blocks.count(number(candidate, "block_id"))) return false;
        }
        const auto& conditions = array(required(*branch, "operands", path), path);
        if (conditions.size() != 1) return false;
        const auto& condition = object(conditions.front(), path);
        if (str(condition, "kind") != "binary" ||
            (str(condition, "operator") != ">" && str(condition, "operator") != ">=")) return false;
        const auto& left = object(required(condition, "left", path), path);
        if (str(left, "kind") != "identifier" || number(left, "symbol_id") != symbol ||
            str(left, "type") != str(operand, "type")) return false;
        std::set<Integer> active;
        const auto zero = constant(required(condition, "right", path), *branch, active, 0);
        return zero && *zero == 0;
    };
    std::function<void(const Value&, const Object&, const std::string&, unsigned)> inspect;
    inspect = [&](const Value& raw, const Object& operation, const std::string& where, unsigned depth) {
        if (depth >= 256) throw Error(where, "integer admission depth limit exceeded");
        if (const auto* entries = std::get_if<Array>(&raw)) {
            for (std::size_t i = 0; i < entries->size(); ++i)
                inspect((*entries)[i], operation, where + "[" + std::to_string(i) + "]", depth + 1);
            return;
        }
        const auto* item = std::get_if<Object>(&raw);
        if (!item) return;
        const auto kind = str(*item, "kind");
        if (kind == "integer_literal" && numeric(str(*item, "type"))) {
            std::set<Integer> active;
            const auto n = constant(raw, operation, active, 0);
            if (!n || ((str(*item, "type") == "c_ulong" || str(*item, "type") == "c_size_t") && *n < 0))
                throw Error(where, "integer literal is outside its admitted source range; conversion cannot repair an invalid source value");
        }
        if ((str(*item, "type") == "int32" || str(*item, "type") == "int64") &&
            (kind == "integer_literal" || kind == "unary" ||
             (kind == "binary" && (str(*item, "operator") == "+" ||
              str(*item, "operator") == "-" || str(*item, "operator") == "*")))) {
            std::set<Integer> active;
            if (!constant(raw, operation, active, 0))
                throw Error(where, "explicit-width integer value/range is unproved; an admitted overflow route is required");
        }
        if (kind == "index" && str(*item, "intrinsic") == "list_index")
            require_identity(str(object(required(*item, "index", where), where), "type"), "c_int", where);
        if (kind == "call" && str(*item, "intrinsic") == "list_length")
            require_identity(str(*item, "type"), "c_int", where);
        if (kind == "identifier") {
            const auto found = symbol_types.find(number(*item, "symbol_id"));
            if (found != symbol_types.end())
                require_identity(str(*item, "type"), found->second, where);
        }
        if (kind == "conversion") {
            const auto source = str(*item, "from_type"), destination = str(*item, "type");
            if (numeric(source) || numeric(destination)) {
                const auto& input = required(*item, "operand", where);
                if (str(object(input, where), "type") != source)
                    throw Error(where, "integer conversion source type does not match its operand");
                if (str(*item, "conversion") == "numeric_promotion" &&
                    (source == "int" || source == "int32" || source == "int64" ||
                     source == "c_int" || source == "c_long" || source == "c_ulong" ||
                     source == "c_size_t") &&
                    (destination == "float" || destination == "float64")) {
                    // Mixed numeric arithmetic has an explicit carrier change
                    // in the lowering plan. It is not an integer conversion
                    // policy route and therefore does not require an
                    // initializer-bound proof.
                } else {
                    if (str(operation, "kind") != "value_definition" || depth != 1 ||
                        str(*item, "conversion") != "explicit_typed_initializer" ||
                        number(*item, "expression_id") != number(operation, "expression_id"))
                        throw Error(where, "integer conversion lacks explicit initializer intent at this boundary");
                    const auto from = minimum(source), to = minimum(destination);
                    const auto unsigned_type = [](const std::string& name) {
                        return name == "c_ulong" || name == "c_size_t";
                    };
                    if (!from || !to)
                        throw Error(where, "integer conversion range contract is unsupported");
                    if (unsigned_type(source) || unsigned_type(destination)) {
                        std::set<Integer> active;
                        const auto n = constant(input, operation, active, 0);
                        const bool known_fit = n && *n >= 0 && (*to == INT64_MIN || *n <= INT32_MAX);
                        const bool guarded_fit = !unsigned_type(source) && unsigned_type(destination) &&
                            positive_branch(input, operation);
                        if (!known_fit && !guarded_fit)
                            throw Error(where, "integer signedness conversion is not proved value-preserving; an explicit failure route is required");
                    } else if (*to > *from) {
                        std::set<Integer> active;
                        const auto n = constant(input, operation, active, 0);
                        if (!n || *n < INT32_MIN || *n > INT32_MAX)
                            throw Error(where, "integer narrowing is not proved value-preserving; an explicit failure route is required");
                    }
                }
            }
        }
        if (kind == "unary" && str(*item, "operator") != "not")
            require_identity(str(*item, "type"),
                str(object(required(*item, "operand", where), where), "type"), where);
        if (kind == "binary") {
            const auto& left = object(required(*item, "left", where), where);
            const auto& right = object(required(*item, "right", where), where);
            require_identity(str(left, "type"), str(right, "type"), where);
            const auto op = str(*item, "operator");
            if (op == "%")
                throw Error(where, "integer remainder is not admitted until its backend opcode contract is available");
            if (op == "+" || op == "-" || op == "*" || op == "div" || op == "%")
                require_identity(str(*item, "type"), str(left, "type"), where);
            if (op == "/" && str(*item, "type") != "float64")
                throw Error(where, "true division requires a float64 result carrier");
        }
        if (kind == "call_result") {
            const auto found = functions.find(number(*item, "callee_symbol_id"));
            if (found != functions.end()) {
                require_identity(str(*item, "type"), str(*found->second, "return_type"), where);
                const auto* raw_arguments = optional(*item, "arguments");
                const Array empty_arguments;
                const auto& arguments = raw_arguments ? array(*raw_arguments, where) : empty_arguments;
                const auto& parameters = array(required(*found->second, "parameters", where), where);
                for (std::size_t i = 0; i < arguments.size() && i < parameters.size(); ++i)
                    require_identity(str(object(arguments[i], where), "type"),
                        str(object(parameters[i], where), "type"), where);
            }
        }
        if (kind == "binary" && (str(*item, "operator") == "div" || str(*item, "operator") == "%")) {
            const auto& left = required(*item, "left", where);
            const auto& right = required(*item, "right", where);
            const auto& lhs = object(left, where);
            const auto& rhs = object(right, where);
            const auto low = minimum(str(lhs, "type"));
            std::set<Integer> active;
            const auto divisor = constant(right, operation, active, 0);
            const auto dividend = constant(left, operation, active, 0);
            if (!low || minimum(str(rhs, "type")) != low || !divisor || *divisor == 0 ||
                (*divisor == -1 && (!dividend || *dividend == *low)))
                throw Error(where, "integer division safety is unproved; explicit runtime failure routing is not admitted");
        }
        for (const auto& [key, child] : *item)
            inspect(child, operation, where + "." + key, depth + 1);
    };
    for (std::size_t i = 0; i < operations.size(); ++i) {
        const auto where = std::string(path) + ".operations[" + std::to_string(i) + "]";
        const auto& operation = object(operations[i], where);
        const auto& operands = array(required(operation, "operands", where), where);
        const auto kind = str(operation, "kind");
        if (kind == "assignment" && operands.size() == 1) {
            const auto found = symbol_types.find(number(operation, "result_symbol_id"));
            const auto source = str(object(operands.front(), where), "type");
            if (found != symbol_types.end()) require_identity(source, found->second, where);
            std::string target_type;
            if (const auto* raw_target = optional(operation, "target_fact")) {
                const auto& target = object(*raw_target, where);
                if (str(target, "kind") == "identifier") target_type = str(target, "base_type");
                else if (const auto* destination = optional(target, "destination_type"))
                    if (const auto* spelling = std::get_if<std::string>(destination)) target_type = *spelling;
            }
            if (!target_type.empty()) require_identity(source, target_type, where);
            else if (found == symbol_types.end() && numeric(source))
                throw Error(where, "integer assignment requires a declared destination type; conversion cannot be inferred");
        }
        if (kind == "return_value" && operands.size() == 1) {
            const auto found = functions.find(number(operation, "function_symbol_id"));
            if (found != functions.end())
                require_identity(str(object(operands.front(), where), "type"), str(*found->second, "return_type"), where);
            else if (numeric(str(object(operands.front(), where), "type")) &&
                     str(object(operands.front(), where), "type") != "c_int")
                throw Error(where, "integer return requires a declared result contract; compatibility lowering cannot infer a conversion");
        }
        // Calls placed directly into an existing destination are assignments
        // too. A separate typed-initializer projection may explicitly convert
        // the call result; otherwise equal ABI widths grant no permission.
        if (number(operation, "result_symbol_id") >= 0 &&
            (kind == "call" || kind == "external_call" || kind == "text_outcome")) {
            std::string result_type;
            if (kind == "call") {
                const auto found = functions.find(number(operation, "callee_symbol_id"));
                if (found != functions.end()) result_type = str(*found->second, "return_type");
            } else if (const auto* provider = optional(operation, "provider"))
                result_type = str(object(*provider, where), "return_type");
            const bool builtin_length = kind == "call" && str(operation, "callee") == "length" &&
                number(operation, "callee_symbol_id") < 0 && operands.size() == 1 &&
                str(object(operands.front(), where), "type") == "list<string>";
            if (builtin_length) result_type = "c_int";
            const auto destination = symbol_types.find(number(operation, "result_symbol_id"));
            std::string destination_type = destination == symbol_types.end() ? std::string{} : destination->second;
            if (destination_type.empty()) if (const auto* target = optional(operation, "target_fact"))
                destination_type = str(object(*target, where), "base_type");
            bool explicit_initializer = false;
            for (const auto& candidate_raw : operations) {
                const auto& candidate = object(candidate_raw, where);
                if (str(candidate, "kind") != "value_definition" ||
                    number(candidate, "result_symbol_id") != number(operation, "result_symbol_id") ||
                    number(candidate, "expression_id") != number(operation, "expression_id")) continue;
                const auto& projected = array(required(candidate, "operands", where), where);
                if (projected.size() != 1) continue;
                const auto& conversion = object(projected.front(), where);
                if (str(conversion, "kind") != "conversion" || str(conversion, "from_type") != result_type ||
                    str(conversion, "type") != destination_type) continue;
                const auto& source = object(required(conversion, "operand", where), where);
                explicit_initializer = (str(source, "kind") == "call_result" &&
                    number(source, "callee_symbol_id") == number(operation, "callee_symbol_id")) ||
                    (builtin_length && str(source, "kind") == "call" && str(source, "intrinsic") == "list_length");
            }
            if (!explicit_initializer) require_identity(result_type, destination_type, where);
        }
        if (kind == "call") {
            const auto found = functions.find(number(operation, "callee_symbol_id"));
            if (found != functions.end()) {
                const auto& parameters = array(required(*found->second, "parameters", where), where);
                for (std::size_t n = 0; n < operands.size() && n < parameters.size(); ++n)
                    require_identity(str(object(operands[n], where), "type"),
                        str(object(parameters[n], where), "type"), where);
            }
        }
        if (kind == "external_call" || kind == "text_outcome") {
            const auto* provider = optional(operation, "provider");
            if (provider) {
                const auto signature = str(object(*provider, where), "parameter_types");
                std::size_t start = 0;
                for (std::size_t n = 0; n < operands.size(); ++n) {
                    const auto end = signature.find(',', start);
                    require_identity(str(object(operands[n], where), "type"),
                        signature.substr(start, end == std::string::npos ? end : end - start), where);
                    if (end == std::string::npos) break;
                    start = end + 1;
                }
            }
        }
        inspect(required(operation, "operands", where), operation, where + ".operands", 0);
    }
}

} // namespace flowcontracts
