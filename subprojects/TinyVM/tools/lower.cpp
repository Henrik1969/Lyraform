#include <flowcontracts/validate.hpp>
#include <flowcontracts/bounded_input.hpp>
#include <flowcontracts/diagnostics.hpp>
extern "C" {
#include <tinyvm/isa_v1.h>
}

#include <openssl/sha.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <deque>
#include <fstream>
#include <iostream>
#include <new>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace flowcontracts;
using namespace flowcontracts::json;

struct InputError : std::runtime_error { using std::runtime_error::runtime_error; };
struct OutputError : std::runtime_error { using std::runtime_error::runtime_error; };
struct OutputUncertain : std::runtime_error { using std::runtime_error::runtime_error; };
struct Unsupported : std::runtime_error { using std::runtime_error::runtime_error; };

void write_structured_failure(std::string_view code, std::string_view stage, std::string_view message,
                              std::string_view disposition = "no_artifact") noexcept {
    std::fputs("{\"status\":\"failed\",\"code\":\"", stderr);
    flowcontracts::write_json_string(stderr, code);
    std::fputs("\",\"stage\":\"", stderr);
    flowcontracts::write_json_string(stderr, stage);
    std::fputs("\",\"message\":\"", stderr);
    flowcontracts::write_json_string(stderr, message);
    std::fputs("\",\"disposition\":\"", stderr);
    flowcontracts::write_json_string(stderr, disposition);
    std::fputs("\"}\n", stderr);
}

std::string read(const char* path) {
    try {
        if (std::strcmp(path, "-") == 0) return flowcontracts::read_bounded(std::cin, "backend lowering artifact");
        std::ifstream file(path); if (!file) throw InputError("cannot open backend lowering artifact");
        return flowcontracts::read_bounded(file, "backend lowering artifact");
    } catch (const std::bad_alloc&) {
        throw;
    } catch (const InputError&) {
        throw;
    } catch (const std::exception& error) {
        throw InputError(error.what());
    }
}

std::string identity(std::string_view prefix, std::string_view meaning) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(meaning.data()), meaning.size(), digest);
    static constexpr char hex[] = "0123456789abcdef";
    std::string result(prefix);
    for (std::size_t index = 0; index < 16; ++index) { result.push_back(hex[digest[index] >> 4]); result.push_back(hex[digest[index] & 15]); }
    return result;
}

void copy(char output[64], const std::string& value) { std::snprintf(output, 64, "%s", value.c_str()); }

class Compiler {
public:
    Compiler(const Object& root, std::string source, std::string derivation)
        : root_(root), source_(std::move(source)), derivation_(std::move(derivation)) {}

    void compile() {
        flowcontracts::require_executable_targets(required(root_, "lowering_plan"));
        const auto& plan = required_object(root_, "lowering_plan");
        if (const auto* layouts = optional(root_, "aggregate_abi_layouts")) {
            const auto& layout_values = array(*layouts, "$.aggregate_abi_layouts");
            for (const auto& value : layout_values) {
                const auto& layout = object(value, "$.aggregate_abi_layouts[]");
                const auto status = string(required(layout, "status", "$.aggregate_abi_layouts[]"), "$.aggregate_abi_layouts[].status");
                const auto bytes = integer(required(layout, "size", "$.aggregate_abi_layouts[]"), "$.aggregate_abi_layouts[].size");
                bool packed_ints = status == "verified" && bytes > 0 && bytes <= 8;
                std::int64_t expected_size = 0;
                std::int64_t expected_alignment = 1;
                for (const auto& field : required_array(layout, "fields", "$.aggregate_abi_layouts[]")) {
                    const auto& field_object = object(field, "$.aggregate_abi_layouts[].fields[]");
                    const auto field_type = string(required(field_object, "type", "$.aggregate_abi_layouts[].fields[]"), "$.aggregate_abi_layouts[].fields[].type");
                    packed_ints = packed_ints && (field_type == "c_int" || field_type == "c_long" || field_type == "c_ulong" || field_type == "c_size_t");
                    const auto offset = integer(required(field_object, "offset", "$.aggregate_abi_layouts[].fields[]"), "$.aggregate_abi_layouts[].fields[].offset");
                    if (offset != expected_size) packed_ints = false;
                    const auto field_size = field_type == "c_int" ? 4 : (field_type == "c_long" || field_type == "c_ulong" || field_type == "c_size_t" ? 8 : 0);
                    expected_size += field_size;
                    expected_alignment = std::max(expected_alignment, static_cast<std::int64_t>(field_size));
                }
                const auto alignment = integer(required(layout, "alignment", "$.aggregate_abi_layouts[]"), "$.aggregate_abi_layouts[].alignment");
                packed_ints = packed_ints && expected_size == bytes && alignment == expected_alignment;
                if (packed_ints) aggregate_types_.insert(string(required(layout, "name", "$.aggregate_abi_layouts[]"), "$.aggregate_abi_layouts[].name"));
            }
        }
        const auto plan_version = integer(required(plan, "version", "$.lowering_plan"), "$.lowering_plan.version");
        if (plan_version == 2) for (const auto& value : required_array(plan, "functions", "$.lowering_plan")) {
            const auto& function = object(value, "$.lowering_plan.functions[]");
            Callable callable; callable.body = integer(required(function, "body_block_id", "$.function"), "$.function.body_block_id");
            callable.result = string(required(function, "return_type", "$.function"), "$.function.return_type");
            callable.entry = boolean(required(function, "entry", "$.function"), "$.function.entry");
            callable.available = string(required(function, "availability", "$.function"), "$.function.availability") == "definition";
            for (const auto& parameter : required_array(function, "parameters", "$.function")) {
                const auto& item = object(parameter, "$.function.parameters[]");
                callable.parameters.emplace_back(integer(required(item, "symbol_id", "$.parameter"), "$.parameter.symbol_id"),
                                                 string(required(item, "type", "$.parameter"), "$.parameter.type"));
            }
            callables_[integer(required(function, "symbol_id", "$.function"), "$.function.symbol_id")] = std::move(callable);
        }
        const auto& operations = required_array(plan, "operations", "$.lowering_plan");
        scan_arguments(required(root_, "lowering_plan"));
        if (uses_arguments_) { next_slot_ = required_argument_count_ + 1; slot_types_[0] = TINYVM_CARRIER_I32; for (std::size_t index = 1; index < next_slot_; ++index) slot_types_[index] = TINYVM_CARRIER_OPAQUE_HANDLE; }
        for (const auto& value : operations) {
            const auto& operation = object(value, "$.lowering_plan.operations[]");
            const auto kind = string(required(operation, "kind", "$.lowering_plan.operations[]"), "$.lowering_plan.operations[].kind");
            if (kind == "call" && plan_version != 2) continue;
            if (kind != "call" && kind != "value_definition" && kind != "assignment" && kind != "return_value" && kind != "branch" && kind != "loop" && kind != "external_call" && kind != "text_outcome")
                throw Unsupported("operation kind '" + kind + "' is not admitted by the scalar slice");
            const auto block = optional(operation, "block_id") ? integer(*optional(operation, "block_id"), "$.operation.block_id") : 0;
            blocks_[block].push_back(&operation);
        }
        std::map<Integer, Integer> block_start;
        for (const auto& [block, items] : blocks_) for (const auto* operation : items) {
            const auto kind = string(required(*operation, "kind", "$.operation"), "$.operation.kind");
            if (kind == "loop") continue;
            const auto statement = integer(required(*operation, "statement_id", "$.operation"), "$.operation.statement_id");
            if (!block_start.contains(block) || statement < block_start[block]) block_start[block] = statement;
        }
        for (auto& [block, items] : blocks_) std::stable_sort(items.begin(), items.end(), [&](const auto* left, const auto* right) {
            auto order = [&](const Object* operation) {
                if (string(required(*operation, "kind", "$.operation"), "$.operation.kind") == "loop") {
                    const auto body = integer(required(*operation, "body_block_id", "$.operation"), "$.operation.body_block_id");
                    if (block_start.contains(body)) return block_start[body];
                }
                return integer(required(*operation, "statement_id", "$.operation"), "$.operation.statement_id");
            };
            const auto a = order(left), b = order(right);
            return a == b ? integer(required(*left, "id", "$.operation"), "$.operation.id") < integer(required(*right, "id", "$.operation"), "$.operation.id") : a < b;
        });
        if (required_argument_count_) emit_argument_guard();
        if (optional(plan, "source_graph")) compile_graph(*optional(plan, "source_graph"));
        if (blocks_.empty()) emit_return_zero();
        else if (plan_version == 2) {
            std::vector<Integer> entries; for (const auto& [identity,function] : callables_) if (function.entry && function.available) entries.push_back(identity);
            if (entries.size() != 1) throw Unsupported("callable plan requires exactly one selected entry definition");
            compile_block(callables_.at(entries.front()).body);
        } else compile_block(0);
        if (code.empty() || (code.back().opcode != TV1_RETURN && code.back().opcode != TV1_TRAP && code.back().opcode != TV1_HALT)) emit_return_zero();
    }

