#pragma once

#include <flowcontracts/artifacts.hpp>

#include <set>
#include <string>
#include <string_view>

namespace flowcontracts {

enum class ValidationClass { valid, blocked, invalid, unsupported };

struct ValidationResult {
    ValidationClass classification = ValidationClass::invalid;
    std::string format;
    json::Integer version = 0;
    std::string source;
    std::string path = "$";
    std::string reason;
};

inline void validate_identity_array(const json::Object& parent, std::string_view key,
                                    std::string_view identity, std::string_view path) {
    const auto& values = required_array(parent, key, path);
    std::set<json::Integer> identities;
    for (std::size_t index = 0; index < values.size(); ++index) {
        const auto item_path = std::string(path) + "." + std::string(key) + "[" + std::to_string(index) + "]";
        const auto& item = json::object(values[index], item_path);
        const auto id = json::integer(json::required(item, identity, item_path), item_path + "." + std::string(identity));
        if (id < 0) throw json::Error(item_path + "." + std::string(identity), "identity must be non-negative");
        if (!identities.insert(id).second) throw json::Error(item_path + "." + std::string(identity), "duplicate identity");
    }
}

inline void validate_lowering_plan(const json::Value& value, std::string_view path = "$") {
    const auto& plan = json::object(value, path);
    if (json::string(json::required(plan, "format", path), std::string(path) + ".format") != "flowcore.lowering_plan")
        throw json::Error(std::string(path) + ".format", "unsupported lowering plan format");
    const auto version = json::integer(json::required(plan, "version", path), std::string(path) + ".version");
    if (version != 1 && version != 2)
        throw json::Error(std::string(path) + ".version", "unsupported lowering plan version");
    validate_identity_array(plan, "operations", "id", path);
    if (version == 2) {
        const auto& functions = required_array(plan, "functions", path);
        std::set<json::Integer> identities;
        std::size_t entries = 0;
        for (std::size_t index = 0; index < functions.size(); ++index) {
            const auto item_path = std::string(path) + ".functions[" + std::to_string(index) + "]";
            const auto& function = json::object(functions[index], item_path);
            const auto identity = json::integer(json::required(function, "symbol_id", item_path), item_path + ".symbol_id");
            if (identity < 0 || !identities.insert(identity).second) throw json::Error(item_path + ".symbol_id", "invalid or duplicate function identity");
            (void)json::string(json::required(function, "name", item_path), item_path + ".name");
            (void)json::integer(json::required(function, "scope_id", item_path), item_path + ".scope_id");
            (void)json::integer(json::required(function, "body_block_id", item_path), item_path + ".body_block_id");
            (void)json::string(json::required(function, "return_type", item_path), item_path + ".return_type");
            (void)json::string(json::required(function, "availability", item_path), item_path + ".availability");
            if (json::boolean(json::required(function, "entry", item_path), item_path + ".entry")) ++entries;
            const auto& parameters = required_array(function, "parameters", item_path);
            std::set<json::Integer> parameter_ids;
            for (std::size_t parameter = 0; parameter < parameters.size(); ++parameter) {
                const auto parameter_path = item_path + ".parameters[" + std::to_string(parameter) + "]";
                const auto& item = json::object(parameters[parameter], parameter_path);
                const auto parameter_id = json::integer(json::required(item, "symbol_id", parameter_path), parameter_path + ".symbol_id");
                if (parameter_id < 0 || !parameter_ids.insert(parameter_id).second) throw json::Error(parameter_path + ".symbol_id", "invalid or duplicate parameter identity");
                (void)json::string(json::required(item, "type", parameter_path), parameter_path + ".type");
            }
        }
        (void)entries;
        const auto& operations = required_array(plan, "operations", path);
        for (std::size_t index = 0; index < operations.size(); ++index) {
            const auto operation_path = std::string(path) + ".operations[" + std::to_string(index) + "]";
            const auto owner = json::integer(json::required(json::object(operations[index], operation_path), "function_symbol_id", operation_path), operation_path + ".function_symbol_id");
            if (!identities.count(owner)) throw json::Error(operation_path + ".function_symbol_id", "operation owner is not in function catalog");
        }
    }
}

inline void validate_frontend_bundle(const json::Value& value) {
    const auto& root = json::object(value);
    if (json::string(json::required(root, "format"), "$.format") != "flowmini.frontend_bundle") throw json::Error("$.format", "unsupported frontend bundle format");
    if (json::integer(json::required(root, "version"), "$.version") != 2) throw json::Error("$.version", "unsupported frontend bundle version");
    const auto& source = required_object(root, "source");
    (void)json::string(json::required(source, "path", "$.source"), "$.source.path");
    const auto& symbols = required_object(root, "symbol_table");
    validate_identity_array(symbols, "symbols", "id", "$.symbol_table");
    validate_identity_array(symbols, "scopes", "id", "$.symbol_table");
    const auto& ast = required_object(root, "ast");
    validate_identity_array(ast, "expression_pool", "id", "$.ast");
    validate_identity_array(ast, "statement_pool", "id", "$.ast");
    validate_identity_array(ast, "block_pool", "id", "$.ast");
    validate_identity_array(ast, "declaration_pool", "id", "$.ast");
}

inline void validate_binding_report(const json::Value& value) {
    const auto artifact = require_header(value, "flowbind.binding_report", 1);
    if (artifact.status != "ready") return;
    const auto& root = json::object(value);
    const auto& capabilities = required_array(root, "capabilities");
    for (std::size_t index = 0; index < capabilities.size(); ++index) {
        const auto path = "$.capabilities[" + std::to_string(index) + "]";
        const auto& item = json::object(capabilities[index], path);
        (void)binding_evidence(item, path);
        for (const auto field : {"contract", "library", "symbol", "convention", "effect", "parameter_types", "return_type", "status"})
            (void)json::string(json::required(item, field, path), path + "." + field);
    }
}

inline void validate_optimization_report(const json::Value& value) {
    const auto artifact = require_header(value, "flowoptimize.optimization_report", 1);
    if (artifact.status != "ready") return;
    const auto& root = json::object(value);
    validate_lowering_plan(json::required(root, "lowering_plan"), "$.lowering_plan");
    validate_graph_schedule(root);
    (void)required_array(root, "targets");
    const auto& transforms = required_array(root, "transforms");
    for (std::size_t index = 0; index < transforms.size(); ++index) {
        const auto path = "$.transforms[" + std::to_string(index) + "]";
        const auto& transform = json::object(transforms[index], path);
        (void)json::string(json::required(transform, "kind", path), path + ".kind");
        (void)json::boolean(json::required(transform, "semantics_preserved", path), path + ".semantics_preserved");
    }
    const auto& projections = required_array(root, "projections");
    for (std::size_t index = 0; index < projections.size(); ++index) {
        const auto path = "$.projections[" + std::to_string(index) + "]";
        const auto& projection = json::object(projections[index], path);
        const auto rows = json::integer(json::required(projection, "rows", path), path + ".rows");
        const auto columns = json::integer(json::required(projection, "columns", path), path + ".columns");
        if (rows < 0 || columns < 0) throw json::Error(path, "projection dimensions must be non-negative");
    }
}

inline void validate_target_policy(const json::Value& value) {
    const auto artifact = require_header(value, "flowcore.target_policy", 1);
    if (artifact.status != "ready") return;
    const auto& root = json::object(value);
    const auto name = json::string(json::required(root, "name"), "$.name");
    if (name.empty()) throw json::Error("$.name", "target policy name is empty");
    const auto& backend = required_object(root, "backend");
    const auto backend_name = json::string(json::required(backend, "name", "$.backend"), "$.backend.name");
    if (backend_name != "llvm" && backend_name != "tinyvm") throw json::Error("$.backend.name", "unsupported backend");
    (void)json::integer(json::required(backend, "artifact_version", "$.backend"), "$.backend.artifact_version");
    const auto& architecture = required_object(root, "architecture");
    (void)json::string(json::required(architecture, "name", "$.architecture"), "$.architecture.name");
    const auto bits = json::integer(json::required(architecture, "word_bits", "$.architecture"), "$.architecture.word_bits");
    if (bits <= 0) throw json::Error("$.architecture.word_bits", "word size must be positive");
    const auto endian = json::string(json::required(architecture, "endianness", "$.architecture"), "$.architecture.endianness");
    if (endian != "little" && endian != "big" && endian != "virtual") throw json::Error("$.architecture.endianness", "unsupported endianness");
    const auto& abi = required_object(root, "abi");
    (void)json::string(json::required(abi, "name", "$.abi"), "$.abi.name");
    (void)json::integer(json::required(abi, "version", "$.abi"), "$.abi.version");
    const auto& capabilities = required_object(root, "capabilities");
    for (const auto field : {"required", "admitted"}) {
        const auto& entries = required_array(capabilities, field, "$.capabilities");
        std::set<std::string> names;
        for (std::size_t index = 0; index < entries.size(); ++index) {
            const auto path = "$.capabilities." + std::string(field) + "[" + std::to_string(index) + "]";
            const auto name = json::string(entries[index], path);
            if (name.empty()) throw json::Error(path, "capability name is empty");
            if (!names.insert(name).second) throw json::Error(path, "duplicate capability name");
        }
    }
    const auto& resources = required_object(root, "resources");
    const auto slots = json::integer(json::required(resources, "maximum_slots", "$.resources"), "$.resources.maximum_slots");
    const auto steps = json::integer(json::required(resources, "maximum_steps", "$.resources"), "$.resources.maximum_steps");
    if (slots <= 0 || steps <= 0) throw json::Error("$.resources", "resource limits must be positive");
    const auto& lifecycle = required_object(root, "lifecycle");
    (void)json::string(json::required(lifecycle, "startup", "$.lifecycle"), "$.lifecycle.startup");
    (void)json::string(json::required(lifecycle, "cleanup", "$.lifecycle"), "$.lifecycle.cleanup");
    const auto& evidence = required_object(root, "evidence");
    (void)json::string(json::required(evidence, "contract", "$.evidence"), "$.evidence.contract");
    (void)json::string(json::required(evidence, "revision", "$.evidence"), "$.evidence.revision");
    const auto& fallback = required_object(root, "fallback");
    const auto mode = json::string(json::required(fallback, "mode", "$.fallback"), "$.fallback.mode");
    if (mode != "none" && mode != "explicit") throw json::Error("$.fallback.mode", "unsupported fallback mode");
    const auto target = json::string(json::required(fallback, "target", "$.fallback"), "$.fallback.target");
    if ((mode == "none" && !target.empty()) || (mode == "explicit" && target.empty()))
        throw json::Error("$.fallback", "fallback target does not match fallback mode");
}

inline void validate_bootstrap_seed(const json::Value& value) {
    const auto artifact = require_header(value, "flowcore.bootstrap_seed", 1);
    if (artifact.status != "captured") throw json::Error("$.status", "bootstrap seed is not captured");
    const auto& root = json::object(value);
    const auto& source = required_object(root, "source");
    (void)json::string(json::required(source, "revision", "$.source"), "$.source.revision");
    const auto digest = json::string(json::required(source, "tracked_sha256", "$.source"), "$.source.tracked_sha256");
    if (digest.size() != 64 || digest.find_first_not_of("0123456789abcdef") != std::string::npos)
        throw json::Error("$.source.tracked_sha256", "tracked source digest is not lowercase SHA-256");
    const auto& tools = required_object(root, "tools");
    for (const auto field : {"cmake", "c_compiler", "cxx_compiler", "clang", "jq"})
        (void)json::string(json::required(tools, field, "$.tools"), "$.tools." + std::string(field));
    const auto& standards = required_object(root, "standards");
    (void)json::string(json::required(standards, "c", "$.standards"), "$.standards.c");
    (void)json::string(json::required(standards, "cxx", "$.standards"), "$.standards.cxx");
    for (const auto field : {"providers", "deterministic_fields", "environment_fields"}) {
        const auto& entries = required_array(root, field);
        for (std::size_t index = 0; index < entries.size(); ++index)
            (void)json::string(entries[index], "$." + std::string(field) + "[" + std::to_string(index) + "]");
    }
}

inline void validate_bootstrap_gap_inventory(const json::Value& value) {
    const auto artifact = require_header(value, "flowcore.bootstrap_gap_inventory", 1);
    if (artifact.status != "incomplete" && artifact.status != "complete") throw json::Error("$.status", "unsupported bootstrap inventory status");
    const auto& root = json::object(value);
    (void)json::string(json::required(root, "baseline", "$"), "$.baseline");
    std::set<std::string> completed;
    for (const auto& item : required_array(root, "completed")) {
        const auto name = json::string(item, "$.completed[]");
        if (name.empty() || !completed.insert(name).second) throw json::Error("$.completed", "empty or duplicate completed identity");
    }
    std::set<std::string> gaps;
    const auto& entries = required_array(root, "gaps");
    for (std::size_t index=0; index<entries.size(); ++index) {
        const auto path="$.gaps["+std::to_string(index)+"]"; const auto& gap=json::object(entries[index],path);
        const auto id=json::string(json::required(gap,"id",path),path+".id");
        if(id.empty()||completed.count(id)||!gaps.insert(id).second)throw json::Error(path+".id","empty, completed, or duplicate gap identity");
        for(const auto field:{"owner","gate","acceptance"})(void)json::string(json::required(gap,field,path),path+"."+field);
        (void)required_array(gap,"requires",path);
    }
    for (std::size_t index=0; index<entries.size(); ++index) {
        const auto path="$.gaps["+std::to_string(index)+"]"; const auto& gap=json::object(entries[index],path);
        for(const auto& dependency:required_array(gap,"requires",path)) {
            const auto id=json::string(dependency,path+".requires[]");
            if(!completed.count(id)&&!gaps.count(id))throw json::Error(path+".requires","unknown bootstrap dependency identity");
        }
    }
    const auto& stages=required_array(root,"stages");
    if(stages.size()!=3)throw json::Error("$.stages","bootstrap inventory must describe Stage 1, 2 and 3");
    for(std::size_t index=0;index<stages.size();++index){const auto path="$.stages["+std::to_string(index)+"]";const auto& stage=json::object(stages[index],path);if(json::integer(json::required(stage,"stage",path),path+".stage")!=static_cast<json::Integer>(index+1))throw json::Error(path+".stage","bootstrap stages must be ordered 1, 2, 3");for(const auto field:{"producer","status","required_evidence"})(void)json::string(json::required(stage,field,path),path+"."+field);}
    if ((artifact.status == "complete") != entries.empty()) throw json::Error("$.status", "inventory completeness does not match remaining gaps");
}

inline void validate_backend_lowering_artifact(const json::Value& value) {
    const auto& header_root = json::object(value);
    if (json::string(json::required(header_root, "format"), "$.format") != "flowcore.backend_lowering_artifact")
        throw json::Error("$.format", "unsupported artifact format");
    const auto artifact_version = json::integer(json::required(header_root, "version"), "$.version");
    if (artifact_version != 1 && artifact_version != 2) throw json::Error("$.version", "unsupported artifact version");
    const Header artifact{"flowcore.backend_lowering_artifact", artifact_version,
                          json::string(json::required(header_root, "status"), "$.status")};
    if (artifact.status != "ready") return;
    const auto& root = json::object(value);
    if (artifact.version == 2) validate_target_policy(json::required(root, "target_policy"));
    const auto& source = required_object(root, "source");
    (void)json::string(json::required(source, "path", "$.source"), "$.source.path");
    validate_targets(root);
    validate_abi_contracts(root);
    validate_lowering_authority(json::required(root, "lowering_plan"));
    validate_graph_schedule(root);
    (void)required_array(root, "external_operations");
    const auto& target = required_object(root, "target");
    const auto target_name = json::string(json::required(target, "name", "$.target"), "$.target.name");
    if (target_name.empty()) throw json::Error("$.target.name", "selected target name is empty");
    const auto& targets = required_array(root, "targets");
    if (!targets.empty()) {
        bool selected = false;
        for (const auto& item : targets) {
            const auto& candidate = json::object(item, "$.targets[]");
            if (json::string(json::required(candidate, "name", "$.targets[]"), "$.targets[].name") == target_name) {
                selected = true;
                if (json::integer(json::required(candidate, "symbol_id", "$.targets[]"), "$.targets[].symbol_id") !=
                    json::integer(json::required(target, "symbol_id", "$.target"), "$.target.symbol_id"))
                    throw json::Error("$.target.symbol_id", "selected target identity does not match target catalog");
            }
        }
        if (!selected) throw json::Error("$.target", "selected target is not present in target catalog");
    }
    const auto& authorization = required_object(root, "authorization");
    const auto authorization_status = json::string(json::required(authorization, "status", "$.authorization"), "$.authorization.status");
    if (authorization_status != "authorized" && authorization_status != "not-required")
        throw json::Error("$.authorization.status", "unsupported authorization state");
    const auto& capabilities = required_array(authorization, "capabilities", "$.authorization");
    std::set<std::string> authorized;
    for (std::size_t index = 0; index < capabilities.size(); ++index) {
        const auto path = "$.authorization.capabilities[" + std::to_string(index) + "]";
        const auto& capability = json::object(capabilities[index], path);
        if (json::string(json::required(capability, "status", path), path + ".status") != "authorized")
            throw json::Error(path + ".status", "capability is not authorized");
        const auto identity = capability_identity(capability, path);
        if (!authorized.insert(identity).second) throw json::Error(path, "duplicate authorized capability identity");
    }
    std::set<std::string> required_capabilities;
    const auto& plan = json::object(json::required(root, "lowering_plan"), "$.lowering_plan");
    const auto& operations = required_array(plan, "operations", "$.lowering_plan");
    for (std::size_t index = 0; index < operations.size(); ++index) {
        const auto path = "$.lowering_plan.operations[" + std::to_string(index) + "]";
        const auto& operation = json::object(operations[index], path);
        const auto kind = json::string(json::required(operation, "kind", path), path + ".kind");
        if (kind != "external_call" && kind != "text_outcome") continue;
        const auto& provider = required_object(operation, "provider", path);
        const auto identity = capability_identity(provider, path + ".provider");
        required_capabilities.insert(identity);
    }
    if (const auto* graph_value = json::optional(plan, "source_graph"))
        for (const auto& node : source_graph(*graph_value).providers) {
            const auto& provider = required_object(json::object(node), "provider");
            required_capabilities.insert(capability_identity(provider, "$.source_graph.providers"));
        }
    if (required_capabilities != authorized)
        throw json::Error("$.authorization.capabilities", "authorized capability identities do not exactly match external operations");
    if ((required_capabilities.empty() && authorization_status != "not-required") ||
        (!required_capabilities.empty() && authorization_status != "authorized"))
        throw json::Error("$.authorization.status", "authorization state does not match external operations");
    const auto& provenance = required_object(root, "provenance");
    (void)required_object(provenance, "optimization", "$.provenance");
    (void)required_object(provenance, "binding", "$.provenance");
    (void)required_array(provenance, "transforms", "$.provenance");
}

inline void validate_lowering_report(const json::Value& value) {
    const auto artifact = require_header(value, "flowlower.lowering_report", 1);
    if (artifact.status == "blocked") return;
    const auto& root = json::object(value);
    const auto& backend = required_object(root, "backend");
    (void)json::string(json::required(backend, "name", "$.backend"), "$.backend.name");
    const auto& output = required_object(root, "artifact");
    (void)json::string(json::required(output, "backend", "$.artifact"), "$.artifact.backend");
    (void)json::string(json::required(output, "status", "$.artifact"), "$.artifact.status");
    const auto& ir = required_object(root, "ir");
    (void)json::string(json::required(ir, "format", "$.ir"), "$.ir.format");
}

inline void validate_abi_manifest(const json::Value& value) {
    const auto& root = json::object(value);
    if (json::integer(json::required(root, "version"), "$.version") != 1) throw json::Error("$.version", "unsupported ABI manifest version");
    (void)json::string(json::required(root, "provider"), "$.provider");
    const auto& types = required_array(root, "types");
    std::set<std::string> names;
    for (std::size_t index = 0; index < types.size(); ++index) {
        const auto path = "$.types[" + std::to_string(index) + "]";
        const auto& type = json::object(types[index], path);
        const auto name = json::string(json::required(type, "name", path), path + ".name");
        (void)json::integer(json::required(type, "size", path), path + ".size");
        (void)json::integer(json::required(type, "alignment", path), path + ".alignment");
        (void)required_array(type, "fields", path);
        if (!names.insert(name).second) throw json::Error(path + ".name", "duplicate ABI type identity");
    }
}

inline void validate_runtime_capabilities(const json::Value& value, std::string_view format) {
    const auto& root = json::object(value);
    if (json::integer(json::required(root, "version"), "$.version") != 1) throw json::Error("$.version", "unsupported runtime capability version");
    if (format == "frankencore.runtime_capabilities") {
        const auto& cuda = required_object(root, "cuda");
        (void)json::string(json::required(cuda, "status", "$.cuda"), "$.cuda.status");
        (void)json::integer(json::required(cuda, "device_count", "$.cuda"), "$.cuda.device_count");
    } else {
        (void)json::string(json::required(root, "status"), "$.status");
        (void)json::integer(json::required(root, "device_count"), "$.device_count");
    }
}

inline void validate_calibration(const json::Value& value, std::string_view format) {
    const auto artifact = require_header(value, format, 1);
    if (artifact.status == "verified") {
        const auto& root = json::object(value);
        const auto speedup = json::number(json::required(root, "end_to_end_speedup"), "$.end_to_end_speedup");
        if (speedup < 0.0) throw json::Error("$.end_to_end_speedup", "speedup must be non-negative");
    }
}

inline ValidationResult validate(const json::Value& value) {
    ValidationResult result;
    try {
        const auto& root = json::object(value);
        result.format = json::string(json::required(root, "format"), "$.format");
        result.version = json::integer(json::required(root, "version"), "$.version");
        if (const auto* source = json::optional(root, "source")) {
            const auto& object = json::object(*source, "$.source");
            if (const auto* path = json::optional(object, "path")) result.source = json::string(*path, "$.source.path");
        }
        if (result.format == "flowmini.frontend_bundle") validate_frontend_bundle(value);
        else if (result.format == "flowanalyst.semantic_report") (void)semantic_report(value);
        else if (result.format == "flowcore.lowering_plan") validate_lowering_plan(value);
        else if (result.format == "flowcore.graph_provider_map") (void)graph_provider_map(value);
        else if (result.format == "flowcore.source_graph") (void)source_graph(value);
        else if (result.format == "flowbind.binding_report") validate_binding_report(value);
        else if (result.format == "flowparallel.execution_plan") (void)execution_plan(value);
        else if (result.format == "flowparallel.graph_provider_decision") (void)provider_decision(value);
        else if (result.format == "flowoptimize.optimization_report") validate_optimization_report(value);
        else if (result.format == "flowcore.bootstrap_seed") validate_bootstrap_seed(value);
        else if (result.format == "flowcore.bootstrap_gap_inventory") validate_bootstrap_gap_inventory(value);
        else if (result.format == "flowcore.target_policy") validate_target_policy(value);
        else if (result.format == "flowcore.backend_lowering_artifact") validate_backend_lowering_artifact(value);
        else if (result.format == "flowlower.lowering_report") validate_lowering_report(value);
        else if (result.format == "flowcore.abi_manifest") validate_abi_manifest(value);
        else if (result.format == "frankencore.runtime_capabilities" || result.format == "flowcore.runtime_capabilities") validate_runtime_capabilities(value, result.format);
        else if (result.format == "flowparallel.matrix_benchmark" || result.format == "flowparallel.graph_cuda") validate_calibration(value, result.format);
        else return {ValidationClass::unsupported, result.format, result.version, result.source, "$.format", "artifact format is not supported"};
        if (const auto* status = json::optional(root, "status")) {
            const auto state = json::string(*status, "$.status");
            if (state == "blocked" || state == "error") return {ValidationClass::blocked, result.format, result.version, result.source, "$.status", "artifact reports " + state};
        }
        return {ValidationClass::valid, result.format, result.version, result.source, "$", "artifact is valid"};
    } catch (const json::Error& error) {
        return {ValidationClass::invalid, result.format, result.version, result.source, error.path(), error.reason()};
    } catch (const std::exception& error) {
        return {ValidationClass::invalid, result.format, result.version, result.source, "$", error.what()};
    }
}

inline std::string_view name(ValidationClass value) {
    switch (value) {
        case ValidationClass::valid: return "valid";
        case ValidationClass::blocked: return "blocked";
        case ValidationClass::invalid: return "invalid";
        case ValidationClass::unsupported: return "unsupported";
    }
    return "invalid";
}

} // namespace flowcontracts
