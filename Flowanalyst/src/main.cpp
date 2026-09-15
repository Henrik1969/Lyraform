#include <flowcontracts/json.hpp>
#include <flowcontracts/bounded_input.hpp>
#include <flowcontracts/diagnostics.hpp>
#include <flowcontracts/graph_provider_map.hpp>
#include <flowcontracts/source_graph.hpp>
#include <cctype>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <new>
#include <optional>
#include <sstream>
#include <set>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace {

constexpr std::string_view FLOWANALYST_VERSION = "0.1.0";

using Json = flowcontracts::json::Value;
using Object = flowcontracts::json::Object;
using Array = flowcontracts::json::Array;

class Parser {
public:
    explicit Parser(std::string text) : text_(std::move(text)) {}
    Json parse() const { return flowcontracts::json::parse(text_); }
private:
    std::string text_;
};

const Json* field(const Json& value, std::string_view name) { if (auto object = std::get_if<Object>(&value)) { auto it = object->find(std::string(name)); return it == object->end() ? nullptr : &it->second; } return nullptr; }
const Json* field(const Json* value, std::string_view name) { return value ? field(*value, name) : nullptr; }
std::string text(const Json* value, std::string fallback = {}) { if (value) if (auto v = std::get_if<std::string>(value)) return *v; return fallback; }
int integer(const Json* value, int fallback = -1) {
    if (!value || std::holds_alternative<std::nullptr_t>(*value)) return fallback;
    const auto parsed = flowcontracts::json::integer(*value, "$ integer field");
    if (parsed < std::numeric_limits<int>::min() || parsed > std::numeric_limits<int>::max()) throw std::runtime_error("JSON integer is outside int range");
    return static_cast<int>(parsed);
}
const Array& list(const Json* value) { static const Array empty; return value && std::holds_alternative<Array>(*value) ? std::get<Array>(*value) : empty; }
std::string quote(std::string_view value) { std::ostringstream out; out << '"'; for (char c : value) { if (c == '"' || c == '\\') out << '\\'; if (c == '\n') out << "\\n"; else if (c == '\r') out << "\\r"; else if (c != '\n') out << c; } return out.str() + '"'; }
void write_structured_failure(std::string_view code, std::string_view stage, std::string_view message) noexcept {
    std::fputs("{\"status\":\"failed\",\"code\":\"", stderr);
    flowcontracts::write_json_string(stderr, code);
    std::fputs("\",\"stage\":\"", stderr);
    flowcontracts::write_json_string(stderr, stage);
    std::fputs("\",\"message\":\"", stderr);
    flowcontracts::write_json_string(stderr, message);
    std::fputs("\",\"disposition\":\"no_artifact\"}\n", stderr);
}
struct FailureClassification { std::string_view code, stage; };
FailureClassification classify_failure(std::string_view) noexcept {
    return {"FLOWANALYST_INPUT_INVALID", "analysis"};
}
struct Diagnostic { std::string code, severity, message, ast_path, region, source; int symbol = -1, line = -1, column = -1; };
struct Target { int symbol = -1, mains = 0; std::string name; };
struct BindingRequirement { std::string contract, library, convention, symbol, effect, parameter_types, return_type, evidence; };
struct AbiTypeContract { std::string contract, name, repr, ownership, access, lifetime, nullable, opaque, cleanup; };
struct AggregateLayout { std::string contract, name; std::vector<std::pair<std::string, std::string>> fields; };
struct Region { std::string id, kind, status; std::vector<std::string> prerequisites; };
struct EffectFact { int declaration = -1, symbol = -1; std::string name, effect, certainty, reason; };
struct CallSite { int expression = -1, statement = -1, scope = -1, callee_symbol = -1, write_symbol = -1; std::string callee; bool pure = false; std::set<int> reads; std::string writes; std::vector<int> arguments; std::vector<int> independent_with; };
struct LoweringOperation { int expression = -1, statement = -1, scope = -1, block = -1, function_symbol = -1, then_block = -1, else_block = -1, body_block = -1, callee_symbol = -1, result_symbol = -1; std::string callee, kind, contract, library, convention, symbol, effect, parameter_types, return_type, evidence; std::vector<int> arguments; };
struct Callable { int symbol = -1, scope = -1, body_block = -1; bool entry = false; std::string name, return_type, availability; std::vector<std::pair<int, std::string>> parameters; };
struct Resolution { int expression = -1, statement = -1, scope = -1, symbol = -1; std::string name; };

std::string trim_copy(std::string value) {
    const auto first = value.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\n\r");
    return value.substr(first, last - first + 1);
}

bool valid_utf8(std::string_view value) {
    for (std::size_t index = 0; index < value.size();) {
        const unsigned char lead = static_cast<unsigned char>(value[index++]);
        std::size_t continuation = 0;
        unsigned int codepoint = 0;
        unsigned int minimum = 0;
        if (lead <= 0x7f) continue;
        if (lead >= 0xc2 && lead <= 0xdf) { continuation = 1; codepoint = lead & 0x1f; minimum = 0x80; }
        else if (lead >= 0xe0 && lead <= 0xef) { continuation = 2; codepoint = lead & 0x0f; minimum = 0x800; }
        else if (lead >= 0xf0 && lead <= 0xf4) { continuation = 3; codepoint = lead & 0x07; minimum = 0x10000; }
        else return false;
        if (index + continuation > value.size()) return false;
        for (std::size_t offset = 0; offset < continuation; ++offset) {
            const unsigned char byte = static_cast<unsigned char>(value[index++]);
            if ((byte & 0xc0) != 0x80) return false;
            codepoint = (codepoint << 6) | (byte & 0x3f);
        }
        if (codepoint < minimum || codepoint > 0x10ffff || (codepoint >= 0xd800 && codepoint <= 0xdfff)) return false;
    }
    return true;
}

std::vector<std::string> split_generic_arguments(const std::string& value) {
    std::vector<std::string> result; std::size_t start = 0, depth = 0;
    for (std::size_t i = 0; i < value.size(); ++i) { if (value[i] == '<') ++depth; else if (value[i] == '>') --depth; else if (value[i] == ',' && depth == 0) { result.push_back(trim_copy(value.substr(start, i - start))); start = i + 1; } }
    result.push_back(trim_copy(value.substr(start))); return result;
}

bool numeric_extents(const std::string& value) {
    if (value.empty() || value.front() != '[' || value.back() != ']') return false;
    for (std::size_t i = 1; i + 1 < value.size(); ++i) if (!std::isdigit(static_cast<unsigned char>(value[i])) && value[i] != ',' && value[i] != ' ') return false;
    return true;
}