    std::vector<InstrWord> code;
    std::vector<TinyvmConstant> constants;
    std::vector<TinyvmString> strings;
    std::vector<TinyvmStorage> storage;
    std::vector<TinyvmImport> imports;
    std::vector<TinyvmProvenance> provenance;
    std::vector<TinyvmGraphActivation> graph_activations;
    std::size_t slot_count() const { return next_slot_; }
    std::uint32_t isa_version() const { return uses_arguments_ || !graph_activations.empty() ? 2 : 1; }

private:
    struct Callable { Integer body=-1; bool entry=false, available=false; std::string result; std::vector<std::pair<Integer,std::string>> parameters; };
    const Object& root_;
    std::string source_, derivation_;
    std::map<Integer, std::size_t> symbols_;
    std::map<std::size_t, std::uint32_t> slot_types_;
    std::map<Integer, std::vector<const Object*>> blocks_;
    std::map<Integer,Callable> callables_;
    std::map<Integer,std::size_t> call_results_;
    std::set<std::string> aggregate_types_;
    std::size_t* function_result_ = nullptr;
    std::vector<std::size_t>* function_return_jumps_ = nullptr;
    std::set<Integer> active_blocks_;
    std::set<const Object*> terminal_branches_;
    std::deque<std::string> string_data_;
    std::size_t next_slot_ = 0;
    std::size_t required_argument_count_ = 0;
    bool uses_arguments_ = false;
    std::uint64_t operation_ = UINT64_MAX, block_ = 1, symbol_ = UINT64_MAX;
    std::uint32_t line_ = 1;

