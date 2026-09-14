#include "flowmini_runtime.h"
#include "flowmini_testabi.h"

#include "flow_common.h"

#include <cctype>
#include <algorithm>
#include <cstring>
#include <dlfcn.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <new>
#include <sstream>
#include <utility>
#include <memory>

namespace flowmini {

namespace {

[[nodiscard]] std::int64_t parseInt64Text(const std::string& text) {
    try {
        std::size_t pos = 0;
        const long long result = std::stoll(text, &pos);

        while (pos < text.size()) {
            const unsigned char c = static_cast<unsigned char>(text[pos]);
            if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
                throw flow::DiagnosticError{"parse.int", "input contains trailing non-integer data"};
            }
            ++pos;
        }

        return static_cast<std::int64_t>(result);
    } catch (const std::invalid_argument&) {
        throw flow::DiagnosticError{"parse.int", "expected integer input"};
    } catch (const std::out_of_range&) {
        throw flow::DiagnosticError{"parse.int", "integer out of range"};
    }
}

[[nodiscard]] std::string endpointText(const Endpoint& endpoint) {
    if (endpoint.port.empty()) {
        return endpoint.node + ".<infer>";
    }
    return endpoint.node + "." + endpoint.port;
}

[[nodiscard]] std::string inferUniquePort(
    const std::map<std::string, std::string>& ports,
    const std::string& nodeId,
    const std::string& side
) {
    if (ports.size() != 1U) {
        std::string names;
        for (const auto& item : ports) {
            if (!names.empty()) { names += ", "; }
            names += item.first;
        }
        throw flow::DiagnosticError{
            "validator",
            "cannot infer " + side + " port for node '" + nodeId + "'; candidates: " + names
        };
    }
    return ports.begin()->first;
}

[[nodiscard]] std::string getPolicyString(const NodeConfig& config, const std::string& key, std::string fallback) {
    const auto it = config.policies.find(key);
    if (it == config.policies.end()) {
        return fallback;
    }
    if (const auto* value = std::get_if<std::string>(&it->second)) {
        return *value;
    }
    throw flow::DiagnosticError{config.kind, "policy '" + config.id + "." + key + "' expected string"};
}

[[nodiscard]] int getPolicyInt(const NodeConfig& config, const std::string& key, int fallback) {
    const auto it = config.policies.find(key);
    if (it == config.policies.end()) {
        return fallback;
    }
    if (const auto* value = std::get_if<int>(&it->second)) {
        return *value;
    }
    throw flow::DiagnosticError{config.kind, "policy '" + config.id + "." + key + "' expected int"};
}

[[maybe_unused]] [[nodiscard]] bool getPolicyBool(const NodeConfig& config, const std::string& key, bool fallback) {
    const auto it = config.policies.find(key);
    if (it == config.policies.end()) {
        return fallback;
    }
    if (const auto* value = std::get_if<bool>(&it->second)) {
        return *value;
    }
    throw flow::DiagnosticError{config.kind, "policy '" + config.id + "." + key + "' expected bool"};
}

[[nodiscard]] std::int64_t rhsInt(const RecordPayload& record, const NodeConfig& config, const std::string& stage) {
    const auto constIt = config.policies.find("rhs_const");
    if (constIt != config.policies.end()) {
        if (const auto* value = std::get_if<int>(&constIt->second)) {
            return *value;
        }
        throw flow::DiagnosticError{stage, "policy 'rhs_const' expected int"};
    }

    const std::string rhs = getPolicyString(config, "rhs", "");
    if (rhs.empty()) {
        throw flow::DiagnosticError{stage, "missing policy rhs or rhs_const"};
    }
    return getPathInt(record, rhs, stage);
}


[[nodiscard]] std::int64_t policyPathOrConstInt(
    const RecordPayload& record,
    const NodeConfig& config,
    const std::string& pathKey,
    const std::string& constKey,
    const std::string& fallbackPath,
    int fallbackConst,
    const std::string& stage
) {
    const auto constIt = config.policies.find(constKey);
    if (constIt != config.policies.end()) {
        if (const auto* value = std::get_if<int>(&constIt->second)) {
            return *value;
        }
        throw flow::DiagnosticError{stage, "policy '" + constKey + "' expected int"};
    }

    const std::string path = getPolicyString(config, pathKey, fallbackPath);
    if (path.empty()) {
        return fallbackConst;
    }
    return getPathInt(record, path, stage);
}

[[nodiscard]] std::size_t checkedIndex(std::int64_t value, std::size_t size, const std::string& stage) {
    if (value < 0) {
        throw flow::DiagnosticError{stage, "negative list index"};
    }
    const auto index = static_cast<std::size_t>(value);
    if (index >= size) {
        throw flow::DiagnosticError{stage, "list index out of range"};
    }
    return index;
}

[[nodiscard]] List parseIntListPolicy(const std::string& text, const std::string& stage) {
    List result;
    std::size_t i = 0;
    while (i < text.size()) {
        while (i < text.size() && (std::isspace(static_cast<unsigned char>(text[i])) || text[i] == ',')) {
            ++i;
        }
        if (i >= text.size()) {
            break;
        }
        try {
            std::size_t pos = 0;
            const long long value = std::stoll(text.substr(i), &pos);
            result.emplace_back(static_cast<std::int64_t>(value));
            i += pos;
        } catch (const std::invalid_argument&) {
            throw flow::DiagnosticError{stage, "invalid integer in list policy near: " + text.substr(i)};
        } catch (const std::out_of_range&) {
            throw flow::DiagnosticError{stage, "integer out of range in list policy near: " + text.substr(i)};
        }
        while (i < text.size() && std::isspace(static_cast<unsigned char>(text[i]))) {
            ++i;
        }
        if (i < text.size() && text[i] != ',') {
            throw flow::DiagnosticError{stage, "expected comma in list policy"};
        }
    }
    return result;
}

[[nodiscard]] std::vector<std::int64_t> parseIntVectorPolicy(const std::string& text, const std::string& stage) {
    std::vector<std::int64_t> result;
    std::size_t i = 0;
    while (i < text.size()) {
        while (i < text.size() && (std::isspace(static_cast<unsigned char>(text[i])) || text[i] == ',')) { ++i; }
        if (i >= text.size()) { break; }
        try {
            std::size_t pos = 0;
            const long long value = std::stoll(text.substr(i), &pos);
            result.push_back(static_cast<std::int64_t>(value));
            i += pos;
        } catch (const std::invalid_argument&) {
            throw flow::DiagnosticError{stage, "invalid integer in policy near: " + text.substr(i)};
        } catch (const std::out_of_range&) {
            throw flow::DiagnosticError{stage, "integer out of range in policy near: " + text.substr(i)};
        }
        while (i < text.size() && std::isspace(static_cast<unsigned char>(text[i]))) { ++i; }
        if (i < text.size() && text[i] != ',') { throw flow::DiagnosticError{stage, "expected comma in integer policy"}; }
    }
    return result;
}

[[nodiscard]] std::vector<std::size_t> parseShapePolicy(const std::string& text, const std::string& stage) {
    const auto raw = parseIntVectorPolicy(text, stage);
    if (raw.empty()) { throw flow::DiagnosticError{stage, "array shape may not be empty"}; }
    std::vector<std::size_t> shape;
    for (const auto dim : raw) {
        if (dim <= 0) { throw flow::DiagnosticError{stage, "array dimensions must be positive"}; }
        shape.push_back(static_cast<std::size_t>(dim));
    }
    return shape;
}

[[nodiscard]] std::size_t shapeProduct(const std::vector<std::size_t>& shape, const std::string& stage) {
    std::size_t product = 1;
    for (const auto dim : shape) {
        if (dim == 0 || product > (std::numeric_limits<std::size_t>::max() / dim)) {
            throw flow::DiagnosticError{stage, "array shape is too large"};
        }
        product *= dim;
    }
    return product;
}

[[nodiscard]] std::vector<std::string> splitPathList(const std::string& text) {
    std::vector<std::string> result;
    std::string current;
    for (char c : text) {
        if (c == ',') {
            if (!current.empty()) { result.push_back(current); current.clear(); }
            continue;
        }
        if (!std::isspace(static_cast<unsigned char>(c))) { current.push_back(c); }
    }
    if (!current.empty()) { result.push_back(current); }
    return result;
}

[[nodiscard]] std::vector<std::int64_t> readIndexPaths(const RecordPayload& record, const std::string& paths, const std::string& stage) {
    std::vector<std::int64_t> result;
    for (const auto& path : splitPathList(paths)) { result.push_back(getPathInt(record, path, stage)); }
    return result;
}

[[nodiscard]] std::size_t flattenIndex(const Array& array, const std::vector<std::int64_t>& indices, const std::string& stage) {
    if (indices.size() != array.shape.size()) {
        throw flow::DiagnosticError{stage, "array rank mismatch: expected " + std::to_string(array.shape.size()) + ", got " + std::to_string(indices.size())};
    }
    std::size_t flat = 0;
    for (std::size_t axis = 0; axis < array.shape.size(); ++axis) {
        const auto raw = indices[axis];
        if (raw < 0) { throw flow::DiagnosticError{stage, "negative array index at axis " + std::to_string(axis)}; }
        const auto idx = static_cast<std::size_t>(raw);
        if (idx >= array.shape[axis]) { throw flow::DiagnosticError{stage, "array index out of range at axis " + std::to_string(axis)}; }
        flat = flat * array.shape[axis] + idx;
    }
    return flat;
}

[[nodiscard]] Array parseIntArrayPolicy(const std::string& shapeText, const std::string& valuesText, const std::string& stage) {
    Array array;
    array.elementType = "int";
    array.shape = parseShapePolicy(shapeText, stage);
    const auto rawValues = parseIntVectorPolicy(valuesText, stage);
    const std::size_t expected = shapeProduct(array.shape, stage);
    if (rawValues.size() != expected) {
        throw flow::DiagnosticError{stage, "array initializer value count " + std::to_string(rawValues.size()) + " does not match shape size " + std::to_string(expected)};
    }
    for (const auto value : rawValues) { array.data.emplace_back(value); }
    return array;
}

[[nodiscard]] std::int64_t valueAsInt(const Value& value, const std::string& stage) {
    if (const auto* typed = std::get_if<std::int64_t>(&value.data)) {
        return *typed;
    }
    throw flow::DiagnosticError{stage, std::string{"expected Int value, got "} + valueTypeName(value)};
}

[[nodiscard]] const std::string& valueAsString(const Value& value, const std::string& stage) {
    if (const auto* typed = std::get_if<std::string>(&value.data)) {
        return *typed;
    }
    throw flow::DiagnosticError{stage, std::string{"expected String value, got "} + valueTypeName(value)};
}

[[nodiscard]] std::int64_t getPathAbiInt(const RecordPayload& record, const std::string& path, const std::string& stage) {
    return getPathInt(record, path, stage);
}

[[nodiscard]] const std::string& getPathAbiString(const RecordPayload& record, const std::string& path, const std::string& stage) {
    return valueAsString(getPathValue(record, path, stage), stage);
}

class ConfiguredNode : public INode {
public:
    explicit ConfiguredNode(NodeConfig config) : config_(std::move(config)) {}

protected:
    NodeConfig config_;
};


class StartRecordNode final : public INode {
public:
    std::vector<Route> run(MiniEnvelope env) override {
        env.payload = RecordPayload{};
        return {Route{"out", std::move(env)}};
    }
};

class RecordPortProbeNode final : public INode {
public:
    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, "record.port_probe");
        record.values["__flow_input_port"] = env.input_port;
        return {Route{"out", std::move(env)}};
    }
};

