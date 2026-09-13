#pragma once

#include <flowcontracts/json.hpp>
#include <flowcontracts/source_graph.hpp>
#include <flowcontracts/graph_execution.hpp>
#include <flowcontracts/binding_evidence.hpp>

#include <set>
#include <string>
#include <string_view>

namespace flowcontracts {

struct Header { std::string format; json::Integer version = 0; std::string status; };

inline Header header(const json::Value& value) {
    const auto& root = json::object(value);
    return {json::string(json::required(root, "format"), "$.format"),
            json::integer(json::required(root, "version"), "$.version"),
            json::string(json::required(root, "status"), "$.status")};
}
inline Header require_header(const json::Value& value, std::string_view format, json::Integer version) {
    auto result = header(value);
    if (result.format != format) throw json::Error("$.format", "unsupported artifact format '" + result.format + "'");
    if (result.version != version) throw json::Error("$.version", "unsupported " + result.format + " version " + std::to_string(result.version));
    return result;
}

struct MatrixEntry { json::Integer row = 0; json::Integer column = 0; bool value = true; };
struct MatrixView { std::string name; json::Integer rows = 0; json::Integer columns = 0; std::string semiring; std::string storage; std::vector<MatrixEntry> entries; };
struct SemanticReport {
    Header artifact; std::string source_path; json::Array targets; json::Array external_operations;
    json::Array abi_type_contracts; json::Value lowering_plan; std::size_t proven_pure_count = 0;
    std::size_t independent_candidate_count = 0; MatrixView dependency_matrix;
};

inline const json::Object& required_object(const json::Object& parent, std::string_view key, std::string_view path = "$") {
    return json::object(json::required(parent, key, path), std::string(path) + "." + std::string(key));
}
inline const json::Array& required_array(const json::Object& parent, std::string_view key, std::string_view path = "$") {
    return json::array(json::required(parent, key, path), std::string(path) + "." + std::string(key));
}

inline void validate_targets(const json::Object& root) {
    const auto& targets = required_array(root, "targets");
    std::set<json::Integer> symbols; std::set<std::string> names;
    for (std::size_t index = 0; index < targets.size(); ++index) {
        const auto path = "$.targets[" + std::to_string(index) + "]";
        const auto& target = json::object(targets[index], path);
        const auto symbol = json::integer(json::required(target, "symbol_id", path), path + ".symbol_id");
        const auto name = json::string(json::required(target, "name", path), path + ".name");
        const auto mains = json::integer(json::required(target, "main_count", path), path + ".main_count");
        if (symbol < 0 || name.empty() || mains < 0) throw json::Error(path, "invalid target identity");
        if (!symbols.insert(symbol).second || !names.insert(name).second) throw json::Error(path, "duplicate target identity");
    }
}

inline void validate_abi_contracts(const json::Object& root) {
    const auto& contracts = required_array(root, "abi_type_contracts");
    std::set<std::pair<std::string, std::string>> identities;
    for (std::size_t index = 0; index < contracts.size(); ++index) {
        const auto path = "$.abi_type_contracts[" + std::to_string(index) + "]";
        const auto& contract = json::object(contracts[index], path);
        const auto owner = json::string(json::required(contract, "contract", path), path + ".contract");
        const auto name = json::string(json::required(contract, "name", path), path + ".name");
        for (const auto field : {"repr", "ownership", "access", "lifetime", "nullable", "opaque", "cleanup"})
            (void)json::string(json::required(contract, field, path), path + "." + field);
        if (!identities.emplace(owner, name).second) throw json::Error(path, "duplicate ABI contract identity");
    }
}

inline void validate_lowering_authority(const json::Value& value, std::string_view base = "$.lowering_plan") {
    const auto& plan = json::object(value, base);
    if (const auto* graph = json::optional(plan, "source_graph")) {
        if (!source_graph(*graph, std::string(base) + ".source_graph").executable)
            throw json::Error(std::string(base) + ".source_graph", "source graph execution is not admitted");
    }
    if (json::string(json::required(plan, "format", base), std::string(base) + ".format") != "flowcore.lowering_plan") throw json::Error(std::string(base) + ".format", "unsupported lowering plan format");
    const auto version = json::integer(json::required(plan, "version", base), std::string(base) + ".version");
    if (version != 1 && version != 2) throw json::Error(std::string(base) + ".version", "unsupported lowering plan version");
    const auto& operations = required_array(plan, "operations", base);
    std::set<json::Integer> function_ids;
    if (version == 2) {
        const auto& functions = required_array(plan, "functions", base);
        std::size_t entries = 0;
        for (std::size_t index = 0; index < functions.size(); ++index) {
            const auto path = std::string(base) + ".functions[" + std::to_string(index) + "]";
            const auto& function = json::object(functions[index], path);
            const auto id = json::integer(json::required(function, "symbol_id", path), path + ".symbol_id");
            if (id < 0 || !function_ids.insert(id).second) throw json::Error(path + ".symbol_id", "invalid or duplicate function identity");
            (void)json::string(json::required(function, "name", path), path + ".name");
            (void)json::integer(json::required(function, "scope_id", path), path + ".scope_id");
            (void)json::integer(json::required(function, "body_block_id", path), path + ".body_block_id");
            (void)json::string(json::required(function, "return_type", path), path + ".return_type");
            const auto availability = json::string(json::required(function, "availability", path), path + ".availability");
            if (availability != "definition" && availability != "declaration") throw json::Error(path + ".availability", "unsupported callable availability");
            if (json::boolean(json::required(function, "entry", path), path + ".entry")) ++entries;
            const auto& parameters = required_array(function, "parameters", path);
            std::set<json::Integer> parameter_ids;
            for (std::size_t parameter = 0; parameter < parameters.size(); ++parameter) {
                const auto parameter_path = path + ".parameters[" + std::to_string(parameter) + "]";
                const auto& item = json::object(parameters[parameter], parameter_path);
                const auto parameter_id = json::integer(json::required(item, "symbol_id", parameter_path), parameter_path + ".symbol_id");
                if (parameter_id < 0 || !parameter_ids.insert(parameter_id).second) throw json::Error(parameter_path + ".symbol_id", "invalid or duplicate parameter identity");
                (void)json::string(json::required(item, "type", parameter_path), parameter_path + ".type");
            }
        }
        (void)entries;
    }
    std::set<json::Integer> ids;
    for (std::size_t index = 0; index < operations.size(); ++index) {
        const auto path = std::string(base) + ".operations[" + std::to_string(index) + "]";
        const auto& operation = json::object(operations[index], path);
        const auto id = json::integer(json::required(operation, "id", path), path + ".id");
        if (id < 0 || !ids.insert(id).second) throw json::Error(path + ".id", id < 0 ? "operation identity must be non-negative" : "duplicate operation identity");
        (void)json::string(json::required(operation, "kind", path), path + ".kind");
        if (version == 2) {
            const auto owner = json::integer(json::required(operation, "function_symbol_id", path), path + ".function_symbol_id");
            if (!function_ids.count(owner)) throw json::Error(path + ".function_symbol_id", "operation owner is not in function catalog");
            if (json::string(json::required(operation, "kind", path), path + ".kind") == "call") {
                const auto callee = json::integer(json::required(operation, "callee_symbol_id", path), path + ".callee_symbol_id");
                if (callee >= 0 && !function_ids.count(callee)) throw json::Error(path + ".callee_symbol_id", "callee is not in function catalog");
            }
        }
        if (const auto* block = json::optional(operation, "block_id")) if (json::integer(*block, path + ".block_id") < 0) throw json::Error(path + ".block_id", "operation block identity must be non-negative");
        (void)required_array(operation, "operands", path);
        const auto operation_kind = json::string(json::required(operation, "kind", path), path + ".kind");
        if (operation_kind == "external_call" || operation_kind == "text_outcome") {
            const auto& provider = required_object(operation, "provider", path);
            for (const auto field : {"contract", "library", "convention", "symbol", "effect", "parameter_types", "return_type"})
                (void)json::string(json::required(provider, field, path + ".provider"), path + ".provider." + field);
            (void)binding_evidence(provider, path + ".provider");
            const auto& effect = required_object(operation, "effect_contract", path);
            for (const auto field : {"external", "determinism", "certainty"})
                (void)json::string(json::required(effect, field, path + ".effect_contract"), path + ".effect_contract." + field);
            const auto* outcome = json::optional(operation, "result_outcome");
            if (operation_kind == "text_outcome" && outcome == nullptr)
                throw json::Error(path + ".result_outcome", "text_outcome operation must declare its typed outcome");
            if (outcome) {
                const auto& value = json::object(*outcome, path + ".result_outcome");
                const auto outcome_type = json::string(json::required(value, "type", path + ".result_outcome"), path + ".result_outcome.type");
                const auto representation = json::string(json::required(value, "representation", path + ".result_outcome"), path + ".result_outcome.representation");
                const auto success_type = json::string(json::required(value, "success_type", path + ".result_outcome"), path + ".result_outcome.success_type");
                const auto failure_type = json::string(json::required(value, "failure_type", path + ".result_outcome"), path + ".result_outcome.failure_type");
                if (operation_kind == "text_outcome" && (outcome_type != "Outcome" || representation != "tagged" || success_type != "Text" || failure_type != "TextFailure"))
                    throw json::Error(path + ".result_outcome", "Text outcome must use the tagged Outcome<Text,TextFailure> contract");
                const auto& codes = required_array(value, "failure_codes", path + ".result_outcome");
                if (codes.empty()) throw json::Error(path + ".result_outcome.failure_codes", "Text outcome must declare at least one failure code");
                for (const auto& code : codes) (void)json::string(code, path + ".result_outcome.failure_codes[]");
            }
            const auto& resources = required_array(operation, "argument_resources", path);
            for (std::size_t resource_index = 0; resource_index < resources.size(); ++resource_index) {
                const auto resource_path = path + ".argument_resources[" + std::to_string(resource_index) + "]";
                const auto& resource = json::object(resources[resource_index], resource_path);
                if (json::integer(json::required(resource, "index", resource_path), resource_path + ".index") != static_cast<json::Integer>(resource_index)) throw json::Error(resource_path + ".index", "resource index does not match position");
                for (const auto field : {"type", "memory_effect", "ownership", "access", "lifetime", "nullable", "opaque"})
                    (void)json::string(json::required(resource, field, resource_path), resource_path + "." + field);
            }
        }
    }
    if (const auto* graph_value = json::optional(plan, "source_graph")) {
        if (version != 2) throw json::Error(std::string(base), "native graph requires callable plan version 2");
        const auto graph = source_graph(*graph_value);
        const auto& functions = required_array(plan, "functions", base);
        std::map<json::Integer, const json::Object*> catalog;
        std::size_t entries = 0;
        for (const auto& item : functions) {
            const auto& fn = json::object(item);
            catalog.emplace(json::integer(json::required(fn, "symbol_id"), "$.symbol_id"), &fn);
            if (json::boolean(json::required(fn, "entry"), "$.entry")) {
                ++entries;
                if (!required_array(fn, "parameters").empty()) throw json::Error(std::string(base), "native graph entry arguments are unsupported");
            }
        }
        if (entries != 1) throw json::Error(std::string(base), "native graph requires one entry");
        auto resolve = [&](const json::Value& item, bool receiver) {
            const auto& node = json::object(item);
            const auto id = json::integer(json::required(node, "function_symbol_id"), "$.function_symbol_id");
            if (!catalog.count(id)) throw json::Error(std::string(base), "graph function identity is absent from callable catalog");
            const auto& fn = *catalog.at(id);
            if (json::boolean(json::required(fn, "entry"), "$.entry")) throw json::Error(std::string(base), "graph node cannot invoke native entry");
            const auto& parameters = required_array(fn, "parameters");
            if (parameters.size() != (receiver ? 1u : 0u) ||
                json::string(json::required(fn, "return_type"), "$.return_type") != json::string(json::required(node, "output_type"), "$.output_type"))
                throw json::Error(std::string(base), "graph function signature differs from callable catalog");
            if (!receiver) {
                const auto& provider = required_object(node, "provider");
                const auto& declared = required_object(fn, "provider");
                if (capability_identity(provider, "$.graph.provider") != capability_identity(declared, "$.function.provider") ||
                    json::string(json::required(node, "source_callable"), "$.source_callable") !=
                        json::string(json::required(provider, "contract"), "$.contract") + "." + json::string(json::required(fn, "name"), "$.name"))
                    throw json::Error(std::string(base), "graph provider differs from external callable identity");
            }
            if (receiver) {
                const auto& param = json::object(parameters.front());
                if (json::integer(json::required(param, "symbol_id"), "$.symbol_id") != json::integer(json::required(node, "parameter_symbol_id"), "$.parameter_symbol_id") ||
                    json::string(json::required(param, "type"), "$.type") != json::string(json::required(node, "input_type"), "$.input_type") ||
                    json::string(json::required(fn, "availability"), "$.availability") != "definition")
                    throw json::Error(std::string(base), "graph receiver definition differs from callable catalog");
            }
        };
        for (const auto& node : graph.receivers) resolve(node, true);
        for (const auto& node : graph.providers) resolve(node, false);
    }
    for (std::size_t index = 0; index < operations.size(); ++index) {
        const auto path = std::string(base) + ".operations[" + std::to_string(index) + "]";
        const auto& operation = json::object(operations[index], path);
        for (const auto field : {"then_block_id", "else_block_id", "body_block_id"}) if (const auto* reference = json::optional(operation, field)) {
            const auto block = json::integer(*reference, path + "." + field);
            if (block < -1) throw json::Error(path + "." + field, "control reference must be -1 or non-negative");
        }
    }
}

inline MatrixView parse_matrix(const json::Object& root) {
    const auto& graph = required_object(root, "analysis_graph");
    if (json::string(json::required(graph, "format", "$.analysis_graph"), "$.analysis_graph.format") != "flowanalyst.analysis_graph") throw json::Error("$.analysis_graph.format", "unsupported analysis graph format");
    if (json::integer(json::required(graph, "version", "$.analysis_graph"), "$.analysis_graph.version") != 1) throw json::Error("$.analysis_graph.version", "unsupported analysis graph version");
    const auto& views = required_array(graph, "matrix_views", "$.analysis_graph");
    const json::Object* selected = nullptr; std::size_t selected_index = 0;
    for (std::size_t index = 0; index < views.size(); ++index) {
        const auto path = "$.analysis_graph.matrix_views[" + std::to_string(index) + "]";
        const auto& view = json::object(views[index], path);
        if (json::string(json::required(view, "name", path), path + ".name") == "region_dependency") {
            if (selected) throw json::Error("$.analysis_graph.matrix_views", "duplicate region_dependency matrix view");
            selected = &view; selected_index = index;
        }
    }
    if (!selected) throw json::Error("$.analysis_graph.matrix_views", "required region_dependency matrix view is missing");
    const auto path = "$.analysis_graph.matrix_views[" + std::to_string(selected_index) + "]";
    MatrixView result;
    result.name = "region_dependency";
    result.rows = json::integer(json::required(*selected, "rows", path), path + ".rows");
    result.columns = json::integer(json::required(*selected, "columns", path), path + ".columns");
    if (result.rows < 0 || result.columns < 0) throw json::Error(path, "matrix dimensions must be non-negative");
    result.semiring = json::string(json::required(*selected, "semiring", path), path + ".semiring");
    result.storage = json::string(json::required(*selected, "storage", path), path + ".storage");
    const auto& entries = required_array(*selected, "entries", path);
    std::set<std::pair<json::Integer, json::Integer>> coordinates;
    for (std::size_t index = 0; index < entries.size(); ++index) {
        const auto entry_path = path + ".entries[" + std::to_string(index) + "]";
        const auto& entry = json::object(entries[index], entry_path);
        MatrixEntry parsed;
        parsed.row = json::integer(json::required(entry, "row", entry_path), entry_path + ".row");
        parsed.column = json::integer(json::required(entry, "column", entry_path), entry_path + ".column");
        if (const auto* item = json::optional(entry, "value")) parsed.value = json::boolean(*item, entry_path + ".value");
        if (parsed.row < 0 || parsed.row >= result.rows) throw json::Error(entry_path + ".row", "matrix row is outside declared dimensions");
        if (parsed.column < 0 || parsed.column >= result.columns) throw json::Error(entry_path + ".column", "matrix column is outside declared dimensions");
        if (!coordinates.emplace(parsed.row, parsed.column).second) throw json::Error(entry_path, "duplicate matrix coordinate");
        result.entries.push_back(parsed);
    }
    return result;
}

inline SemanticReport semantic_report(const json::Value& value) {
    SemanticReport result; result.artifact = require_header(value, "flowanalyst.semantic_report", 1);
    if (result.artifact.status != "ok") return result;
    const auto& root = json::object(value);
    const auto& source = required_object(root, "source");
    result.source_path = json::string(json::required(source, "path", "$.source"), "$.source.path");
    validate_targets(root); result.targets = required_array(root, "targets");
    result.external_operations = required_array(root, "external_operations");
    validate_abi_contracts(root); result.abi_type_contracts = required_array(root, "abi_type_contracts");
    result.lowering_plan = json::required(root, "lowering_plan");
    validate_lowering_authority(result.lowering_plan);
    const auto& plan = json::object(result.lowering_plan, "$.lowering_plan");
    if (json::string(json::required(plan, "format", "$.lowering_plan"), "$.lowering_plan.format") != "flowcore.lowering_plan") throw json::Error("$.lowering_plan.format", "unsupported lowering plan format");
    const auto plan_version = json::integer(json::required(plan, "version", "$.lowering_plan"), "$.lowering_plan.version");
    if (plan_version != 1 && plan_version != 2) throw json::Error("$.lowering_plan.version", "unsupported lowering plan version");
    for (const auto& item : required_array(root, "effect_facts")) {
        const auto& fact = json::object(item, "$.effect_facts[]");
        if (const auto* certainty = json::optional(fact, "certainty")) if (json::string(*certainty, "$.effect_facts[].certainty") == "proven") ++result.proven_pure_count;
    }
    for (const auto& item : required_array(root, "parallel_candidates")) {
        const auto& candidate = json::object(item, "$.parallel_candidates[]");
        if (const auto* proof = json::optional(candidate, "proof")) if (json::string(*proof, "$.parallel_candidates[].proof") == "pure-callee-disjoint-inputs") ++result.independent_candidate_count;
    }
    result.dependency_matrix = parse_matrix(root);
    return result;
}

inline json::Value matrix_entries(const MatrixView& matrix) {
    json::Array entries;
    for (const auto& entry : matrix.entries) entries.emplace_back(json::Object{{"column", entry.column}, {"row", entry.row}, {"value", entry.value}});
    return entries;
}

struct ExecutionPlan {
    Header artifact; std::string source_path; json::Array targets; json::Array external_operations;
    json::Array abi_type_contracts; json::Value lowering_plan; json::Value graph_schedule; MatrixView dependency_matrix;
};

inline MatrixView execution_matrix(const json::Object& root) {
    const auto& view = required_object(root, "graph_projection");
    MatrixView result;
    result.name = json::string(json::required(view, "name", "$.graph_projection"), "$.graph_projection.name");
    if (result.name != "region_dependency") throw json::Error("$.graph_projection.name", "unsupported graph projection");
    result.rows = json::integer(json::required(view, "rows", "$.graph_projection"), "$.graph_projection.rows");
    result.columns = json::integer(json::required(view, "columns", "$.graph_projection"), "$.graph_projection.columns");
    if (result.rows < 0 || result.columns < 0) throw json::Error("$.graph_projection", "matrix dimensions must be non-negative");
    result.semiring = json::string(json::required(view, "semiring", "$.graph_projection"), "$.graph_projection.semiring");
    result.storage = json::string(json::required(view, "storage", "$.graph_projection"), "$.graph_projection.storage");
    const auto& entries = required_array(view, "entries", "$.graph_projection");
    for (std::size_t index = 0; index < entries.size(); ++index) {
        const auto path = "$.graph_projection.entries[" + std::to_string(index) + "]";
        const auto& entry = json::object(entries[index], path);
        MatrixEntry parsed;
        parsed.row = json::integer(json::required(entry, "row", path), path + ".row");
        parsed.column = json::integer(json::required(entry, "column", path), path + ".column");
        if (const auto* item = json::optional(entry, "value")) parsed.value = json::boolean(*item, path + ".value");
        if (parsed.row < 0 || parsed.row >= result.rows) throw json::Error(path + ".row", "matrix row is outside declared dimensions");
        if (parsed.column < 0 || parsed.column >= result.columns) throw json::Error(path + ".column", "matrix column is outside declared dimensions");
        result.entries.push_back(parsed);
    }
    return result;
}

inline ExecutionPlan execution_plan(const json::Value& value) {
    ExecutionPlan result; result.artifact = require_header(value, "flowparallel.execution_plan", 1);
    if (result.artifact.status != "ready") return result;
    const auto& root = json::object(value);
    const auto& source = required_object(root, "source");
    result.source_path = json::string(json::required(source, "path", "$.source"), "$.source.path");
    validate_targets(root); result.targets = required_array(root, "targets");
    result.external_operations = required_array(root, "external_operations");
    validate_abi_contracts(root); result.abi_type_contracts = required_array(root, "abi_type_contracts");
    result.lowering_plan = json::required(root, "lowering_plan");
    validate_lowering_authority(result.lowering_plan);
    validate_graph_schedule(root);
    if (const auto* schedule = json::optional(root, "graph_schedule")) result.graph_schedule = *schedule;
    const auto& plan = json::object(result.lowering_plan, "$.lowering_plan");
    if (json::string(json::required(plan, "format", "$.lowering_plan"), "$.lowering_plan.format") != "flowcore.lowering_plan") throw json::Error("$.lowering_plan.format", "unsupported lowering plan format");
    const auto plan_version = json::integer(json::required(plan, "version", "$.lowering_plan"), "$.lowering_plan.version");
    if (plan_version != 1 && plan_version != 2) throw json::Error("$.lowering_plan.version", "unsupported lowering plan version");
    result.dependency_matrix = execution_matrix(root);
    return result;
}

struct ProviderDecision { Header artifact; std::string provider; std::string representation; std::string reason; std::string fallback; };
inline ProviderDecision provider_decision(const json::Value& value) {
    ProviderDecision result; result.artifact = require_header(value, "flowparallel.graph_provider_decision", 1);
    if (result.artifact.status != "verified") throw json::Error("$.status", "provider decision is not verified");
    const auto& root = json::object(value);
    result.provider = json::string(json::required(root, "provider"), "$.provider");
    result.representation = json::string(json::required(root, "representation"), "$.representation");
    if (const auto* reason = json::optional(root, "reason")) result.reason = json::string(*reason, "$.reason");
    if (const auto* fallback = json::optional(root, "fallback")) result.fallback = json::string(*fallback, "$.fallback");
    return result;
}

} // namespace flowcontracts