    static bool same_provider(const Object& left, const Object& right) {
        for (const auto* key : {"contract", "library", "convention", "symbol", "effect", "parameter_types", "return_type", "evidence"})
            if (string(required(left, key, "$.provider"), "$.provider") != string(required(right, key, "$.provider"), "$.provider")) return false;
        return true;
    }
    bool authorized_graph_provider(const Object& provider) const {
        const auto* authorization = optional(root_, "authorization");
        if (!authorization) return false;
        const auto& authorization_object = object(*authorization, "$.authorization");
        for (const auto& capability : required_array(authorization_object, "capabilities", "$.authorization")) {
            const auto& item = object(capability, "$.authorization.capabilities[]");
            if (optional(item, "status") && string(*optional(item, "status"), "$.authorization.capabilities[].status") != "authorized") continue;
            if (same_provider(provider, item)) return true;
        }
        return false;
    }
    std::size_t invoke_graph_callable(Integer function_id, const std::vector<std::size_t>& inputs) {
        if (!callables_.contains(function_id) || !callables_.at(function_id).available)
            throw Unsupported("graph receiver function definition is unavailable");
        const auto& callable = callables_.at(function_id);
        if (callable.parameters.size() != inputs.size()) throw Unsupported("graph receiver parameter count mismatch");
        for (std::size_t index = 0; index < inputs.size(); ++index) {
            const auto parameter_type = carrier(callable.parameters[index].second);
            if (slot_types_.at(inputs[index]) != parameter_type) throw Unsupported("graph receiver input carrier mismatch");
            const auto parameter = symbol_slot(callable.parameters[index].first);
            slot_types_[parameter] = parameter_type;
            emit(TV1_MOVE, parameter, inputs[index], 0);
        }
        auto result = slot();
        slot_types_[result] = carrier(callable.result);
        std::vector<std::size_t> return_jumps;
        auto* previous_result = function_result_;
        auto* previous_jumps = function_return_jumps_;
        function_result_ = &result;
        function_return_jumps_ = &return_jumps;
        if (!compile_block(callable.body)) throw Unsupported("graph receiver has a path without a result");
        const auto continuation = static_cast<std::int64_t>(code.size());
        for (const auto jump : return_jumps) code[jump].a = continuation;
        function_result_ = previous_result;
        function_return_jumps_ = previous_jumps;
        return result;
    }
    std::size_t invoke_graph_callable(Integer function_id, std::size_t input) {
        return invoke_graph_callable(function_id, std::vector<std::size_t>{input});
    }
    static std::string graph_text(const Object& step, const char* key, const char* fallback = "-") {
        const auto* value = optional(step, key);
        if (!value) return fallback;
        const auto result = string(*value, std::string{"$.graph_schedule.steps[]."} + key);
        return result.empty() ? fallback : result;
    }
    static std::uint64_t graph_number(const Object& step, const char* key) {
        return static_cast<std::uint64_t>(integer(required(step, key, "$.graph_schedule.steps[]"), std::string{"$.graph_schedule.steps[]."} + key));
    }
    void emit_graph_activation(const Object& step, std::int64_t stream_slot = -1) {
        TinyvmGraphActivation activation{};
        activation.id = graph_activations.size() + 1;
        activation.activation_id = graph_number(step, "activation_id");
        activation.input_activation_id = graph_number(step, "input_activation_id");
        activation.input_signal_id = graph_number(step, "input_signal_id");
        activation.output_signal_id = graph_number(step, "output_signal_id");
        activation.delivery_id = graph_number(step, "delivery_id");
        copy(activation.kind, graph_text(step, "kind").c_str());
        copy(activation.node_id, graph_text(step, "node_id").c_str());
        copy(activation.wire_id, graph_text(step, "wire_id").c_str());
        copy(activation.input_port, graph_text(step, "input_port").c_str());
        copy(activation.output_port, graph_text(step, "output_port").c_str());
        graph_activations.push_back(activation);
        emit(TV1_GRAPH_ACTIVATE, static_cast<std::int64_t>(activation.id), stream_slot, 0);
    }
    std::size_t emit_graph_provider(const Object& provider, Integer activation_id, const Array& operands = {}) {
        if (!authorized_graph_provider(provider)) throw Unsupported("graph provider is not exactly authorized by the backend artifact");
        const auto operation_id = static_cast<Integer>(1000000) + activation_id;
        const auto expression_id = operation_id + 1000000;
        Object operation{{"id", operation_id}, {"kind", "external_call"}, {"expression_id", expression_id},
                         {"statement_id", activation_id}, {"scope_id", Integer{0}}, {"block_id", Integer{0}},
                         {"callee", ""}, {"callee_symbol_id", Integer{-1}}, {"arguments", operands},
                         {"operands", operands}, {"provider", provider}};
        compile_operation(operation);
        return call_results_.at(expression_id);
    }
    bool parallel_graph_is_pure(const std::map<std::string, const Object*>& receivers) const {
        std::set<Integer> receiver_functions;
        for (const auto& [node, receiver] : receivers)
            receiver_functions.insert(integer(required(*receiver, "function_symbol_id", "$.source_graph.receivers[]"), "$.source_graph.receivers[].function_symbol_id"));
        const auto& plan = required_object(root_, "lowering_plan");
        for (const auto& value : required_array(plan, "operations", "$.lowering_plan")) {
            const auto& operation = object(value, "$.lowering_plan.operations[]");
            if (!optional(operation, "function_symbol_id")) continue;
            if (!receiver_functions.count(integer(*optional(operation, "function_symbol_id"), "$.lowering_plan.operations[].function_symbol_id"))) continue;
            const auto kind = string(required(operation, "kind", "$.lowering_plan.operations[]"), "$.lowering_plan.operations[].kind");
            if (kind == "call") return false;
            if (kind == "external_call" || kind == "text_outcome") {
                const auto& provider = object(required(operation, "provider", "$.lowering_plan.operations[]"), "$.lowering_plan.operations[].provider");
                if (string(required(provider, "effect", "$.lowering_plan.operations[].provider"), "$.lowering_plan.operations[].provider.effect") != "pure") return false;
            }
        }
        return true;
    }
    void compile_graph(const Value& graph_value) {
        const auto& graph = object(graph_value, "$.lowering_plan.source_graph");
        const auto& schedule = required_object(root_, "graph_schedule", "$.graph_schedule");
        const auto schedule_version = integer(required(schedule, "version", "$.graph_schedule"), "$.graph_schedule.version");
        std::map<std::string, const Object*> providers, receivers;
        for (const auto& value : required_array(graph, "providers", "$.source_graph")) {
            const auto& item = object(value, "$.source_graph.providers[]");
            providers.emplace(string(required(item, "node_id", "$.source_graph.providers[]"), "$.source_graph.providers[].node_id"), &item);
        }
        for (const auto& value : required_array(graph, "receivers", "$.source_graph")) {
            const auto& item = object(value, "$.source_graph.receivers[]");
            receivers.emplace(string(required(item, "node_id", "$.source_graph.receivers[]"), "$.source_graph.receivers[].node_id"), &item);
        }
        const auto schedule_policy = string(required(schedule, "policy", "$.graph_schedule"), "$.graph_schedule.policy");
        if (schedule_policy == "parallel_independent_v1") {
            if (schedule_version != 4 || string(required(schedule, "parallel_contract", "$.graph_schedule"), "$.graph_schedule.parallel_contract") != "dependency_waves_v1")
                throw Unsupported("TinyVM parallel graph requires dependency-wave schedule v4");
            if (!parallel_graph_is_pure(receivers))
                throw Unsupported("TinyVM parallel graph requires pure receiver activations");
        } else if (schedule_policy != "fifo_per_root_source_order_v1") {
            throw Unsupported("TinyVM graph lowering currently requires FIFO graph scheduling");
        }
        if (schedule_version == 2 || schedule_version == 5) {
            const auto stream_contract = string(required(schedule, "stream_contract", "$.graph_schedule"), "$.graph_schedule.stream_contract");
            const bool aggregate_stream = stream_contract == "finite_aggregate_stream_v1" || stream_contract == "finite_aggregate_stream_pipeline_v1";
            if ((schedule_version == 2 && stream_contract != "finite_scalar_stream_v1" && stream_contract != "finite_aggregate_stream_v1") ||
                (schedule_version == 5 && stream_contract != "finite_scalar_stream_pipeline_v1" && stream_contract != "finite_aggregate_stream_pipeline_v1"))
                throw Unsupported("TinyVM stream graph contains an unsupported stream contract");
            if (required_array(schedule, "streams", "$.graph_schedule").size() != 1)
                throw Unsupported("TinyVM stream graph requires one stream descriptor");
            const auto& stream = object(required_array(schedule, "streams", "$.graph_schedule").front(), "$.graph_schedule.streams[]");
            const auto root_node = string(required(stream, "root_node", "$.graph_schedule.streams[]"), "$.graph_schedule.streams[].root_node");
            if (!providers.count(root_node) || string(required(*providers.at(root_node), "activation", "$.source_graph.providers[]"), "$.source_graph.providers[].activation") != "finite_stream_once")
                throw Unsupported("TinyVM stream graph root is not a finite stream provider");
            const auto& provider_node = *providers.at(root_node);
            const auto& item_provider = object(required(provider_node, "provider", "$.source_graph.providers[]"), "$.source_graph.providers[].provider");
            const auto& count_provider = object(required(provider_node, "count_provider", "$.source_graph.providers[]"), "$.source_graph.providers[].count_provider");
            if (!authorized_graph_provider(item_provider) || !authorized_graph_provider(count_provider))
                throw Unsupported("TinyVM stream providers are not exactly authorized by the backend artifact");
            const auto item_type = string(required(provider_node, "output_type", "$.source_graph.providers[]"), "$.source_graph.providers[].output_type");
            if (aggregate_stream != (aggregate_types_.count(item_type) != 0))
                throw Unsupported("TinyVM stream contract and verified item carrier differ");
            const auto max_items = integer(required(stream, "max_items", "$.graph_schedule.streams[]"), "$.graph_schedule.streams[].max_items");
            if (max_items < 0) throw Unsupported("TinyVM stream bound is negative");
            const auto& root_step = object(required_array(schedule, "steps", "$.graph_schedule").front(), "$.graph_schedule.steps[]");
            emit_graph_activation(root_step);
            const auto count = emit_graph_provider(count_provider, 3000000);
            const auto maximum = literal(TINYVM_CARRIER_I64, static_cast<std::uint64_t>(max_items));
            const auto bounded = slot(); slot_types_[bounded] = TINYVM_CARRIER_I1; emit(TV1_CMP_LE, bounded, count, maximum);
            const auto bound_branch = code.size(); emit(TV1_BRANCH, bounded, 0, 0);
            code[bound_branch].b = static_cast<std::int64_t>(code.size());
            const auto index = literal(TINYVM_CARRIER_I64, 0);
            constexpr Integer index_identity = 4000000000LL;
            symbols_[index_identity] = index; slot_types_[index] = TINYVM_CARRIER_I64;
            const auto condition = code.size();
            const auto active = slot(); slot_types_[active] = TINYVM_CARRIER_I1; emit(TV1_CMP_LT, active, index, count);
            const auto loop_branch = code.size(); emit(TV1_BRANCH, active, 0, 0);
            const auto body = code.size(); code[loop_branch].b = static_cast<std::int64_t>(body);
            Array item_operands{Object{{"kind", "identifier"}, {"symbol_id", index_identity}}};
            auto stream_value = emit_graph_provider(item_provider, 3000001, item_operands);
            Integer previous_activation = 0;
            for (const auto& step_value : required_array(schedule, "steps", "$.graph_schedule")) {
                const auto& step = object(step_value, "$.graph_schedule.steps[]");
                const auto kind = string(required(step, "kind", "$.graph_schedule.steps[]"), "$.graph_schedule.steps[].kind");
                if (kind == "stream_root") continue;
                if (kind != "stream_receiver" || string(required(step, "stream_index", "$.graph_schedule.steps[]"), "$.graph_schedule.steps[].stream_index") != "$index")
                    throw Unsupported("TinyVM stream graph contains an unsupported activation step");
                const auto input_activation = integer(required(step, "input_activation_id", "$.graph_schedule.steps[]"), "$.graph_schedule.steps[].input_activation_id");
                if ((schedule_version == 2 && input_activation != 0) || (schedule_version == 5 && input_activation != previous_activation))
                    throw Unsupported("TinyVM stream graph contains an invalid pipeline activation identity");
                const auto node = string(required(step, "node_id", "$.graph_schedule.steps[]"), "$.graph_schedule.steps[].node_id");
                if (!receivers.count(node)) throw Unsupported("TinyVM stream receiver identity is unavailable");
                const auto function = integer(required(*receivers.at(node), "function_symbol_id", "$.source_graph.receivers[]"), "$.source_graph.receivers[].function_symbol_id");
                emit_graph_activation(step, static_cast<std::int64_t>(index));
                const auto result = invoke_graph_callable(function, stream_value);
                if (schedule_version == 5) stream_value = result;
                previous_activation = integer(required(step, "activation_id", "$.graph_schedule.steps[]"), "$.graph_schedule.steps[].activation_id");
            }
            const auto one = literal(TINYVM_CARRIER_I64, 1);
            emit(TV1_ADD, index, index, one);
            emit(TV1_JMP, condition, 0, 0);
            const auto trap = code.size(); emit(TV1_TRAP, TV1_TRAP_EXPLICIT, 0, 0);
            code[bound_branch].pad = static_cast<std::int64_t>(trap);
            code[loop_branch].pad = static_cast<std::int64_t>(code.size());
            return;
        }
        if (schedule_version == 3) {
            if (providers.size() != 1 || receivers.empty())
                throw Unsupported("TinyVM persistent graph requires one provider and persistent receivers");
            std::map<Integer, std::size_t> values;
            std::map<std::string, std::size_t> states;
            for (const auto& value : required_array(schedule, "steps", "$.graph_schedule")) {
                const auto& step = object(value, "$.graph_schedule.steps[]");
                const auto id = integer(required(step, "activation_id", "$.graph_schedule.steps[]"), "$.graph_schedule.steps[].activation_id");
                const auto kind = string(required(step, "kind", "$.graph_schedule.steps[]"), "$.graph_schedule.steps[].kind");
                const auto node = string(required(step, "node_id", "$.graph_schedule.steps[]"), "$.graph_schedule.steps[].node_id");
                operation_ = static_cast<std::uint64_t>(id) + 1; block_ = 1; symbol_ = UINT64_MAX; line_ = static_cast<std::uint32_t>(id + 1);
                emit_graph_activation(step);
                if (kind == "startup") {
                    if (!providers.count(node) || values.size()) throw Unsupported("TinyVM persistent graph requires one startup provider");
                    const auto& provider = object(required(*providers.at(node), "provider", "$.source_graph.providers[]"), "$.source_graph.providers[].provider");
                    values.emplace(id, emit_graph_provider(provider, id));
                } else if (kind == "persistent_receiver") {
                    if (!receivers.count(node)) throw Unsupported("TinyVM persistent receiver identity is unavailable");
                    const auto input_id = integer(required(step, "input_activation_id", "$.graph_schedule.steps[]"), "$.graph_schedule.steps[].input_activation_id");
                    if (!values.count(input_id)) throw Unsupported("TinyVM persistent receiver input identity is unavailable");
                    const auto& receiver = *receivers.at(node);
                    if (!states.count(node)) {
                        const auto initial = string(required(receiver, "state_initial_value", "$.source_graph.receivers[]"), "$.source_graph.receivers[].state_initial_value");
                        std::size_t consumed = 0;
                        const auto state_value = std::stoll(initial, &consumed, 10);
                        if (consumed != initial.size()) throw Unsupported("TinyVM persistent state is not a canonical integer");
                        states.emplace(node, literal(TINYVM_CARRIER_I64, static_cast<std::uint64_t>(state_value)));
                    }
                    const auto function = integer(required(receiver, "function_symbol_id", "$.source_graph.receivers[]"), "$.source_graph.receivers[].function_symbol_id");
                    const auto result = invoke_graph_callable(function, std::vector<std::size_t>{values.at(input_id), states.at(node)});
                    emit(TV1_MOVE, states.at(node), result, 0);
                    values.emplace(id, result);
                } else throw Unsupported("TinyVM graph schedule contains an unsupported persistent activation kind");
            }
            if (values.empty()) throw Unsupported("TinyVM persistent graph schedule has no activations");
            return;
        }
        if (schedule_version != 1 && schedule_version != 4)
            throw Unsupported("TinyVM graph lowering currently requires the serial fresh-activation schedule");
        std::map<Integer, std::size_t> values;
        for (const auto& value : required_array(schedule, "steps", "$.graph_schedule")) {
            const auto& step = object(value, "$.graph_schedule.steps[]");
            const auto id = integer(required(step, "activation_id", "$.graph_schedule.steps[]"), "$.graph_schedule.steps[].activation_id");
            const auto kind = string(required(step, "kind", "$.graph_schedule.steps[]"), "$.graph_schedule.steps[].kind");
            const auto node = string(required(step, "node_id", "$.graph_schedule.steps[]"), "$.graph_schedule.steps[].node_id");
            operation_ = static_cast<std::uint64_t>(id) + 1; block_ = 1; symbol_ = UINT64_MAX; line_ = static_cast<std::uint32_t>(id + 1);
            emit_graph_activation(step);
            if (kind == "startup") {
                if (!providers.count(node) || values.size()) throw Unsupported("TinyVM graph requires one startup provider");
                const auto& provider = object(required(*providers.at(node), "provider", "$.source_graph.providers[]"), "$.source_graph.providers[].provider");
                values.emplace(id, emit_graph_provider(provider, id));
            } else if (kind == "receiver") {
                const auto input = integer(required(step, "input_activation_id", "$.graph_schedule.steps[]"), "$.graph_schedule.steps[].input_activation_id");
                if (!values.count(input) || !receivers.count(node)) throw Unsupported("TinyVM graph receiver input identity is unavailable");
                const auto function = integer(required(*receivers.at(node), "function_symbol_id", "$.source_graph.receivers[]"), "$.source_graph.receivers[].function_symbol_id");
                values.emplace(id, invoke_graph_callable(function, values.at(input)));
            } else throw Unsupported("TinyVM graph schedule contains an unsupported activation kind");
        }
        if (values.empty()) throw Unsupported("TinyVM graph schedule has no activations");
    }