class StdinTextNode final : public INode {
public:
    std::vector<Route> run(MiniEnvelope env) override {
        if (env.ctx == nullptr) {
            throw flow::DiagnosticError{"stdin.text", "missing pipeline context"};
        }

        std::ostringstream buffer;
        buffer << std::cin.rdbuf();
        env.payload = TextPayload{buffer.str()};
        return {Route{"out", std::move(env)}};
    }
};

class ParseIntNode final : public INode {
public:
    std::vector<Route> run(MiniEnvelope env) override {
        const std::string text = requirePayloadConst<TextPayload>(env, "parse.int").value;
        env.payload = IntPayload{parseInt64Text(text)};
        return {Route{"out", std::move(env)}};
    }
};

class ParseIntToRecordNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        const std::string text = requirePayloadConst<TextPayload>(env, "parse.int.to_record").value;
        RecordPayload record;
        setPathInt(record, getPolicyString(config_, "out", "n"), parseInt64Text(text), "parse.int.to_record");
        env.payload = std::move(record);
        return {Route{"out", std::move(env)}};
    }
};

class ConstIntNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, "const.int");
        const std::string out = getPolicyString(config_, "out", "value");
        const int value = getPolicyInt(config_, "value", 0);
        setPathInt(record, out, value, "const.int");
        return {Route{"out", std::move(env)}};
    }
};

class ConstBoolNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, "const.bool");
        const std::string out = getPolicyString(config_, "out", "value");
        const bool value = getPolicyBool(config_, "value", false);
        setPathBool(record, out, value, "const.bool");
        return {Route{"out", std::move(env)}};
    }
};


class ConstTextNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, "const.text");
        const std::string out = getPolicyString(config_, "out", "value");
        const std::string value = getPolicyString(config_, "value", "");
        setPathValue(record, out, Value{value}, "const.text");
        return {Route{"out", std::move(env)}};
    }
};


class ListFromIntsNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, "list.from_ints");
        const std::string out = getPolicyString(config_, "out", "list");
        const std::string values = getPolicyString(config_, "values", "");
        setPathList(record, out, parseIntListPolicy(values, "list.from_ints"), "list.from_ints");
        return {Route{"out", std::move(env)}};
    }
};

class ListLengthNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, "list.length");
        const std::string listPath = getPolicyString(config_, "list", "list");
        const std::string out = getPolicyString(config_, "out", "length");
        setPathInt(record, out, static_cast<std::int64_t>(getPathList(record, listPath, "list.length").size()), "list.length");
        return {Route{"out", std::move(env)}};
    }
};

class ListGetNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, "list.get");
        const std::string listPath = getPolicyString(config_, "list", "list");
        const std::string out = getPolicyString(config_, "out", "value");
        const std::int64_t rawIndex = policyPathOrConstInt(record, config_, "index", "index_const", "i", 0, "list.get");
        const List list = getPathList(record, listPath, "list.get");
        const Value item = list.at(checkedIndex(rawIndex, list.size(), "list.get"));
        setPathValue(record, out, item, "list.get");
        return {Route{"out", std::move(env)}};
    }
};

class ListSetNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, "list.set");
        const std::string listPath = getPolicyString(config_, "list", "list");
        List& list = getPathListMutable(record, listPath, "list.set");
        const std::int64_t rawIndex = policyPathOrConstInt(record, config_, "index", "index_const", "i", 0, "list.set");
        const std::size_t index = checkedIndex(rawIndex, list.size(), "list.set");

        const auto valueConstIt = config_.policies.find("value_const");
        if (valueConstIt != config_.policies.end()) {
            if (const auto* value = std::get_if<int>(&valueConstIt->second)) {
                list[index] = Value{*value};
            } else {
                throw flow::DiagnosticError{"list.set", "policy 'value_const' expected int"};
            }
        } else {
            const std::string valuePath = getPolicyString(config_, "value", "value");
            list[index] = getPathValue(record, valuePath, "list.set");
        }
        return {Route{"out", std::move(env)}};
    }
};

class ListSwapNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, "list.swap");
        const std::string listPath = getPolicyString(config_, "list", "list");
        List& list = getPathListMutable(record, listPath, "list.swap");
        const std::int64_t rawA = policyPathOrConstInt(record, config_, "a", "a_const", "a", 0, "list.swap");
        const std::int64_t rawB = policyPathOrConstInt(record, config_, "b", "b_const", "b", 0, "list.swap");
        std::swap(list[checkedIndex(rawA, list.size(), "list.swap")], list[checkedIndex(rawB, list.size(), "list.swap")]);
        return {Route{"out", std::move(env)}};
    }
};

class ListCompareSwapNode final : public ConfiguredNode {
public:
    enum class Order { Asc, Desc };

    ListCompareSwapNode(NodeConfig config, Order order, std::string stage)
        : ConfiguredNode(std::move(config)), order_(order), stage_(std::move(stage)) {}

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, stage_);
        const std::string listPath = getPolicyString(config_, "list", "list");
        List& list = getPathListMutable(record, listPath, stage_);
        const std::int64_t rawA = policyPathOrConstInt(record, config_, "a", "a_const", "i", 0, stage_);
        const std::int64_t rawB = policyPathOrConstInt(record, config_, "b", "b_const", "", static_cast<int>(rawA + getPolicyInt(config_, "b_offset", 1)), stage_);
        const std::size_t a = checkedIndex(rawA, list.size(), stage_);
        const std::size_t b = checkedIndex(rawB, list.size(), stage_);
        const bool shouldSwap = (order_ == Order::Asc)
            ? (valueAsInt(list[a], stage_) > valueAsInt(list[b], stage_))
            : (valueAsInt(list[a], stage_) < valueAsInt(list[b], stage_));
        if (shouldSwap) {
            std::swap(list[a], list[b]);
        }
        return {Route{"out", std::move(env)}};
    }

private:
    Order order_;
    std::string stage_;
};

class ListPrintIntLinesNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        const auto* record = std::get_if<RecordPayload>(&env.payload);
        if (record == nullptr) {
            throw flow::DiagnosticError{"list.print_int_lines", "payload contract mismatch"};
        }
        const std::string listPath = getPolicyString(config_, "list", "list");
        const List list = getPathList(*record, listPath, "list.print_int_lines");
        for (const auto& item : list) {
            std::cout << valueAsInt(item, "list.print_int_lines") << '\n';
        }
        return {Route{"out", std::move(env)}};
    }
};

class IntBinaryOpNode final : public ConfiguredNode {
public:
    enum class Op { Add, Sub, Mul, Div, Mod };

    IntBinaryOpNode(NodeConfig config, Op op, std::string stage)
        : ConfiguredNode(std::move(config)), op_(op), stage_(std::move(stage)) {}

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, stage_);
        const std::int64_t lhs = getPathInt(record, getPolicyString(config_, "lhs", "lhs"), stage_);
        const std::int64_t rhs = rhsInt(record, config_, stage_);
        const std::string out = getPolicyString(config_, "out", "out");

        std::int64_t result = 0;
        switch (op_) {
            case Op::Add:
                if ((rhs > 0 && lhs > std::numeric_limits<std::int64_t>::max() - rhs) ||
                    (rhs < 0 && lhs < std::numeric_limits<std::int64_t>::min() - rhs)) {
                    throw flow::DiagnosticError{stage_, "integer addition overflow"};
                }
                result = lhs + rhs;
                break;
            case Op::Sub:
                if ((rhs < 0 && lhs > std::numeric_limits<std::int64_t>::max() + rhs) ||
                    (rhs > 0 && lhs < std::numeric_limits<std::int64_t>::min() + rhs)) {
                    throw flow::DiagnosticError{stage_, "integer subtraction overflow"};
                }
                result = lhs - rhs;
                break;
            case Op::Mul:
                if (lhs != 0 && rhs != 0) {
                    const auto max = std::numeric_limits<std::int64_t>::max();
                    const auto min = std::numeric_limits<std::int64_t>::min();
                    if ((lhs == -1 && rhs == min) || (rhs == -1 && lhs == min) ||
                        (lhs > 0 && rhs > 0 && lhs > max / rhs) ||
                        (lhs > 0 && rhs < 0 && rhs < min / lhs) ||
                        (lhs < 0 && rhs > 0 && lhs < min / rhs) ||
                        (lhs < 0 && rhs < 0 && lhs < max / rhs)) {
                        throw flow::DiagnosticError{stage_, "integer multiplication overflow"};
                    }
                }
                result = lhs * rhs;
                break;
            case Op::Div:
                if (rhs == 0) {
                    throw flow::DiagnosticError{stage_, "division by zero"};
                }
                if (lhs == std::numeric_limits<std::int64_t>::min() && rhs == -1) {
                    throw flow::DiagnosticError{stage_, "integer division overflow"};
                }
                result = lhs / rhs;
                break;
            case Op::Mod:
                if (rhs == 0) {
                    throw flow::DiagnosticError{stage_, "modulo by zero"};
                }
                result = lhs % rhs;
                break;
        }

        setPathInt(record, out, result, stage_);
        return {Route{"out", std::move(env)}};
    }

private:
    Op op_;
    std::string stage_;
};

class IntCompareNode final : public ConfiguredNode {
public:
    enum class Op { Eq, Lt, Gt };

    IntCompareNode(NodeConfig config, Op op, std::string stage)
        : ConfiguredNode(std::move(config)), op_(op), stage_(std::move(stage)) {}

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, stage_);
        const std::int64_t lhs = getPathInt(record, getPolicyString(config_, "lhs", "lhs"), stage_);
        const std::int64_t rhs = rhsInt(record, config_, stage_);
        const std::string out = getPolicyString(config_, "out", "cond");

        bool result = false;
        switch (op_) {
            case Op::Eq: result = (lhs == rhs); break;
            case Op::Lt: result = (lhs < rhs); break;
            case Op::Gt: result = (lhs > rhs); break;
        }

        setPathBool(record, out, result, stage_);
        return {Route{"out", std::move(env)}};
    }

private:
    Op op_;
    std::string stage_;
};

class BoolNotNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, "bool.not");
        const bool value = getPathBool(record, getPolicyString(config_, "in_path", "value"), "bool.not");
        const std::string out = getPolicyString(config_, "out", "out");
        setPathBool(record, out, !value, "bool.not");
        return {Route{"out", std::move(env)}};
    }
};

class BoolEqNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, "bool.eq");
        const bool lhs = getPathBool(record, getPolicyString(config_, "lhs", "lhs"), "bool.eq");
        const bool rhs = getPathBool(record, getPolicyString(config_, "rhs", "rhs"), "bool.eq");
        const std::string out = getPolicyString(config_, "out", "out");
        setPathBool(record, out, lhs == rhs, "bool.eq");
        return {Route{"out", std::move(env)}};
    }
};

class RouteBoolNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        const auto* record = std::get_if<RecordPayload>(&env.payload);
        if (record == nullptr) {
            throw flow::DiagnosticError{"route.bool", "payload contract mismatch"};
        }
        const bool value = getPathBool(*record, getPolicyString(config_, "path", "cond"), "route.bool");
        return {Route{value ? "true" : "false", std::move(env)}};
    }
};

class StdoutIntLineRecordNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        const auto* record = std::get_if<RecordPayload>(&env.payload);
        if (record == nullptr) {
            throw flow::DiagnosticError{"stdout.int_line", "payload contract mismatch"};
        }
        const std::int64_t value = getPathInt(*record, getPolicyString(config_, "path", "n"), "stdout.int_line");
        std::cout << value << '\n';
        return {Route{"out", std::move(env)}};
    }
};



class StdoutBoolLineRecordNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        const auto* record = std::get_if<RecordPayload>(&env.payload);
        if (record == nullptr) {
            throw flow::DiagnosticError{"stdout.bool_line", "payload contract mismatch"};
        }
        const bool value = getPathBool(*record, getPolicyString(config_, "path", "flag"), "stdout.bool_line");
        std::cout << (value ? "true" : "false") << '\n';
        return {Route{"out", std::move(env)}};
    }
};

class RecordNopNode final : public INode {
public:
    std::vector<Route> run(MiniEnvelope env) override {
        requirePayload<RecordPayload>(env, "record.nop");
        return {Route{"out", std::move(env)}};
    }
};

class RecordCopyNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, "record.copy");
        const std::string from = getPolicyString(config_, "from", "from");
        const std::string to = getPolicyString(config_, "to", "to");
        setPathValue(record, to, getPathValue(record, from, "record.copy"), "record.copy");
        return {Route{"out", std::move(env)}};
    }
};

class RecordSwapNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, "record.swap");
        const std::string a = getPolicyString(config_, "a", "a");
        const std::string b = getPolicyString(config_, "b", "b");

        Value av = getPathValue(record, a, "record.swap");
        Value bv = getPathValue(record, b, "record.swap");
        setPathValue(record, a, std::move(bv), "record.swap");
        setPathValue(record, b, std::move(av), "record.swap");

        return {Route{"out", std::move(env)}};
    }
};

class IntCompareSwapNode final : public ConfiguredNode {
public:
    enum class Order { Asc, Desc };

    IntCompareSwapNode(NodeConfig config, Order order, std::string stage)
        : ConfiguredNode(std::move(config)), order_(order), stage_(std::move(stage)) {}

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, stage_);
        const std::string a = getPolicyString(config_, "a", "a");
        const std::string b = getPolicyString(config_, "b", "b");
        const std::int64_t av = getPathInt(record, a, stage_);
        const std::int64_t bv = getPathInt(record, b, stage_);

        const bool shouldSwap = (order_ == Order::Asc) ? (av > bv) : (av < bv);
        if (shouldSwap) {
            setPathInt(record, a, bv, stage_);
            setPathInt(record, b, av, stage_);
        }

        return {Route{"out", std::move(env)}};
    }

private:
    Order order_;
    std::string stage_;
};



class ArrayFromIntsNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, "array.from_ints");
        const std::string out = getPolicyString(config_, "out", "array");
        const std::string shape = getPolicyString(config_, "shape", "");
        const std::string values = getPolicyString(config_, "values", "");
        setPathArray(record, out, parseIntArrayPolicy(shape, values, "array.from_ints"), "array.from_ints");
        return {Route{"out", std::move(env)}};
    }
};

class ArrayGetNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, "array.get");
        const std::string arrayPath = getPolicyString(config_, "array", "array");
        const std::string indices = getPolicyString(config_, "indices", "");
        const std::string out = getPolicyString(config_, "out", "value");
        const Array array = getPathArray(record, arrayPath, "array.get");
        const std::size_t flat = flattenIndex(array, readIndexPaths(record, indices, "array.get"), "array.get");
        setPathValue(record, out, array.data.at(flat), "array.get");
        return {Route{"out", std::move(env)}};
    }
};

class ArraySetNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, "array.set");
        const std::string arrayPath = getPolicyString(config_, "array", "array");
        const std::string indices = getPolicyString(config_, "indices", "");
        const std::string valuePath = getPolicyString(config_, "value", "value");
        Array& array = getPathArrayMutable(record, arrayPath, "array.set");
        if (array.elementType != "int") { throw flow::DiagnosticError{"array.set", "only array<int> is supported"}; }
        const Value value = getPathValue(record, valuePath, "array.set");
        if (!std::holds_alternative<std::int64_t>(value.data)) { throw flow::DiagnosticError{"array.set", "assigned value must be Int"}; }
        const std::size_t flat = flattenIndex(array, readIndexPaths(record, indices, "array.set"), "array.set");
        array.data.at(flat) = value;
        return {Route{"out", std::move(env)}};
    }
};

class ArrayPrintIntNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        const auto* record = std::get_if<RecordPayload>(&env.payload);
        if (record == nullptr) { throw flow::DiagnosticError{"array.print_int", "payload contract mismatch"}; }
        const std::string arrayPath = getPolicyString(config_, "array", "array");
        const Array array = getPathArray(*record, arrayPath, "array.print_int");
        if (array.elementType != "int") { throw flow::DiagnosticError{"array.print_int", "only array<int> is supported"}; }
        if (array.shape.empty()) { return {Route{"out", std::move(env)}}; }
        const std::size_t rowWidth = array.shape.back();
        for (std::size_t i = 0; i < array.data.size(); ++i) {
            if (i > 0 && i % rowWidth == 0) { std::cout << '\n'; }
            else if (i % rowWidth != 0) { std::cout << ' '; }
            std::cout << valueAsInt(array.data[i], "array.print_int");
        }
        std::cout << '\n';
        return {Route{"out", std::move(env)}};
    }
};

class RecordFromFieldsNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        auto& payload = requirePayload<RecordPayload>(env, "record.from_fields");
        const auto fields = splitPathList(getPolicyString(config_, "fields", ""));
        const auto fieldPaths = splitPathList(getPolicyString(config_, "field_paths", ""));
        const std::string out = getPolicyString(config_, "out", "out");
        if (fields.size() != fieldPaths.size()) {
            throw flow::DiagnosticError{"record.from_fields", "field/path count mismatch"};
        }
        Record value;
        for (std::size_t i = 0; i < fields.size(); ++i) {
            value[fields[i]] = getPathValue(payload, fieldPaths[i], "record.from_fields");
        }
        setPathValue(payload, out, Value{std::move(value)}, "record.from_fields");
        return {Route{"out", std::move(env)}};
    }
};

[[nodiscard]] const Value& getNestedRecordField(const Value& root, const std::vector<std::string>& fields, const std::string& stage) {
    const Value* current = &root;
    for (const auto& field : fields) {
        const auto* record = std::get_if<Record>(&current->data);
        if (record == nullptr) {
            throw flow::DiagnosticError{stage, "field access expected Record, got " + std::string(valueTypeName(*current))};
        }
        const auto it = record->find(field);
        if (it == record->end()) {
            throw flow::DiagnosticError{stage, "record field not found: " + field};
        }
        current = &it->second;
    }
    return *current;
}