int run(const Json& bundle, int lowering_plan_version, const Json& provider_map, int graph_plan_version) {
#ifdef FLOWANALYST_TEST_ALLOCATION_FAILURE
    (void)bundle;
    (void)lowering_plan_version;
    (void)provider_map;
    (void)graph_plan_version;
    throw std::bad_alloc();
#endif
    const auto graph_provider_selections = flowcontracts::graph_provider_map(provider_map);
    std::string graph_schedule_policy = "serial";
    bool graph_schedule_policy_set = false;
    for (const auto& selection : graph_provider_selections) {
        if (!graph_schedule_policy_set) { graph_schedule_policy = selection.schedule_policy; graph_schedule_policy_set = true; }
        else if (selection.schedule_policy != graph_schedule_policy) throw std::runtime_error("graph provider selections disagree on schedule policy");
    }
    if (text(field(bundle, "format")) != "flowmini.frontend_bundle" || integer(field(bundle, "version")) != 2) throw std::runtime_error("unsupported FlowMini frontend bundle");
    const auto* snapshot = field(bundle, "symbol_table"); if (!snapshot) throw std::runtime_error("bundle has no symbol_table");
    std::map<int, const Json*> symbols, scopes, origins;
    auto insert_identity = [](auto& index, int identity, const Json& entry, std::string_view kind) {
        if (identity < 0) throw std::runtime_error(std::string(kind) + " identity must be non-negative");
        if (!index.emplace(identity, &entry).second) throw std::runtime_error("duplicate " + std::string(kind) + " identity " + std::to_string(identity));
    };
    for (const auto& entry : list(field(snapshot, "symbols"))) insert_identity(symbols, integer(field(entry, "id")), entry, "symbol");
    for (const auto& entry : list(field(snapshot, "scopes"))) insert_identity(scopes, integer(field(entry, "id")), entry, "scope");
    for (const auto& entry : list(field(bundle, "symbol_origins"))) insert_identity(origins, integer(field(entry, "symbol_id")), entry, "symbol origin");
    const auto* ast = field(bundle, "ast");
    std::map<int, const Json*> expressions, statements, blocks, declarations;
    if (ast) {
        for (const auto& entry : list(field(ast, "expression_pool"))) insert_identity(expressions, integer(field(entry, "id")), entry, "expression");
        for (const auto& entry : list(field(ast, "statement_pool"))) insert_identity(statements, integer(field(entry, "id")), entry, "statement");
        for (const auto& entry : list(field(ast, "block_pool"))) insert_identity(blocks, integer(field(entry, "id")), entry, "block");
        for (const auto& entry : list(field(ast, "declaration_pool"))) insert_identity(declarations, integer(field(entry, "id")), entry, "declaration");
    }
    std::vector<Diagnostic> diagnostics;
    auto add_diagnostic = [&](std::string code, std::string message, int symbol, std::string region = {}) {
        Diagnostic item{std::move(code), "error", std::move(message), {}, std::move(region), text(field(field(bundle, "source"), "path")), symbol};
        if (symbol >= 0 && origins.count(symbol)) {
            const auto* origin = origins[symbol]; item.ast_path = text(field(*origin, "ast_path"));
            item.line = integer(field(field(*origin, "source_location"), "line"));
            item.column = integer(field(field(*origin, "source_location"), "column"));
        }
        diagnostics.push_back(std::move(item));
    };
    for (const auto& entry : list(field(bundle, "diagnostics"))) {
        add_diagnostic(text(field(entry, "code"), "FLOWMINI_FRONTEND_DIAGNOSTIC"), text(field(entry, "message"), "FlowMini frontend diagnostic"), -1);
        if (const auto* provenance = field(entry, "provenance")) {
            diagnostics.back().source = text(field(provenance, "source"), diagnostics.back().source);
            diagnostics.back().line = integer(field(provenance, "line"));
            diagnostics.back().column = integer(field(provenance, "column"));
        }
    }
    std::vector<Target> targets;
    std::vector<BindingRequirement> binding_requirements;
    std::vector<AbiTypeContract> abi_type_contracts;
    std::vector<AggregateLayout> aggregate_layouts;
    std::map<int, BindingRequirement> provider_functions;
    auto fact_value = [&](const Json& symbol, const std::string& key) {
        for (const auto& fact : list(field(symbol, "facts"))) if (text(field(fact, "key")) == key) return text(field(field(fact, "value"), "value"));
        return std::string{};
    };
    for (const auto& [contract_id, contract] : symbols) if (text(field(*contract, "kind")) == "Contract") {
        const auto library = fact_value(*contract, "library_spelling");
        const auto convention = fact_value(*contract, "convention_spelling");
        const int contract_scope = integer(field(*contract, "introduced_scope_id"));
        if (!scopes.count(contract_scope)) continue;
        for (const auto& child : list(field(*scopes[contract_scope], "symbol_ids"))) {
            const int child_id = integer(&child); if (!symbols.count(child_id) || text(field(*symbols[child_id], "kind")) != "Function") continue;
            const auto external = fact_value(*symbols[child_id], "external_symbol_spelling"); if (external.empty()) continue;
            std::string parameter_types;
            const int function_scope = integer(field(*symbols[child_id], "introduced_scope_id"));
            if (scopes.count(function_scope)) for (const auto& parameter : list(field(*scopes[function_scope], "symbol_ids"))) {
                const int parameter_id = integer(&parameter); if (!symbols.count(parameter_id) || text(field(*symbols[parameter_id], "kind")) != "Parameter") continue;
                if (!parameter_types.empty()) parameter_types += ',';
                parameter_types += fact_value(*symbols[parameter_id], "declared_type_spelling");
            }
            provider_functions.emplace(child_id, BindingRequirement{text(field(*contract, "name")), library, convention, external, fact_value(*symbols[child_id], "effect_spelling"), parameter_types, fact_value(*symbols[child_id], "return_type_spelling"), fact_value(*contract, "evidence_spelling")});
        }
        for (const auto& child : list(field(*scopes[contract_scope], "symbol_ids"))) {
            const int child_id = integer(&child);
            if (!symbols.count(child_id) || text(field(*symbols[child_id], "kind")) != "Struct") continue;
            AggregateLayout layout{text(field(*contract, "name")), text(field(*symbols[child_id], "name")), {}};
            const int struct_scope = integer(field(*symbols[child_id], "introduced_scope_id"));
            if (scopes.count(struct_scope)) for (const auto& field_id_json : list(field(*scopes[struct_scope], "symbol_ids"))) {
                const int field_id = integer(&field_id_json);
                if (!symbols.count(field_id) || text(field(*symbols[field_id], "kind")) != "Field") continue;
                layout.fields.emplace_back(text(field(*symbols[field_id], "name")), fact_value(*symbols[field_id], "declared_type_spelling"));
            }
            aggregate_layouts.push_back(std::move(layout));
        }
        for (const auto& child : list(field(*scopes[contract_scope], "symbol_ids"))) {
            const int child_id = integer(&child);
            if (!symbols.count(child_id) || text(field(*symbols[child_id], "kind")) != "Type") continue;
            const auto& type = *symbols[child_id];
            abi_type_contracts.push_back({
                text(field(*contract, "name")), text(field(type, "name")),
                fact_value(type, "repr_spelling"), fact_value(type, "ownership_spelling"),
                fact_value(type, "access_spelling"), fact_value(type, "lifetime_spelling"),
                fact_value(type, "nullable_spelling"), fact_value(type, "opaque_spelling"),
                fact_value(type, "cleanup_spelling")
            });
        }
    }
    int resolved_types = 0, unresolved_types = 0;
    const std::vector<std::string> builtin = {"bool", "Bool", "int8", "int16", "int32", "int64", "int128", "uint8", "uint16", "uint32", "uint64", "uint128", "float16", "float32", "float64", "float128", "char8", "char16", "char32", "int", "float", "string", "Text", "void"};
    auto is_builtin = [&](const std::string& value) { for (const auto& item : builtin) if (item == value) return true; return false; };
    const std::vector<std::string> abi_types = {"c_int", "c_long", "c_ulong", "c_size_t", "c_string", "c_pointer"};
    auto is_abi_type = [&](const std::string& value) { for (const auto& item : abi_types) if (item == value) return true; return false; };
    const std::vector<std::string> intrinsic_types = {"stdin.text", "start.record"};
    auto is_intrinsic_type = [&](const std::string& value) { for (const auto& item : intrinsic_types) if (item == value) return true; return false; };
    const std::vector<std::string> intrinsic_roots = {"stdin", "start"};
    const std::vector<std::string> intrinsic_functions = {"length"};
    std::map<std::string, int> type_symbols;
    for (const auto& [id, symbol] : symbols) { auto kind = text(field(*symbol, "kind")); if (kind == "Type" || kind == "Struct" || kind == "Contract") type_symbols[text(field(*symbol, "name"))] = id; }
    const std::vector<std::string> generic_constructors = {"list", "array", "optional", "collection.list", "result.Result"};
    std::function<bool(const std::string&)> is_resolved_type = [&](const std::string& raw_value) {
        const auto value = trim_copy(raw_value); if (is_builtin(value) || is_abi_type(value) || is_intrinsic_type(value) || type_symbols.count(value) != 0) return true;
        std::string core = value; const auto shape = value.find("["); if (shape != std::string::npos) { if (!numeric_extents(value.substr(shape)) || shape == 0) return false; core = value.substr(0, shape); }
        const auto open = core.find('<'); if (open == std::string::npos || core.back() != '>') return false;
        const auto constructor = core.substr(0, open); bool known = false; for (const auto& candidate : generic_constructors) if (constructor == candidate) known = true; if (!known) return false;
        const auto arguments = split_generic_arguments(core.substr(open + 1, core.size() - open - 2)); if (arguments.empty()) return false;
        for (const auto& argument : arguments) if (!is_resolved_type(argument)) return false;
        return true;
    };
    std::map<int, int> declaration_scopes, block_scopes, statement_scopes;
    for (const auto& [scope_id, scope] : scopes) {
        for (const auto& origin : list(field(bundle, "scope_origins"))) if (integer(field(origin, "scope_id")) == scope_id) {
            const auto path = text(field(origin, "ast_path"));
            const auto declaration_marker = std::string("/declaration_pool/");
            const auto block_marker = std::string("/block_pool/");
            if (path.rfind(declaration_marker, 0) == 0) declaration_scopes[std::stoi(path.substr(declaration_marker.size()))] = scope_id;
            if (path.rfind(block_marker, 0) == 0) block_scopes[std::stoi(path.substr(block_marker.size()))] = scope_id;
        }
    }
    auto nested_block = [&](const Json& statement) -> std::vector<int> {
        std::vector<int> result; const auto* payload = field(statement, "payload");
        for (const auto& key : {"body_block", "then_block"}) { int block = integer(field(payload, key)); if (block >= 0) result.push_back(block); }
        const auto* else_arm = field(payload, "else_arm"); int else_block = integer(field(else_arm, "block")); if (else_block >= 0) result.push_back(else_block);
        return result;
    };
    std::function<void(int, int)> assign_statements = [&](int block_id, int owner_scope) {
        if (!blocks.count(block_id)) return;
        int scope_id = block_scopes.count(block_id) ? block_scopes[block_id] : owner_scope;
        for (const auto& statement : list(field(*blocks[block_id], "statements"))) {
            int statement_id = integer(&statement); statement_scopes[statement_id] = scope_id;
            for (int child : nested_block(*statements[statement_id])) assign_statements(child, owner_scope);
            const int else_if = integer(field(field(field(*statements[statement_id], "payload"), "else_arm"), "if_statement"));
            if (else_if >= 0 && statements.count(else_if)) {
                statement_scopes[else_if] = scope_id;
                for (int child : nested_block(*statements[else_if])) assign_statements(child, owner_scope);
            }
        }
    };
    for (const auto& [declaration_id, declaration] : declarations) { int scope_id = declaration_scopes.count(declaration_id) ? declaration_scopes[declaration_id] : -1; int body = integer(field(*declaration, "body_block")); if (scope_id >= 0 && body >= 0) assign_statements(body, scope_id); }
    std::vector<Callable> callables;
    for (const auto& [declaration_id, declaration] : declarations) {
        const auto kind = text(field(*declaration, "kind"));
        if (kind != "function" && kind != "main_block") continue;
        const int scope_id = declaration_scopes.count(declaration_id) ? declaration_scopes[declaration_id] : -1;
        if (!scopes.count(scope_id)) continue;
        const int symbol_id = integer(field(*scopes.at(scope_id), "owner_symbol_id"));
        if (!symbols.count(symbol_id)) continue;
        Callable callable{symbol_id, scope_id, integer(field(*declaration, "body_block")), kind == "main_block",
                          kind == "main_block" ? "main" : text(field(*declaration, "name")),
                          kind == "main_block" ? "c_int" : fact_value(*symbols.at(symbol_id), "return_type_spelling"), "definition", {}};
        for (const auto& child : list(field(*scopes.at(scope_id), "symbol_ids"))) {
            const int parameter = integer(&child);
            if (symbols.count(parameter) && text(field(*symbols.at(parameter), "kind")) == "Parameter")
                callable.parameters.emplace_back(parameter, fact_value(*symbols.at(parameter), "declared_type_spelling"));
        }
        callables.push_back(std::move(callable));
    }
    std::set<int> catalogued_callables;
    for (const auto& callable : callables) catalogued_callables.insert(callable.symbol);
    for (const auto& [symbol_id, symbol] : symbols) {
        if (text(field(*symbol, "kind")) != "Function" || catalogued_callables.count(symbol_id)) continue;
        const int scope_id = integer(field(*symbol, "introduced_scope_id"));
        Callable callable{symbol_id, scope_id, -1, false, text(field(*symbol, "name")),
                          fact_value(*symbol, "return_type_spelling"), "declaration", {}};
        if (scopes.count(scope_id)) for (const auto& child : list(field(*scopes.at(scope_id), "symbol_ids"))) {
            const int parameter = integer(&child);
            if (symbols.count(parameter) && text(field(*symbols.at(parameter), "kind")) == "Parameter")
                callable.parameters.emplace_back(parameter, fact_value(*symbols.at(parameter), "declared_type_spelling"));
        }
        callables.push_back(std::move(callable));
    }
    Array graph_receivers, graph_providers;
    Json graph_model = nullptr;
    bool graph_native = false;
    if (const auto* graph = field(bundle, "graph_syntax")) {
        if (text(field(graph, "format")) != "flowmini.graph_syntax" || integer(field(graph, "version")) != 1)
            add_diagnostic("FLOWANALYST_GRAPH_VERSION", "unsupported graph syntax contract", -1);
        const auto* nodes = field(graph, "nodes");
        const auto* wires = field(graph, "wires");
        if (!nodes || !wires || !std::holds_alternative<Array>(*nodes) || !std::holds_alternative<Array>(*wires))
            add_diagnostic("FLOWANALYST_GRAPH_SCHEMA", "graph nodes and wires must be arrays", -1);
        auto graph_diagnostic = [&](const std::string& code, const std::string& message, const Json& subject) {
            add_diagnostic(code, message, -1);
            if (const auto* provenance = field(subject, "provenance")) {
                diagnostics.back().source = text(field(provenance, "source"), diagnostics.back().source);
                diagnostics.back().line = integer(field(provenance, "line"));
                diagnostics.back().column = integer(field(provenance, "column"));
            }
        };
        // A captured file cannot become executable by deleting frontend diagnostics.
        if (!list(nodes).empty() || !list(wires).empty())
            graph_diagnostic("FLOWANALYST_GRAPH_EXECUTION_UNSUPPORTED",
                "graph syntax is preserved but graph execution lowering is not yet implemented",
                !list(nodes).empty() ? list(nodes).front() : list(wires).front());
        std::set<std::string> node_ids, wire_ids;
        std::map<std::string, const Callable*> receiver_functions;
        std::map<std::string, const Json*> graph_states;
        for (const auto& state : list(field(graph, "states"))) {
            const auto state_node = text(field(state, "node_id"));
            if (state_node.empty() || !graph_states.emplace(state_node, &state).second)
                graph_diagnostic("FLOWANALYST_GRAPH_STATE_ID", "empty or duplicate persistent state identity", state);
            const auto state_type = text(field(state, "type"));
            const auto state_value = text(field(state, "value_text"));
            const bool admitted_state_type = state_type == "c_long" || std::any_of(aggregate_layouts.begin(), aggregate_layouts.end(), [&](const auto& layout) { return layout.name == state_type; });
            if (!admitted_state_type) graph_diagnostic("FLOWANALYST_GRAPH_STATE_TYPE", "persistent state requires c_long or a declared aggregate ABI layout", state);
            try {
                std::size_t parsed = 0;
                (void)std::stoll(state_value, &parsed);
                if (parsed != state_value.size()) throw std::invalid_argument("trailing state literal");
            } catch (const std::exception&) {
                graph_diagnostic("FLOWANALYST_GRAPH_STATE_VALUE", "persistent state requires a signed 64-bit literal", state);
            }
        }
        for (const auto& node : list(nodes)) {
            const auto id = text(field(node, "node_id"));
            if (id.empty() || !node_ids.insert(id).second)
                graph_diagnostic("FLOWANALYST_GRAPH_NODE_ID", "empty or duplicate graph node identity", node);
            const auto implementation_kind = text(field(node, "implementation_kind"));
            if (implementation_kind == "provider_atom") {
                const auto implementation = text(field(node, "implementation_name"));
                for (const auto& selection : graph_provider_selections) if (selection.implementation == implementation) {
                    const auto selected_callable = selection.activation == "finite_stream_once" ? selection.item_callable : selection.source_callable;
                    std::vector<int> candidates;
                    for (const auto& [identity, provider] : provider_functions)
                        if (provider.contract + "." + text(field(*symbols.at(identity), "name")) == selected_callable)
                            candidates.push_back(identity);
                    if (candidates.size() != 1) {
                        graph_diagnostic("FLOWANALYST_GRAPH_PROVIDER_RESOLUTION", "graph provider requires one resolved external declaration: " + selected_callable, node);
                        continue;
                    }
                    const auto identity = candidates.front();
                    const auto& provider = provider_functions.at(identity);
                    const auto stream = selection.activation == "finite_stream_once";
                    if (text(field(node, "role")) != "producer" ||
                        (!stream && !provider.parameter_types.empty()) ||
                        (stream && provider.parameter_types != "c_size_t") ||
                        provider.return_type.empty() || provider.return_type == "void") {
                        graph_diagnostic("FLOWANALYST_GRAPH_PROVIDER_CONTRACT", stream
                            ? "stream item provider requires one c_size_t argument and a value result"
                            : "startup provider requires a producer role and zero-argument value-returning external declaration", node);
                        continue;
                    }
                    auto provider_value = [&]() {
                        return Object{{"contract", provider.contract}, {"library", provider.library},
                            {"convention", provider.convention}, {"symbol", provider.symbol},
                            {"effect", provider.effect}, {"parameter_types", provider.parameter_types},
                            {"return_type", provider.return_type}, {"evidence", provider.evidence}};
                    };
                    if (!stream) {
                        graph_providers.push_back(Object{{"node_id", id}, {"function_symbol_id", identity},
                            {"implementation", implementation}, {"source_callable", selection.source_callable},
                            {"activation", std::string("startup_once")}, {"output_port", std::string("out")},
                            {"output_type", provider.return_type},
                            {"provenance", field(node, "provenance") ? *field(node, "provenance") : Json(nullptr)},
                            {"provider", provider_value()}});
                    } else {
                        std::vector<int> count_candidates;
                        for (const auto& [count_identity, count_provider] : provider_functions)
                            if (count_provider.contract + "." + text(field(*symbols.at(count_identity), "name")) == selection.count_callable)
                                count_candidates.push_back(count_identity);
                        if (count_candidates.size() != 1) {
                            graph_diagnostic("FLOWANALYST_GRAPH_PROVIDER_RESOLUTION", "stream count requires one resolved external declaration: " + selection.count_callable, node);
                            continue;
                        }
                        const auto count_identity = count_candidates.front();
                        const auto& count_provider = provider_functions.at(count_identity);
                        if (!count_provider.parameter_types.empty() || count_provider.return_type != "c_size_t") {
                            graph_diagnostic("FLOWANALYST_GRAPH_PROVIDER_CONTRACT", "stream count provider requires a zero-argument c_size_t result", node);
                            continue;
                        }
                        graph_providers.push_back(Object{{"node_id", id}, {"function_symbol_id", identity},
                            {"count_function_symbol_id", count_identity}, {"implementation", implementation},
                            {"count_callable", selection.count_callable}, {"item_callable", selection.item_callable},
                            {"activation", std::string("finite_stream_once")}, {"max_items", selection.max_items},
                            {"output_port", std::string("out")}, {"output_type", provider.return_type},
                            {"provenance", field(node, "provenance") ? *field(node, "provenance") : Json(nullptr)},
                            {"provider", provider_value()},
                            {"count_provider", Object{{"contract", count_provider.contract}, {"library", count_provider.library},
                                {"convention", count_provider.convention}, {"symbol", count_provider.symbol},
                                {"effect", count_provider.effect}, {"parameter_types", count_provider.parameter_types},
                                {"return_type", count_provider.return_type}, {"evidence", count_provider.evidence}}}});
                    }
                }
                continue;
            }
            if (implementation_kind != "source_function") {
                graph_diagnostic("FLOWANALYST_GRAPH_IMPLEMENTATION", "unknown graph implementation kind", node);
                continue;
            }
            const auto name = text(field(node, "implementation_name"));
            const bool persistent = field(node, "persistent") && std::holds_alternative<bool>(*field(node, "persistent")) && std::get<bool>(*field(node, "persistent"));
            std::vector<const Callable*> candidates;
            for (const auto& callable : callables)
                if (!callable.entry && callable.name == name) candidates.push_back(&callable);
            if (candidates.size() != 1) {
                graph_diagnostic("FLOWANALYST_GRAPH_RECEIVER_RESOLUTION",
                    "source receiver requires one unambiguous function: " + name, node);
                continue;
            }
            const auto& callable = *candidates.front();
            const auto state_type = persistent && graph_states.count(id) ? text(field(*graph_states.at(id), "type")) : std::string{};
            if (text(field(node, "role")) != "node" || callable.availability != "definition" ||
                callable.body_block < 0 || callable.parameters.size() != (persistent ? 2u : 1u) ||
                callable.return_type.empty() || callable.return_type == "void" ||
                (persistent && (!graph_states.count(id) || callable.parameters[1].second != state_type || callable.return_type != state_type))) {
                graph_diagnostic("FLOWANALYST_GRAPH_RECEIVER_CONTRACT",
                    persistent ? "persistent receiver requires a defined (input, state_type) -> state_type function and state declaration"
                               : "source receiver requires a defined one-input one-result function and node role", node);
                continue;
            }
            if (graph_plan_version == 2) {
                const auto native_carrier = [&](const std::string& type) {
                    return type == "int" || type == "bool" || type == "Bool" || type == "Text" ||
                        type == "c_int" || type == "c_long" || type == "c_ulong" ||
                        type == "c_size_t" || type == "c_string" || std::any_of(aggregate_layouts.begin(), aggregate_layouts.end(), [&](const auto& layout) { return layout.name == type; });
                };
                if (!native_carrier(callable.parameters.front().second) || (persistent && !native_carrier(callable.parameters[1].second)) || !native_carrier(callable.return_type)) {
                    graph_diagnostic("FLOWANALYST_GRAPH_RECEIVER_CARRIER",
                        "native source receiver requires an admitted scalar carrier", node);
                    continue;
                }
            }
            Object receiver = Object{{"node_id", id}, {"function_symbol_id", callable.symbol},
                {"parameter_symbol_id", callable.parameters.front().first},
                {"input_port", std::string("in")}, {"input_type", callable.parameters.front().second},
                {"output_port", std::string("out")}, {"output_type", callable.return_type},
                {"provenance", field(node, "provenance") ? *field(node, "provenance") : Json(nullptr)},
                {"activation_contract", std::string("fresh_single_input_v1")}};
            if (persistent) {
                const auto& state = *graph_states.at(id);
                receiver.emplace("state_contract", state_type == "c_long" ? "persistent_scalar_v1" : "persistent_aggregate_v1");
                receiver.emplace("state_type", state_type);
                receiver.emplace("state_initial_value", text(field(state, "value_text")));
                receiver.emplace("state_parameter_symbol_id", callable.parameters[1].first);
            }
            graph_receivers.push_back(std::move(receiver));
            receiver_functions.emplace(id, &callable);
        }
        for (const auto& [state_node, state] : graph_states) {
            bool target = false;
            for (const auto& node : list(nodes))
                if (text(field(node, "node_id")) == state_node && text(field(node, "implementation_kind")) == "source_function" &&
                    field(node, "persistent") && std::holds_alternative<bool>(*field(node, "persistent")) && std::get<bool>(*field(node, "persistent"))) target = true;
            if (!target) graph_diagnostic("FLOWANALYST_GRAPH_STATE_TARGET", "persistent state must target a persistent source receiver", *state);
        }
        std::set<std::string> connected_receivers;
        for (const auto& wire : list(wires)) {
            const auto id = text(field(wire, "wire_id"));
            if (id.empty() || !wire_ids.insert(id).second)
                graph_diagnostic("FLOWANALYST_GRAPH_WIRE_ID", "empty or duplicate graph wire identity", wire);
            for (const auto* side : {"from", "to"}) {
                const auto* endpoint = field(wire, side);
                if (!node_ids.count(text(field(endpoint, "node_id"))) || text(field(endpoint, "port_id")).empty())
                    graph_diagnostic("FLOWANALYST_GRAPH_ENDPOINT", "unknown node or empty graph port", wire);
                const auto node = text(field(endpoint, "node_id"));
                if (receiver_functions.count(node)) {
                    const bool input = std::string_view(side) == "to";
                    if (text(field(endpoint, "port_id")) != (input ? "in" : "out"))
                        graph_diagnostic("FLOWANALYST_GRAPH_RECEIVER_PORT", "source receiver endpoint has wrong port direction or identity", wire);
                    else if (input) connected_receivers.insert(node);
                }
            }
            const auto from = text(field(field(wire, "from"), "node_id"));
            const auto to = text(field(field(wire, "to"), "node_id"));
            for (const auto& provider : graph_providers) {
                const auto provider_node = text(field(provider, "node_id"));
                if (to == provider_node || (from == provider_node && text(field(field(wire, "from"), "port_id")) != "out"))
                    graph_diagnostic("FLOWANALYST_GRAPH_PROVIDER_PORT", "startup provider exposes only the out port", wire);
                if (from == provider_node && receiver_functions.count(to) &&
                    text(field(provider, "output_type")) != receiver_functions.at(to)->parameters.front().second)
                    graph_diagnostic("FLOWANALYST_GRAPH_PROVIDER_TYPE", "provider output and receiver input types differ", wire);
            }
            if (receiver_functions.count(from) && receiver_functions.count(to) &&
                receiver_functions.at(from)->return_type != receiver_functions.at(to)->parameters.front().second)
                graph_diagnostic("FLOWANALYST_GRAPH_RECEIVER_TYPE", "source receiver output and input types differ", wire);
        }
        for (const auto& node : list(nodes)) {
            const auto id = text(field(node, "node_id"));
            if (receiver_functions.count(id) && !connected_receivers.count(id))
                graph_diagnostic("FLOWANALYST_GRAPH_RECEIVER_INPUT", "source receiver requires a connected input", node);
        }
        std::set<std::pair<std::string, std::string>> policy_ids;
        for (const auto& policy : list(field(graph, "policies"))) {
            const auto node = text(field(policy, "node_id"));
            const auto key = text(field(policy, "key"));
            if (!node_ids.count(node) || key.empty() || !policy_ids.emplace(node, key).second)
                graph_diagnostic("FLOWANALYST_GRAPH_POLICY", "unknown policy node or empty/duplicate policy identity", policy);
            const auto kind = text(field(policy, "value_kind"));
            if (kind != "integer" && kind != "string" && kind != "boolean")
                graph_diagnostic("FLOWANALYST_GRAPH_POLICY_VALUE", "unsupported graph policy value kind", policy);
            if (receiver_functions.count(node))
                graph_diagnostic("FLOWANALYST_GRAPH_RECEIVER_POLICY", "source receiver policies are unsupported", policy);
        }
        if (!list(nodes).empty() || !list(wires).empty() || !list(field(graph, "policies")).empty()) {
            graph_model = Object{{"format", std::string("flowcore.source_graph")}, {"version", graph_plan_version},
                {"status", std::string(graph_plan_version == 2 ? "ready" : "non_executable")},
                {"syntax", *graph}, {"receivers", graph_receivers}, {"providers", graph_providers},
                {"aggregate_abi_layouts", [&]() { Array layouts; for (const auto& layout : aggregate_layouts) { Array fields; for (const auto& field : layout.fields) fields.emplace_back(Object{{"name", field.first}, {"type", field.second}}); layouts.emplace_back(Object{{"contract", layout.contract}, {"fields", fields}, {"layout_policy", "provider_verified_required"}, {"name", layout.name}, {"status", "declared"}, {"version", flowcontracts::json::Integer{1}}}); } return layouts; }()},
                {"schedule_policy", graph_schedule_policy},
                {"provider_selection", provider_map}};
            if (graph_plan_version == 2) {
                try {
                    if (lowering_plan_version != 2) throw std::runtime_error("native graph requires callable lowering plan version 2");
                    (void)flowcontracts::source_graph(graph_model);
                    graph_native = true;
                    diagnostics.erase(std::remove_if(diagnostics.begin(), diagnostics.end(), [](const Diagnostic& diagnostic) {
                        return diagnostic.code == "FLOWMINI_GRAPH_LOWERING_UNSUPPORTED" || diagnostic.code == "FLOWANALYST_GRAPH_EXECUTION_UNSUPPORTED";
                    }), diagnostics.end());
                } catch (const std::exception& error) {
                    graph_diagnostic("FLOWANALYST_NATIVE_GRAPH_CONTRACT", error.what(), list(nodes).empty() ? *graph : list(nodes).front());
                }
            }
        }
    }
    std::vector<Resolution> resolutions;
    std::map<int, std::pair<int, int>> expression_context;
    std::set<std::pair<int, int>> visited_expressions;
    std::function<void(int, int, int)> resolve_expression = [&](int expression_id, int statement_id, int scope_id) {
        if (!expressions.count(expression_id) || scope_id < 0 || !visited_expressions.emplace(expression_id, scope_id).second) return;
        expression_context[expression_id] = {statement_id, scope_id};
        const auto* expression = expressions[expression_id]; if (text(field(*expression, "kind")) == "identifier") {
            const auto name = text(field(field(*expression, "payload"), "name")); int current = scope_id, found = -1; bool ambiguous = false;
            while (current >= 0 && scopes.count(current) && found < 0) {
                for (const auto& candidate : list(field(*scopes[current], "symbol_ids"))) { int candidate_id = integer(&candidate); if (symbols.count(candidate_id) && text(field(*symbols[candidate_id], "name")) == name) { found = candidate_id; break; } }
                if (found < 0) {
                    std::vector<int> contract_matches;
                    for (const auto& child : list(field(*scopes[current], "child_scope_ids"))) {
                        const int child_id = integer(&child); if (!scopes.count(child_id) || text(field(*scopes[child_id], "kind")) != "Contract") continue;
                        for (const auto& candidate : list(field(*scopes[child_id], "symbol_ids"))) {
                            const int candidate_id = integer(&candidate);
                            if (symbols.count(candidate_id) && text(field(*symbols[candidate_id], "name")) == name) contract_matches.push_back(candidate_id);
                        }
                    }
                    if (contract_matches.size() == 1) found = contract_matches.front();
                    else if (contract_matches.size() > 1) {
                        ambiguous = true;
                        add_diagnostic("FLOWANALYST_AMBIGUOUS_NAME", "unqualified name '" + name + "' is provided by multiple imported contracts; qualify it with its namespace", -1, "scope:" + std::to_string(scope_id));
                        break;
                    }
                }
                current = integer(field(*scopes[current], "parent_id"));
            }
            resolutions.push_back({expression_id, statement_id, scope_id, found, name});
            bool intrinsic = false; for (const auto& item : intrinsic_roots) if (item == name) intrinsic = true; for (const auto& item : intrinsic_functions) if (item == name) intrinsic = true;
            if (found < 0 && !intrinsic && !ambiguous) add_diagnostic("FLOWANALYST_UNRESOLVED_NAME", "name '" + name + "' cannot be resolved", -1, "scope:" + std::to_string(scope_id));
        }
        for (const auto& child : list(field(*expression, "child_expressions"))) resolve_expression(integer(&child), statement_id, scope_id);
    };
    for (const auto& [statement_id, statement] : statements) { int scope_id = statement_scopes.count(statement_id) ? statement_scopes[statement_id] : -1; for (const auto& expression : list(field(*statement, "expression_ids"))) resolve_expression(integer(&expression), statement_id, scope_id); }
    std::map<int, int> resolved_expression_symbols;
    for (const auto& resolution : resolutions) if (resolution.symbol >= 0) resolved_expression_symbols[resolution.expression] = resolution.symbol;
    // A qualified call is represented by the AST as call(field_access(namespace, member)).
    // Resolve that field against the imported contract scope so downstream stages
    // receive the actual provider function symbol, not merely the namespace root.
    for (const auto& [expression_id, expression] : expressions) {
        if (text(field(*expression, "kind")) != "field_access") continue;
        const auto* payload = field(*expression, "payload");
        const int base = integer(field(*payload, "base"));
        if (!resolved_expression_symbols.count(base)) continue;
        const int namespace_symbol = resolved_expression_symbols[base];
        const auto member_name = text(field(*payload, "field"));
        int member_symbol = -1;
        for (const auto& [scope_id, scope] : scopes) {
            if (integer(field(*scope, "owner_symbol_id")) != namespace_symbol) continue;
            for (const auto& candidate : list(field(*scope, "symbol_ids"))) {
                const int candidate_id = integer(&candidate);
                if (symbols.count(candidate_id) && text(field(*symbols[candidate_id], "name")) == member_name) {
                    member_symbol = candidate_id;
                    break;
                }
            }
            if (member_symbol >= 0) break;
        }
        if (member_symbol >= 0) {
            const auto context = expression_context.count(expression_id)
                ? expression_context[expression_id]
                : std::pair<int, int>{-1, -1};
            resolutions.push_back({expression_id, context.first, context.second, member_symbol,
                                   text(field(*expression, "text"), member_name)});
            resolved_expression_symbols[expression_id] = member_symbol;
        }
    }
    std::map<int, std::string> symbol_types;
    for (const auto& [id, symbol] : symbols) for (const auto& fact : list(field(*symbol, "facts"))) {
        const auto key = text(field(fact, "key"));
        if (key == "declared_type_spelling" || key == "return_type_spelling") {
            symbol_types[id] = text(field(field(fact, "value"), "value"));
            break;
        }
    }
    std::map<int, int> text_initializers;
    for (const auto& [statement_id, statement] : statements) {
        if (text(field(*statement, "kind")) != "let") continue;
        const auto* payload = field(*statement, "payload");
        const int initializer = integer(field(payload, "initializer_expression"));
        const int scope_id = statement_scopes.count(statement_id) ? statement_scopes.at(statement_id) : -1;
        const auto name = text(field(*statement, "name"));
        if (initializer < 0 || scope_id < 0 || !scopes.count(scope_id)) continue;
        for (const auto& candidate : list(field(*scopes.at(scope_id), "symbol_ids"))) {
            const int symbol_id = integer(&candidate);
            if (symbols.count(symbol_id) && text(field(*symbols.at(symbol_id), "name")) == name &&
                symbol_types[symbol_id] == "Text") {
                text_initializers[symbol_id] = initializer;
                break;
            }
        }
    }
    std::function<bool(int)> expression_is_text = [&](int expression_id) {
        if (!expressions.count(expression_id)) return false;
        const auto* expression = expressions.at(expression_id);
        const auto kind = text(field(*expression, "kind"));
        if (kind == "string_literal") return true;
        if (kind == "identifier") {
            const int symbol = resolved_expression_symbols.count(expression_id) ? resolved_expression_symbols.at(expression_id) : -1;
            return symbol >= 0 && symbol_types[symbol] == "Text";
        }
        if (kind == "call") {
            const int base = integer(field(field(*expression, "payload"), "base"));
            const int symbol = resolved_expression_symbols.count(base) ? resolved_expression_symbols.at(base) : -1;
            if (symbol >= 0 && symbol_types[symbol] == "Text") return true;
            const auto callee = text(field(*expression, "text"));
            for (const auto& callable : callables) if (callable.name == callee && callable.return_type == "Text") return true;
            return false;
        }
        if (kind == "field_access") {
            const auto* payload = field(*expression, "payload");
            const int base = integer(field(payload, "base"));
            const int symbol = resolved_expression_symbols.count(base) ? resolved_expression_symbols.at(base) : -1;
            return symbol >= 0 && symbol_types[symbol] == "TextOutcome" &&
                text(field(payload, "field")) == "value";
        }
        if (kind != "binary") return false;
        const auto* payload = field(*expression, "payload");
        return text(field(payload, "operator")) == "+" &&
               expression_is_text(integer(field(payload, "left"))) &&
               expression_is_text(integer(field(payload, "right")));
    };
    std::function<std::optional<std::string>(int)> constant_text = [&](int expression_id) -> std::optional<std::string> {
        if (!expressions.count(expression_id)) return std::nullopt;
        const auto* expression = expressions.at(expression_id);
        const auto kind = text(field(*expression, "kind"));
        if (kind == "string_literal") return text(field(field(*expression, "payload"), "value_text"));
        if (kind == "identifier") {
            const int symbol = resolved_expression_symbols.count(expression_id) ? resolved_expression_symbols.at(expression_id) : -1;
            if (text_initializers.count(symbol)) return constant_text(text_initializers.at(symbol));
            return std::nullopt;
        }
        if (kind != "binary") return std::nullopt;
        const auto* payload = field(*expression, "payload");
        if (text(field(payload, "operator")) != "+") return std::nullopt;
        const auto left = constant_text(integer(field(payload, "left")));
        const auto right = constant_text(integer(field(payload, "right")));
        if (!left || !right) return std::nullopt;
        return *left + *right;
    };
    std::optional<std::pair<int, BindingRequirement>> text_concat_provider;
    for (const auto& [symbol_id, requirement] : provider_functions)
        if (requirement.parameter_types == "Text,Text" && requirement.return_type == "Text" && requirement.effect == "memory") {
            if (text_concat_provider) {
                text_concat_provider.reset();
                break;
            }
            text_concat_provider = std::make_pair(symbol_id, requirement);
        }
    std::map<int, std::pair<int, BindingRequirement>> runtime_text_concats;
    for (const auto& [expression_id, expression] : expressions) {
        if (text(field(*expression, "kind")) != "string_literal") continue;
        if (!valid_utf8(text(field(field(*expression, "payload"), "value_text"))))
            add_diagnostic("FLOWANALYST_TEXT_INVALID_UTF8", "Text literal is not valid UTF-8", -1, "expression:" + std::to_string(expression_id));
    }
    for (const auto& [statement_id, statement] : statements) {
        if (text(field(*statement, "kind")) != "let") continue;
        const auto* payload = field(*statement, "payload");
        const int initializer = integer(field(payload, "initializer_expression"));
        const auto name = text(field(*statement, "name"));
        const int scope_id = statement_scopes.count(statement_id) ? statement_scopes.at(statement_id) : -1;
        int symbol_id = -1;
        if (scopes.count(scope_id)) for (const auto& candidate : list(field(*scopes.at(scope_id), "symbol_ids"))) {
            const int candidate_id = integer(&candidate);
            if (symbols.count(candidate_id) && text(field(*symbols.at(candidate_id), "name")) == name) { symbol_id = candidate_id; break; }
        }
        if (symbol_id >= 0 && symbol_types[symbol_id] == "Text" && initializer >= 0 && !expression_is_text(initializer))
            add_diagnostic("FLOWANALYST_TEXT_CSTRING_CONFUSION", "Text initializer requires a Text value; c_string is borrowed and cannot convert implicitly", symbol_id, "symbol:" + std::to_string(symbol_id));
    }
    for (const auto& [expression_id, expression] : expressions) {
        if (text(field(*expression, "kind")) != "binary") continue;
        const auto* payload = field(*expression, "payload");
        const bool left_text = expression_is_text(integer(field(payload, "left")));
        const bool right_text = expression_is_text(integer(field(payload, "right")));
        if (text(field(payload, "operator")) == "+" && left_text != right_text)
            add_diagnostic("FLOWANALYST_TEXT_CSTRING_CONFUSION", "Text concatenation does not implicitly accept c_string", -1, "expression:" + std::to_string(expression_id));
        if (text(field(payload, "operator")) == "+" && left_text && right_text && !constant_text(expression_id)) {
            if (text_concat_provider)
                runtime_text_concats.emplace(expression_id, *text_concat_provider);
            else
                add_diagnostic("FLOWANALYST_TEXT_DYNAMIC_CONCAT", "Text concatenation requires the declared bounded Text runtime capability", -1, "expression:" + std::to_string(expression_id));
        }
    }
    for (const auto& [expression_id, expression] : expressions) if (text(field(*expression, "kind")) == "field_access") {
        const auto* payload = field(*expression, "payload");
        const int base = integer(field(*payload, "base"));
        const auto field_name = text(field(*payload, "field"));
        if (!resolved_expression_symbols.count(base)) continue;
        const int base_symbol = resolved_expression_symbols[base];
        if (!symbol_types.count(base_symbol) || !type_symbols.count(symbol_types[base_symbol])) continue;
        const int record_symbol = type_symbols[symbol_types[base_symbol]];
        bool found_field = false;
        for (const auto& [scope_id, scope] : scopes) {
            if (text(field(*scope, "kind")) != "Struct") continue;
            if (integer(field(*scope, "owner_symbol_id")) != record_symbol) continue;
            for (const auto& candidate : list(field(*scope, "symbol_ids"))) {
                const int candidate_id = integer(&candidate);
                if (symbols.count(candidate_id) && text(field(*symbols[candidate_id], "name")) == field_name) found_field = true;
            }
        }
        if (!found_field) add_diagnostic("FLOWANALYST_UNKNOWN_FIELD", "record type '" + symbol_types[base_symbol] + "' has no field '" + field_name + "'", base_symbol, "expression:" + std::to_string(expression_id));
    }
    for (const auto& [expression_id, expression] : expressions) if (text(field(*expression, "kind")) == "call") {
        int base = integer(field(field(*expression, "payload"), "base")); if (!resolved_expression_symbols.count(base)) continue;
        int callable = resolved_expression_symbols[base], declaration_id = -1; const auto* origin = origins.count(callable) ? origins[callable] : nullptr;
        if (origin) { const auto path = text(field(*origin, "ast_path")); const auto marker = std::string("/declaration_pool/"); if (path.rfind(marker, 0) == 0) declaration_id = std::stoi(path.substr(marker.size())); }
        if (!declarations.count(declaration_id)) continue;
        int expected = static_cast<int>(list(field(*declarations[declaration_id], "parameters")).size());
        for (const auto& [scope_id, scope] : scopes) if (integer(field(*scope, "owner_symbol_id")) == callable) {
            int scoped_parameters = 0;
            for (const auto& candidate : list(field(*scope, "symbol_ids"))) {
                const int candidate_id = integer(&candidate);
                if (symbols.count(candidate_id) && text(field(*symbols[candidate_id], "kind")) == "Parameter") ++scoped_parameters;
            }
            expected = scoped_parameters;
            break;
        }
        const int actual = static_cast<int>(list(field(field(*expression, "payload"), "arguments")).size());
        if (expected != actual) add_diagnostic("FLOWANALYST_CALL_ARITY", "call to '" + text(field(*symbols[callable], "name")) + "' expects " + std::to_string(expected) + " argument(s), got " + std::to_string(actual), callable, "symbol:" + std::to_string(callable));
    }
    for (const auto& [scope_id, scope] : scopes) {
        std::map<std::string, std::vector<int>> names;
        for (const auto& child : list(field(*scope, "symbol_ids"))) { int id = integer(&child); if (symbols.count(id)) names[text(field(*symbols[id], "name"))].push_back(id); }
        for (const auto& [name, ids] : names) {
            if (name.empty() || ids.size() <= 1) continue;
            bool imports_only = true;
            for (const int id : ids) if (text(field(*symbols[id], "kind")) != "Import") imports_only = false;
            if (!imports_only) add_diagnostic("FLOWANALYST_DUPLICATE_NAME", "name '" + name + "' is declared more than once in scope " + std::to_string(scope_id), ids.front(), "scope:" + std::to_string(scope_id));
        }
    }
    for (const auto& [id, symbol] : symbols) for (const auto& fact : list(field(*symbol, "facts"))) if (text(field(fact, "key")) == "declared_type_spelling" || text(field(fact, "key")) == "return_type_spelling") {
        auto value = text(field(field(fact, "value"), "value")); if (value.empty()) continue; if (is_resolved_type(value)) ++resolved_types; else { ++unresolved_types; add_diagnostic("FLOWANALYST_UNKNOWN_TYPE", "declared type '" + value + "' cannot be resolved", id, "symbol:" + std::to_string(id)); }
    }
    int refined_types = 0;
    std::function<void(int, int)> check_invariant = [&](int expression_id, int refined_symbol) {
        if (!expressions.count(expression_id)) return;
        const auto* expression = expressions[expression_id];
        if (text(field(*expression, "kind")) == "identifier" && text(field(field(*expression, "payload"), "name")) != "value") add_diagnostic("FLOWANALYST_INVARIANT_NAME", "refined-type invariant name is not bound: '" + text(field(field(*expression, "payload"), "name")) + "'", refined_symbol, "symbol:" + std::to_string(refined_symbol));
        for (const auto& child : list(field(*expression, "child_expressions"))) check_invariant(integer(&child), refined_symbol);
    };
    for (const auto& [declaration_id, declaration] : declarations) if (text(field(*declaration, "kind")) == "refined_type") {
        ++refined_types; int refined_symbol = -1; for (const auto& [symbol_id, origin] : origins) { const auto path = text(field(*origin, "ast_path")); if (path == "/declaration_pool/" + std::to_string(declaration_id)) { refined_symbol = symbol_id; break; } }
        const auto base = text(field(*declaration, "base_type")); if (!is_resolved_type(base)) add_diagnostic("FLOWANALYST_REFINED_BASE_TYPE", "refined type base '" + base + "' cannot be resolved", refined_symbol, "symbol:" + std::to_string(refined_symbol));
        for (const auto& invariant : list(field(*declaration, "invariants"))) check_invariant(integer(field(invariant, "condition_expression")), refined_symbol);
    }
    for (const auto& [id, symbol] : symbols) if (text(field(*symbol, "kind")) == "Namespace") {
        int scope_id = integer(field(*symbol, "introduced_scope_id")); int mains = 0; if (scopes.count(scope_id)) for (const auto& child : list(field(*scopes[scope_id], "symbol_ids"))) { int child_id = integer(&child); if (symbols.count(child_id) && text(field(*symbols[child_id], "name")) == "main" && text(field(*symbols[child_id], "kind")) == "Procedure") ++mains; }
        targets.push_back({id, mains, text(field(*symbol, "name"))}); if (mains != 1) add_diagnostic("FLOWANALYST_TARGET_ENTRYPOINT", "target '" + targets.back().name + "' must contain exactly one main procedure", id, "target:" + targets.back().name);
    }
    std::function<bool(int)> expression_is_pure = [&](int expression_id) {
        if (!expressions.count(expression_id)) return false;
        const auto* expression = expressions[expression_id];
        const auto kind = text(field(*expression, "kind"));
        if (kind == "integer_literal" || kind == "float_literal" || kind == "bool_literal" || kind == "string_literal" || kind == "identifier") return true;
        if (kind != "binary" && kind != "unary") return false;
        for (const auto& child : list(field(*expression, "child_expressions"))) if (!expression_is_pure(integer(&child))) return false;
        return true;
    };
    std::vector<EffectFact> effect_facts;
    for (const auto& [declaration_id, declaration] : declarations) {
        if (text(field(*declaration, "kind")) != "function") continue;
        EffectFact fact;
        fact.declaration = declaration_id;
        fact.name = text(field(*declaration, "name"), "<anonymous>");
        for (const auto& [symbol_id, origin] : origins) if (text(field(*origin, "ast_path")) == "/declaration_pool/" + std::to_string(declaration_id)) { fact.symbol = symbol_id; break; }
        const int body = integer(field(*declaration, "body_block"));
        const auto& body_statements = blocks.count(body) ? list(field(*blocks[body], "statements")) : list(nullptr);
        bool pure = !body_statements.empty();
        for (const auto& statement_id : body_statements) {
            if (!statements.count(integer(&statement_id)) || text(field(*statements[integer(&statement_id)], "kind")) != "return") { pure = false; break; }
            const auto* payload = field(*statements[integer(&statement_id)], "payload");
            const int value = integer(field(payload, "value_expression"));
            if (!expression_is_pure(value)) { pure = false; break; }
        }
        fact.effect = pure ? "pure" : "unknown";
        fact.certainty = pure ? "proven" : "unresolved";
        fact.reason = pure ? "return-only expression with no calls or external effects" : "body contains mutation, control state, calls, or unsupported effects";
        effect_facts.push_back(std::move(fact));
    }
    std::map<int, bool> pure_symbols;
    for (const auto& fact : effect_facts) if (fact.symbol >= 0) pure_symbols[fact.symbol] = fact.effect == "pure" && fact.certainty == "proven";
    std::function<void(int, std::set<int>&)> collect_reads = [&](int expression_id, std::set<int>& reads) {
        if (!expressions.count(expression_id)) return;
        const auto* expression = expressions[expression_id];
        if (text(field(*expression, "kind")) == "identifier" && resolved_expression_symbols.count(expression_id)) reads.insert(resolved_expression_symbols[expression_id]);
        for (const auto& child : list(field(*expression, "child_expressions"))) collect_reads(integer(&child), reads);
    };
    auto visible_symbol = [&](int scope_id, const std::string& name) {
        int current = scope_id;
        while (current >= 0 && scopes.count(current)) {
            for (const auto& symbol_id : list(field(*scopes.at(current), "symbol_ids"))) {
                const int candidate = integer(&symbol_id);
                if (symbols.count(candidate) && text(field(*symbols.at(candidate), "name")) == name) return candidate;
            }
            current = integer(field(*scopes.at(current), "parent_id"));
        }
        return -1;
    };
    std::vector<CallSite> call_sites;
    for (const auto& [expression_id, expression] : expressions) {
        if (text(field(*expression, "kind")) != "call") continue;
        const auto* payload = field(*expression, "payload");
        const int base = integer(field(payload, "base"));
        const Resolution* base_resolution = nullptr;
        for (const auto& resolution : resolutions) if (resolution.expression == base) { base_resolution = &resolution; break; }
        if (!base_resolution) continue;
        CallSite site;
        site.expression = expression_id;
        site.statement = base_resolution->statement;
        site.scope = base_resolution->scope;
        site.callee_symbol = base_resolution->symbol;
        site.callee = text(field(*expression, "text"), base_resolution->name);
        site.pure = pure_symbols.count(site.callee_symbol) && pure_symbols[site.callee_symbol];
        for (const auto& argument : list(field(payload, "arguments"))) { const int argument_id = integer(&argument); site.arguments.push_back(argument_id); collect_reads(argument_id, site.reads); }
        if (statements.count(site.statement)) {
            const auto* statement = statements[site.statement];
            const auto* statement_payload = field(*statement, "payload");
            site.writes = text(field(*statement, "name"));
            if (site.writes.empty()) site.writes = text(field(field(statement_payload, "target"), "name"));
            if (!site.writes.empty()) site.write_symbol = visible_symbol(site.scope, site.writes);
        }
        call_sites.push_back(std::move(site));
    }
    for (std::size_t left = 0; left < call_sites.size(); ++left) for (std::size_t right = left + 1; right < call_sites.size(); ++right) {
        auto& first = call_sites[left]; auto& second = call_sites[right];
        if (!first.pure || !second.pure || first.scope != second.scope || first.statement == second.statement) continue;
        bool shared_read = false;
        for (const auto symbol : first.reads) if (second.reads.count(symbol)) shared_read = true;
        const bool output_conflict = first.write_symbol >= 0 && first.write_symbol == second.write_symbol;
        const bool read_after_write = (first.write_symbol >= 0 && second.reads.count(first.write_symbol)) || (second.write_symbol >= 0 && first.reads.count(second.write_symbol));
        if (!shared_read && !output_conflict && !read_after_write) {
            first.independent_with.push_back(second.expression);
            second.independent_with.push_back(first.expression);
        }
    }
    std::set<int> called_provider_symbols;
    std::set<int> text_output_symbols;
    std::vector<LoweringOperation> lowering_operations;
    auto containing_function = [&](int scope_id) {
        int current = scope_id;
        while (current >= 0 && scopes.count(current)) {
            const int owner = integer(field(*scopes.at(current), "owner_symbol_id"));
            if (symbols.count(owner)) {
                const auto kind = text(field(*symbols.at(owner), "kind"));
                if (kind == "Function" || kind == "Procedure") return owner;
            }
            current = integer(field(*scopes.at(current), "parent_id"));
        }
        return -1;
    };
    auto containing_block = [&](int statement_id) {
        for (const auto& [block_id, block] : blocks) for (const auto& member : list(field(*block, "statements"))) if (integer(&member) == statement_id) return block_id;
        return -1;
    };
    for (const auto& [expression_id, provider] : runtime_text_concats) {
        int statement_id = -1;
        int scope_id = -1;
        for (const auto& [candidate_id, statement] : statements) {
            const auto kind = text(field(*statement, "kind"));
            const auto* payload = field(*statement, "payload");
            const int candidate_expression = kind == "let" ? integer(field(payload, "initializer_expression")) :
                (kind == "return" ? integer(field(payload, "value_expression")) : -1);
            if (candidate_expression == expression_id) { statement_id = candidate_id; scope_id = statement_scopes.count(candidate_id) ? statement_scopes.at(candidate_id) : -1; break; }
        }
        if (statement_id < 0) continue;
        LoweringOperation operation;
        operation.expression = expression_id;
        operation.statement = statement_id;
        operation.scope = scope_id;
        operation.block = containing_block(statement_id);
        operation.function_symbol = containing_function(scope_id);
        operation.callee_symbol = provider.first;
        operation.callee = "text_concat";
        operation.kind = "text_outcome";
        const auto* payload = field(*expressions.at(expression_id), "payload");
        operation.arguments = {integer(field(payload, "left")), integer(field(payload, "right"))};
        operation.contract = provider.second.contract;
        operation.evidence = provider.second.evidence;
        operation.library = provider.second.library;
        operation.convention = provider.second.convention;
        operation.symbol = provider.second.symbol;
        operation.effect = provider.second.effect;
        operation.parameter_types = provider.second.parameter_types;
        operation.return_type = provider.second.return_type;
        lowering_operations.push_back(std::move(operation));
    }
    for (const auto& [statement_id, statement] : statements) {
        if (text(field(*statement, "kind")) != "expression") continue;
        const auto* payload = field(*statement, "payload");
        const auto* print = field(payload, "print");
        if (!print || !std::holds_alternative<bool>(*print) || !std::get<bool>(*print)) continue;
        const int expression_id = integer(field(payload, "expression"));
        if (!expression_is_text(expression_id)) {
            const auto* expression = expressions.count(expression_id) ? expressions.at(expression_id) : nullptr;
            const int symbol = expression && text(field(*expression, "kind")) == "identifier" &&
                resolved_expression_symbols.count(expression_id) ? resolved_expression_symbols.at(expression_id) : -1;
            if (symbol >= 0 && symbol_types[symbol] == "c_string")
                add_diagnostic("FLOWANALYST_TEXT_CSTRING_CONFUSION", "print does not implicitly reinterpret c_string as Text", symbol, "symbol:" + std::to_string(symbol));
            continue;
        }
        std::vector<int> candidates;
        for (const auto& [symbol_id, requirement] : provider_functions) {
            if (requirement.parameter_types == "Text" && requirement.return_type == "c_int" &&
                requirement.effect == "io" && text(field(*symbols.at(symbol_id), "name")) == "puts_text")
                candidates.push_back(symbol_id);
        }
        if (candidates.size() != 1) {
            add_diagnostic("FLOWANALYST_TEXT_OUTPUT_CAPABILITY", "print requires exactly one declared Text output capability (puts_text)", -1, "statement:" + std::to_string(statement_id));
            continue;
        }
        const int provider_symbol = candidates.front();
        text_output_symbols.insert(provider_symbol);
        LoweringOperation operation;
        operation.expression = expression_id;
        operation.statement = statement_id;
        operation.scope = statement_scopes.count(statement_id) ? statement_scopes.at(statement_id) : -1;
        operation.block = containing_block(statement_id);
        operation.function_symbol = containing_function(operation.scope);
        operation.callee_symbol = provider_symbol;
        operation.callee = "print";
        operation.kind = "external_call";
        operation.arguments.push_back(expression_id);
        const auto& requirement = provider_functions.at(provider_symbol);
        operation.contract = requirement.contract;
        operation.evidence = requirement.evidence;
        operation.library = requirement.library;
        operation.convention = requirement.convention;
        operation.symbol = requirement.symbol;
        operation.effect = requirement.effect;
        operation.parameter_types = requirement.parameter_types;
        operation.return_type = requirement.return_type;
        lowering_operations.push_back(std::move(operation));
    }
    for (const auto& site : call_sites) if (provider_functions.count(site.callee_symbol))
        called_provider_symbols.insert(site.callee_symbol);
    called_provider_symbols.insert(text_output_symbols.begin(), text_output_symbols.end());
    for (const auto& [expression_id, provider] : runtime_text_concats) { (void)expression_id; called_provider_symbols.insert(provider.first); }
    if (graph_native) for (const auto& provider : graph_providers) {
        called_provider_symbols.insert(integer(field(provider, "function_symbol_id")));
        if (const auto* count = field(provider, "count_function_symbol_id")) called_provider_symbols.insert(integer(count));
    }
    for (const auto symbol : called_provider_symbols) binding_requirements.push_back(provider_functions.at(symbol));
    for (const auto& site : call_sites) {
        LoweringOperation operation;
        operation.expression = site.expression;
        operation.statement = site.statement;
        operation.scope = site.scope;
        operation.block = containing_block(site.statement);
        operation.function_symbol = containing_function(site.scope);
        operation.callee_symbol = site.callee_symbol;
        operation.result_symbol = site.write_symbol;
        operation.callee = site.callee;
        operation.kind = "call";
        operation.arguments = site.arguments;
        if (const auto provider = provider_functions.find(site.callee_symbol); provider != provider_functions.end()) {
            const auto& requirement = provider->second;
            operation.kind = requirement.contract == "text_runtime" &&
                requirement.symbol == "flow_text_concat_value" &&
                requirement.parameter_types == "Text,Text" &&
                requirement.return_type == "TextOutcome"
                ? "text_outcome" : "external_call";
            operation.contract = requirement.contract;
            operation.evidence = requirement.evidence;
            operation.library = requirement.library;
            operation.convention = requirement.convention;
            operation.symbol = requirement.symbol;
            operation.effect = requirement.effect;
            operation.parameter_types = requirement.parameter_types;
            operation.return_type = requirement.return_type;
        }
        lowering_operations.push_back(std::move(operation));
    }
    for (const auto& [statement_id, statement] : statements) {
        if (text(field(*statement, "kind")) != "let") continue;
        const auto* payload = field(*statement, "payload");
        const int initializer = integer(field(payload, "initializer_expression"));
        if (initializer < 0) continue;
        const auto name = text(field(*statement, "name"));
        const int scope_id = statement_scopes.count(statement_id) ? statement_scopes.at(statement_id) : -1;
        int result_symbol = -1;
        if (scopes.count(scope_id)) for (const auto& candidate : list(field(*scopes.at(scope_id), "symbol_ids"))) {
            const int candidate_id = integer(&candidate);
            if (symbols.count(candidate_id) && text(field(*symbols.at(candidate_id), "name")) == name) { result_symbol = candidate_id; break; }
        }
        LoweringOperation operation;
        operation.expression = initializer;
        operation.statement = statement_id;
        operation.scope = scope_id;
        operation.block = containing_block(statement_id);
        operation.function_symbol = containing_function(scope_id);
        operation.result_symbol = result_symbol;
        operation.kind = "value_definition";
        operation.arguments.push_back(initializer);
        lowering_operations.push_back(std::move(operation));
    }
    for (const auto& [statement_id, statement] : statements) {
        if (text(field(*statement, "kind")) != "return") continue;
        const auto* payload = field(*statement, "payload");
        const int value_expression = integer(field(payload, "value_expression"));
        if (value_expression < 0) continue;
        LoweringOperation operation;
        operation.expression = value_expression;
        operation.statement = statement_id;
        operation.scope = statement_scopes.count(statement_id) ? statement_scopes.at(statement_id) : -1;
        operation.block = containing_block(statement_id);
        operation.function_symbol = containing_function(operation.scope);
        operation.kind = "return_value";
        operation.arguments.push_back(value_expression);
        lowering_operations.push_back(std::move(operation));
    }
    for (const auto& [statement_id, statement] : statements) {
        if (text(field(*statement, "kind")) != "if") continue;
        const auto* payload = field(*statement, "payload");
        LoweringOperation operation;
        operation.expression = integer(field(payload, "condition_expression"));
        operation.statement = statement_id;
        operation.scope = statement_scopes.count(statement_id) ? statement_scopes.at(statement_id) : -1;
        operation.block = containing_block(statement_id);
        operation.function_symbol = containing_function(operation.scope);
        operation.then_block = integer(field(payload, "then_block"));
        operation.else_block = integer(field(field(payload, "else_arm"), "block"));
        operation.kind = "branch";
        if (operation.expression >= 0) operation.arguments.push_back(operation.expression);
        lowering_operations.push_back(std::move(operation));
    }
    for (const auto& [statement_id, statement] : statements) {
        if (text(field(*statement, "kind")) != "placement") continue;
        const auto* payload = field(*statement, "payload");
        const int value_expression = integer(field(payload, "value_expression"));
        if (value_expression < 0 || (expressions.count(value_expression) && text(field(*expressions.at(value_expression), "kind")) == "call")) continue;
        const int scope_id = statement_scopes.count(statement_id) ? statement_scopes.at(statement_id) : -1;
        LoweringOperation operation;
        operation.expression = value_expression;
        operation.statement = statement_id;
        operation.scope = scope_id;
        operation.block = containing_block(statement_id);
        operation.function_symbol = containing_function(scope_id);
        operation.result_symbol = visible_symbol(scope_id, text(field(field(payload, "target"), "name")));
        operation.kind = "assignment";
        operation.arguments.push_back(value_expression);
        lowering_operations.push_back(std::move(operation));
    }
    for (const auto& [statement_id, statement] : statements) {
        if (text(field(*statement, "kind")) != "while") continue;
        const auto* payload = field(*statement, "payload");
        LoweringOperation operation;
        operation.expression = integer(field(payload, "condition_expression"));
        operation.statement = statement_id;
        operation.scope = statement_scopes.count(statement_id) ? statement_scopes.at(statement_id) : -1;
        operation.block = containing_block(statement_id);
        operation.function_symbol = containing_function(operation.scope);
        operation.body_block = integer(field(payload, "body_block"));
        operation.kind = "loop";
        if (operation.expression >= 0) operation.arguments.push_back(operation.expression);
        lowering_operations.push_back(std::move(operation));
    }
    std::vector<Region> regions;
    for (const auto& [id, scope] : scopes) regions.push_back({"scope:" + std::to_string(id), "scope", "sane", {}});
    for (const auto& [id, symbol] : symbols) {
        Region region{"symbol:" + std::to_string(id), "symbol", "sane", {"scope:" + std::to_string(integer(field(*symbol, "owning_scope_id")))} };
        int introduced = integer(field(*symbol, "introduced_scope_id")); if (introduced >= 0) region.prerequisites.push_back("scope:" + std::to_string(introduced));
        regions.push_back(std::move(region));
    }
    for (const auto& target : targets) {
        Region region{"target:" + target.name, "target", target.mains == 1 ? "sane" : "rejected", {}};
        const auto* symbol = symbols[target.symbol]; int scope_id = integer(field(*symbol, "introduced_scope_id"));
        if (scopes.count(scope_id)) for (const auto& child : list(field(*scopes[scope_id], "symbol_ids"))) region.prerequisites.push_back("symbol:" + std::to_string(integer(&child)));
        regions.push_back(std::move(region));
    }
    for (const auto& resolution : resolutions) if (resolution.symbol >= 0) {
        const auto region_id = "symbol:" + std::to_string(resolution.symbol);
        for (auto& region : regions) if (region.id == "scope:" + std::to_string(resolution.scope) && region_id != region.id) region.prerequisites.push_back(region_id);
    }
    for (auto& region : regions) { std::sort(region.prerequisites.begin(), region.prerequisites.end()); region.prerequisites.erase(std::unique(region.prerequisites.begin(), region.prerequisites.end()), region.prerequisites.end()); }
    for (const auto& diagnostic : diagnostics) for (auto& region : regions) if (region.id == diagnostic.region) region.status = "rejected";
    std::map<std::string, int> region_index;
    for (std::size_t index = 0; index < regions.size(); ++index) region_index[regions[index].id] = static_cast<int>(index);
    std::cout << "{\n  \"format\": \"flowanalyst.semantic_report\",\n  \"version\": 1,\n  \"status\": \"" << (diagnostics.empty() ? "ok" : "error") << "\",\n  \"source\": {\"path\": " << quote(text(field(field(bundle, "source"), "path"))) << "},\n  \"frontend_bundle\": {\"format\": \"flowmini.frontend_bundle\", \"version\": 2},\n  \"diagnostics\": [";
    for (std::size_t i = 0; i < diagnostics.size(); ++i) { const auto& d = diagnostics[i]; if (i) std::cout << ','; std::cout << "{\"code\":" << quote(d.code) << ",\"severity\":" << quote(d.severity) << ",\"message\":" << quote(d.message) << ",\"root_cause\":true"; if (d.symbol >= 0) { std::cout << ",\"subject\":{\"kind\":\"symbol\",\"id\":" << d.symbol << "}"; } if (d.symbol >= 0 || d.line >= 0) { std::cout << ",\"provenance\":{\"source\":" << quote(d.source) << ",\"ast_path\":" << quote(d.ast_path) << ",\"line\":" << d.line << ",\"column\":" << d.column << "}"; } if (!d.region.empty()) std::cout << ",\"region\":" << quote(d.region); std::cout << '}'; }
    std::cout << "],\n  \"binding_requirements\": [";
    for (std::size_t i = 0; i < binding_requirements.size(); ++i) { if (i) std::cout << ','; const auto& requirement = binding_requirements[i]; std::cout << "{\"contract\":" << quote(requirement.contract) << ",\"library\":" << quote(requirement.library) << ",\"convention\":" << quote(requirement.convention) << ",\"symbol\":" << quote(requirement.symbol) << ",\"effect\":" << quote(requirement.effect) << ",\"parameter_types\":" << quote(requirement.parameter_types) << ",\"return_type\":" << quote(requirement.return_type) << ",\"evidence\":" << quote(requirement.evidence) << "}"; }
    std::cout << "],\n  \"aggregate_abi_layouts\": [";
    for (std::size_t i = 0; i < aggregate_layouts.size(); ++i) {
        if (i) std::cout << ',';
        const auto& layout = aggregate_layouts[i];
        std::cout << "{\"contract\":" << quote(layout.contract)
                  << ",\"name\":" << quote(layout.name)
                  << ",\"version\":1,\"status\":\"declared\",\"layout_policy\":\"provider_verified_required\",\"fields\":[";
        for (std::size_t field_index = 0; field_index < layout.fields.size(); ++field_index) {
            if (field_index) std::cout << ',';
            std::cout << "{\"name\":" << quote(layout.fields[field_index].first)
                      << ",\"type\":" << quote(layout.fields[field_index].second) << "}";
        }
        std::cout << "]}";
    }
    std::cout << "],\n  \"abi_type_contracts\": [";
    for (std::size_t i = 0; i < abi_type_contracts.size(); ++i) {
        if (i) std::cout << ',';
        const auto& type = abi_type_contracts[i];
        std::cout << "{\"contract\":" << quote(type.contract)
                  << ",\"name\":" << quote(type.name)
                  << ",\"repr\":" << quote(type.repr)
                  << ",\"ownership\":" << quote(type.ownership)
                  << ",\"access\":" << quote(type.access)
                  << ",\"lifetime\":" << quote(type.lifetime)
                  << ",\"nullable\":" << quote(type.nullable)
                  << ",\"opaque\":" << quote(type.opaque)
                  << ",\"cleanup\":" << quote(type.cleanup) << "}";
    }
    std::cout << "],\n  \"graph_analysis\":{\"format\":\"flowanalyst.graph_analysis\",\"version\":1,\"status\":\"" << (graph_native ? "ready" : "non_executable") << "\",\"receivers\":"
              << flowcontracts::json::serialize(graph_receivers) << "},\n  \"lowering_plan\": {\"format\":\"flowcore.lowering_plan\",\"version\":" << lowering_plan_version << ",\"status\":\""
              << (diagnostics.empty() ? "ready" : "blocked") << "\"";
    if (!std::holds_alternative<std::nullptr_t>(graph_model))
        std::cout << ",\"source_graph\":" << flowcontracts::json::serialize(graph_model);
    if (lowering_plan_version == 2) {
        std::cout << ",\"functions\":[";
        for (std::size_t index = 0; index < callables.size(); ++index) {
            if (index) std::cout << ',';
            const auto& callable = callables[index];
            std::cout << "{\"symbol_id\":" << callable.symbol << ",\"name\":" << quote(callable.name)
                      << ",\"scope_id\":" << callable.scope << ",\"body_block_id\":" << callable.body_block
                      << ",\"return_type\":" << quote(callable.return_type) << ",\"entry\":" << (callable.entry ? "true" : "false")
                      << ",\"availability\":" << quote(callable.availability)
                      << ",\"parameters\":[";
            for (std::size_t parameter = 0; parameter < callable.parameters.size(); ++parameter) {
                if (parameter) std::cout << ',';
                std::cout << "{\"symbol_id\":" << callable.parameters[parameter].first
                          << ",\"type\":" << quote(callable.parameters[parameter].second) << "}";
            }
            std::cout << "]";
            if (provider_functions.count(callable.symbol)) {
                const auto& p = provider_functions.at(callable.symbol);
                std::cout << ",\"provider\":" << flowcontracts::json::serialize(Object{
                    {"contract", p.contract}, {"library", p.library}, {"symbol", p.symbol},
                    {"convention", p.convention}, {"effect", p.effect},
                    {"parameter_types", p.parameter_types}, {"return_type", p.return_type}, {"evidence", p.evidence}});
            }
            std::cout << "}";
        }
        std::cout << "]";
    }
    std::cout << ",\"operations\":[";
    std::function<void(int, const std::string&)> emit_operand = [&](int expression_id, const std::string& declared_type) {
        const auto* expression = expressions.count(expression_id) ? expressions.at(expression_id) : nullptr;
        const auto kind = text(field(expression, "kind"));
        const auto literal = text(field(field(expression, "payload"), "value_text"), "0");
        const int identifier_symbol = kind == "identifier" && resolved_expression_symbols.count(expression_id)
            ? resolved_expression_symbols.at(expression_id) : -1;
        const auto identifier_type = symbol_types.count(identifier_symbol) ? symbol_types.at(identifier_symbol) : std::string{};
        const bool carrier_conversion = kind == "identifier" && !declared_type.empty() &&
            !identifier_type.empty() && identifier_type != declared_type;
        const bool writable_storage = kind == "integer_literal" && declared_type == "c_pointer" &&
            !literal.empty() && literal != "0" && literal.front() != '-';
        bool ordinary_call = false;
        if (lowering_plan_version == 2 && kind == "call") {
            const int base = integer(field(field(expression, "payload"), "base"));
            const auto callee = expressions.count(base) && text(field(*expressions.at(base), "kind")) == "identifier"
                ? text(field(field(*expressions.at(base), "payload"), "name")) : std::string{};
            ordinary_call = callee != "length";
        }
        const bool folded_text = kind == "binary" && text(field(field(expression, "payload"), "operator")) == "+" &&
            expression_is_text(expression_id) && constant_text(expression_id).has_value();
        const bool runtime_text = runtime_text_concats.count(expression_id) != 0;
        std::cout << "{\"expression_id\":" << expression_id << ",\"kind\":"
                  << quote(carrier_conversion ? "conversion" : (writable_storage ? "writable_storage" : (ordinary_call || runtime_text ? "call_result" : (folded_text ? "string_literal" : kind))));
        if (carrier_conversion) {
            std::cout << ",\"type\":" << quote(declared_type) << ",\"from_type\":" << quote(identifier_type)
                      << ",\"conversion\":\"explicit_typed_initializer\",\"operand\":";
            emit_operand(expression_id, {});
        } else if (writable_storage) {
            std::cout << ",\"type\":\"c_pointer\",\"storage\":{\"bytes\":" << literal
                      << ",\"access\":\"read_write\",\"lifetime\":\"call\"}";
        } else if (runtime_text) {
            std::cout << ",\"type\":\"Text\",\"callee_symbol_id\":" << runtime_text_concats.at(expression_id).first;
        } else
        if (kind == "integer_literal") {
            std::cout << ",\"type\":" << quote(declared_type.empty() ? "c_int" : declared_type) << ",\"value\":" << quote(literal);
        } else if (kind == "float_literal") {
            const auto value = text(field(field(expression, "payload"), "value_text"));
            std::cout << ",\"type\":" << quote(declared_type.empty() ? "float64" : declared_type)
                      << ",\"value\":" << quote(value);
        } else if (kind == "string_literal") {
            const auto value = text(field(field(expression, "payload"), "value_text"));
            std::cout << ",\"type\":" << quote(declared_type == "Text" ? "Text" : "c_string")
                      << ",\"value\":" << quote(value);
        } else if (kind == "field_access") {
            const auto* payload = field(expression, "payload");
            const auto field_name = text(field(payload, "field"));
            const int base = integer(field(payload, "base"));
            const int base_symbol = resolved_expression_symbols.count(base) ? resolved_expression_symbols.at(base) : -1;
            const auto base_type = symbol_types.count(base_symbol) ? symbol_types.at(base_symbol) : std::string{};
            if (base_type != "TextOutcome" || (field_name != "code" && field_name != "value")) {
                std::cout << ",\"type\":\"unsupported\"";
            } else {
                std::cout << ",\"type\":" << quote(field_name == "code" ? "c_int" : "Text")
                          << ",\"field\":" << quote(field_name) << ",\"base\":";
                emit_operand(base, "TextOutcome");
            }
        } else if (kind == "bool_literal") {
            std::cout << ",\"type\":\"bool\",\"value\":" << quote(text(field(field(expression, "payload"), "value_text"), "false"));
        } else if (kind == "identifier") {
            std::cout << ",\"type\":" << quote(identifier_type) << ",\"symbol_id\":" << identifier_symbol;
        } else if (kind == "index") {
            const auto* payload = field(expression, "payload");
            const int base = integer(field(payload, "base"));
            const int symbol = resolved_expression_symbols.count(base) ? resolved_expression_symbols.at(base) : -1;
            const auto type = symbol_types.count(symbol) ? symbol_types.at(symbol) : std::string{};
            const auto indexes = list(field(payload, "indexes"));
            if (type == "list<string>" && indexes.size() == 1) {
                std::cout << ",\"intrinsic\":\"list_index\",\"type\":\"c_string\",\"symbol_id\":" << symbol << ",\"index\":";
                emit_operand(integer(&indexes.front()), "c_int");
            } else std::cout << ",\"type\":\"unsupported\"";
        } else if (kind == "call") {
            const auto* payload = field(expression, "payload");
            const int base = integer(field(payload, "base"));
            const auto callee = expressions.count(base) && text(field(*expressions.at(base), "kind")) == "identifier"
                ? text(field(field(*expressions.at(base), "payload"), "name")) : std::string{};
            const auto arguments = list(field(payload, "arguments"));
            if (callee == "length" && arguments.size() == 1) {
                const int argument = integer(&arguments.front());
                const int symbol = resolved_expression_symbols.count(argument) ? resolved_expression_symbols.at(argument) : -1;
                const auto type = symbol_types.count(symbol) ? symbol_types.at(symbol) : std::string{};
                if (type == "list<string>")
                    std::cout << ",\"intrinsic\":\"list_length\",\"type\":\"c_int\",\"symbol_id\":" << symbol;
                else std::cout << ",\"type\":\"unsupported\"";
            } else {
                const int callee_symbol = resolved_expression_symbols.count(base) ? resolved_expression_symbols.at(base) : -1;
                if (lowering_plan_version == 2 && symbols.count(callee_symbol) && text(field(*symbols.at(callee_symbol), "kind")) == "Function") {
                    std::cout << ",\"type\":" << quote(fact_value(*symbols.at(callee_symbol), "return_type_spelling"))
                              << ",\"callee_symbol_id\":" << callee_symbol << ",\"arguments\":[";
                    for (std::size_t index = 0; index < arguments.size(); ++index) {
                        if (index) std::cout << ',';
                        emit_operand(integer(&arguments[index]), {});
                    }
                    std::cout << "]";
                } else std::cout << ",\"type\":\"unsupported\"";
            }
        } else if (kind == "unary") {
            const auto* payload = field(expression, "payload");
            std::cout << ",\"type\":\"c_int\",\"operator\":" << quote(text(field(payload, "operator"))) << ",\"operand\":";
            emit_operand(integer(field(payload, "operand")), {});
        } else if (kind == "binary") {
            const auto* payload = field(expression, "payload");
            const auto operator_name = text(field(payload, "operator"));
            const bool text_binary = operator_name == "+" &&
                expression_is_text(integer(field(payload, "left"))) &&
                expression_is_text(integer(field(payload, "right")));
            if (text_binary) {
                const auto value = constant_text(expression_id);
                if (value) {
                    std::cout << ",\"type\":\"Text\",\"value\":" << quote(*value);
                } else {
                    std::cout << ",\"type\":\"Text\",\"operator\":\"+\",\"left\":";
                    emit_operand(integer(field(payload, "left")), "Text");
                    std::cout << ",\"right\":";
                    emit_operand(integer(field(payload, "right")), "Text");
                }
                std::cout << "}";
                return;
            }
            const bool comparison = operator_name == "==" || operator_name == "!=" || operator_name == "<" || operator_name == "<=" || operator_name == ">" || operator_name == ">=";
            std::cout << ",\"type\":" << (comparison ? "\"bool\"" : "\"c_int\"") << ",\"operator\":" << quote(operator_name) << ",\"left\":";
            emit_operand(integer(field(payload, "left")), {});
            std::cout << ",\"right\":";
            emit_operand(integer(field(payload, "right")), {});
        } else {
            std::cout << ",\"type\":\"unsupported\"";
        }
        std::cout << "}";
    };
    for (std::size_t i = 0; i < lowering_operations.size(); ++i) {
        if (i) std::cout << ',';
        const auto& operation = lowering_operations[i];
        std::cout << "{\"id\":" << i
                  << ",\"kind\":" << quote(operation.kind)
                  << ",\"expression_id\":" << operation.expression
                  << ",\"statement_id\":" << operation.statement
                  << ",\"scope_id\":" << operation.scope;
        if (lowering_plan_version == 2) std::cout << ",\"function_symbol_id\":" << operation.function_symbol;
        std::cout
                  << (operation.block >= 0 ? ",\"block_id\":" + std::to_string(operation.block) : std::string{})
                  << ",\"callee\":" << quote(operation.callee)
                  << ",\"callee_symbol_id\":" << operation.callee_symbol
                  << ",\"arguments\":[";
        for (std::size_t argument = 0; argument < operation.arguments.size(); ++argument) {
            if (argument) std::cout << ',';
            std::cout << operation.arguments[argument];
        }
        std::cout << "],\"operands\":[";
        for (std::size_t argument = 0; argument < operation.arguments.size(); ++argument) {
            if (argument) std::cout << ',';
            auto declared_type = (operation.kind == "value_definition" || operation.kind == "assignment") && operation.result_symbol >= 0 && symbol_types.count(operation.result_symbol)
                ? symbol_types.at(operation.result_symbol) : std::string{};
            if (operation.kind == "return_value" && lowering_plan_version == 2)
                for (const auto& callable : callables) if (callable.symbol == operation.function_symbol) {
                    declared_type = callable.return_type;
                    break;
                }
            if (operation.kind == "external_call" || operation.kind == "text_outcome") {
                const auto parameter_types = split_generic_arguments(operation.parameter_types);
                if (argument < parameter_types.size()) declared_type = parameter_types[argument];
            }
            emit_operand(operation.arguments[argument], declared_type);
        }
        std::cout << "]";
        if (operation.result_symbol >= 0) std::cout << ",\"result_symbol_id\":" << operation.result_symbol;
        if (operation.kind == "branch") std::cout << ",\"then_block_id\":" << operation.then_block << ",\"else_block_id\":" << operation.else_block;
        if (operation.kind == "loop") std::cout << ",\"body_block_id\":" << operation.body_block;
        if (operation.kind == "external_call" || operation.kind == "text_outcome") {
            std::cout << ",\"provider\":{\"contract\":" << quote(operation.contract) << ",\"evidence\":" << quote(operation.evidence)
                      << ",\"library\":" << quote(operation.library)
                      << ",\"convention\":" << quote(operation.convention)
                      << ",\"symbol\":" << quote(operation.symbol)
                      << ",\"effect\":" << quote(operation.effect)
                      << ",\"parameter_types\":" << quote(operation.parameter_types)
                      << ",\"return_type\":" << quote(operation.return_type) << "}";
            std::cout << ",\"effect_contract\":{\"external\":" << quote(operation.effect)
                      << ",\"determinism\":" << quote(operation.effect == "pure" ? "deterministic" : "unspecified")
                      << ",\"certainty\":\"declared\"}";
            if (operation.kind == "text_outcome")
                std::cout << ",\"result_outcome\":{\"type\":\"Outcome\",\"representation\":\"tagged\",\"success_type\":\"Text\",\"failure_type\":\"TextFailure\",\"failure_codes\":[\"invalid_input\",\"exhausted\",\"provider_unavailable\"]}";
            std::cout << ",\"argument_resources\":[";
            const auto parameter_carriers = operation.parameter_types.empty()
                ? std::vector<std::string>{} : split_generic_arguments(operation.parameter_types);
            for (std::size_t parameter = 0; parameter < parameter_carriers.size(); ++parameter) {
                if (parameter) std::cout << ',';
                const AbiTypeContract* contract = nullptr;
                for (const auto& type : abi_type_contracts) if (type.contract == operation.contract && type.name == parameter_carriers[parameter]) { contract = &type; break; }
                const auto memory_effect = contract == nullptr ? "none" :
                    (contract->access == "read" ? "read" : (contract->access == "read_write" || contract->access == "write" ? "read_write" : "opaque"));
                std::cout << "{\"index\":" << parameter << ",\"type\":" << quote(parameter_carriers[parameter])
                          << ",\"memory_effect\":" << quote(memory_effect)
                          << ",\"ownership\":" << quote(contract ? contract->ownership : "none")
                          << ",\"access\":" << quote(contract ? contract->access : "value")
                          << ",\"lifetime\":" << quote(contract ? contract->lifetime : "value")
                          << ",\"nullable\":" << quote(contract ? contract->nullable : "not_applicable")
                          << ",\"opaque\":" << quote(contract ? contract->opaque : "false") << "}";
            }
            std::cout << "]";
            for (const auto& type : abi_type_contracts) {
                if (type.contract != operation.contract || type.name != operation.return_type || type.cleanup.empty()) continue;
                std::cout << ",\"result_resource\":{\"type\":" << quote(type.name)
                          << ",\"ownership\":" << quote(type.ownership)
                          << ",\"access\":" << quote(type.access)
                          << ",\"lifetime\":" << quote(type.lifetime)
                          << ",\"nullable\":" << quote(type.nullable)
                          << ",\"opaque\":" << quote(type.opaque)
                          << ",\"cleanup_capability\":" << quote(type.cleanup) << "}";
            }
        }
        std::cout << "}";
    }
    std::cout << "]},\n  \"effect_facts\": [";
    for (std::size_t i = 0; i < effect_facts.size(); ++i) { if (i) std::cout << ','; const auto& fact = effect_facts[i]; std::cout << "{\"declaration_id\":" << fact.declaration << ",\"symbol_id\":" << fact.symbol << ",\"name\":" << quote(fact.name) << ",\"effect\":" << quote(fact.effect) << ",\"certainty\":" << quote(fact.certainty) << ",\"reason\":" << quote(fact.reason) << "}"; }
    std::cout << "],\n  \"external_operations\": [";
    for (std::size_t i = 0; i < call_sites.size(); ++i) {
        if (i) std::cout << ',';
        const auto& site = call_sites[i];
        std::cout << "{\"operation\":\"call\",\"expression_id\":" << site.expression
                  << ",\"statement_id\":" << site.statement
                  << ",\"scope_id\":" << site.scope
                  << ",\"callee\":" << quote(site.callee)
                  << ",\"callee_symbol_id\":" << site.callee_symbol
                  << ",\"arguments\":[";
        for (std::size_t argument = 0; argument < site.arguments.size(); ++argument) { if (argument) std::cout << ','; std::cout << site.arguments[argument]; }
        std::cout << "]";
        if (site.write_symbol >= 0) std::cout << ",\"result_symbol_id\":" << site.write_symbol;
        std::cout << ",\"purity\":" << (site.pure ? "\"pure\"" : "\"effectful\"") << "}";
    }
    std::cout << "],\n  \"parallel_candidates\": [";
    bool first_candidate = true;
    for (const auto& site : call_sites) if (!site.independent_with.empty()) {
        if (!first_candidate) std::cout << ',';
        first_candidate = false;
        std::cout << "{\"call_expression\":" << site.expression << ",\"statement_id\":" << site.statement << ",\"callee\":" << quote(site.callee) << ",\"proof\":\"pure-callee-disjoint-inputs\",\"status\":\"deferred\",\"independent_with\":[";
        for (std::size_t i = 0; i < site.independent_with.size(); ++i) { if (i) std::cout << ','; std::cout << site.independent_with[i]; }
        std::cout << "]}";
    }
    std::cout << "],\n  \"facts\": [{\"kind\":\"semantic_summary\",\"scopes\":" << scopes.size() << ",\"symbols\":" << symbols.size() << ",\"resolved_types\":" << resolved_types << ",\"unresolved_types\":" << unresolved_types << ",\"refined_types\":" << refined_types << ",\"resolved_names\":" << resolutions.size() << ",\"targets\":" << targets.size() << ",\"regions\":" << regions.size() << "}],\n  \"resolved_names\": [";
    bool first_resolution = true; for (const auto& resolution : resolutions) if (resolution.symbol >= 0) { if (!first_resolution) std::cout << ','; first_resolution = false; std::cout << "{\"expression_id\":" << resolution.expression << ",\"statement_id\":" << resolution.statement << ",\"name\":" << quote(resolution.name) << ",\"symbol_id\":" << resolution.symbol << ",\"scope_id\":" << resolution.scope << "}"; }
    std::cout << "],\n  \"analysis_regions\": [";
    for (std::size_t i = 0; i < regions.size(); ++i) { if (i) std::cout << ','; const auto& region = regions[i]; std::cout << "{\"id\":" << quote(region.id) << ",\"kind\":" << quote(region.kind) << ",\"status\":" << quote(region.status) << ",\"requires\":["; for (std::size_t j = 0; j < region.prerequisites.size(); ++j) { if (j) std::cout << ','; std::cout << quote(region.prerequisites[j]); } std::cout << "]}"; }
    std::cout << "],\n  \"analysis_graph\": {\"format\":\"flowanalyst.analysis_graph\",\"version\":1,\"nodes\":[";
    for (std::size_t i = 0; i < regions.size(); ++i) { if (i) std::cout << ','; const auto& region = regions[i]; std::cout << "{\"index\":" << i << ",\"id\":" << quote(region.id) << ",\"kind\":" << quote(region.kind) << ",\"status\":" << quote(region.status) << '}'; }
    std::cout << "],\"edges\":[";
    bool first_edge = true;
    for (const auto& region : regions) for (const auto& prerequisite : region.prerequisites) if (region_index.count(prerequisite)) {
        if (!first_edge) std::cout << ',';
        first_edge = false;
        std::cout << "{\"from\":" << region_index[prerequisite] << ",\"to\":" << region_index[region.id] << ",\"kind\":\"requires\"}";
    }
    std::cout << "],\"matrix_views\":[{\"name\":\"region_dependency\",\"orientation\":\"prerequisite_to_dependent\",\"semiring\":\"boolean\",\"storage\":\"coo\",\"rows\":" << regions.size() << ",\"columns\":" << regions.size() << ",\"entries\":[";
    first_edge = true;
    for (const auto& region : regions) for (const auto& prerequisite : region.prerequisites) if (region_index.count(prerequisite)) {
        if (!first_edge) std::cout << ',';
        first_edge = false;
        std::cout << "{\"row\":" << region_index[prerequisite] << ",\"column\":" << region_index[region.id] << ",\"value\":true}";
    }
    std::cout << "]}]},\n  \"targets\": [";
    for (std::size_t i = 0; i < targets.size(); ++i) { if (i) std::cout << ','; std::cout << "{\"symbol_id\":" << targets[i].symbol << ",\"name\":" << quote(targets[i].name) << ",\"main_count\":" << targets[i].mains << ",\"status\":\"" << (targets[i].mains == 1 ? "sane" : "rejected") << "\"}"; }
    std::cout << "]\n}\n"; return diagnostics.empty() ? 0 : 2;
}
}