    std::uint32_t carrier(std::string_view type) const {
        if (type == "bool" || type == "Bool") return TINYVM_CARRIER_I1;
        if (type == "int" || type == "c_int") return TINYVM_CARRIER_I32;
        if (type == "c_long" || type == "c_ulong" || type == "c_size_t") return TINYVM_CARRIER_I64;
        if (type == "c_string" || type == "c_pointer" || type == "Text") return TINYVM_CARRIER_OPAQUE_HANDLE;
        if (type == "TextOutcome") return TINYVM_CARRIER_TEXT_OUTCOME;
        if (aggregate_types_.count(std::string{type})) return TINYVM_CARRIER_I64;
        throw Unsupported("type carrier '" + std::string(type) + "' is not admitted by the scalar slice");
    }
    std::size_t slot() { return next_slot_++; }
    std::size_t symbol_slot(Integer identity) {
        const auto found = symbols_.find(identity);
        if (found != symbols_.end()) return found->second;
        return symbols_.emplace(identity, slot()).first->second;
    }
    void emit(std::int64_t opcode, std::int64_t a, std::int64_t b, std::int64_t pad) {
        code.push_back({opcode, a, b, pad});
        TinyvmProvenance item{};
        item.instruction = provenance.size(); item.operation = operation_; item.block = block_; item.symbol = symbol_;
        item.line = line_; item.column = 1; copy(item.source, source_); copy(item.derivation, derivation_);
        provenance.push_back(item);
    }
    std::size_t constant(std::uint32_t type, std::uint64_t bits) {
        for (const auto& item : constants) if (item.carrier == type && item.bits == bits) return item.id;
        const auto id = constants.size() + 1; constants.push_back({id, type, bits}); return id;
    }
    std::size_t literal(std::uint32_t type, std::uint64_t bits) {
        const auto result = slot(); slot_types_[result] = type; emit(TV1_CONST, result, constant(type, bits), 0); return result;
    }
    std::size_t expression(const Value& value) {
        const auto& node = object(value, "$.expression");
        const auto kind = string(required(node, "kind", "$.expression"), "$.expression.kind");
        if (kind == "integer_literal") {
            const auto type = carrier(string(required(node, "type", "$.expression"), "$.expression.type"));
            const auto text = string(required(node, "value", "$.expression"), "$.expression.value");
            std::size_t consumed = 0; const auto signed_value = std::stoll(text, &consumed, 10);
            if (consumed != text.size()) throw Unsupported("non-canonical integer literal");
            if (type == TINYVM_CARRIER_I32 && (signed_value < INT32_MIN || signed_value > INT32_MAX)) throw Unsupported("i32 literal is out of range");
            const auto bits = type == TINYVM_CARRIER_I32 ? static_cast<std::uint64_t>(static_cast<std::int64_t>(static_cast<std::int32_t>(signed_value))) : static_cast<std::uint64_t>(signed_value);
            return literal(type, bits);
        }
        if (kind == "bool_literal") {
            const auto text = string(required(node, "value", "$.expression"), "$.expression.value");
            if (text != "true" && text != "false") throw Unsupported("non-canonical boolean literal");
            return literal(TINYVM_CARRIER_I1, text == "true");
        }
        if (kind == "string_literal") {
            string_data_.push_back(string(required(node, "value", "$.expression"), "$.expression.value"));
            const auto id = string_data_.size();
            strings.push_back({id, reinterpret_cast<std::uint8_t*>(string_data_.back().data()), string_data_.back().size()});
            const auto result = slot(); slot_types_[result] = TINYVM_CARRIER_OPAQUE_HANDLE; emit(TV1_STRING_HANDLE, result, id, 0); return result;
        }
        if (kind == "writable_storage") {
            const auto& declaration = object(required(node, "storage", "$.expression"), "$.expression.storage");
            const auto bytes = integer(required(declaration, "bytes", "$.expression.storage"), "$.expression.storage.bytes");
            if (bytes <= 0) throw Unsupported("writable storage size is not positive");
            const auto id = storage.size() + 1; storage.push_back({id, static_cast<std::uint64_t>(bytes), 1, 1});
            const auto result = slot(); slot_types_[result] = TINYVM_CARRIER_OPAQUE_HANDLE; emit(TV1_STORAGE_HANDLE, result, id, 0); return result;
        }
        if (kind == "identifier") return symbol_slot(integer(required(node, "symbol_id", "$.expression"), "$.expression.symbol_id"));
        if (kind == "field_access") {
            const auto base = expression(required(node, "base", "$.expression"));
            if (slot_types_.at(base) != TINYVM_CARRIER_TEXT_OUTCOME) throw Unsupported("field access requires a TextOutcome carrier");
            const auto field = string(required(node, "field", "$.expression"), "$.expression.field");
            const auto result = slot();
            if (field == "code") { slot_types_[result] = TINYVM_CARRIER_I32; emit(TV1_TEXT_OUTCOME_CODE, result, base, 0); }
            else if (field == "value") { slot_types_[result] = TINYVM_CARRIER_OPAQUE_HANDLE; emit(TV1_TEXT_OUTCOME_VALUE, result, base, 0); }
            else throw Unsupported("unknown TextOutcome field '" + field + "'");
            return result;
        }
        if (kind == "call_result") {
            const auto identity = integer(required(node, "expression_id", "$.expression"), "$.expression.expression_id");
            if (!call_results_.contains(identity)) throw Unsupported("call result expression is not available");
            return call_results_.at(identity);
        }
        if (kind == "call" && optional(node, "intrinsic") && string(*optional(node, "intrinsic"), "$.expression.intrinsic") == "list_length") return 0;
        if (kind == "index" && optional(node, "intrinsic") && string(*optional(node, "intrinsic"), "$.expression.intrinsic") == "list_index") {
            const auto& index = object(required(node, "index", "$.expression"), "$.expression.index");
            const auto text = string(required(index, "value", "$.expression.index"), "$.expression.index.value");
            std::size_t consumed = 0; const auto value = std::stoull(text, &consumed, 10);
            if (consumed != text.size() || value + 1 > required_argument_count_) throw Unsupported("dynamic argument index is not admitted");
            return value + 1;
        }
        if (kind == "conversion") {
            const auto source = expression(required(node, "operand", "$.expression")); const auto result = slot(); const auto type = carrier(string(required(node, "type", "$.expression"), "$.expression.type"));
            slot_types_[result] = type; emit(TV1_CONVERT, result, source, type); return result;
        }
        if (kind == "unary") {
            const auto operation = string(required(node, "operator", "$.expression"), "$.expression.operator");
            const auto operand = expression(required(node, "operand", "$.expression"));
            if (operation == "+") return operand;
            if (operation != "-") throw Unsupported("unary operator '" + operation + "' is not admitted");
            // Negation preserves the operand carrier, as the LLVM backend does.
            // The surrounding expression's type may describe its return context.
            const auto zero = literal(slot_types_.at(operand), 0), result = slot();
            slot_types_[result] = slot_types_.at(operand); emit(TV1_SUB, result, zero, operand); return result;
        }
        if (kind == "binary") {
            const auto left = expression(required(node, "left", "$.expression"));
            auto right = expression(required(node, "right", "$.expression"));
            const auto left_type = slot_types_.at(left), right_type = slot_types_.at(right);
            if (left_type != right_type) {
                const bool integers = (left_type == TINYVM_CARRIER_I32 || left_type == TINYVM_CARRIER_I64) &&
                                      (right_type == TINYVM_CARRIER_I32 || right_type == TINYVM_CARRIER_I64);
                if (!integers) throw Unsupported("binary operand carrier mismatch");
                const auto converted = slot(); slot_types_[converted] = left_type;
                emit(TV1_CONVERT, converted, right, left_type); right = converted;
            }
            if (left_type == TINYVM_CARRIER_OPAQUE_HANDLE)
                throw Unsupported("opaque handle binary operations are not admitted");
            const auto operation = string(required(node, "operator", "$.expression"), "$.expression.operator");
            const std::map<std::string, std::int64_t> opcodes{{"+",TV1_ADD},{"-",TV1_SUB},{"*",TV1_MUL},{"/",TV1_SDIV},{"==",TV1_CMP_EQ},{"!=",TV1_CMP_NE},{"<",TV1_CMP_LT},{"<=",TV1_CMP_LE},{">",TV1_CMP_GT},{">=",TV1_CMP_GE}};
            const auto found = opcodes.find(operation); if (found == opcodes.end()) throw Unsupported("binary operator '" + operation + "' is not admitted");
            const auto result = slot(); slot_types_[result] = found->second >= TV1_CMP_EQ ? static_cast<std::uint32_t>(TINYVM_CARRIER_I1) : slot_types_.at(left); emit(found->second, result, left, right); return result;
        }
        throw Unsupported("expression kind '" + kind + "' is not admitted by the scalar slice");
    }
    void set_provenance(const Object& operation) {
        operation_ = static_cast<std::uint64_t>(integer(required(operation, "id", "$.operation"), "$.operation.id")) + 1;
        block_ = optional(operation, "block_id") ? static_cast<std::uint64_t>(integer(*optional(operation, "block_id"), "$.operation.block_id")) + 1 : 1;
        symbol_ = optional(operation, "result_symbol_id") ? static_cast<std::uint64_t>(integer(*optional(operation, "result_symbol_id"), "$.operation.result_symbol_id")) + 1 : UINT64_MAX;
        line_ = optional(operation, "statement_id") ? static_cast<std::uint32_t>(integer(*optional(operation, "statement_id"), "$.operation.statement_id") + 1) : 1;
    }
    void compile_operation(const Object& operation) {
        set_provenance(operation);
        const auto kind = string(required(operation, "kind", "$.operation"), "$.operation.kind");
        const auto& operands = required_array(operation, "operands", "$.operation");
        if (operands.empty() && kind != "external_call" && kind != "text_outcome" && kind != "call") throw Unsupported("scalar operation has no operand");
        if (kind == "branch") {
            const auto value = expression(operands.front());
            const auto branch_index = code.size(); emit(TV1_BRANCH, value, 0, 0);
            const auto then_block = integer(required(operation, "then_block_id", "$.operation"), "$.operation.then_block_id");
            code[branch_index].b = static_cast<std::int64_t>(code.size());
            const bool then_terminal = compile_block(then_block);
            std::size_t then_jump = SIZE_MAX;
            if (!then_terminal) { set_provenance(operation); then_jump = code.size(); emit(TV1_JMP, 0, 0, 0); }
            const auto* otherwise = optional(operation, "else_block_id");
            const auto otherwise_block = otherwise ? integer(*otherwise, "$.operation.else_block_id") : -1;
            if (otherwise_block >= 0) {
                code[branch_index].pad = static_cast<std::int64_t>(code.size());
                const bool else_terminal = compile_block(otherwise_block);
                if (then_terminal && else_terminal) terminal_branches_.insert(&operation);
                std::size_t else_jump = SIZE_MAX;
                if (!else_terminal) { set_provenance(operation); else_jump = code.size(); emit(TV1_JMP, 0, 0, 0); }
                const auto join = static_cast<std::int64_t>(code.size());
                if (then_jump != SIZE_MAX) code[then_jump].a = join;
                if (else_jump != SIZE_MAX) code[else_jump].a = join;
            } else {
                const auto join = static_cast<std::int64_t>(code.size());
                code[branch_index].pad = join;
                if (then_jump != SIZE_MAX) code[then_jump].a = join;
            }
            return;
        }
        if (kind == "loop") {
            const auto condition = code.size();
            const auto value = expression(operands.front());
            const auto branch_index = code.size(); emit(TV1_BRANCH, value, 0, 0);
            code[branch_index].b = static_cast<std::int64_t>(code.size());
            const auto body = integer(required(operation, "body_block_id", "$.operation"), "$.operation.body_block_id");
            const bool body_terminal = compile_block(body);
            if (!body_terminal) { set_provenance(operation); emit(TV1_JMP, condition, 0, 0); }
            code[branch_index].pad = static_cast<std::int64_t>(code.size());
            return;
        }
        if (kind == "external_call" || kind == "text_outcome") {
            const auto& provider = object(required(operation, "provider", "$.operation"), "$.operation.provider");
            const auto symbol = string(required(provider, "symbol", "$.operation.provider"), "$.operation.provider.symbol");
            const auto parameters = string(required(provider, "parameter_types", "$.operation.provider"), "$.operation.provider.parameter_types");
            const auto result_type = string(required(provider, "return_type", "$.operation.provider"), "$.operation.provider.return_type");
            const auto contract = string(required(provider, "contract", "$.operation.provider"), "$.operation.provider.contract");
            const auto effect = string(required(provider, "effect", "$.operation.provider"), "$.operation.provider.effect");
            const bool aggregate_result = aggregate_types_.count(result_type) != 0;
            const bool aggregate_parameter = parameters.find(',') == std::string::npos && aggregate_types_.count(parameters) != 0;
            const bool admitted = (symbol == "abs" && parameters == "c_int" && result_type == "c_int") ||
                                  (symbol == "labs" && parameters == "c_long" && result_type == "c_long") ||
                                  (symbol == "strlen" && parameters == "c_string" && result_type == "c_size_t") ||
                                  (symbol == "strnlen" && parameters == "c_string,c_size_t" && result_type == "c_size_t") ||
                                  ((symbol == "tolower" || symbol == "toupper") && parameters == "c_int" && result_type == "c_int") ||
                                  (symbol == "open" && parameters == "c_string,c_int" && result_type == "c_int") ||
                                  (symbol == "close" && parameters == "c_int" && result_type == "c_int") ||
                                  ((symbol == "read" || symbol == "write") && parameters == "c_int,c_pointer,c_size_t" && result_type == "c_long") ||
                                  (symbol == "puts" && (parameters == "c_string" || parameters == "Text") && result_type == "c_int") ||
                                  (symbol == "flow_text_concat" && parameters == "Text,Text" && result_type == "Text") ||
                                  (symbol == "flow_text_concat_value" && parameters == "Text,Text" && result_type == "TextOutcome") ||
                                  (symbol == "flow_text_concat_status" && parameters == "Text,Text" && result_type == "c_int") ||
                                  (symbol == "flow_text_dispose" && parameters == "Text" && result_type == "c_int") ||
                                  (symbol == "memset" && parameters == "c_pointer,c_int,c_size_t" && result_type == "c_pointer") ||
                                  (symbol == "memcpy" && parameters == "c_pointer,c_pointer,c_size_t" && result_type == "c_pointer") ||
                                  (symbol == "memmove" && parameters == "c_pointer,c_pointer,c_size_t" && result_type == "c_pointer") ||
                                  (symbol == "memcmp" && parameters == "c_pointer,c_pointer,c_size_t" && result_type == "c_int") ||
                                  ((symbol == "getpgid" || symbol == "getsid") && parameters == "c_int" && result_type == "c_int") ||
                                  (symbol == "getpriority" && parameters == "c_int,c_int" && result_type == "c_int") ||
                                  ((symbol == "getpid" || symbol == "getuid" || symbol == "getgid" || symbol == "geteuid" || symbol == "getegid" || symbol == "getppid" || symbol == "getpgrp") && parameters.empty() && result_type == "c_int") ||
                                  (effect == "readonly" && ((parameters.empty() && result_type == "c_size_t") || (parameters == "c_size_t" && result_type == "c_int"))) ||
                                  ((aggregate_result && ((parameters.empty() && (effect == "pure" || effect == "io")) || (effect == "readonly" && parameters == "c_size_t"))) || (aggregate_parameter && result_type == "c_int"));
            const bool authority = ((effect == "pure" || effect == "io") && (contract == "libc" || contract == "memory" || contract == "ctype" || contract == "file_io")) ||
                                   (effect == "memory" && contract == "text_runtime") ||
                                   (effect == "readonly" && (contract == "kernel" || contract == "linux")) ||
                                   (contract == "stream" && effect == "readonly") ||
                                   (effect == "readonly" && parameters.empty() && result_type == "c_size_t") ||
                                   ((aggregate_result && ((effect == "pure" || effect == "io") || (effect == "readonly" && parameters == "c_size_t"))) || (aggregate_parameter && (effect == "pure" || effect == "io")));
            const auto library = string(required(provider, "library", "$.operation.provider"), "$.operation.provider.library");
            const bool library_admitted = ((contract == "libc" || contract == "memory" || contract == "ctype" || contract == "file_io" || contract == "kernel" || contract == "linux") && library == "libc.so.6") ||
                                          (contract == "text_runtime" && library == "libflowtext.so") ||
                                          (contract == "stream" && !library.empty()) ||
                                          (effect == "readonly" && parameters.empty() && result_type == "c_size_t" && !library.empty()) ||
                                          ((aggregate_result || aggregate_parameter) && !library.empty());
            if (!admitted || !authority || !library_admitted ||
                string(required(provider, "convention", "$.operation.provider"), "$.operation.provider.convention") != "c")
                throw Unsupported("external provider tuple is not admitted by the typed-call slice");
            std::vector<std::uint32_t> expected; for (std::size_t start = 0; start < parameters.size();) { const auto end = parameters.find(',', start); expected.push_back(carrier(parameters.substr(start, end == std::string::npos ? parameters.size() - start : end - start))); if (end == std::string::npos) break; start = end + 1; }
            std::vector<std::size_t> values; for (std::size_t index = 0; index < operands.size(); ++index) { auto value = expression(operands[index]); if (index >= expected.size()) throw Unsupported("provider argument count mismatch"); if (slot_types_.at(value) != expected[index]) { const auto converted = slot(); slot_types_[converted] = expected[index]; emit(TV1_CONVERT, converted, value, expected[index]); value = converted; } values.push_back(value); }
            const auto argument_start = slot();
            for (std::size_t index = 1; index < values.size(); ++index) (void)slot();
            for (std::size_t index = 0; index < values.size(); ++index) emit(TV1_MOVE, argument_start + index, values[index], 0);
            TinyvmImport imported{}; imported.id = imports.size() + 1;
            auto field = [&](char output[64], std::string_view name) { copy(output, string(required(provider, name, "$.operation.provider"), "$.operation.provider." + std::string(name))); };
            field(imported.contract,"contract"); field(imported.library,"library"); field(imported.convention,"convention"); field(imported.symbol,"symbol"); field(imported.effect,"effect"); field(imported.parameters,"parameter_types"); field(imported.result,"return_type");
            if (!imported.parameters[0]) copy(imported.parameters, "none");
            copy(imported.evidence, identity("authorization-", serialize(provider))); imports.push_back(imported);
            const auto* result_identity = optional(operation, "result_symbol_id");
            const auto destination = result_identity ? symbol_slot(integer(*result_identity, "$.operation.result_symbol_id")) : slot();
            slot_types_[destination] = carrier(result_type);
            emit(TV1_CALL_IMPORT, destination, imported.id, argument_start);
            call_results_[integer(required(operation, "expression_id", "$.operation"), "$.operation.expression_id")] = destination;
            return;
        }
        if (kind == "call") {
            const auto callee = integer(required(operation, "callee_symbol_id", "$.operation"), "$.operation.callee_symbol_id");
            if (!callables_.contains(callee) || !callables_.at(callee).available) throw Unsupported("ordinary call definition is unavailable");
            const auto& callable = callables_.at(callee);
            if (operands.size() != callable.parameters.size()) throw Unsupported("ordinary call operand count mismatch");
            std::vector<std::size_t> values; for (const auto& operand : operands) values.push_back(expression(operand));
            for (std::size_t index=0; index<values.size(); ++index) {
                const auto destination=symbol_slot(callable.parameters[index].first); slot_types_[destination]=carrier(callable.parameters[index].second);
                if(slot_types_.at(values[index])!=slot_types_.at(destination))throw Unsupported("ordinary call argument carrier mismatch");
                emit(TV1_MOVE,destination,values[index],0);
            }
            auto result=slot(); slot_types_[result]=carrier(callable.result);
            std::vector<std::size_t> return_jumps; auto* previous_result=function_result_; auto* previous_jumps=function_return_jumps_;
            function_result_=&result; function_return_jumps_=&return_jumps;
            if (!compile_block(callable.body)) throw Unsupported("callable function has a path without a result");
            const auto continuation=static_cast<std::int64_t>(code.size()); for(const auto jump:return_jumps)code[jump].a=continuation;
            function_result_=previous_result; function_return_jumps_=previous_jumps;
            const auto expression_id=integer(required(operation,"expression_id","$.operation"),"$.operation.expression_id"); call_results_[expression_id]=result;
            if(const auto* result_identity=optional(operation,"result_symbol_id")) {
                const auto destination=symbol_slot(integer(*result_identity,"$.operation.result_symbol_id")); slot_types_[destination]=carrier(callable.result);
                emit(TV1_MOVE,destination,result,0);
            }
            return;
        }
        const auto value = expression(operands.front());
        if (kind == "return_value") {
            if(function_result_) {
                if (slot_types_.at(*function_result_) != slot_types_.at(value)) throw Unsupported("callable return carrier mismatch");
                emit(TV1_MOVE,*function_result_,value,0); const auto jump=code.size(); emit(TV1_JMP,0,0,0); function_return_jumps_->push_back(jump); }
            else emit(TV1_RETURN, value, 0, 0);
            return;
        }
        const auto identity = integer(required(operation, "result_symbol_id", "$.operation"), "$.operation.result_symbol_id");
        const auto destination = symbol_slot(identity);
        slot_types_[destination] = slot_types_.at(value);
        if (destination != value) emit(TV1_MOVE, destination, value, 0);
    }
    bool compile_block(Integer block) {
        if (!active_blocks_.insert(block).second) throw Unsupported("cyclic structured block ownership");
        const auto found = blocks_.find(block);
        if (found == blocks_.end()) throw Unsupported("referenced structured block is absent");
        bool terminal = false;
        for (const auto* operation : found->second) {
            if (terminal) break;
            compile_operation(*operation);
            terminal = string(required(*operation, "kind", "$.operation"), "$.operation.kind") == "return_value" || terminal_branches_.count(operation);
        }
        active_blocks_.erase(block);
        return terminal;
    }
    void emit_return_zero() {
        operation_ = UINT64_MAX; block_ = 1; symbol_ = UINT64_MAX; line_ = 1;
        const auto zero = literal(TINYVM_CARRIER_I32, 0); emit(TV1_RETURN, zero, 0, 0);
    }
    void scan_arguments(const Value& value) {
        if (const auto* node = std::get_if<Object>(&value)) {
            const auto* intrinsic = optional(*node, "intrinsic");
            if (intrinsic) {
                const auto name = string(*intrinsic, "$.intrinsic");
                if (name == "list_length") uses_arguments_ = true;
                if (name == "list_index") {
                    uses_arguments_ = true;
                    const auto& index = object(required(*node, "index"), "$.index");
                    const auto text = string(required(index, "value", "$.index"), "$.index.value");
                    std::size_t consumed = 0; const auto position = std::stoull(text, &consumed, 10);
                    if (consumed != text.size()) throw Unsupported("dynamic argument index is not admitted");
                    required_argument_count_ = std::max(required_argument_count_, static_cast<std::size_t>(position + 1));
                }
            }
            for (const auto& [key, child] : *node) { (void)key; scan_arguments(child); }
        } else if (const auto* items = std::get_if<Array>(&value)) for (const auto& child : *items) scan_arguments(child);
    }
    void emit_argument_guard() {
        operation_ = UINT64_MAX; block_ = 1; symbol_ = UINT64_MAX; line_ = 1;
        const auto required = literal(TINYVM_CARRIER_I32, required_argument_count_);
        const auto ready = slot(); emit(TV1_CMP_GE, ready, 0, required);
        const auto branch = code.size(); emit(TV1_BRANCH, ready, 0, 0);
        const auto failure = literal(TINYVM_CARRIER_I32, 64); emit(TV1_RETURN, failure, 0, 0);
        code[branch].b = static_cast<std::int64_t>(code.size());
        code[branch].pad = static_cast<std::int64_t>(branch + 1);
    }
};

