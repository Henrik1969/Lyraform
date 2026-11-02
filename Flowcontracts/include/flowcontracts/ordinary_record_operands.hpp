#pragma once

#include <flowcontracts/ordinary_record_types.hpp>
#include <cctype>
#include <algorithm>

namespace flowcontracts {
inline void validate_ordinary_record_operands(const json::Value& value) {
    using namespace json;
    const std::string path = "$.lowering_plan.record_operand";
    const auto& plan = object(value);
    std::map<Integer, const Object*> types;
    if (const auto* catalog = optional(plan, "ordinary_record_types")) {
        validate_ordinary_record_types(*catalog);
        for (const auto& type : array(required(object(*catalog), "types"), path)) {
            const auto& record = object(type);
            types.emplace(integer(required(record, "symbol_id"), path), &record);
        }
    }
    const auto fail = [&](const std::string& reason) { throw Error(path, reason); };
    const auto text = [&](const Object& item, const char* key) { return string(required(item, key), path); };
    const auto exact = [&](const Object& item, const std::set<std::string>& keys) {
        if (item.size() != keys.size()) fail("unexpected typed record operand fields");
        for (const auto& [key, field] : item) { (void)field; if (!keys.count(key)) fail("unknown typed record operand field"); }
    };
    const auto type_key = [&](const Value& type) {
        const auto& reference = object(type);
        return text(reference, "kind") == "record"
            ? "record:" + std::to_string(integer(required(reference, "symbol_id"), path)) : text(reference, "name");
    };
    struct Binding { Integer type; Integer function; const Value* operand; bool parameter = false; };
    std::map<Integer, Binding> bindings;
    if (const auto* functions = optional(plan, "functions")) for (const auto& value : array(*functions, path)) {
        const auto& function = object(value);
        const auto function_id = integer(required(function, "symbol_id"), path);
        for (const auto& parameter_value : array(required(function, "parameters"), path)) {
            const auto& parameter = object(parameter_value);
            const auto* record = optional(parameter, "record_symbol_id");
            if (record) {
                const auto symbol = integer(required(parameter, "symbol_id"), path);
                const auto type = integer(*record, path);
                if (symbol < 0 || !types.count(type) || !bindings.emplace(symbol, Binding{type, function_id, nullptr, true}).second)
                    fail("record parameter has invalid or duplicate identity");
            }
        }
    }
    const auto* operation_values = optional(plan, "operations");
    if (operation_values) for (const auto& value : array(*operation_values, path)) {
        const auto& operation = object(value);
        if (text(operation, "kind") != "value_definition") continue;
        const auto* operands = optional(operation, "operands");
        const auto* destination = optional(operation, "result_symbol_id");
        if (!operands || !destination || array(*operands, path).size() != 1) continue;
        const auto& operand_value = array(*operands, path).front();
        const auto& operand = object(operand_value);
        const auto kind = text(operand, "kind");
        Integer type = -1;
        if (kind == "record_construct" || kind == "record_value")
            type = integer(required(operand, "record_symbol_id"), path);
        else if (kind == "record_project" && text(operand, "type") == "ordinary_record")
            type = integer(required(object(required(operand, "result_type")), "symbol_id"), path);
        else if (kind == "call_result" && text(operand, "type") == "ordinary_record")
            type = integer(required(operand, "record_symbol_id"), path);
        if (type < 0) continue;
        const auto symbol = integer(*destination, path);
        if (symbol < 0 || !types.count(type) || !bindings.emplace(symbol, Binding{type,
            integer(required(operation, "function_symbol_id"), path), &operand_value}).second)
            fail("record value has invalid or duplicate definition identity");
    }
    Integer current_function = -1;
    std::set<Integer> active_bindings;
    std::function<std::string(const Value&, unsigned)> validate;
    validate = [&](const Value& value, unsigned depth) -> std::string {
        if (depth >= 256) fail("record operand depth bound exceeded");
        const auto& operand = object(value);
        if (integer(required(operand, "expression_id"), path) < 0) fail("record operand needs expression identity");
        const auto kind = text(operand, "kind");
        if (kind == "record_value") {
            exact(operand, {"kind", "expression_id", "type", "symbol_id", "record_symbol_id"});
            const auto symbol = integer(required(operand, "symbol_id"), path);
            const auto type = integer(required(operand, "record_symbol_id"), path);
            if (text(operand, "type") != "ordinary_record" || !bindings.count(symbol) ||
                bindings.at(symbol).type != type || bindings.at(symbol).function != current_function)
                fail("record read does not match a typed local definition in this function");
            if (bindings.at(symbol).parameter) return "record:" + std::to_string(type);
            if (!active_bindings.insert(symbol).second) fail("record value definitions are cyclic");
            const auto actual = validate(*bindings.at(symbol).operand, depth + 1);
            active_bindings.erase(symbol);
            if (actual != "record:" + std::to_string(type)) fail("record read definition has a different type");
            return actual;
        }
        if (kind == "identifier") {
            exact(operand, {"kind", "expression_id", "type", "symbol_id"});
            if (integer(required(operand, "symbol_id"), path) < 0 ||
                !lyraform::scalar::record_selected(lyraform::scalar::type(text(operand, "type"))))
                fail("record scalar identifier has invalid type or symbol identity");
            return text(operand, "type");
        }
        if (kind == "record_project") {
            exact(operand, {"kind", "expression_id", "type", "record_symbol_id", "result_type", "field", "base"});
            const auto record = integer(required(operand, "record_symbol_id"), path);
            if (!types.count(record) || validate(required(operand, "base"), depth + 1) != "record:" + std::to_string(record))
                fail("record projection base type differs from its declared identity");
            for (const auto& value : array(required(*types.at(record), "fields"), path)) {
                const auto& field = object(value);
                if (text(field, "name") != text(operand, "field")) continue;
                const auto& expected = required(field, "type");
                if (serialize(expected) != serialize(required(operand, "result_type")))
                    fail("record projection result type differs from declared field type");
                const auto result = type_key(expected);
                if (text(operand, "type") != (result.rfind("record:", 0) == 0 ? "ordinary_record" : result))
                    fail("record projection type category is inconsistent");
                return result;
            }
            fail("record projection field does not exist");
        }
        if (kind == "record_construct") {
            exact(operand, {"kind", "expression_id", "type", "record_symbol_id", "fields"});
            if (text(operand, "type") != "ordinary_record") fail("record construction has wrong type category");
            const auto symbol = integer(required(operand, "record_symbol_id"), path);
            if (!types.count(symbol)) fail("record construction type identity is absent");
            const auto& fields = array(required(operand, "fields"), path);
            const auto& declared = array(required(*types.at(symbol), "fields"), path);
            if (fields.size() != declared.size()) fail("record construction must supply every field exactly once");
            for (std::size_t index = 0; index < fields.size(); ++index) {
                const auto& field = object(fields[index]);
                const auto& declaration = object(declared[index]);
                exact(field, {"name", "operand"});
                if (text(field, "name") != text(declaration, "name"))
                    fail("record construction field identities must match declaration order");
                if (validate(required(field, "operand"), depth + 1) != type_key(required(declaration, "type")))
                    fail("record field operand type differs from its declaration");
            }
            return "record:" + std::to_string(symbol);
        }
        if (kind == "call_result") {
            exact(operand, {"kind", "expression_id", "type", "record_symbol_id", "callee_symbol_id", "arguments"});
            const auto symbol = integer(required(operand, "record_symbol_id"), path);
            if (text(operand, "type") != "ordinary_record" || !types.count(symbol))
                fail("record call result has no finite record identity");
            return "record:" + std::to_string(symbol);
        }
        const auto type = text(operand, "type");
        if (!lyraform::scalar::record_selected(lyraform::scalar::type(type))) fail("unsupported record scalar operand type");
        if (kind == "integer_literal" || kind == "bool_literal" || kind == "string_literal") {
            exact(operand, {"kind", "expression_id", "type", "value"});
            const auto literal = text(operand, "value");
            if (kind == "integer_literal") {
                if (type != "int" || literal.empty()) fail("invalid record integer operand");
                for (const unsigned char character : literal)
                    if (!std::isdigit(character)) fail("record integer literal must be unsigned decimal syntax; signs use unary operators");
            } else if (kind == "bool_literal") {
                if (type != "Bool" || (literal != "true" && literal != "false")) fail("invalid record boolean operand");
            } else if (type != "Text") fail("invalid record text operand");
            return type;
        }
        const auto operation = text(operand, "operator");
        auto result = lyraform::scalar::Type::invalid;
        if (kind == "unary") {
            exact(operand, {"kind", "expression_id", "type", "operator", "operand"});
            result = lyraform::scalar::unary(operation, lyraform::scalar::type(validate(required(operand, "operand"), depth + 1)));
        } else if (kind == "binary") {
            exact(operand, {"kind", "expression_id", "type", "operator", "left", "right"});
            result = lyraform::scalar::binary(operation,
                lyraform::scalar::type(validate(required(operand, "left"), depth + 1)),
                lyraform::scalar::type(validate(required(operand, "right"), depth + 1)));
        } else fail("unsupported record value operand");
        if (!lyraform::scalar::selected(result) || lyraform::scalar::name(result) != type)
            fail("record scalar operator type is invalid");
        return type;
    };
    const Object* current_operation = nullptr;
    std::set<const Object*> record_operations;
    std::function<void(const Value&, unsigned)> scan = [&](const Value& item, unsigned depth) {
        if (depth >= 256) fail("operand nesting exceeds inspection bound");
        if (const auto* object_value = std::get_if<Object>(&item)) {
            const auto* kind = optional(*object_value, "kind");
            if (kind && std::holds_alternative<std::string>(*kind) && (string(*kind, path) == "record_construct" ||
                string(*kind, path) == "record_value" || string(*kind, path) == "record_project")) {
                if (integer(required(plan, "version"), path) != 2)
                    fail("record construction operands require lowering-plan version 2");
                const auto* execution = optional(plan, "ordinary_record_execution");
                if (!execution) {
                    if (text(plan, "status") != "blocked") fail("record execution contract is absent");
                } else {
                    const auto& contract = object(*execution);
                    if (string(required(contract, "format"), path) != "lyraform.ordinary_record_execution" ||
                        integer(required(contract, "version"), path) != 1 ||
                        string(required(contract, "status"), path) != "ready" ||
                        string(required(contract, "bound"), path) != "finite_scalar_nested" ||
                        string(required(contract, "semantics"), path) != "independent_value" ||
                        string(required(contract, "layout"), path) != "private")
                        fail("record execution contract is inconsistent");
                    if (text(plan, "status") != "ready") fail("ready record execution requires a ready lowering plan");
                }
                record_operations.insert(current_operation);
                (void)validate(item, 0);
                return;
            }
            for (const auto& [key, child] : *object_value) { (void)key; scan(child, depth + 1); }
        } else if (const auto* array_value = std::get_if<Array>(&item))
            for (const auto& child : *array_value) scan(child, depth + 1);
    };
    if (operation_values) for (const auto& operation : array(*operation_values, path)) {
        current_operation = &object(operation);
        const auto* function = optional(*current_operation, "function_symbol_id");
        current_function = function ? integer(*function, path) : -1;
        scan(operation, 0);
    }
    if (record_operations.empty()) return;
    using Block = std::pair<Integer, Integer>;
    std::map<Block, std::vector<const Object*>> blocks;
    std::map<Block, Integer> starts;
    for (const auto& value : array(*operation_values, path)) {
        const auto& operation = object(value);
        const Block block{integer(required(operation, "function_symbol_id"), path),
                          integer(required(operation, "block_id"), path)};
        blocks[block].push_back(&operation);
        const auto statement = integer(required(operation, "statement_id"), path);
        if (text(operation, "kind") != "loop" && (!starts.count(block) || statement < starts.at(block)))
            starts[block] = statement;
    }
    const auto order = [&](const Object& operation) {
        if (text(operation, "kind") == "loop") {
            const Block body{integer(required(operation, "function_symbol_id"), path),
                             integer(required(operation, "body_block_id"), path)};
            if (starts.count(body)) return starts.at(body);
        }
        return integer(required(operation, "statement_id"), path);
    };
    for (auto& [block, operations] : blocks) {
        (void)block;
        std::stable_sort(operations.begin(), operations.end(), [&](const Object* left, const Object* right) {
            const auto a = order(*left), b = order(*right);
            return a == b ? integer(required(*left, "id"), path) < integer(required(*right, "id"), path) : a < b;
        });
    }
    std::function<void(const Value&, const std::set<Integer>&, unsigned)> reads;
    reads = [&](const Value& value, const std::set<Integer>& available, unsigned depth) {
        if (depth >= 256) fail("record read inspection depth exceeded");
        if (const auto* item = std::get_if<Object>(&value)) {
            const auto* kind = optional(*item, "kind");
            if (kind && std::holds_alternative<std::string>(*kind) && string(*kind, path) == "record_value")
                if (!available.count(integer(required(*item, "symbol_id"), path)))
                    fail("record read is not dominated by its local definition");
            for (const auto& [key, child] : *item) { (void)key; reads(child, available, depth + 1); }
        } else if (const auto* items = std::get_if<Array>(&value))
            for (const auto& item : *items) reads(item, available, depth + 1);
    };
    std::set<Block> visited;
    std::set<const Object*> inspected;
    std::function<void(Block, std::set<Integer>, unsigned)> walk;
    walk = [&](Block block, std::set<Integer> available, unsigned depth) {
        if (depth >= 256 || !visited.insert(block).second)
            fail("record control blocks are cyclic or have multiple owners");
        if (!blocks.count(block)) fail("record control references an absent block in this function");
        for (const auto* operation : blocks.at(block)) {
            inspected.insert(operation);
            if (const auto* operands = optional(*operation, "operands")) reads(*operands, available, 0);
            const auto kind = text(*operation, "kind");
            if (kind != "call") if (const auto* result = optional(*operation, "result_symbol_id")) {
                const auto symbol = integer(*result, path);
                if (bindings.count(symbol)) {
                    if (kind != "value_definition" || available.count(symbol))
                        fail("record local overwrite is not admitted");
                    available.insert(symbol);
                }
            }
            if (kind == "branch") {
                walk({block.first, integer(required(*operation, "then_block_id"), path)}, available, depth + 1);
                if (const auto* otherwise = optional(*operation, "else_block_id"))
                    if (integer(*otherwise, path) >= 0) walk({block.first, integer(*otherwise, path)}, available, depth + 1);
            } else if (kind == "loop") {
                walk({block.first, integer(required(*operation, "body_block_id"), path)}, available, depth + 1);
            }
            // Child-local definitions never become available in the enclosing
            // block or its sibling. Existing definitions remain available.
        }
    };
    std::set<Integer> record_functions;
    for (const auto* operation : record_operations)
        record_functions.insert(integer(required(*operation, "function_symbol_id"), path));
    for (const auto& value : array(required(plan, "functions"), path)) {
        const auto& function = object(value);
        const auto identity = integer(required(function, "symbol_id"), path);
        const auto body = integer(required(function, "body_block_id"), path);
        if (body >= 0 && record_functions.count(identity)) {
            std::set<Integer> parameters;
            for (const auto& [symbol, binding] : bindings)
                if (binding.parameter && binding.function == identity) parameters.insert(symbol);
            walk({identity, body}, std::move(parameters), 0);
        }
    }
    for (const auto* operation : record_operations)
        if (!inspected.count(operation)) fail("record operation is outside its function control tree");

}
}