int main(int argc, char** argv) {
    bool structured_diagnostics = false;
    try {
        if (argc == 2) {
            const std::string option = argv[1];
            if (option == "-h" || option == "--help" || option == "-?") {
                std::cout << "flowanalyst - semantic checks for FlowMini frontend bundles\n\n"
                             "Usage: flowanalyst [bundle.json]\n"
                             "       flowmini --dump-frontend-bundle source.flow | flowanalyst\n\n"
                             "Options: -h, -?, --help  show help\n"
                             "         -a, --about    show about information\n"
                             "         -v, --version  print the raw version number\n"
                             "         --diagnostics json  emit machine-readable failures on stderr\n\n"
                             "More help: Flowanalyst/README.md and the Flowanalyst consumer contract.\n";
                return 0;
            }
            if (option == "-a" || option == "--about") {
                std::cout << "Flowanalyst independently checks the semantic sanity of FlowMini frontend bundles.\n"
                             "More help: Flowanalyst/README.md and the Flowanalyst consumer contract.\n";
                return 0;
            }
            if (option == "-v" || option == "--version") { std::cout << FLOWANALYST_VERSION << '\n'; return 0; }
        }
        int lowering_plan_version = 1, graph_plan_version = 1; std::string input_path, graph_provider_path;
        for (int index = 1; index < argc; ++index) {
            const std::string argument = argv[index];
            if (argument == "--diagnostics") {
                if (++index >= argc || std::string(argv[index]) != "json") throw std::runtime_error("--diagnostics requires json");
                structured_diagnostics = true;
            } else if (argument == "--lowering-plan-version") {
                if (++index >= argc) throw std::runtime_error("--lowering-plan-version requires 1 or 2");
                lowering_plan_version = std::stoi(argv[index]);
                if (lowering_plan_version != 1 && lowering_plan_version != 2) throw std::runtime_error("unsupported lowering plan version");
            } else if (argument == "--graph-plan-version") {
                if (++index >= argc) throw std::runtime_error("--graph-plan-version requires 1 or 2");
                graph_plan_version = std::stoi(argv[index]);
                if (graph_plan_version != 1 && graph_plan_version != 2) throw std::runtime_error("unsupported graph plan version");
            } else if (argument == "--graph-providers") {
                if (++index >= argc || !graph_provider_path.empty()) throw std::runtime_error("--graph-providers requires one selection artifact");
                graph_provider_path = argv[index];
            } else if (!argument.empty() && argument.front() == '-') throw std::runtime_error("unknown option: " + argument);
            else if (input_path.empty()) input_path = argument;
            else throw std::runtime_error("too many input paths");
        }
        std::string input_text;
        if (!input_path.empty()) { std::ifstream file(input_path); if (!file) throw std::runtime_error("cannot open bundle"); input_text = flowcontracts::read_bounded(file, "frontend bundle"); }
        else input_text = flowcontracts::read_bounded(std::cin, "frontend bundle");
        Json provider_map = Object{{"format", std::string("flowcore.graph_provider_map")}, {"version", 1}, {"providers", Array{}}};
        if (!graph_provider_path.empty()) {
            std::ifstream file(graph_provider_path);
            if (!file) throw std::runtime_error("cannot open graph provider map");
            provider_map = Parser(flowcontracts::read_bounded(file, "graph provider map")).parse();
        }
        return run(Parser(input_text).parse(), lowering_plan_version, provider_map, graph_plan_version);
    }
    catch (const std::bad_alloc&) {
        if (structured_diagnostics) write_structured_failure("FLOWANALYST_RESOURCE_EXHAUSTED", "runtime", "allocation failed");
        else std::cerr << "flowanalyst error: allocation failed\n";
        return 1;
    }
    catch (const std::exception& error) {
        if (structured_diagnostics) {
            const auto classification = classify_failure(error.what());
            write_structured_failure(classification.code, classification.stage, error.what());
        }
        else std::cerr << "flowanalyst error: " << error.what() << '\n';
        return 1;
    } catch (...) {
        if (structured_diagnostics) write_structured_failure("FLOWANALYST_UNKNOWN_FAILURE", "cli", "unknown non-standard failure");
        else std::cerr << "flowanalyst error: unknown non-standard failure\n";
        return 1;
    }
}