int lower(const char* input_path, const char* output_path) {
#ifdef FLOWTINYLLOWER_TEST_ALLOCATION_FAILURE
    (void)input_path;
    (void)output_path;
    throw std::bad_alloc();
#endif
    const auto input = parse(read(input_path));
    validate_backend_lowering_artifact(input);
    const auto& root = object(input);
    const auto artifact_version = integer(required(root, "version"), "$.version");
    if (artifact_version == 2) {
        const auto& policy = required_object(root, "target_policy");
        const auto& backend = required_object(policy, "backend", "$.target_policy");
        const auto& architecture = required_object(policy, "architecture", "$.target_policy");
        const auto& abi = required_object(policy, "abi", "$.target_policy");
        if (string(required(backend, "name", "$.target_policy.backend"), "$.target_policy.backend.name") != "tinyvm" ||
            integer(required(backend, "artifact_version", "$.target_policy.backend"), "$.target_policy.backend.artifact_version") != 2 ||
            string(required(architecture, "name", "$.target_policy.architecture"), "$.target_policy.architecture.name") != "tinyvm" ||
            integer(required(architecture, "word_bits", "$.target_policy.architecture"), "$.target_policy.architecture.word_bits") != 64 ||
            string(required(abi, "name", "$.target_policy.abi"), "$.target_policy.abi.name") != "flowcore-tinyvm-slot" ||
            integer(required(abi, "version", "$.target_policy.abi"), "$.target_policy.abi.version") != 2) {
            std::cout << serialize(Object{{"backend", "tinyvm"}, {"format", "flowtiny.lowering_result"},
                                         {"reason", "target policy is incompatible with the TinyVM backend"}, {"status", "unsupported"},
                                         {"version", Integer{1}}}) << '\n';
            return 2;
        }
        const auto& capabilities = required_object(policy, "capabilities", "$.target_policy");
        for (const auto& required_capability : required_array(capabilities, "required", "$.target_policy.capabilities")) {
            const auto name = string(required_capability, "$.target_policy.capabilities.required[]");
            if (name != "tinyvm-isa-2" && name != "exact-import-policy") {
                std::cout << serialize(Object{{"backend", "tinyvm"}, {"format", "flowtiny.lowering_result"},
                                             {"reason", "target policy requires an unavailable provider capability"},
                                             {"status", "unsupported"}, {"version", Integer{1}}}) << '\n';
                return 2;
            }
        }
    }
    const auto canonical = serialize(input);
    const auto source = serialize(required(root, "source"));
    const auto plan_text = serialize(required(root, "lowering_plan"));
    const auto optimization = serialize(required(required_object(root, "provenance"), "optimization", "$.provenance"));
    const auto target = artifact_version == 2 ? serialize(required(root, "target_policy")) : serialize(required(root, "target"));
    const auto source_id = identity("source-", source);
    const auto plan_id = identity("plan-", plan_text);
    Compiler compiler(root, source_id, plan_id);
    try { compiler.compile(); }
    catch (const Unsupported& unsupported) {
        std::cout << serialize(Object{{"backend", "tinyvm"}, {"format", "flowtiny.lowering_result"},
                                     {"reason", std::string(unsupported.what())}, {"status", "unsupported"},
                                     {"version", Integer{1}}}) << '\n';
        return 2;
    }

    TinyvmArtifactV2 artifact;
    tinyvm_artifact_v2_init(&artifact);
    artifact.isa_version = compiler.isa_version();
    artifact.data_words = compiler.slot_count();
    artifact.stack_words = 16;
    copy(artifact.artifact_id, identity("tinyvm-", canonical));
    copy(artifact.source_id, source_id);
    copy(artifact.target_policy_id, identity("target-", target));
    copy(artifact.lowering_plan_id, plan_id);
    copy(artifact.optimization_id, identity("opt-", optimization));
    artifact.code = compiler.code.data(); artifact.code_count = compiler.code.size();
    artifact.constants = compiler.constants.data(); artifact.constant_count = compiler.constants.size();
    artifact.strings = compiler.strings.data(); artifact.string_count = compiler.strings.size();
    artifact.storage = compiler.storage.data(); artifact.storage_count = compiler.storage.size();
    artifact.imports = compiler.imports.data(); artifact.import_count = compiler.imports.size();
    artifact.provenance = compiler.provenance.data(); artifact.provenance_count = compiler.provenance.size();
    artifact.graph_activations = compiler.graph_activations.data(); artifact.graph_activation_count = compiler.graph_activations.size();
    char diagnostic[256];
    const auto write_result = tinyvm_artifact_v2_write_result(
        output_path, &artifact, diagnostic, sizeof diagnostic);
    if (write_result == TINYVM_ARTIFACT_WRITE_DURABILITY_UNCERTAIN)
        throw OutputUncertain(std::string("TinyVM artifact publication durability is uncertain: ") + diagnostic);
    if (write_result != TINYVM_ARTIFACT_WRITE_PUBLISHED)
        throw OutputError(std::string("cannot emit TinyVM artifact: ") + diagnostic);
    std::cout << serialize(Object{{"artifact_id", std::string(artifact.artifact_id)}, {"backend", "tinyvm"},
                                 {"format", "flowtiny.lowering_result"}, {"isa_version", Integer{compiler.isa_version()}},
                                 {"status", "emitted"}, {"version", Integer{1}}}) << '\n';
    return 0;
}
} // namespace