Value& getNestedRecordFieldMutable(Value& root, const std::vector<std::string>& fields, const std::string& stage) {
    if (fields.empty()) { return root; }
    Value* current = &root;
    for (std::size_t i = 0; i + 1 < fields.size(); ++i) {
        auto* record = std::get_if<Record>(&current->data);
        if (record == nullptr) {
            throw flow::DiagnosticError{stage, "field assignment expected Record, got " + std::string(valueTypeName(*current))};
        }
        const auto it = record->find(fields[i]);
        if (it == record->end()) {
            throw flow::DiagnosticError{stage, "record field not found: " + fields[i]};
        }
        current = &it->second;
    }
    auto* record = std::get_if<Record>(&current->data);
    if (record == nullptr) {
        throw flow::DiagnosticError{stage, "field assignment expected Record, got " + std::string(valueTypeName(*current))};
    }
    const auto it = record->find(fields.back());
    if (it == record->end()) {
        throw flow::DiagnosticError{stage, "record field not found: " + fields.back()};
    }
    return it->second;
}

class RecordFieldGetNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        auto& payload = requirePayload<RecordPayload>(env, "record.field.get");
        const std::string recordPath = getPolicyString(config_, "record", "");
        const auto fields = splitPathList(getPolicyString(config_, "fields", ""));
        const std::string out = getPolicyString(config_, "out", "out");
        if (recordPath.empty() || fields.empty()) {
            throw flow::DiagnosticError{"record.field.get", "missing record or fields policy"};
        }
        const Value root = getPathValue(payload, recordPath, "record.field.get");
        setPathValue(payload, out, getNestedRecordField(root, fields, "record.field.get"), "record.field.get");
        return {Route{"out", std::move(env)}};
    }
};

class RecordFieldSetNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        auto& payload = requirePayload<RecordPayload>(env, "record.field.set");
        const std::string recordPath = getPolicyString(config_, "record", "");
        const auto fields = splitPathList(getPolicyString(config_, "fields", ""));
        const std::string valuePath = getPolicyString(config_, "value", "");
        if (recordPath.empty() || fields.empty() || valuePath.empty()) {
            throw flow::DiagnosticError{"record.field.set", "missing record, fields, or value policy"};
        }
        Value& root = getPathValueMutable(payload, recordPath, "record.field.set");
        Value& target = getNestedRecordFieldMutable(root, fields, "record.field.set");
        target = getPathValue(payload, valuePath, "record.field.set");
        return {Route{"out", std::move(env)}};
    }
};

class AbiStructFromFieldsNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, "abi.struct.from_fields");
        const std::string type = getPolicyString(config_, "type", "");
        const auto fields = splitPathList(getPolicyString(config_, "fields", ""));
        const auto fieldPaths = splitPathList(getPolicyString(config_, "field_paths", ""));
        const auto fieldTypes = splitPathList(getPolicyString(config_, "field_types", ""));
        const std::string out = getPolicyString(config_, "out", "out");
        if (type.empty()) { throw flow::DiagnosticError{"abi.struct.from_fields", "missing ABI struct type"}; }
        if (fields.size() != fieldPaths.size() || fields.size() != fieldTypes.size()) {
            throw flow::DiagnosticError{"abi.struct.from_fields", "field/path/type count mismatch"};
        }
        AbiStruct value;
        value.typeName = type;
        for (std::size_t i = 0; i < fields.size(); ++i) {
            if (!(fieldTypes[i] == "c_int" || fieldTypes[i] == "c_long" || fieldTypes[i] == "c_size_t")) {
                throw flow::DiagnosticError{"abi.struct.from_fields", "unsupported ABI struct field type: " + fieldTypes[i]};
            }
            value.fields[fields[i]] = Value{getPathInt(record, fieldPaths[i], "abi.struct.from_fields")};
        }
        setPathAbiStruct(record, out, std::move(value), "abi.struct.from_fields");
        return {Route{"out", std::move(env)}};
    }
};

class AbiCallNode final : public ConfiguredNode {
public:
    using ConfiguredNode::ConfiguredNode;

    std::vector<Route> run(MiniEnvelope env) override {
        auto& record = requirePayload<RecordPayload>(env, "abi.call");
        const std::string library = getPolicyString(config_, "library", "");
        const std::string symbol = getPolicyString(config_, "symbol", "");
        const std::string convention = getPolicyString(config_, "convention", "c");
        const std::string argPathsText = getPolicyString(config_, "arg_paths", "");
        const std::string argTypesText = getPolicyString(config_, "arg_types", "");
        const std::string returnType = getPolicyString(config_, "return_type", "c_int");
        const std::string out = getPolicyString(config_, "out", "out");

        if (library.empty() || symbol.empty()) {
            throw flow::DiagnosticError{"abi.call", "missing library or symbol policy"};
        }
        if (convention != "c") {
            throw flow::DiagnosticError{"abi.call", "only C calling convention is supported in flowmini v16"};
        }

        struct DlCloser { void operator()(void* handle) const { if (handle != nullptr) { dlclose(handle); } } };
        std::unique_ptr<void, DlCloser> handle{dlopen(library.c_str(), RTLD_LAZY)};
        if (!handle) {
            throw flow::DiagnosticError{"abi.call", "dlopen failed for '" + library + "': " + std::string(dlerror())};
        }
        dlerror();
        void* raw = dlsym(handle.get(), symbol.c_str());
        const char* err = dlerror();
        if (err != nullptr) {
            throw flow::DiagnosticError{"abi.call", "dlsym failed for '" + symbol + "': " + std::string(err)};
        }

        const auto argPaths = splitPathList(argPathsText);
        const auto argTypes = splitPathList(argTypesText);
        if (argPaths.size() != argTypes.size()) {
            throw flow::DiagnosticError{"abi.call", "arg path/type count mismatch"};
        }

        if (argTypes.size() == 1 && argTypes[0] == "c_string" && returnType == "c_size_t") {
            using Fn = std::size_t (*)(const char*);
            auto fn = reinterpret_cast<Fn>(raw);
            const std::string arg0 = getPathAbiString(record, argPaths[0], "abi.call");
            setPathInt(record, out, static_cast<std::int64_t>(fn(arg0.c_str())), "abi.call");
        } else if (argTypes.size() == 1 && argTypes[0] == "c_string" && returnType == "c_int") {
            using Fn = int (*)(const char*);
            auto fn = reinterpret_cast<Fn>(raw);
            const std::string arg0 = getPathAbiString(record, argPaths[0], "abi.call");
            setPathInt(record, out, static_cast<std::int64_t>(fn(arg0.c_str())), "abi.call");
        } else if (argTypes.size() == 1 && argTypes[0] == "c_int" && returnType == "c_int") {
            using Fn = int (*)(int);
            auto fn = reinterpret_cast<Fn>(raw);
            const auto arg0 = static_cast<int>(getPathAbiInt(record, argPaths[0], "abi.call"));
            setPathInt(record, out, static_cast<std::int64_t>(fn(arg0)), "abi.call");
        } else if (argTypes.size() == 1 && argTypes[0] == "c_long" && returnType == "c_long") {
            using Fn = long (*)(long);
            auto fn = reinterpret_cast<Fn>(raw);
            const auto arg0 = static_cast<long>(getPathAbiInt(record, argPaths[0], "abi.call"));
            setPathInt(record, out, static_cast<std::int64_t>(fn(arg0)), "abi.call");
        } else if (argTypes.size() == 1 && argTypes[0] == "c_ulong" && returnType == "c_ulong") {
            using Fn = unsigned long (*)(unsigned long);
            auto fn = reinterpret_cast<Fn>(raw);
            const auto arg0 = static_cast<unsigned long>(getPathAbiInt(record, argPaths[0], "abi.call"));
            setPathInt(record, out, static_cast<std::int64_t>(fn(arg0)), "abi.call");
        } else if (argTypes.size() == 1 && argTypes[0] == "Point" && returnType == "c_int") {
            using Fn = int (*)(FlowminiTestAbiPoint);
            auto fn = reinterpret_cast<Fn>(raw);
            const AbiStruct arg = getPathAbiStruct(record, argPaths[0], "abi.call");
            if (arg.typeName != "Point") { throw flow::DiagnosticError{"abi.call", "expected ABI struct Point, got " + arg.typeName}; }
            const auto xIt = arg.fields.find("x");
            const auto yIt = arg.fields.find("y");
            if (xIt == arg.fields.end() || yIt == arg.fields.end()) { throw flow::DiagnosticError{"abi.call", "Point argument missing x or y field"}; }
            const auto x = std::get_if<std::int64_t>(&xIt->second.data);
            const auto y = std::get_if<std::int64_t>(&yIt->second.data);
            if (x == nullptr || y == nullptr) { throw flow::DiagnosticError{"abi.call", "Point fields must be integer ABI values"}; }
            FlowminiTestAbiPoint p{static_cast<int>(*x), static_cast<int>(*y)};
            setPathInt(record, out, static_cast<std::int64_t>(fn(p)), "abi.call");
        } else {
            throw flow::DiagnosticError{"abi.call", "unsupported v17 ABI signature: " + returnType + "(" + argTypesText + ")"};
        }

        return {Route{"out", std::move(env)}};
    }
};

