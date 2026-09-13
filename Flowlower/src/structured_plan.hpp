#pragma once

#include <flowcontracts/json.hpp>
#include <flowcontracts/source_graph.hpp>

#include <algorithm>
#include <cctype>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace flowlower::structured {

using Json = flowcontracts::json::Value;
using Array = flowcontracts::json::Array;
using Object = flowcontracts::json::Object;
class Parser {
public:
    explicit Parser(std::string text) : text_(std::move(text)) {}
    Json parse() const { return flowcontracts::json::parse(text_); }
private:
    std::string text_;
};
inline const Json* field(const Json& value, std::string_view name) {
    const auto* object = std::get_if<Object>(&value);
    return object ? flowcontracts::json::optional(*object, name) : nullptr;
}
inline std::string text(const Json* value) {
    return value && std::holds_alternative<std::string>(*value) ? std::get<std::string>(*value) : std::string{};
}
inline int integer(const Json* value, const char* name, int fallback = -1) {
    if (!value) return fallback;
    const auto parsed = flowcontracts::json::integer(*value, name);
    if (parsed < std::numeric_limits<int>::min() || parsed > std::numeric_limits<int>::max()) throw std::runtime_error(std::string("JSON field '") + name + "' is outside int range");
    return static_cast<int>(parsed);
}
inline const Array& array(const Json* value, const char* name) {
    if (!value || !std::holds_alternative<Array>(*value)) throw std::runtime_error(std::string("JSON field '") + name + "' must be an array");
    return std::get<Array>(*value);
}

struct Provider {
    std::string contract, library, convention, symbol, effect, parameters, result, evidence;
    auto tie() const { return std::tie(contract, library, convention, symbol, effect, parameters, result, evidence); }
    bool operator<(const Provider& other) const { return tie() < other.tie(); }
};
struct Operation {
    int id = -1, expression = -1, statement = -1, block = -1, function_symbol = -1, callee_symbol = -1, result_symbol = -1, then_block = -1, else_block = -1, body_block = -1;
    std::string kind;
    const Json* operand = nullptr;
    std::optional<Provider> provider;
};
struct Callable { int symbol=-1, body_block=-1; bool entry=false; std::string name, result; std::vector<std::pair<int,std::string>> parameters; };

inline std::string llvm_type(std::string_view carrier) {
    if (carrier == "int") return "i32";
    if (carrier == "bool" || carrier == "Bool") return "i1";
    if (carrier == "c_int") return "i32";
    if (carrier == "c_long" || carrier == "c_ulong" || carrier == "c_size_t") return "i64";
    if (carrier == "c_double" || carrier == "float64") return "double";
    if (carrier == "c_string" || carrier == "c_pointer" || carrier == "Text") return "ptr";
    if (carrier == "TextFailure") return "i32";
    if (carrier == "TextOutcome") return "{ i32, ptr }";
    return {};
}
inline bool c_symbol(std::string_view symbol) {
    if (symbol.empty() || (!std::isalpha(static_cast<unsigned char>(symbol.front())) && symbol.front() != '_')) return false;
    return std::all_of(symbol.begin() + 1, symbol.end(), [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; });
}
inline std::vector<std::string> carriers(std::string_view joined) {
    std::vector<std::string> result;
    for (std::size_t start = 0; start < joined.size();) {
        const auto end = joined.find(',', start); result.emplace_back(joined.substr(start, end == std::string_view::npos ? joined.size() - start : end - start));
        if (end == std::string_view::npos) break;
        start = end + 1;
    }
    return result;
}
inline std::string slot(int symbol) { return "%flow_slot_" + std::to_string(symbol); }

class Emitter {
public:
    Emitter(const Json& root, const Json& binding) : root_(root), binding_(binding) { load(); }
    bool applicable() const { return !invalid_control_ && !unsupported_; }
    bool requires_structured_control() const { return invalid_control_ || (has_nonroot_block_ && !unsupported_); }
    std::string emit() {
        authorize();
        std::ostringstream out;
        out << "; Flowcore generic structured lowering plan: ordered blocks, calls, branches and returns\n"
               "target triple = \"x86_64-pc-linux-gnu\"\n";
        emit_globals(out); emit_declarations(out);
        if(graph_native_) emit_graph_globals(out);
        if(plan_version_==2) {
            for(const auto& [identity,function]:callables_)if(!function.entry&&function.body_block>=0)emit_function(function,out);
            for(const auto& [identity,function]:callables_)if(function.entry){emit_function(function,out); if(graph_native_) emit_graph_main(function,out); return out.str();}
            throw std::runtime_error("callable lowering plan has no selected entry definition");
        }
        out << (uses_args_ ? "define i32 @main(i32 %argc, ptr %argv) {\n" : "define i32 @main() {\n") << "entry:\n";
        emit_allocations(out);
        if (required_argc_ > 0) {
            out << "  %flow_args_ready = icmp sge i32 %argc, " << required_argc_ << "\n"
                   "  br i1 %flow_args_ready, label %flow_block_0, label %flow_args_error\n"
                   "flow_args_error:\n  ret i32 64\n";
        } else out << "  br label %flow_block_0\n";
        emit_block(0, out, "flow_exit");
        out << "flow_exit:\n  ret i32 0\n}\n";
        return out.str();
    }
private:
    const Json& root_; const Json& binding_;
    std::vector<Operation> operations_;
    std::map<int,Callable> callables_;
    std::map<int, std::vector<const Operation*>> blocks_;
    std::map<int, std::string> symbol_types_;
    std::map<int, const Json*> definitions_;
    std::map<std::string, std::string> carrier_representations_;
    std::map<std::string, std::string> aggregate_types_;
    std::set<Provider> providers_, authorized_;
    bool has_branch_ = false, has_declared_carrier_ = false, has_nonroot_block_ = false, invalid_control_ = false, unsupported_ = false, uses_args_ = false, has_text_outcome_ = false;
    int temporary_ = 0, label_ = 0, required_argc_ = 0;
    int plan_version_ = 1;
    bool graph_native_ = false;
    const Json* graph_json_ = nullptr;
    std::optional<flowcontracts::SourceGraph> graph_model_;
    int active_operation_ = -1;
    std::string return_carrier_ = "c_int";
    std::map<int,std::pair<std::string,std::string>> call_results_;
    bool has_list_length_ = false;
    std::map<int, std::size_t> string_sizes_;