int main(int argc, char** argv) {
    bool structured_diagnostics = false;
    try {
        for (int index = 1; index + 1 < argc; ++index)
            if (std::strcmp(argv[index], "--diagnostics") == 0 && std::strcmp(argv[index + 1], "json") == 0)
                structured_diagnostics = true;

        const char* input_path = nullptr;
        const char* output_path = nullptr;
        for (int index = 1; index < argc; ++index) {
            if (std::strcmp(argv[index], "--diagnostics") == 0) {
                if (++index >= argc || std::strcmp(argv[index], "json") != 0)
                    throw InputError("--diagnostics requires json");
            } else if (!input_path) {
                input_path = argv[index];
            } else if (!output_path) {
                output_path = argv[index];
            } else {
                throw InputError("unexpected extra argument");
            }
        }
        if (!input_path || !output_path)
            throw InputError("usage: flowtinylower [--diagnostics json] INPUT.json OUTPUT.tvm");
        return lower(input_path, output_path);
    } catch (const flowcontracts::json::Error& error) {
        if (structured_diagnostics) write_structured_failure("FLOWTINYLOWER_CONTRACT_FAILURE", "contract", error.what());
        else std::cerr << "flowtinylower contract error: " << error.what() << '\n';
        return 1;
    } catch (const std::bad_alloc&) {
        if (structured_diagnostics) write_structured_failure("FLOWTINYLOWER_RESOURCE_EXHAUSTED", "runtime", "allocation failed");
        else std::cerr << "flowtinylower error: allocation failed\n";
        return 1;
    } catch (const InputError& error) {
        if (structured_diagnostics) write_structured_failure("FLOWTINYLOWER_INPUT_INVALID", "input", error.what());
        else std::cerr << "flowtinylower input error: " << error.what() << '\n';
        return 1;
    } catch (const OutputError& error) {
        if (structured_diagnostics) write_structured_failure("FLOWTINYLOWER_OUTPUT_FAILURE", "output", error.what());
        else std::cerr << "flowtinylower output error: " << error.what() << '\n';
        return 1;
    } catch (const OutputUncertain& error) {
        if (structured_diagnostics)
            write_structured_failure("FLOWTINYLOWER_OUTPUT_DURABILITY_UNCERTAIN", "output", error.what(),
                                     "artifact_published_durability_uncertain");
        else std::cerr << "flowtinylower output durability uncertain: " << error.what() << '\n';
        return 1;
    } catch (const std::exception& error) {
        if (structured_diagnostics) write_structured_failure("FLOWTINYLOWER_RUNTIME_FAILURE", "runtime", error.what());
        else std::cerr << "flowtinylower error: " << error.what() << '\n';
        return 1;
    } catch (...) {
        if (structured_diagnostics) write_structured_failure("FLOWTINYLOWER_UNKNOWN_FAILURE", "runtime", "unknown non-standard failure");
        else std::cerr << "flowtinylower error: unknown non-standard failure\n";
        return 1;
    }
}