class HaltNode final : public INode {
public:
    std::vector<Route> run(MiniEnvelope) override {
        return {};
    }
};

} // namespace

void AtomRegistry::registerAtom(AtomContract contract, Factory factory) {
    const std::string kind = contract.kind;
    contracts_[kind] = std::move(contract);
    factories_[kind] = std::move(factory);
}

const AtomContract& AtomRegistry::contractFor(const std::string& kind) const {
    const auto it = contracts_.find(kind);
    if (it == contracts_.end()) {
        throw flow::DiagnosticError{"validator", "unknown atom kind: " + kind};
    }
    return it->second;
}

std::unique_ptr<INode> AtomRegistry::create(NodeConfig config) const {
    const auto it = factories_.find(config.kind);
    if (it == factories_.end()) {
        throw flow::DiagnosticError{"runtime", "no factory for atom kind: " + config.kind};
    }
    return it->second(std::move(config));
}

bool AtomRegistry::contains(const std::string& kind) const {
    return contracts_.find(kind) != contracts_.end();
}

AtomRegistry makeCoreAtomRegistry() {
    AtomRegistry registry;

    registry.registerAtom(AtomContract{"start.record", {{"in", "Unit"}}, {{"out", "Record"}}, {}},
        [](NodeConfig) { return std::make_unique<StartRecordNode>(); });

    registry.registerAtom(AtomContract{"stdin.text", {{"in", "Unit"}}, {{"out", "Text"}}, {"stdin.read"}},
        [](NodeConfig) { return std::make_unique<StdinTextNode>(); });



    registry.registerAtom(AtomContract{"parse.int", {{"in", "Text"}}, {{"out", "Int"}}, {}},
        [](NodeConfig) { return std::make_unique<ParseIntNode>(); });

    registry.registerAtom(AtomContract{"parse.int.to_record", {{"in", "Text"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<ParseIntToRecordNode>(std::move(config)); });

    registry.registerAtom(AtomContract{"const.int", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<ConstIntNode>(std::move(config)); });
    registry.registerAtom(AtomContract{"const.bool", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<ConstBoolNode>(std::move(config)); });
    registry.registerAtom(AtomContract{"const.text", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<ConstTextNode>(std::move(config)); });


    registry.registerAtom(AtomContract{"list.from_ints", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<ListFromIntsNode>(std::move(config)); });
    registry.registerAtom(AtomContract{"list.length", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<ListLengthNode>(std::move(config)); });
    registry.registerAtom(AtomContract{"list.get", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<ListGetNode>(std::move(config)); });
    registry.registerAtom(AtomContract{"list.set", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<ListSetNode>(std::move(config)); });
    registry.registerAtom(AtomContract{"list.swap", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<ListSwapNode>(std::move(config)); });
    registry.registerAtom(AtomContract{"list.compare_swap_asc", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<ListCompareSwapNode>(std::move(config), ListCompareSwapNode::Order::Asc, "list.compare_swap_asc"); });
    registry.registerAtom(AtomContract{"list.compare_swap_desc", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<ListCompareSwapNode>(std::move(config), ListCompareSwapNode::Order::Desc, "list.compare_swap_desc"); });
    registry.registerAtom(AtomContract{"list.print_int_lines", {{"in", "Record"}}, {{"out", "Record"}}, {"stdout.write"}},
        [](NodeConfig config) { return std::make_unique<ListPrintIntLinesNode>(std::move(config)); });


    registry.registerAtom(AtomContract{"array.from_ints", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<ArrayFromIntsNode>(std::move(config)); });
    registry.registerAtom(AtomContract{"array.get", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<ArrayGetNode>(std::move(config)); });
    registry.registerAtom(AtomContract{"array.set", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<ArraySetNode>(std::move(config)); });
    registry.registerAtom(AtomContract{"array.print_int", {{"in", "Record"}}, {{"out", "Record"}}, {"stdout.write"}},
        [](NodeConfig config) { return std::make_unique<ArrayPrintIntNode>(std::move(config)); });

    registry.registerAtom(AtomContract{"record.from_fields", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<RecordFromFieldsNode>(std::move(config)); });
    registry.registerAtom(AtomContract{"record.field.get", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<RecordFieldGetNode>(std::move(config)); });
    registry.registerAtom(AtomContract{"record.field.set", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<RecordFieldSetNode>(std::move(config)); });

    registry.registerAtom(AtomContract{"abi.struct.from_fields", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<AbiStructFromFieldsNode>(std::move(config)); });
    registry.registerAtom(AtomContract{"abi.call", {{"in", "Record"}}, {{"out", "Record"}}, {"external.abi"}},
        [](NodeConfig config) { return std::make_unique<AbiCallNode>(std::move(config)); });

    registry.registerAtom(AtomContract{"int.add", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<IntBinaryOpNode>(std::move(config), IntBinaryOpNode::Op::Add, "int.add"); });
    registry.registerAtom(AtomContract{"int.sub", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<IntBinaryOpNode>(std::move(config), IntBinaryOpNode::Op::Sub, "int.sub"); });
    registry.registerAtom(AtomContract{"int.mul", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<IntBinaryOpNode>(std::move(config), IntBinaryOpNode::Op::Mul, "int.mul"); });
    registry.registerAtom(AtomContract{"int.div", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<IntBinaryOpNode>(std::move(config), IntBinaryOpNode::Op::Div, "int.div"); });
    registry.registerAtom(AtomContract{"int.mod", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<IntBinaryOpNode>(std::move(config), IntBinaryOpNode::Op::Mod, "int.mod"); });

    registry.registerAtom(AtomContract{"int.eq", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<IntCompareNode>(std::move(config), IntCompareNode::Op::Eq, "int.eq"); });
    registry.registerAtom(AtomContract{"int.lt", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<IntCompareNode>(std::move(config), IntCompareNode::Op::Lt, "int.lt"); });
    registry.registerAtom(AtomContract{"int.gt", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<IntCompareNode>(std::move(config), IntCompareNode::Op::Gt, "int.gt"); });

    registry.registerAtom(AtomContract{"bool.not", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<BoolNotNode>(std::move(config)); });
    registry.registerAtom(AtomContract{"bool.eq", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<BoolEqNode>(std::move(config)); });

    registry.registerAtom(AtomContract{"route.bool", {{"in", "Record"}}, {{"true", "Record"}, {"false", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<RouteBoolNode>(std::move(config)); });

    registry.registerAtom(AtomContract{"stdout.int_line", {{"in", "Record"}}, {{"out", "Record"}}, {"stdout.write"}},
        [](NodeConfig config) { return std::make_unique<StdoutIntLineRecordNode>(std::move(config)); });
    registry.registerAtom(AtomContract{"stdout.bool_line", {{"in", "Record"}}, {{"out", "Record"}}, {"stdout.write"}},
        [](NodeConfig config) { return std::make_unique<StdoutBoolLineRecordNode>(std::move(config)); });


    registry.registerAtom(AtomContract{"record.nop", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig) { return std::make_unique<RecordNopNode>(); });
    registry.registerAtom(AtomContract{"record.port_probe", {{"left", "Record"}, {"right", "Record"}}, {{"out", "Record"}}, {"diagnostic.routing"}, {"left", "right"}},
        [](NodeConfig) { return std::make_unique<RecordPortProbeNode>(); });

    registry.registerAtom(AtomContract{"record.copy", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<RecordCopyNode>(std::move(config)); });
    registry.registerAtom(AtomContract{"record.swap", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<RecordSwapNode>(std::move(config)); });

    registry.registerAtom(AtomContract{"int.compare_swap_asc", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<IntCompareSwapNode>(std::move(config), IntCompareSwapNode::Order::Asc, "int.compare_swap_asc"); });
    registry.registerAtom(AtomContract{"int.compare_swap_desc", {{"in", "Record"}}, {{"out", "Record"}}, {}},
        [](NodeConfig config) { return std::make_unique<IntCompareSwapNode>(std::move(config), IntCompareSwapNode::Order::Desc, "int.compare_swap_desc"); });


        registry.registerAtom(AtomContract{"halt.record", {{"in", "Record"}}, {}, {}, {}, true},
        [](NodeConfig) { return std::make_unique<HaltNode>(); });

    return registry;
}

void RuntimeGraph::addNode(std::string id, std::unique_ptr<INode> node) {
    nodes_[std::move(id)] = std::move(node);
}

void RuntimeGraph::connect(std::string fromNode, std::string fromPort, std::string toNode, std::string toPort, std::string wireId) {
    wires_[wireKey(fromNode, fromPort)].push_back(Connection{std::move(toNode), std::move(toPort), std::move(wireId)});
}

void RuntimeGraph::startAt(const std::string& nodeId, MiniEnvelope env) {
    queue_.push(Pending{nodeId, std::move(env)});

    while (!queue_.empty()) {
        Pending pending = std::move(queue_.front());
        queue_.pop();

        const auto it = nodes_.find(pending.nodeId);
        if (it == nodes_.end()) {
            throw flow::DiagnosticError{"runtime", "unknown node: " + pending.nodeId};
        }

        trace("enter " + pending.nodeId + " with " + payloadTypeName(pending.envelope.payload), pending.envelope);

        auto* const failureContext = pending.envelope.ctx;
        const auto failureInputPort = pending.envelope.input_port;
        const auto failureWire = pending.envelope.wire_id;
        const auto failureSignal = pending.envelope.signal_id;
        const auto failureDelivery = pending.envelope.delivery_id;
        std::vector<Route> routes;
        try {
            routes = it->second->run(std::move(pending.envelope));
        } catch (const flow::DiagnosticError& error) {
            if (failureContext != nullptr) {
                failureContext->diagnostics.push_back({
                    flow::Severity::Error,
                    "runtime",
                    "failure at " + pending.nodeId + "." + failureInputPort +
                        " via " + failureWire + " " + failureSignal +
                        ": " + error.what() + " [" + failureDelivery + "]"
                });
            }
            throw;
        }
        for (auto& route : routes) {
            deliver(pending.nodeId, route.port, std::move(route.envelope));
        }
    }
}

std::string RuntimeGraph::wireKey(const std::string& node, const std::string& port) {
    return node + "." + port;
}

void RuntimeGraph::deliver(const std::string& fromNode, const std::string& fromPort, MiniEnvelope env) {
    if (env.ctx == nullptr) {
        throw flow::DiagnosticError{"runtime", "missing pipeline context"};
    }

    const std::string key = wireKey(fromNode, fromPort);
    env.signal_id = identity_scope_ + "signal:" + std::to_string(next_signal_id_++);
    const auto it = wires_.find(key);

    if (it == wires_.end()) {
        env.ctx->diagnostics.push_back({
            flow::Severity::Warning,
            "runtime",
            "no wire connected from " + key + "; dropping envelope"
        });
        return;
    }

    for (const auto& target : it->second) {
        MiniEnvelope routed = env;
        routed.input_port = target.port;
        routed.wire_id = target.wire_id;
        routed.delivery_id = identity_scope_ + "delivery:" + std::to_string(next_delivery_id_++);
        trace("route " + key + " => " + target.node + "." + target.port + " [" + target.wire_id + "] [" + routed.signal_id + "] [" + routed.delivery_id + "]", routed);
        queue_.push(Pending{target.node, std::move(routed)});
    }
}

void RuntimeGraph::trace(const std::string& message, MiniEnvelope& env) const {
    if (env.ctx == nullptr) {
        return;
    }

    const bool enabled = env.ctx->policies.getBoolOrDefault(
        "runtime.trace",
        false,
        env.ctx->diagnostics,
        "runtime"
    );

    if (enabled) {
        env.ctx->diagnostics.push_back({flow::Severity::Info, "runtime", message});
    }
}

namespace {
class ReceiverResult final : public INode {
public:
    ReceiverResult(const ReceiverFrame& frame, std::vector<Value>& results)
        : frame_(frame), results_(results) {}
    std::vector<Route> run(MiniEnvelope env) override {
        results_.push_back(getPathValue(requirePayload<RecordPayload>(env, "receiver"), frame_.result_path, "receiver"));
        return {};
    }
private:
    const ReceiverFrame& frame_;
    std::vector<Value>& results_;
};

class SourceReceiver final : public INode {
public:
    SourceReceiver(std::shared_ptr<const ReceiverFrame> frame, AtomRegistry registry)
        : frame_(std::move(frame)), registry_(std::move(registry)) {
        // Validate the body even when no delivery will reach this receiver.
        (void)buildCheckedGraph(frame_->body, registry_);
    }
    std::vector<Route> run(MiniEnvelope env) override {
        if (env.input_port != "in") throw flow::DiagnosticError{"receiver", "receiver requires input port in"};
        Value argument;
        if (frame_->input_type == "Int") argument = requirePayload<IntPayload>(env, "receiver").value;
        else if (frame_->input_type == "Bool") argument = requirePayload<BoolPayload>(env, "receiver").value;
        else if (frame_->input_type == "Text") argument = requirePayload<TextPayload>(env, "receiver").value;
        else throw flow::DiagnosticError{"receiver", "unsupported input carrier"};
        auto activation = env;
        activation.payload = RecordPayload{};
        setPathValue(requirePayload<RecordPayload>(activation, "receiver"), frame_->input_path, std::move(argument), "receiver");
        // Neither function-local record storage nor runtime queues survive a delivery.
        auto body = buildCheckedGraph(frame_->body, registry_);
        body.graph.setIdentityScope(env.delivery_id + "/");
        std::vector<Value> results;
        body.graph.addNode("__receiver_capture", std::make_unique<ReceiverResult>(*frame_, results));
        body.graph.connect(frame_->result.node, frame_->result.port, "__receiver_capture", "in", "receiver:return");
        body.graph.startAt(frame_->entry.node, std::move(activation));
        if (results.size() != 1) throw flow::DiagnosticError{"receiver", "activation must return exactly one result"};
        const auto& value = results.front().data;
        if (frame_->output_type == "Int" && std::holds_alternative<std::int64_t>(value)) env.payload = IntPayload{std::get<std::int64_t>(value)};
        else if (frame_->output_type == "Bool" && std::holds_alternative<bool>(value)) env.payload = BoolPayload{std::get<bool>(value)};
        else if (frame_->output_type == "Text" && std::holds_alternative<std::string>(value)) env.payload = TextPayload{std::get<std::string>(value)};
        else throw flow::DiagnosticError{"receiver", "activation result carrier mismatch"};
        return {Route{"out", std::move(env)}};
    }
private:
    std::shared_ptr<const ReceiverFrame> frame_;
    AtomRegistry registry_;
};
}

BuildResult buildCheckedGraph(const ModuleSpec& module, const AtomRegistry& registry) {
    std::map<std::string, NodeDecl> nodesById;
    std::map<std::string, NodePolicyMap> policiesByNode;
    std::vector<std::string> producerIds;
    std::map<std::string, AtomContract> contracts;

    for (const auto& node : module.nodes) {
        if (nodesById.find(node.id) != nodesById.end()) {
            throw flow::DiagnosticError{"validator", "duplicate node id: " + node.id};
        }

        if (node.source_function) {
            const auto frame = module.receivers.find(node.id);
            if (node.role != "node" || frame == module.receivers.end() || !frame->second)
                throw flow::DiagnosticError{"validator", "unresolved source receiver: " + node.id};
            contracts.emplace(node.id, AtomContract{node.kind, {{"in", frame->second->input_type}}, {{"out", frame->second->output_type}}, {}});
        } else if (!registry.contains(node.kind)) {
            throw flow::DiagnosticError{"validator", "unknown atom kind: " + node.kind};
        } else {
            contracts.emplace(node.id, registry.contractFor(node.kind));
        }

        if (node.role == "producer") {
            producerIds.push_back(node.id);
        }

        nodesById[node.id] = node;
    }

    for (const auto& policy : module.policies) {
        if (nodesById.find(policy.node) == nodesById.end()) {
            throw flow::DiagnosticError{"validator", "policy references unknown node: " + policy.node};
        }
        if (nodesById.at(policy.node).source_function)
            throw flow::DiagnosticError{"validator", "source receiver policies are unsupported: " + policy.node};
        policiesByNode[policy.node][policy.key] = policy.value;
    }

    if (producerIds.empty()) {
        throw flow::DiagnosticError{"validator", "module has no producer"};
    }

    std::vector<WireDecl> resolvedWires;
    std::set<std::string> connectedInputs;

    for (const auto& wire : module.wires) {
        const auto fromNodeIt = nodesById.find(wire.from.node);
        if (fromNodeIt == nodesById.end()) {
            throw flow::DiagnosticError{"validator", "wire references unknown source node: " + wire.from.node};
        }

        const auto toNodeIt = nodesById.find(wire.to.node);
        if (toNodeIt == nodesById.end()) {
            throw flow::DiagnosticError{"validator", "wire references unknown target node: " + wire.to.node};
        }

        const auto& fromContract = contracts.at(fromNodeIt->first);
        const auto& toContract = contracts.at(toNodeIt->first);

        WireDecl resolved = wire;
        if (resolved.from.port.empty()) {
            resolved.from.port = inferUniquePort(fromContract.outputs, resolved.from.node, "output");
        }
        if (resolved.to.port.empty()) {
            resolved.to.port = inferUniquePort(toContract.inputs, resolved.to.node, "input");
        }

        const auto fromPortIt = fromContract.outputs.find(resolved.from.port);
        if (fromPortIt == fromContract.outputs.end()) {
            throw flow::DiagnosticError{"validator", "source endpoint " + endpointText(resolved.from) + " is not an output port"};
        }

        const auto toPortIt = toContract.inputs.find(resolved.to.port);
        if (toPortIt == toContract.inputs.end()) {
            throw flow::DiagnosticError{"validator", "target endpoint " + endpointText(resolved.to) + " is not an input port"};
        }

        if (fromPortIt->second != toPortIt->second) {
            throw flow::DiagnosticError{
                "validator",
                "type mismatch on wire " + endpointText(resolved.from) + " => " + endpointText(resolved.to) +
                ": " + fromPortIt->second + " cannot connect to " + toPortIt->second
            };
        }

        resolvedWires.push_back(std::move(resolved));
        connectedInputs.insert(endpointText(resolvedWires.back().to));
    }

    for (const auto& [id, node] : nodesById) {
        if (node.role == "producer" || (!node.source_function && id.rfind("__", 0) == 0)) continue;
        const auto& contract = contracts.at(id);
        if (contract.terminal && !contract.outputs.empty())
            throw flow::DiagnosticError{"validator", "terminal atom exposes output ports: " + node.kind};
        for (const auto& [port, type] : contract.inputs) {
            (void)type;
            if (!contract.optional_inputs.count(port) && !connectedInputs.count(id + "." + port))
                throw flow::DiagnosticError{"validator", "required input is not connected: " + id + "." + port};
        }
    }

    BuildResult result;
    // Stateful/cyclic receiver activation is outside the bounded contract.
    // Existing interpreter control-flow loops inside a frame remain valid.
    for (const auto& [id, node] : nodesById) {
        if (!node.source_function) continue;
        std::set<std::string> visited;
        std::vector<std::string> pending{id};
        while (!pending.empty()) {
            const auto current = pending.back();
            pending.pop_back();
            if (!visited.insert(current).second) continue;
            for (const auto& wire : resolvedWires) if (wire.from.node == current) {
                if (wire.to.node == id)
                    throw flow::DiagnosticError{"validator", "cyclic source receiver graph is unsupported: " + id};
                pending.push_back(wire.to.node);
            }
        }
    }
    result.producerIds = producerIds;

    for (const auto& item : nodesById) {
        NodeConfig config;
        config.id = item.second.id;
        config.kind = item.second.kind;
        config.policies = policiesByNode[item.second.id];
        if (item.second.source_function)
            result.graph.addNode(item.first, std::make_unique<SourceReceiver>(module.receivers.at(item.first), registry));
        else result.graph.addNode(item.first, registry.create(std::move(config)));
    }

    for (std::size_t index = 0; index < resolvedWires.size(); ++index) {
        const auto& wire = resolvedWires[index];
        result.graph.connect(wire.from.node, wire.from.port, wire.to.node, wire.to.port, "wire:" + std::to_string(index));
    }

    return result;
}

void runModule(const ModuleSpec& module, flow::PipelineContext& ctx, const AtomRegistry& registry) {
    BuildResult build = buildCheckedGraph(module, registry);

    for (const auto& producerId : build.producerIds) {
        build.graph.startAt(producerId, MiniEnvelope{UnitPayload{}, &ctx, {}, {}, {}});
    }
}

RuntimeResult runModuleChecked(
    const ModuleSpec& module,
    flow::PipelineContext& ctx,
    const AtomRegistry& registry
) {
    try {
        runModule(module, ctx, registry);
        return {true, {}, {}, {}};
    } catch (const flow::DiagnosticError& error) {
        return {false, error.code(), error.stage(), error.what()};
    } catch (const std::bad_alloc&) {
        return {false, "FLOW_RESOURCE_EXHAUSTED", "runtime", "allocation failed"};
    } catch (const std::exception& error) {
        return {false, "FLOW_UNEXPECTED_EXCEPTION", "runtime", error.what()};
    }
}

} // namespace flowmini