    static Provider provider(const Json& value) {
        return {text(field(value,"contract")), text(field(value,"library")), text(field(value,"convention")),
                text(field(value,"symbol")), text(field(value,"effect")), text(field(value,"parameter_types")), text(field(value,"return_type")), flowcontracts::binding_evidence(flowcontracts::json::object(value), "$.provider")};
    }
    std::string llvm_type(std::string_view carrier) const {
        const auto builtin = flowlower::structured::llvm_type(carrier);
        if (!builtin.empty()) return builtin;
        const auto found = carrier_representations_.find(std::string{carrier});
        if (found != carrier_representations_.end() && (found->second == "void*" || found->second == "const void*")) return "ptr";
        const auto aggregate = aggregate_types_.find(std::string{carrier});
        if (aggregate != aggregate_types_.end()) return aggregate->second;
        return {};
    }
    void load() {
        const auto format = text(field(root_, "format"));
        const auto version = integer(field(root_, "version"), "version");
        if ((format == "flowoptimize.optimization_report" && version != 1) ||
            (format == "flowcore.backend_lowering_artifact" && version != 1 && version != 2) ||
            (format != "flowoptimize.optimization_report" && format != "flowcore.backend_lowering_artifact")) return;
        if (const auto* contracts = field(root_, "abi_type_contracts"))
            for (const auto& item : array(contracts, "abi_type_contracts")) {
                const auto name = text(field(item, "name"));
                const auto representation = text(field(item, "repr"));
                if (!name.empty() && !representation.empty()) carrier_representations_[name] = representation;
            }
        if (const auto* layouts = field(root_, "aggregate_abi_layouts")) {
            for (const auto& item : array(layouts, "aggregate_abi_layouts")) {
                const auto name = text(field(item, "name"));
                const auto status = text(field(item, "status"));
                if (name.empty() || status != "verified") continue;
                const auto size = integer(field(item, "size"), "aggregate_abi_layout.size");
                std::string type;
                if (size > 0 && size <= 4) type = "i32";
                else if (size > 4 && size <= 8) type = "i64";
                else throw std::runtime_error("unsupported aggregate ABI size for native value carrier");
                const auto& fields = array(field(item, "fields"), "aggregate_abi_layout.fields");
                for (const auto& field_value : fields) {
                    if (text(field(field_value, "type")) != "c_int") throw std::runtime_error("unsupported aggregate ABI field carrier");
                }
                aggregate_types_[name] = type;
            }
        }
        const auto* plan = field(root_, "lowering_plan");
        if (!plan || text(field(*plan,"format")) != "flowcore.lowering_plan") return;
        if (const auto* graph = field(*plan, "source_graph")) {
            graph_model_ = flowcontracts::source_graph(*graph, "$.lowering_plan.source_graph");
            if (!graph_model_->executable) throw std::runtime_error("source graph execution is not admitted");
            graph_native_ = true; graph_json_ = graph;
            flowcontracts::validate_graph_schedule(flowcontracts::json::object(root_));
            for (const auto& node : graph_model_->providers) {
                providers_.insert(provider(*field(node, "provider")));
                if (const auto* count = field(node, "count_provider")) providers_.insert(provider(*count));
            }
        }
        plan_version_=integer(field(*plan,"version"),"lowering_plan.version");
        if(plan_version_!=1&&plan_version_!=2)return;
        if(plan_version_==2) for(const auto& item:array(field(*plan,"functions"),"lowering_plan.functions")) {
            Callable function; function.symbol=integer(field(item,"symbol_id"),"function.symbol_id");
            function.body_block=integer(field(item,"body_block_id"),"function.body_block_id"); function.entry=std::get<bool>(*field(item,"entry"));
            function.name=text(field(item,"name")); function.result=text(field(item,"return_type"));
            for(const auto& parameter:array(field(item,"parameters"),"function.parameters"))
                function.parameters.emplace_back(integer(field(parameter,"symbol_id"),"parameter.symbol_id"),text(field(parameter,"type")));
            for(const auto& [symbol,type]:function.parameters)symbol_types_[symbol]=type;
            callables_[function.symbol]=std::move(function);
        }
        for (const auto& item : array(field(*plan,"operations"), "lowering_plan.operations")) {
            Operation op; op.id=integer(field(item,"id"),"id"); op.expression=integer(field(item,"expression_id"),"expression_id"); op.statement=integer(field(item,"statement_id"),"statement_id");
            op.block=integer(field(item,"block_id"),"block_id"); op.kind=text(field(item,"kind"));
            op.function_symbol=integer(field(item,"function_symbol_id"),"function_symbol_id"); op.callee_symbol=integer(field(item,"callee_symbol_id"),"callee_symbol_id");
            op.result_symbol=integer(field(item,"result_symbol_id"),"result_symbol_id");
            op.then_block=integer(field(item,"then_block_id"),"then_block_id"); op.else_block=integer(field(item,"else_block_id"),"else_block_id");
            op.body_block=integer(field(item,"body_block_id"),"body_block_id");
            const auto& operands=array(field(item,"operands"),"operation.operands"); if (!operands.empty()) op.operand=&operands.front();
            if (const auto* facts=field(item,"provider")) {
                op.provider=provider(*facts); providers_.insert(*op.provider);
                if (op.kind == "text_outcome") has_text_outcome_ = true;
                if (flowlower::structured::llvm_type(op.provider->result).empty() && !llvm_type(op.provider->result).empty()) has_declared_carrier_=true;
                for (const auto& carrier : carriers(op.provider->parameters))
                    if (flowlower::structured::llvm_type(carrier).empty() && !llvm_type(carrier).empty()) has_declared_carrier_=true;
                if (op.result_symbol>=0) symbol_types_[op.result_symbol]=op.provider->result;
            }
            if (op.kind=="value_definition" && op.result_symbol>=0 && op.operand) { definitions_[op.result_symbol]=op.operand; symbol_types_[op.result_symbol]=text(field(*op.operand,"type")); }
            if(op.kind=="call"&&plan_version_==2&&callables_.count(op.callee_symbol)) {
                if(op.result_symbol>=0)symbol_types_[op.result_symbol]=callables_.at(op.callee_symbol).result;
            }
            if (op.kind=="branch") {
                has_branch_=true;
            }
            if (op.block!=0) has_nonroot_block_=true;
            if (op.kind!="call" && op.kind!="external_call" && op.kind!="text_outcome" && op.kind!="value_definition" && op.kind!="branch" && op.kind!="return_value" && op.kind!="loop" && op.kind!="assignment") unsupported_=true;
            operations_.push_back(std::move(op));
        }
        for (auto& op:operations_) if (op.kind!="call" || plan_version_==2) blocks_[op.block].push_back(&op);
        for (const auto& op : operations_) {
            if (op.kind=="branch" && (op.then_block<0 || !blocks_.count(op.then_block) || (op.else_block>=0 && !blocks_.count(op.else_block)))) invalid_control_=true;
            if (op.kind=="loop" && (op.body_block<0 || !blocks_.count(op.body_block))) invalid_control_=true;
        }
        std::map<int,int> block_start;
        for (const auto& [block,ops]:blocks_) for (const auto* op:ops)
            if (op->kind!="loop" && (!block_start.count(block) || op->statement<block_start[block])) block_start[block]=op->statement;
        for (auto& [block, ops]:blocks_) std::stable_sort(ops.begin(),ops.end(),[&](auto* a,auto* b){
            const int a_statement=a->kind=="loop"&&block_start.count(a->body_block)?block_start[a->body_block]:a->statement;
            const int b_statement=b->kind=="loop"&&block_start.count(b->body_block)?block_start[b->body_block]:b->statement;
            return a_statement < b_statement || (a_statement==b_statement && a->id<b->id);
        });
        std::set<int> reachable;
        if(plan_version_==2) {
            for(const auto& [identity,function]:callables_)if(function.body_block>=0)reachable.insert(function.body_block);
        } else reachable.insert(0);
        bool changed=true;
        while(changed) {
            changed=false;
            const auto snapshot=reachable;
            for(int block:snapshot) for(const auto* op:blocks_[block]) {
                const int children[]={op->then_block,op->else_block,op->body_block};
                for(int child:children) if(child>=0 && reachable.insert(child).second) changed=true;
            }
        }
        for(const auto& [block,ops]:blocks_) if(!ops.empty()&&!reachable.count(block)) invalid_control_=true;
        const Json* authorization = nullptr;
        if (text(field(binding_,"format"))=="flowbind.binding_report" && text(field(binding_,"status"))=="ready") authorization = &binding_;
        else if (format == "flowcore.backend_lowering_artifact") authorization = field(root_, "authorization");
        if (authorization)
            for (const auto& item:array(field(*authorization,"capabilities"),"authorization.capabilities")) if (text(field(item,"status"))=="authorized") authorized_.insert(provider(item));
        for (const auto& [symbol, definition]:definitions_) {
            const auto kind=text(field(*definition,"kind")); const auto intrinsic=text(field(*definition,"intrinsic"));
            if (intrinsic=="list_length" || intrinsic=="list_index") uses_args_=true;
            if (intrinsic=="list_length") has_list_length_=true;
            if (intrinsic=="list_index") {
                const auto* index = field(*definition,"index");
                const auto value = index ? text(field(*index,"value")) : std::string{};
                if (!value.empty()) required_argc_=std::max(required_argc_,std::stoi(value)+1);
            }
        }
        if (has_list_length_) required_argc_=0;
    }
    void authorize() const {
        if (!providers_.empty() && authorized_.empty()) throw std::runtime_error("generic structured plan requires a ready typed binding report");
        for (const auto& required:providers_) if (!authorized_.count(required)) throw std::runtime_error("generic structured operation is not exactly authorized: "+required.symbol);
    }
    static std::string escaped_string(std::string_view value) {
        static constexpr char hex[]="0123456789ABCDEF"; std::string result;
        for (unsigned char c:value) {
            if (c>=32 && c<=126 && c!='"' && c!='\\') result.push_back(static_cast<char>(c));
            else { result.push_back('\\'); result.push_back(hex[c>>4]); result.push_back(hex[c&15]); }
        }
        return result+"\\00";
    }
    void emit_globals(std::ostringstream& out) {
        const auto* plan = field(root_, "lowering_plan");
        const auto& operations = array(field(*plan, "operations"), "lowering_plan.operations");
        std::map<int, std::string> strings;
        std::function<void(const Json&)> collect = [&](const Json& value) {
            if (const auto* object = std::get_if<Object>(&value)) {
                const auto kind = text(field(value, "kind"));
                if (kind == "string_literal") {
                    const int expression_id = integer(field(value, "expression_id"), "string_literal.expression_id");
                    const auto literal = text(field(value, "value"));
                    strings[expression_id] = literal;
                }
                for (const auto& [key, child] : *object) { (void)key; collect(child); }
            } else if (const auto* values = std::get_if<Array>(&value)) {
                for (const auto& child : *values) collect(child);
            }
        };
        for (const auto& operation : operations) collect(operation);
        for (const auto& [expression_id, literal] : strings) {
            string_sizes_[expression_id] = literal.size() + 1;
            out << "@flow_string_expr_" << expression_id << " = private unnamed_addr constant ["
                << literal.size() + 1 << " x i8] c\"" << escaped_string(literal) << "\"\n";
        }
    }
    void emit_declarations(std::ostringstream& out) const {
        if (has_text_outcome_) out << "declare i32 @flow_text_concat_outcome(ptr, ptr, ptr)\n";
        std::map<std::string, std::pair<std::string, std::string>> native_symbols;
        for (const auto& p:providers_) {
            if (!c_symbol(p.symbol) || llvm_type(p.result).empty()) throw std::runtime_error("unsupported structured provider ABI");
            if (graph_native_ && (p.symbol == "flow_graph_enter" || p.symbol == "flow_graph_event" || p.symbol == "flow_graph_drop" || p.symbol == "flow_graph_fail" || p.symbol == "flow_graph_operation"))
                throw std::runtime_error("external symbol conflicts with native graph runtime");
            if (p.symbol == "main") throw std::runtime_error("external symbol conflicts with native entry point");
            std::ostringstream declaration;
            declaration << "declare "<<llvm_type(p.result)<<" @"<<p.symbol<<"("; const auto params=carriers(p.parameters);
            for (std::size_t i=0;i<params.size();++i) { if(i) declaration<<", "; const auto type=llvm_type(params[i]); if(type.empty()) throw std::runtime_error("unsupported structured parameter carrier"); declaration<<type; }
            declaration << ")\n";
            const auto identity = std::make_pair(p.library + ":" + p.convention, declaration.str());
            const auto [found, inserted] = native_symbols.emplace(p.symbol, identity);
            if (!inserted && found->second != identity)
                throw std::runtime_error("native symbol has conflicting provider libraries or ABI declarations: " + p.symbol);
            if (inserted) out << declaration.str();
        }
        out << "declare void @llvm.trap()\n";
    }
    std::string callable_name(const Callable& function) const {
        if (function.symbol < 0) throw std::runtime_error("invalid callable symbol identity");
        return function.entry ? (graph_native_ ? "flow.source.entry" : "main") : "flow.function." + std::to_string(function.symbol);
    }
    void emit_allocations(std::ostringstream& out) const {
        for (const auto& [symbol,type]:symbol_types_) { const auto llvm=llvm_type(type); if(!llvm.empty()) out<<"  "<<slot(symbol)<<" = alloca "<<llvm<<", align "<<(llvm=="i32"?4:(llvm.rfind("{ ", 0) == 0 ? 4 : 8))<<"\n"; }
        for (const auto& [symbol,value]:definitions_) if(text(field(*value,"kind"))=="writable_storage") {
            const auto* storage=field(*value,"storage"); const int bytes=integer(storage?field(*storage,"bytes"):nullptr,"storage.bytes");
            if(bytes<=0) throw std::runtime_error("invalid compatibility writable storage size");
            out<<"  %flow_storage_"<<symbol<<" = alloca ["<<bytes<<" x i8], align 1\n"
               <<"  %flow_storage_ptr_"<<symbol<<" = getelementptr ["<<bytes<<" x i8], ptr %flow_storage_"<<symbol<<", i64 0, i64 0\n";
        }
    }
    const Array& graph_steps() const {
        return array(field(*field(root_, "graph_schedule"), "steps"), "graph_schedule.steps");
    }
    std::string graph_record(const Json& step, const std::string& event) const {
        auto record = flowcontracts::json::object(step);
        record.emplace("format", "flowcore.graph_activation"); record.emplace("version", flowcontracts::json::Integer{1});
        record.emplace("event", event);
        const auto node = text(field(step, "node_id"));
        for (const auto& item : array(field(*field(*graph_json_, "syntax"), "nodes"), "graph.nodes"))
            if (text(field(item, "node_id")) == node) record.emplace("node_provenance", *field(item, "provenance"));
        for (const auto& item : array(field(*field(*graph_json_, "syntax"), "wires"), "graph.wires"))
            if (text(field(item, "wire_id")) == text(field(step, "wire_id"))) record.emplace("wire_provenance", *field(item, "provenance"));
        return flowcontracts::json::serialize(record);
    }
    void emit_graph_globals(std::ostringstream& out) const {
        out<<"declare void @flow_graph_enter(ptr)\ndeclare void @flow_graph_operation(i64)\ndeclare void @flow_graph_event(ptr)\ndeclare void @flow_graph_drop(ptr)\ndeclare void @flow_graph_fail(i64, ptr) noreturn\ndeclare void @flow_graph_state_before(ptr, i64, i64)\ndeclare void @flow_graph_state_after(ptr, i64, i64)\n"
           <<"declare void @flow_graph_stream_item_enter(ptr, i64, i64, i64)\ndeclare void @flow_graph_stream_enter(ptr, ptr, i64, i64, i64, i64)\ndeclare void @flow_graph_stream_event(ptr, i64, i64, i64)\ndeclare void @flow_graph_stream_drop(ptr, ptr, i64, i64, i64)\n";
        out<<"@flow.graph.division = private constant [17 x i8] c\"invalid_division\\00\"\n";
        for (const auto& step : graph_steps()) {
            const auto id = integer(field(step, "activation_id"), "activation_id");
            for (const auto* event : {"enter", "output", "drop"}) {
                const auto value = graph_record(step, event);
                out<<"@flow.graph."<<event<<"."<<id<<" = private constant ["<<value.size()+1<<" x i8] c\""<<escaped_string(value)<<"\"\n";
            }
            if (text(field(step, "kind")) == "stream_root" || text(field(step, "kind")) == "stream_receiver" || text(field(step, "kind")) == "persistent_receiver") {
                const auto node = text(field(step, "node_id"));
                const auto wire = text(field(step, "wire_id"));
                out<<"@flow.graph.stream.node."<<id<<" = private constant ["<<node.size()+1<<" x i8] c\""<<escaped_string(node)<<"\"\n";
                out<<"@flow.graph.stream.wire."<<id<<" = private constant ["<<wire.size()+1<<" x i8] c\""<<escaped_string(wire)<<"\"\n";
            }
        }
        out<<"@flow.graph.stream.bound = private constant [13 x i8] c\"stream_bound\\00\"\n";
    }
    void emit_graph_main(const Callable& entry, std::ostringstream& out) {
        std::map<std::string, const Json*> providers, receivers;
        for (const auto& node : graph_model_->providers) providers.emplace(text(field(node, "node_id")), &node);
        for (const auto& node : graph_model_->receivers) receivers.emplace(text(field(node, "node_id")), &node);
        const auto* schedule = field(root_, "graph_schedule");
        const auto schedule_version = schedule ? integer(field(*schedule, "version"), "graph_schedule.version") : 1;
        if (schedule_version == 2) {
            emit_stream_graph_main(entry, out, providers, receivers);
            return;
        }
        if (schedule_version == 3) {
            emit_persistent_graph_main(entry, out, providers, receivers);
            return;
        }
        std::map<int, std::string> output_types;
        out<<"define i32 @main() {\nentry:\n";
        for (const auto& step : graph_steps()) {
            const auto id = integer(field(step, "activation_id"), "activation_id");
            const auto node = text(field(step, "node_id"));
            out<<"  call void @flow_graph_enter(ptr @flow.graph.enter."<<id<<")\n";
            if (text(field(step, "kind")) == "startup") {
                if (!providers.count(node)) throw std::runtime_error("graph startup provider is absent");
                const auto p = provider(*field(*providers.at(node), "provider"));
                output_types[id] = llvm_type(p.result);
                out<<"  %graph.value."<<id<<" = call "<<output_types[id]<<" @"<<p.symbol<<"()\n";
            } else {
                if (!receivers.count(node)) throw std::runtime_error("graph receiver is absent");
                const auto& receiver = *receivers.at(node);
                const auto function = integer(field(receiver, "function_symbol_id"), "function_symbol_id");
                const auto input = integer(field(step, "input_activation_id"), "input_activation_id");
                if (!callables_.count(function) || !output_types.count(input)) throw std::runtime_error("graph receiver invocation identity is unavailable");
                const auto& callable = callables_.at(function);
                if (callable.parameters.size()!=1 || llvm_type(callable.parameters.front().second)!=output_types.at(input))
                    throw std::runtime_error("graph delivery carrier mismatch");
                output_types[id] = llvm_type(callable.result);
                out<<"  %graph.value."<<id<<" = call "<<output_types[id]<<" @"<<callable_name(callable)<<"("<<output_types.at(input)<<" %graph.value."<<input<<")\n";
            }
            out<<"  call void @flow_graph_event(ptr @flow.graph.output."<<id<<")\n";
            if (!std::get<bool>(*field(step, "output_connected"))) out<<"  call void @flow_graph_drop(ptr @flow.graph.drop."<<id<<")\n";
        }
        out<<"  call void @flow_graph_enter(ptr null)\n  %graph.exit = call i32 @"<<callable_name(entry)<<"()\n  ret i32 %graph.exit\n}\n";
    }
    void emit_stream_graph_main(const Callable& entry, std::ostringstream& out,
                                const std::map<std::string, const Json*>& providers,
                                const std::map<std::string, const Json*>& receivers) {
        if (providers.size() != 1) throw std::runtime_error("finite stream lowering requires one stream provider");
        const auto& stream = *providers.begin()->second;
        const auto item = provider(*field(stream, "provider"));
        const auto count = provider(*field(stream, "count_provider"));
        const auto item_type = llvm_type(item.result);
        if (item_type.empty() || item.parameters != "c_size_t" || count.parameters != "" || count.result != "c_size_t")
            throw std::runtime_error("unsupported finite stream provider ABI");
        const auto max_items = integer(field(stream, "max_items"), "stream.max_items");
        const auto& steps = graph_steps();
        if (steps.size() < 2 || text(field(steps.front(), "kind")) != "stream_root")
            throw std::runtime_error("finite stream schedule has no root template");
        const auto deliveries = steps.size() - 1;
        out<<"define i32 @main() {\nentry:\n"
           <<"  call void @flow_graph_enter(ptr @flow.graph.enter.0)\n"
           <<"  %flow.stream.count = call i64 @"<<count.symbol<<"()\n"
           <<"  %flow.stream.cap_ok = icmp ule i64 %flow.stream.count, "<<max_items<<"\n"
           <<"  br i1 %flow.stream.cap_ok, label %flow.stream.check, label %flow.stream.bound_failure\n"
           <<"flow.stream.bound_failure:\n  call void @flow_graph_fail(i64 0, ptr @flow.graph.stream.bound)\n  unreachable\n"
           <<"flow.stream.check:\n"
           <<"  %flow.stream.index = phi i64 [ 0, %entry ], [ %flow.stream.next, %flow.stream.body ]\n"
           <<"  %flow.stream.done = icmp uge i64 %flow.stream.index, %flow.stream.count\n"
           <<"  br i1 %flow.stream.done, label %flow.stream.exit, label %flow.stream.body\n"
           <<"flow.stream.body:\n"
           <<"  %flow.stream.signal = add i64 %flow.stream.index, 1\n"
           <<"  %flow.stream.item.activation = mul i64 %flow.stream.index, "<<deliveries<<"\n"
           <<"  call void @flow_graph_stream_item_enter(ptr @flow.graph.stream.node.0, i64 %flow.stream.item.activation, i64 %flow.stream.index, i64 %flow.stream.signal)\n"
           <<"  %flow.stream.item = call "<<item_type<<" @"<<item.symbol<<"(i64 %flow.stream.index)\n"
           <<"  call void @flow_graph_stream_event(ptr @flow.graph.stream.node.0, i64 0, i64 %flow.stream.index, i64 %flow.stream.signal)\n";
        for (std::size_t index = 1; index < steps.size(); ++index) {
            const auto& step = steps[index];
            const auto id = integer(field(step, "activation_id"), "activation_id");
            const auto node = text(field(step, "node_id"));
            if (!receivers.count(node)) throw std::runtime_error("finite stream receiver is absent");
            const auto& receiver = *receivers.at(node);
            const auto function = integer(field(receiver, "function_symbol_id"), "function_symbol_id");
            if (!callables_.count(function)) throw std::runtime_error("finite stream receiver identity is unavailable");
            const auto& callable = callables_.at(function);
            if (callable.parameters.size() != 1 || llvm_type(callable.parameters.front().second) != item_type)
                throw std::runtime_error("finite stream delivery carrier mismatch");
            const auto ordinal = static_cast<flowcontracts::json::Integer>(index - 1);
            out<<"  %flow.stream.activation."<<id<<" = add i64 %flow.stream.index, "<<(1 + ordinal * deliveries)<<"\n"
               <<"  call void @flow_graph_stream_enter(ptr @flow.graph.stream.node."<<id<<", ptr @flow.graph.stream.wire."<<id
               <<", i64 %flow.stream.activation."<<id<<", i64 %flow.stream.index, i64 %flow.stream.signal, i64 "<<id<<")\n"
               <<"  %flow.stream.receiver."<<id<<" = call "<<item_type<<" @"<<callable_name(callable)<<"("<<item_type<<" %flow.stream.item)\n"
               <<"  call void @flow_graph_stream_event(ptr @flow.graph.stream.node."<<id<<", i64 %flow.stream.activation."<<id<<", i64 %flow.stream.index, i64 %flow.stream.signal)\n"
               <<"  call void @flow_graph_stream_drop(ptr @flow.graph.stream.node."<<id<<", ptr @flow.graph.stream.wire."<<id<<", i64 %flow.stream.activation."<<id<<", i64 %flow.stream.index, i64 "<<id<<")\n";
        }
        out<<"  %flow.stream.next = add i64 %flow.stream.index, 1\n"
           <<"  br label %flow.stream.check\n"
           <<"flow.stream.exit:\n  call void @flow_graph_enter(ptr null)\n  %graph.exit = call i32 @"<<callable_name(entry)<<"()\n  ret i32 %graph.exit\n}\n";
    }
    void emit_persistent_graph_main(const Callable& entry, std::ostringstream& out,
                                    const std::map<std::string, const Json*>& providers,
                                    const std::map<std::string, const Json*>& receivers) {
        if (providers.size() != 1) throw std::runtime_error("persistent lowering requires one startup provider");
        const auto& root = *providers.begin()->second;
        const auto root_provider = provider(*field(root, "provider"));
        const auto root_type = llvm_type(root_provider.result);
        const auto& steps = graph_steps();
        if (steps.size() < 2 || text(field(steps.front(), "kind")) != "startup")
            throw std::runtime_error("persistent schedule has no startup root");
        std::map<std::string, std::string> state_slots;
        for (std::size_t index = 1; index < steps.size(); ++index) {
            const auto& step = steps[index];
            const auto node = text(field(step, "node_id"));
            if (text(field(step, "kind")) != "persistent_receiver" || !receivers.count(node))
                throw std::runtime_error("persistent schedule contains a non-persistent delivery");
            if (!state_slots.count(node)) {
                const auto initial = text(field(step, "state_initial_value"));
                try { (void)std::stoll(initial); } catch (...) { throw std::runtime_error("invalid persistent state initial value"); }
                state_slots.emplace(node, "%flow.state." + std::to_string(state_slots.size()));
            }
        }
        out<<"define i32 @main() {\nentry:\n";
        for (const auto& [node, slot_name] : state_slots) {
            const auto& step = *std::find_if(steps.begin(), steps.end(), [&](const auto& item) { return text(field(item, "node_id")) == node; });
            out<<"  "<<slot_name<<" = alloca i64, align 8\n  store i64 "<<text(field(step, "state_initial_value"))<<", ptr "<<slot_name<<"\n";
        }
        out<<"  call void @flow_graph_enter(ptr @flow.graph.enter.0)\n"
           <<"  %persistent.root = call "<<root_type<<" @"<<root_provider.symbol<<"()\n";
        for (std::size_t index = 1; index < steps.size(); ++index) {
            const auto& step = steps[index];
            const auto id = integer(field(step, "activation_id"), "activation_id");
            const auto node = text(field(step, "node_id"));
            const auto& receiver = *receivers.at(node);
            const auto function = integer(field(receiver, "function_symbol_id"), "function_symbol_id");
            if (!callables_.count(function)) throw std::runtime_error("persistent receiver identity is unavailable");
            const auto& callable = callables_.at(function);
            if (callable.parameters.size() != 2 || llvm_type(callable.parameters[0].second) != root_type ||
                callable.parameters[1].second != "c_long" || callable.result != "c_long")
                throw std::runtime_error("persistent receiver carrier mismatch");
            const auto& slot_name = state_slots.at(node);
            out<<"  call void @flow_graph_enter(ptr @flow.graph.enter."<<id<<")\n"
               <<"  %persistent.state.before."<<id<<" = load i64, ptr "<<slot_name<<"\n"
               <<"  call void @flow_graph_state_before(ptr @flow.graph.stream.node."<<id<<", i64 "<<id<<", i64 %persistent.state.before."<<id<<")\n"
               <<"  %persistent.state.after."<<id<<" = call i64 @"<<callable_name(callable)<<"("<<root_type<<" %persistent.root, i64 %persistent.state.before."<<id<<")\n"
               <<"  store i64 %persistent.state.after."<<id<<", ptr "<<slot_name<<"\n"
               <<"  call void @flow_graph_state_after(ptr @flow.graph.stream.node."<<id<<", i64 "<<id<<", i64 %persistent.state.after."<<id<<")\n"
               <<"  call void @flow_graph_event(ptr @flow.graph.output."<<id<<")\n";
            if (!std::get<bool>(*field(step, "output_connected"))) out<<"  call void @flow_graph_drop(ptr @flow.graph.drop."<<id<<")\n";
        }
        out<<"  call void @flow_graph_enter(ptr null)\n  %graph.exit = call i32 @"<<callable_name(entry)<<"()\n  ret i32 %graph.exit\n}\n";
    }
    void emit_function(const Callable& function,std::ostringstream& out) {
        temporary_=0; label_=0; call_results_.clear();
        const auto name=callable_name(function);
        return_carrier_ = function.result;
        const auto result_type = llvm_type(return_carrier_);
        if(result_type.empty() || (result_type == "ptr" && function.result != "c_string" && function.result != "Text") || (function.entry && result_type != "i32"))throw std::runtime_error("unsupported callable function signature");
        out<<"define "<<result_type<<" @"<<name<<"(";
        for(std::size_t index=0;index<function.parameters.size();++index){if(index)out<<", ";const auto type=llvm_type(function.parameters[index].second);if(type.empty())throw std::runtime_error("unsupported callable parameter type");out<<type<<" %flow_arg_"<<function.parameters[index].first;}
        out<<") {\nentry:\n"; emit_allocations(out);
        for(const auto& [symbol,type]:function.parameters)out<<"  store "<<llvm_type(type)<<" %flow_arg_"<<symbol<<", ptr "<<slot(symbol)<<"\n";
        out<<"  br label %flow_block_"<<function.body_block<<"\n";
        if (!emit_block(function.body_block,out,"flow_function_exit") && !function.entry)
            throw std::runtime_error("callable function has a path without a result: " + std::to_string(function.symbol));
        out<<"flow_function_exit:\n  ret "<<result_type<<" "<<(result_type == "ptr" ? "null" : "0")<<"\n}\n";
    }
    std::string load_symbol(int symbol,std::ostringstream& out) {
        const auto found=symbol_types_.find(symbol); if(found==symbol_types_.end()) return {};
        const auto name="%flow_load_"+std::to_string(temporary_++); out<<"  "<<name<<" = load "<<llvm_type(found->second)<<", ptr "<<slot(symbol)<<"\n"; return name;
    }
    std::pair<std::string,std::string> expression(const Json& value,std::ostringstream& out,std::string expected={}) {
        const auto kind=text(field(value,"kind")); auto type=text(field(value,"type")); if(!expected.empty()) type=expected;
        if(kind=="integer_literal") {
            const auto literal=text(field(value,"value"));
            if (llvm_type(type)=="ptr" && literal=="0") return {"ptr","null"};
            return {llvm_type(type),literal};
        }
        if(kind=="float_literal") {
            const auto literal=text(field(value,"value"));
            return llvm_type(type)=="double" && !literal.empty()?std::pair<std::string,std::string>{"double",literal}:std::pair<std::string,std::string>{};
        }
        if(kind=="bool_literal") {
            const auto literal=text(field(value,"value"));
            if(literal=="true") return {"i1","true"};
            if(literal=="false") return {"i1","false"};
            return {};
        }
        if(kind=="string_literal") {
            const auto literal = text(field(value, "value"));
            if (literal.empty() && type != "Text") return {"ptr", "null"};
            const int expression_id = integer(field(value, "expression_id"), "string_literal.expression_id");
            const auto found = string_sizes_.find(expression_id);
            if (found == string_sizes_.end()) return {};
            const auto result = "%flow_string_ptr_" + std::to_string(temporary_++);
            out << "  " << result << " = getelementptr [" << found->second << " x i8], ptr @flow_string_expr_"
                << expression_id << ", i64 0, i64 0\n";
            return {"ptr", result};
        }
        if(kind=="identifier") {
            const int symbol=integer(field(value,"symbol_id"),"symbol_id"); const auto native_type=llvm_type(symbol_types_[symbol]); auto loaded=load_symbol(symbol,out);
            const auto wanted=expected.empty()?native_type:llvm_type(expected); if(wanted==native_type) return {native_type,loaded};
            const auto converted="%flow_promote_"+std::to_string(temporary_++);
            if(native_type=="i32"&&wanted=="i64") out<<"  "<<converted<<" = sext i32 "<<loaded<<" to i64\n";
            else if(native_type=="i64"&&wanted=="i32") out<<"  "<<converted<<" = trunc i64 "<<loaded<<" to i32\n"; else return {};
            return {wanted,converted};
        }
        if(kind=="field_access") {
            const auto field_name=text(field(value,"field"));
            const auto* base=field(value,"base");
            if(!base || (field_name!="code" && field_name!="value")) return {};
            auto [base_type,base_value]=expression(*base,out,"TextOutcome");
            if(base_type!="{ i32, ptr }" || base_value.empty()) return {};
            const auto result="%flow_text_outcome_field_"+std::to_string(temporary_++);
            out<<"  "<<result<<" = extractvalue { i32, ptr } "<<base_value<<", "<<(field_name=="code"?"0":"1")<<"\n";
            return {field_name=="code"?"i32":"ptr",result};
        }
        if(kind=="call_result") {
            const int expression_id=integer(field(value,"expression_id"),"expression_id");
            const auto found=call_results_.find(expression_id); return found==call_results_.end()?std::pair<std::string,std::string>{}:found->second;
        }
        if(kind=="call" && text(field(value,"intrinsic"))=="list_length") return {"i32","%argc"};
        if(kind=="index" && text(field(value,"intrinsic"))=="list_index") {
            const auto [index_type,index_value]=expression(*field(value,"index"),out,"c_int");
            const auto address="%flow_arg_address_"+std::to_string(temporary_++), loaded="%flow_arg_"+std::to_string(temporary_++);
            out<<"  "<<address<<" = getelementptr ptr, ptr %argv, i32 "<<index_value<<"\n  "<<loaded<<" = load ptr, ptr "<<address<<"\n"; return {"ptr",loaded};
        }
        if(kind=="conversion") {
            const auto* operand=field(value,"operand"); if(!operand) return {};
            auto [from_type,from]=expression(*operand,out); const auto to=llvm_type(text(field(value,"type")));
            if(from_type==to) return {to,from};
            const auto result="%flow_convert_"+std::to_string(temporary_++);
            if(from_type=="i64"&&to=="i32") out<<"  "<<result<<" = trunc i64 "<<from<<" to i32\n";
            else if(from_type=="i32"&&to=="i64") out<<"  "<<result<<" = sext i32 "<<from<<" to i64\n"; else return {};
            return {to,result};
        }
        if(kind=="unary") {
            auto [operand_type,operand]=expression(*field(value,"operand"),out,type); const auto op=text(field(value,"operator"));
            if(op=="+") return {operand_type,operand};
            if(op!="-") return {};
            const auto result="%flow_unary_"+std::to_string(temporary_++); out<<"  "<<result<<" = sub "<<operand_type<<" 0, "<<operand<<"\n"; return {operand_type,result};
        }
        if(kind=="binary") {
            auto [left_type,left]=expression(*field(value,"left"),out); auto [right_type,right]=expression(*field(value,"right"),out,text(field(*field(value,"left"),"type")));
            const auto op=text(field(value,"operator")); std::string instruction;
            if(op=="==")instruction="eq"; else if(op=="!=")instruction="ne"; else if(op=="<")instruction="slt"; else if(op=="<=")instruction="sle"; else if(op==">")instruction="sgt"; else if(op==">=")instruction="sge";
            if(!instruction.empty()&&!left.empty()&&!right.empty()&&left_type==right_type) {
                const auto result="%flow_condition_"+std::to_string(temporary_++);
                if (left_type == "double") {
                    const auto float_instruction = op == "==" ? "oeq" : op == "!=" ? "one" : op == "<" ? "olt" : op == "<=" ? "ole" : op == ">" ? "ogt" : "oge";
                    out<<"  "<<result<<" = fcmp "<<float_instruction<<" double "<<left<<", "<<right<<"\n";
                } else out<<"  "<<result<<" = icmp "<<instruction<<" "<<left_type<<" "<<left<<", "<<right<<"\n";
                return {"i1",result};
            }
            if(op=="+")instruction="add"; else if(op=="-")instruction="sub"; else if(op=="*")instruction="mul"; else if(op=="/")instruction="sdiv"; else return {};
            if(left.empty()||right.empty()||left_type!=right_type) return {};
            if (graph_native_ && instruction == "sdiv") {
                if (left_type != "i32" && left_type != "i64") throw std::runtime_error("unsupported graph division carrier");
                const auto id = std::to_string(temporary_++);
                const auto minimum = left_type == "i32" ? "-2147483648" : "-9223372036854775808";
                out<<"  %graph.zero."<<id<<" = icmp eq "<<left_type<<" "<<right<<", 0\n"
                   <<"  %graph.min."<<id<<" = icmp eq "<<left_type<<" "<<left<<", "<<minimum<<"\n"
                   <<"  %graph.neg."<<id<<" = icmp eq "<<left_type<<" "<<right<<", -1\n"
                   <<"  %graph.overflow."<<id<<" = and i1 %graph.min."<<id<<", %graph.neg."<<id<<"\n"
                   <<"  %graph.bad."<<id<<" = or i1 %graph.zero."<<id<<", %graph.overflow."<<id<<"\n"
                   <<"  br i1 %graph.bad."<<id<<", label %graph.fail."<<id<<", label %graph.valid."<<id<<"\n"
                   <<"graph.fail."<<id<<":\n  call void @flow_graph_fail(i64 "<<active_operation_<<", ptr @flow.graph.division)\n  unreachable\n"
                   <<"graph.valid."<<id<<":\n";
            }
            const auto result="%flow_arithmetic_"+std::to_string(temporary_++); out<<"  "<<result<<" = "<<instruction<<" "<<left_type<<" "<<left<<", "<<right<<"\n"; return {left_type,result};
        }
        return {};
    }
    bool emit_block(int block,std::ostringstream& out,const std::string& continuation) {
        out<<"flow_block_"<<block<<":\n"; bool terminated=false;
        for(const auto* op:blocks_[block]) {
            if(terminated) break;
            active_operation_ = op->id;
            if(op->kind=="value_definition") {
                const auto kind=text(field(*op->operand,"kind"));
                if(kind=="writable_storage") out<<"  store ptr %flow_storage_ptr_"<<op->result_symbol<<", ptr "<<slot(op->result_symbol)<<"\n";
                else if(kind=="string_literal") { const auto value=expression(*op->operand,out,"Text").second; if(value.empty()) throw std::runtime_error("unsupported structured string definition"); out<<"  store ptr "<<value<<", ptr "<<slot(op->result_symbol)<<"\n"; }
                else { auto [type,value]=expression(*op->operand,out); if(value.empty()) throw std::runtime_error("unsupported structured value definition"); out<<"  store "<<type<<" "<<value<<", ptr "<<slot(op->result_symbol)<<"\n"; }
            } else if(op->kind=="call") {
                if(!callables_.count(op->callee_symbol))throw std::runtime_error("ordinary call target is unavailable");
                const auto& function=callables_.at(op->callee_symbol); if(function.body_block<0)throw std::runtime_error("ordinary call definition is unavailable");
                const auto& operands=array(field(find_json_operation(op->id),"operands"),"operation.operands");
                if(operands.size()!=function.parameters.size())throw std::runtime_error("ordinary call operand count mismatch");
                std::vector<std::pair<std::string,std::string>> args;
                for(std::size_t index=0;index<operands.size();++index)args.push_back(expression(operands[index],out,function.parameters[index].second));
                for (std::size_t index=0; index<args.size(); ++index)
                    if (args[index].first != llvm_type(function.parameters[index].second) || args[index].second.empty())
                        throw std::runtime_error("ordinary call argument carrier mismatch");
                const auto result_type = llvm_type(function.result);
                const auto result="%flow_call_"+std::to_string(op->id); out<<"  "<<result<<" = call "<<result_type<<" @"<<callable_name(function)<<"(";
                for(std::size_t index=0;index<args.size();++index){if(index)out<<", ";out<<args[index].first<<" "<<args[index].second;}out<<")\n";
                call_results_[op->expression]={result_type,result};
                if(op->result_symbol>=0)out<<"  store "<<result_type<<" "<<result<<", ptr "<<slot(op->result_symbol)<<"\n";
            } else if(op->kind=="external_call" || op->kind=="text_outcome") {
                if (graph_native_) out<<"  call void @flow_graph_operation(i64 "<<op->id<<")\n";
                const auto& p=*op->provider; const auto params=carriers(p.parameters); const auto& operands=array(field(find_json_operation(op->id),"operands"),"operation.operands");
                if(params.size()!=operands.size()) throw std::runtime_error("structured call operand count mismatch");
                std::vector<std::pair<std::string,std::string>> args; for(std::size_t i=0;i<params.size();++i) args.push_back(expression(operands[i],out,params[i]));
                std::string result;
                if (op->kind == "text_outcome" && p.result == "TextOutcome") {
                    result = "%flow_text_outcome_value_" + std::to_string(op->id);
                    out << "  " << result << " = call { i32, ptr } @" << p.symbol << "(";
                    for(std::size_t i=0;i<args.size();++i){if(i)out<<", ";out<<args[i].first<<" "<<args[i].second;} out << ")\n";
                } else if (op->kind == "text_outcome") {
                    out << "  %flow_text_outcome_" << op->id << " = alloca { i32, ptr }, align 8\n"
                        << "  %flow_text_status_" << op->id << " = call i32 @flow_text_concat_outcome(ptr " << args[0].second << ", ptr " << args[1].second << ", ptr %flow_text_outcome_" << op->id << ")\n"
                        << "  %flow_text_code_ptr_" << op->id << " = getelementptr { i32, ptr }, ptr %flow_text_outcome_" << op->id << ", i32 0, i32 0\n"
                        << "  %flow_text_code_" << op->id << " = load i32, ptr %flow_text_code_ptr_" << op->id << "\n"
                        << "  %flow_text_value_ptr_" << op->id << " = getelementptr { i32, ptr }, ptr %flow_text_outcome_" << op->id << ", i32 0, i32 1\n"
                        << "  %flow_text_value_" << op->id << " = load ptr, ptr %flow_text_value_ptr_" << op->id << "\n"
                        << "  %flow_text_code_ok_" << op->id << " = icmp eq i32 %flow_text_code_" << op->id << ", 0\n"
                        << "  %flow_text_value_ok_" << op->id << " = icmp ne ptr %flow_text_value_" << op->id << ", null\n"
                        << "  %flow_text_valid_" << op->id << " = and i1 %flow_text_code_ok_" << op->id << ", %flow_text_value_ok_" << op->id << "\n"
                        << "  br i1 %flow_text_valid_" << op->id << ", label %flow_text_ok_" << op->id << ", label %flow_text_fail_" << op->id << "\n"
                        << "flow_text_fail_" << op->id << ":\n  call void @llvm.trap()\n  unreachable\n"
                        << "flow_text_ok_" << op->id << ":\n";
                    result = "%flow_text_value_" + std::to_string(op->id);
                } else {
                    result = "%flow_call_" + std::to_string(op->id);
                    out << "  " << result << " = call " << llvm_type(p.result) << " @" << p.symbol << "(";
                    for(std::size_t i=0;i<args.size();++i){if(i)out<<", ";out<<args[i].first<<" "<<args[i].second;} out << ")\n";
                    if (p.result == "Text") {
                        out << "  %flow_text_valid_" << op->id << " = icmp ne ptr " << result << ", null\n"
                            << "  br i1 %flow_text_valid_" << op->id << ", label %flow_text_ok_" << op->id << ", label %flow_text_fail_" << op->id << "\n"
                            << "flow_text_fail_" << op->id << ":\n  call void @llvm.trap()\n  unreachable\n"
                            << "flow_text_ok_" << op->id << ":\n";
                    }
                }
                call_results_[op->expression]={llvm_type(p.result),result};
                if(op->result_symbol>=0) out<<"  store "<<llvm_type(p.result)<<" "<<result<<", ptr "<<slot(op->result_symbol)<<"\n";
            } else if(op->kind=="branch") {
                auto [type,condition]=expression(*op->operand,out); if(type!="i1"||condition.empty()) throw std::runtime_error("unsupported structured branch condition");
                const auto join="flow_join_"+std::to_string(label_++); const auto then_label="flow_block_"+std::to_string(op->then_block); const auto else_label=op->else_block>=0?"flow_block_"+std::to_string(op->else_block):join;
                out<<"  br i1 "<<condition<<", label %"<<then_label<<", label %"<<else_label<<"\n";
                const bool then_returns = emit_block(op->then_block,out,join);
                const bool else_returns = op->else_block>=0 && emit_block(op->else_block,out,join);
                out<<join<<":\n";
                if (then_returns && else_returns) { out<<"  unreachable\n"; terminated=true; }
            } else if(op->kind=="loop") {
                const auto condition_label="flow_loop_condition_"+std::to_string(label_++), exit_label="flow_loop_exit_"+std::to_string(label_++);
                out<<"  br label %"<<condition_label<<"\n"<<condition_label<<":\n";
                auto [type,condition]=expression(*op->operand,out); if(type!="i1"||condition.empty()) throw std::runtime_error("unsupported structured loop condition");
                out<<"  br i1 "<<condition<<", label %flow_block_"<<op->body_block<<", label %"<<exit_label<<"\n";
                emit_block(op->body_block,out,condition_label); out<<exit_label<<":\n";
            } else if(op->kind=="assignment") {
                auto [type,value]=expression(*op->operand,out); const auto target=symbol_types_.find(op->result_symbol);
                if(target==symbol_types_.end()||type!=llvm_type(target->second)||value.empty()) throw std::runtime_error("unsupported structured assignment");
                out<<"  store "<<type<<" "<<value<<", ptr "<<slot(op->result_symbol)<<"\n";
            } else if(op->kind=="return_value") {
                auto [type,value]=expression(*op->operand,out,return_carrier_); if(type!=llvm_type(return_carrier_)||value.empty()) throw std::runtime_error("unsupported structured return"); out<<"  ret "<<type<<" "<<value<<"\n"; terminated=true;
            }
        }
        if(!terminated) out<<"  br label %"<<continuation<<"\n";
        return terminated;
    }
    const Json& find_json_operation(int id) const {
        const auto& ops=array(field(*field(root_,"lowering_plan"),"operations"),"operations");
        for(const auto& item:ops) if(integer(field(item,"id"),"id")==id) return item;
        throw std::runtime_error("operation identity lost");
    }
};

inline std::optional<std::string> emit(std::string_view report,std::string_view binding) {
    const auto root=Parser{std::string(report)}.parse(); const auto auth=Parser{binding.empty()?"{}":std::string(binding)}.parse(); Emitter emitter(root,auth);
    if(!emitter.applicable()) {
        if (emitter.requires_structured_control()) throw std::runtime_error("structured plan lost its controlling branch operation");
        return std::nullopt;
    }
    return emitter.emit();
}

} // namespace flowlower::structured
