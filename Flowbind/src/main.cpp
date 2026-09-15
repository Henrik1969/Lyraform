#include <dlfcn.h>
#include <link.h>
#include <openssl/evp.h>
#include <array>
#include <memory>
#include <flowcontracts/artifacts.hpp>
#include <flowcontracts/bounded_input.hpp>
#include <flowcontracts/diagnostics.hpp>
#include <algorithm>
#include <cstddef>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <new>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

namespace {

constexpr std::string_view VERSION = "0.1.0";

struct Requirement { std::string contract, library, convention, symbol, effect, parameter_types, return_type, evidence; };
struct Grant { std::string library, symbol, convention, effect, parameter_types, return_type, evidence; bool exact_signature = false; };

struct Options { std::string report_path, policy_path, abi_manifest_path; bool structured_diagnostics = false; };

using Json = flowcontracts::json::Value;
using JsonArray = flowcontracts::json::Array;
using JsonObject = flowcontracts::json::Object;

class JsonParser {
public:
    explicit JsonParser(std::string text) : text_(std::move(text)) {}
    Json parse() const { return flowcontracts::json::parse(text_); }
private:
    std::string text_;
};

const Json* json_field(const Json& value, std::string_view name) {
    const auto* object = std::get_if<JsonObject>(&value);
    if (!object) return nullptr;
    const auto it = object->find(std::string{name});
    return it == object->end() ? nullptr : &it->second;
}

std::string json_text(const Json* value) {
    return value && std::holds_alternative<std::string>(*value) ? std::get<std::string>(*value) : std::string{};
}

long long json_integer(const Json* value, const char* field_name) {
    if (!value) throw std::runtime_error(std::string("JSON field '") + field_name + "' must be an integer");
    return flowcontracts::json::integer(*value, std::string("$.") + field_name);
}

const JsonArray& json_array(const Json* value, const char* field_name) {
    if (!value || !std::holds_alternative<JsonArray>(*value)) throw std::runtime_error(std::string("JSON field '") + field_name + "' must be an array");
    return std::get<JsonArray>(*value);
}

Options parse_options(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--diagnostics") {
            if (++i >= argc || std::string(argv[i]) != "json") throw std::runtime_error("--diagnostics requires json");
            options.structured_diagnostics = true;
        } else if (argument == "--policy") {
            if (++i >= argc) throw std::runtime_error("--policy requires a path");
            options.policy_path = argv[i];
        } else if (argument == "--abi-manifest") {
            if (++i >= argc) throw std::runtime_error("--abi-manifest requires a path");
            options.abi_manifest_path = argv[i];
        } else if (argument == "-h" || argument == "--help" || argument == "-?" || argument == "-a" || argument == "--about" || argument == "-v" || argument == "--version") {
            continue;
        } else if (!argument.empty() && argument.front() == '-') {
            throw std::runtime_error("unknown option '" + argument + "'");
        } else if (options.report_path.empty()) {
            options.report_path = argument;
        } else {
            throw std::runtime_error("too many input paths");
        }
    }
    return options;
}

std::string read_input(const Options& options) {
    if (!options.report_path.empty()) { std::ifstream file(options.report_path); if (!file) throw std::runtime_error("cannot open semantic report"); return flowcontracts::read_bounded(file, "semantic report"); }
    return flowcontracts::read_bounded(std::cin, "semantic report");
}

std::string read_path(const std::string& path, const char* description) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error(std::string("cannot open ") + description);
    return flowcontracts::read_bounded(file, description);
}

std::vector<Grant> read_policy(const std::string& path) {
    std::vector<Grant> grants;
    if (path.empty()) return grants;
    std::ifstream file(path);
    if (!file) throw std::runtime_error("cannot open binding policy");
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line.front() == '#') continue;
        std::istringstream words(line);
        std::string verb; Grant grant;
        words >> verb >> grant.library >> grant.symbol >> grant.convention >> grant.effect;
        if (!words || verb != "allow") throw std::runtime_error("invalid binding policy line");
        const bool has_parameter_types = static_cast<bool>(words >> grant.parameter_types);
        const bool has_return_type = static_cast<bool>(words >> grant.return_type);
        if (has_parameter_types != has_return_type) throw std::runtime_error("incomplete binding signature grant");
        if (has_return_type) {
            if (words >> grant.evidence) {
                (void)flowcontracts::binding_evidence(JsonObject{{"evidence", grant.evidence}}, "$.policy");
                std::string extra;
                if (words >> extra) throw std::runtime_error("unexpected binding policy fields");
            }
        }
        grant.exact_signature = has_parameter_types || has_return_type;
        if (!grant.parameter_types.empty() && grant.parameter_types == "-") grant.parameter_types.clear();
        if (!grant.return_type.empty() && grant.return_type == "-") grant.return_type.clear();
        grants.push_back(std::move(grant));
    }
    return grants;
}

bool granted(const std::vector<Grant>& grants, const Requirement& requirement) {
    for (const auto& grant : grants) if (grant.library == requirement.library && grant.symbol == requirement.symbol && grant.convention == requirement.convention && grant.effect == requirement.effect && grant.evidence == requirement.evidence && (!grant.exact_signature || (grant.parameter_types == requirement.parameter_types && grant.return_type == requirement.return_type))) return true;
    return false;
}

std::string json_string(const std::string& text) {
    return flowcontracts::json::serialize(Json{text});
}

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

FailureClassification classify_failure(std::string_view message) noexcept {
    if (message.find("library unavailable") != std::string_view::npos ||
        (message.find(" symbol '") != std::string_view::npos && message.find(" unavailable") != std::string_view::npos) ||
        message.find("loaded provider") != std::string_view::npos ||
        message.find("resolved symbol") != std::string_view::npos)
        return {"FLOWBIND_PROVIDER_FAILURE", "provider"};
    if (message.find("denied by capability policy") != std::string_view::npos ||
        message.find("binding policy") != std::string_view::npos)
        return {"FLOWBIND_POLICY_FAILURE", "policy"};
    if (message.find("unsupported") != std::string_view::npos &&
        (message.find("ABI") != std::string_view::npos || message.find("calling convention") != std::string_view::npos))
        return {"FLOWBIND_ABI_FAILURE", "abi"};
    return {"FLOWBIND_INPUT_INVALID", "input"};
}

FailureClassification classify_blocked(const std::vector<std::string>& failures) noexcept {
    for (const auto& failure : failures) {
        const auto classification = classify_failure(failure);
        if (classification.stage != "input") return classification;
    }
    return {"FLOWBIND_BINDING_REJECTED", "binding"};
}

std::string provider_digest(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("cannot read loaded provider bytes: " + path);
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> context(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    if (!context || EVP_DigestInit_ex(context.get(), EVP_sha256(), nullptr) != 1)
        throw std::runtime_error("cannot initialize provider SHA-256");
    std::array<char, 65536> buffer;
    while (file.read(buffer.data(), buffer.size()) || file.gcount()) {
        if (EVP_DigestUpdate(context.get(), buffer.data(), static_cast<std::size_t>(file.gcount())) != 1)
            throw std::runtime_error("cannot hash provider bytes");
    }
    if (!file.eof()) throw std::runtime_error("cannot read complete provider bytes");
    std::array<unsigned char, EVP_MAX_MD_SIZE> digest;
    unsigned int size = 0;
    if (EVP_DigestFinal_ex(context.get(), digest.data(), &size) != 1 || size != 32)
        throw std::runtime_error("cannot finish provider SHA-256");
    const char hex[] = "0123456789abcdef";
    std::string result;
    for (unsigned int i = 0; i < size; ++i) { result += hex[digest[i] >> 4]; result += hex[digest[i] & 15]; }
    return result;
}

std::vector<Requirement> requirements(const std::string& report) {
    const Json root = JsonParser{report}.parse();
    constexpr std::size_t max_binding_requirements = 100000;
    const auto& entries = json_array(json_field(root, "binding_requirements"), "binding_requirements");
    if (entries.size() > max_binding_requirements)
        throw std::runtime_error("binding requirement count exceeds the 100000-entry limit");
    std::vector<Requirement> result;
    result.reserve(entries.size());
    for (const auto& item : entries) {
        result.push_back({
            json_text(json_field(item, "contract")),
            json_text(json_field(item, "library")),
            json_text(json_field(item, "convention")),
            json_text(json_field(item, "symbol")),
            json_text(json_field(item, "effect")),
            json_text(json_field(item, "parameter_types")),
            json_text(json_field(item, "return_type")),
            flowcontracts::binding_evidence(flowcontracts::json::object(item), "$.binding_requirements")
        });
    }
    return result;
}

void validate_lowering_plan(const std::string& report, const std::vector<Requirement>& needed) {
    const Json root = JsonParser{report}.parse();
    const Json* plan = json_field(root, "lowering_plan");
    if (plan == nullptr) return;
    if (const auto* graph = json_field(*plan, "source_graph")) {
        const auto model = flowcontracts::source_graph(*graph, "$.lowering_plan.source_graph");
        if (!model.executable) throw std::runtime_error("source graph execution is not admitted");
        for (const auto& node : model.providers) {
            const auto& provider = flowcontracts::json::object(*json_field(node, "provider"));
            bool found = false;
            for (const auto& requirement : needed) {
                const JsonObject tuple{{"contract", requirement.contract}, {"library", requirement.library},
                    {"symbol", requirement.symbol}, {"convention", requirement.convention},
                    {"effect", requirement.effect}, {"parameter_types", requirement.parameter_types},
                    {"return_type", requirement.return_type}, {"evidence", requirement.evidence}};
                if (flowcontracts::capability_identity(tuple, "$.binding_requirements") ==
                    flowcontracts::capability_identity(provider, "$.source_graph.providers")) found = true;
            }
            if (!found) throw std::runtime_error("graph provider does not match a semantic binding requirement");
        }
    }
    const auto plan_version = json_integer(json_field(*plan, "version"), "lowering_plan.version");
    if (json_text(json_field(*plan, "format")) != "flowcore.lowering_plan" ||
        (plan_version != 1 && plan_version != 2)) {
        throw std::runtime_error("unsupported lowering plan contract");
    }
    const auto& operations = json_array(json_field(*plan, "operations"), "lowering_plan.operations");
    for (const auto& operation : operations) {
        for (const auto& operand : json_array(json_field(operation, "operands"), "lowering operation operands")) {
            if (json_text(json_field(operand, "kind")) != "writable_storage") continue;
            if (json_text(json_field(operand, "type")) != "c_pointer") throw std::runtime_error("writable storage must have c_pointer carrier type");
            const Json* storage = json_field(operand, "storage");
            if (storage == nullptr || json_integer(json_field(*storage, "bytes"), "writable storage bytes") <= 0 ||
                json_text(json_field(*storage, "access")) != "read_write" ||
                json_text(json_field(*storage, "lifetime")) != "call") {
                throw std::runtime_error("invalid writable storage descriptor");
            }
        }
        const auto kind = json_text(json_field(operation, "kind"));
        if (kind != "external_call" && kind != "text_outcome") continue;
        const Json* provider = json_field(operation, "provider");
        if (provider == nullptr) throw std::runtime_error("external lowering operation has no provider identity");
        const auto contract = json_text(json_field(*provider, "contract"));
        const auto library = json_text(json_field(*provider, "library"));
        const auto convention = json_text(json_field(*provider, "convention"));
        const auto symbol = json_text(json_field(*provider, "symbol"));
        const auto effect = json_text(json_field(*provider, "effect"));
        const auto parameter_types = json_text(json_field(*provider, "parameter_types"));
        const auto return_type = json_text(json_field(*provider, "return_type"));
        bool found = false;
        for (const auto& requirement : needed) {
            if (requirement.contract == contract && requirement.library == library && requirement.convention == convention &&
                requirement.symbol == symbol && requirement.effect == effect &&
                requirement.parameter_types == parameter_types && requirement.return_type == return_type &&
                requirement.evidence == flowcontracts::binding_evidence(flowcontracts::json::object(*provider), "$.lowering_plan.provider")) {
                found = true;
                break;
            }
        }
        if (!found) throw std::runtime_error("lowering operation provider does not match a semantic binding requirement");
        std::vector<std::string> expected_parameters;
        for (std::size_t start = 0; start < parameter_types.size();) {
            const auto end = parameter_types.find(',', start);
            expected_parameters.push_back(parameter_types.substr(start, end == std::string::npos ? std::string::npos : end - start));
            if (end == std::string::npos) break;
            start = end + 1;
        }
        const auto& operands = json_array(json_field(operation, "operands"), "external lowering operation operands");
        if (operands.size() != expected_parameters.size())
            throw std::runtime_error("external lowering operation operand count does not match its ABI contract");
        for (std::size_t index = 0; index < operands.size(); ++index) {
            if (json_text(json_field(operands[index], "type")) != expected_parameters[index])
                throw std::runtime_error("external lowering operation operand carrier does not match its ABI contract");
        }
        const Json* effect_contract = json_field(operation, "effect_contract");
        if (effect_contract == nullptr || json_text(json_field(*effect_contract, "external")) != effect ||
            json_text(json_field(*effect_contract, "determinism")) != (effect == "pure" ? "deterministic" : "unspecified") ||
            json_text(json_field(*effect_contract, "certainty")) != "declared") {
            throw std::runtime_error("lowering operation effect contract does not match provider declaration");
        }
        const auto& argument_resources = json_array(json_field(operation, "argument_resources"), "external lowering argument resources");
        if (argument_resources.size() != expected_parameters.size()) throw std::runtime_error("lowering argument resource count does not match ABI parameters");
        for (std::size_t index = 0; index < expected_parameters.size(); ++index) {
            const Json* expected_type = nullptr;
            for (const auto& type : json_array(json_field(root, "abi_type_contracts"), "abi_type_contracts"))
                if (json_text(json_field(type, "contract")) == contract && json_text(json_field(type, "name")) == expected_parameters[index]) { expected_type = &type; break; }
            const auto expected_access = expected_type ? json_text(json_field(*expected_type, "access")) : "value";
            const auto expected_memory = expected_type == nullptr ? "none" :
                (expected_access == "read" ? "read" : (expected_access == "read_write" || expected_access == "write" ? "read_write" : "opaque"));
            if (json_integer(json_field(argument_resources[index], "index"), "argument resource index") != static_cast<long long>(index) ||
                json_text(json_field(argument_resources[index], "type")) != expected_parameters[index] ||
                json_text(json_field(argument_resources[index], "memory_effect")) != expected_memory ||
                json_text(json_field(argument_resources[index], "ownership")) != (expected_type ? json_text(json_field(*expected_type, "ownership")) : "none") ||
                json_text(json_field(argument_resources[index], "access")) != expected_access ||
                json_text(json_field(argument_resources[index], "lifetime")) != (expected_type ? json_text(json_field(*expected_type, "lifetime")) : "value") ||
                json_text(json_field(argument_resources[index], "nullable")) != (expected_type ? json_text(json_field(*expected_type, "nullable")) : "not_applicable") ||
                json_text(json_field(argument_resources[index], "opaque")) != (expected_type ? json_text(json_field(*expected_type, "opaque")) : "false")) {
                throw std::runtime_error("lowering argument resource does not match its ABI type contract");
            }
        }
        const Json* expected_resource = nullptr;
        for (const auto& type : json_array(json_field(root, "abi_type_contracts"), "abi_type_contracts")) {
            if (json_text(json_field(type, "contract")) == contract && json_text(json_field(type, "name")) == return_type &&
                !json_text(json_field(type, "cleanup")).empty()) {
                expected_resource = &type;
                break;
            }
        }
        const Json* resource = json_field(operation, "result_resource");
        if (expected_resource != nullptr) {
            if (resource == nullptr || json_text(json_field(*resource, "type")) != return_type ||
                json_text(json_field(*resource, "ownership")) != json_text(json_field(*expected_resource, "ownership")) ||
                json_text(json_field(*resource, "access")) != json_text(json_field(*expected_resource, "access")) ||
                json_text(json_field(*resource, "lifetime")) != json_text(json_field(*expected_resource, "lifetime")) ||
                json_text(json_field(*resource, "nullable")) != json_text(json_field(*expected_resource, "nullable")) ||
                json_text(json_field(*resource, "opaque")) != json_text(json_field(*expected_resource, "opaque")) ||
                json_text(json_field(*resource, "cleanup_capability")) != json_text(json_field(*expected_resource, "cleanup"))) {
                throw std::runtime_error("lowering result resource does not match its ABI type contract");
            }
            bool cleanup_present = false;
            const auto cleanup = json_text(json_field(*expected_resource, "cleanup"));
            for (const auto& candidate : operations) {
                const Json* candidate_provider = json_field(candidate, "provider");
                if (candidate_provider != nullptr && json_text(json_field(*candidate_provider, "contract")) == contract &&
                    json_text(json_field(*candidate_provider, "symbol")) == cleanup) {
                    cleanup_present = true;
                    break;
                }
            }
            if (!cleanup_present) throw std::runtime_error("lowering plan acquires a resource without its declared cleanup capability");
        } else if (resource != nullptr) {
            throw std::runtime_error("lowering operation invents an undeclared result resource");
        }
    }

    struct ResourceContract { const Json* acquisition; std::string contract, cleanup; };
    std::vector<ResourceContract> resources;
    for (const auto& operation : operations) {
        const auto* resource = json_field(operation, "result_resource");
        const auto* provider = json_field(operation, "provider");
        if (resource != nullptr && provider != nullptr)
            resources.push_back({&operation, json_text(json_field(*provider, "contract")), json_text(json_field(*resource, "cleanup_capability"))});
    }
    auto optional_integer = [](const Json& value, std::string_view field_name, long long fallback) {
        const auto* item = json_field(value, field_name);
        return item == nullptr ? fallback : json_integer(item, "lowering operation identity");
    };
    std::map<long long, std::vector<const Json*>> blocks;
    for (const auto& operation : operations) blocks[optional_integer(operation, "block_id", 0)].push_back(&operation);
    for (auto& [block, items] : blocks) std::stable_sort(items.begin(), items.end(), [&](const Json* left, const Json* right) {
        const auto left_statement = optional_integer(*left, "statement_id", -1);
        const auto right_statement = optional_integer(*right, "statement_id", -1);
        if (left_statement != right_statement) return left_statement < right_statement;
        return optional_integer(*left, "id", -1) < optional_integer(*right, "id", -1);
    });
    for (const auto& resource : resources) {
        auto is_cleanup = [&](const Json& operation) {
            const auto* provider = json_field(operation, "provider");
            return provider != nullptr && json_text(json_field(*provider, "contract")) == resource.contract &&
                json_text(json_field(*provider, "symbol")) == resource.cleanup;
        };
        std::function<bool(long long)> block_has_resource_action = [&](long long block) {
            for (const auto* operation : blocks[block]) {
                if (operation == resource.acquisition || is_cleanup(*operation)) return true;
                if (json_text(json_field(*operation, "kind")) == "branch") {
                    const auto then_block = optional_integer(*operation, "then_block_id", -1);
                    const auto else_block = optional_integer(*operation, "else_block_id", -1);
                    if ((then_block >= 0 && block_has_resource_action(then_block)) || (else_block >= 0 && block_has_resource_action(else_block))) return true;
                }
            }
            return false;
        };
        std::function<std::set<int>(long long, std::set<int>)> walk = [&](long long block, std::set<int> states) {
            for (const auto* operation : blocks[block]) {
                const auto kind = json_text(json_field(*operation, "kind"));
                if (operation == resource.acquisition) {
                    if (states.count(0)) throw std::runtime_error("resource acquired again before its declared cleanup");
                    states = {0};
                } else if (is_cleanup(*operation)) {
                    if (states.count(-1)) throw std::runtime_error("resource cleanup is reachable before acquisition");
                    if (states.count(1)) throw std::runtime_error("resource path executes its declared cleanup more than once");
                    states = {1};
                } else if (kind == "branch") {
                    const auto then_block = optional_integer(*operation, "then_block_id", -1);
                    const auto else_block = optional_integer(*operation, "else_block_id", -1);
                    auto joined = then_block >= 0 ? walk(then_block, states) : states;
                    const auto alternative = else_block >= 0 ? walk(else_block, states) : states;
                    joined.insert(alternative.begin(), alternative.end());
                    states = std::move(joined);
                } else if (kind == "loop") {
                    const auto body_block = optional_integer(*operation, "body_block_id", -1);
                    if (body_block >= 0 && block_has_resource_action(body_block))
                        throw std::runtime_error("resource acquisition or cleanup inside a loop requires an explicit lifetime proof");
                } else if (kind == "return_value") {
                    if (states.count(0)) throw std::runtime_error("resource path exits without its declared cleanup capability");
                    return std::set<int>{};
                }
                if (states.empty()) break;
            }
            return states;
        };
        const auto exits = walk(0, {-1});
        if (exits.count(0)) throw std::runtime_error("resource path reaches program exit without its declared cleanup capability");
    }
}

JsonArray verified_aggregate_layouts(const std::string& report, const std::string& manifest) {
    const Json report_root = JsonParser{report}.parse();
    const Json manifest_root = JsonParser{manifest}.parse();
    if (json_text(json_field(manifest_root, "format")) != "flowcore.abi_manifest") throw std::runtime_error("unsupported ABI manifest format");
    if (json_text(json_field(manifest_root, "provider")).empty()) throw std::runtime_error("ABI manifest provider is empty");
    if (json_integer(json_field(manifest_root, "version"), "version") != 1) throw std::runtime_error("unsupported ABI manifest version");

    std::map<std::string, const Json*> semantic_layouts;
    for (const auto& layout : json_array(json_field(report_root, "aggregate_abi_layouts"), "aggregate_abi_layouts")) {
        const auto name = json_text(json_field(layout, "name"));
        if (name.empty() || !semantic_layouts.emplace(name, &layout).second) throw std::runtime_error("semantic report contains duplicate aggregate layout");
    }
    if (semantic_layouts.empty()) return {};

    const auto& types = json_array(json_field(manifest_root, "types"), "types");
    if (types.size() != semantic_layouts.size()) throw std::runtime_error("ABI manifest type count does not match semantic aggregate layouts");
    JsonArray verified;
    const auto scalar_size = [](const std::string& field_type) -> std::size_t {
        if (field_type == "c_int") return sizeof(int);
        if (field_type == "c_long") return sizeof(long);
        if (field_type == "c_ulong") return sizeof(unsigned long);
        if (field_type == "c_size_t") return sizeof(std::size_t);
        throw std::runtime_error("ABI manifest aggregate contains unsupported scalar field type '" + field_type + "'");
    };
    const auto scalar_alignment = [](const std::string& field_type) -> std::size_t {
        if (field_type == "c_int") return alignof(int);
        if (field_type == "c_long") return alignof(long);
        if (field_type == "c_ulong") return alignof(unsigned long);
        if (field_type == "c_size_t") return alignof(std::size_t);
        throw std::runtime_error("ABI manifest aggregate contains unsupported scalar field type '" + field_type + "'");
    };
    for (const auto& type : types) {
        const auto name = json_text(json_field(type, "name"));
        const auto found = semantic_layouts.find(name);
        if (found == semantic_layouts.end()) throw std::runtime_error("ABI manifest contains an undeclared aggregate layout");
        const auto size = json_integer(json_field(type, "size"), "size");
        const auto alignment = json_integer(json_field(type, "alignment"), "alignment");
        if (size <= 0 || alignment <= 0) throw std::runtime_error("ABI manifest aggregate size/alignment must be positive");
        const auto& manifest_fields = json_array(json_field(type, "fields"), "fields");
        const auto& semantic_fields = json_array(json_field(*found->second, "fields"), "fields");
        if (semantic_fields.size() != manifest_fields.size()) throw std::runtime_error("ABI manifest field count does not match semantic aggregate layout");
        JsonArray fields;
        std::size_t expected_size = 0;
        std::size_t expected_alignment = 1;
        for (std::size_t index = 0; index < semantic_fields.size(); ++index) {
            const auto& semantic_field = semantic_fields[index];
            const auto& manifest_field = manifest_fields[index];
            if (json_text(json_field(semantic_field, "name")) != json_text(json_field(manifest_field, "name")) ||
                json_text(json_field(semantic_field, "type")) != json_text(json_field(manifest_field, "type")))
                throw std::runtime_error("ABI manifest fields do not match semantic aggregate layout");
            const auto field_type = json_text(json_field(manifest_field, "type"));
            const auto offset = json_integer(json_field(manifest_field, "offset"), "offset");
            if (offset != static_cast<long long>(expected_size)) throw std::runtime_error("ABI manifest aggregate fields are not packed in declaration order");
            expected_size += scalar_size(field_type);
            expected_alignment = std::max(expected_alignment, scalar_alignment(field_type));
            fields.emplace_back(JsonObject{{"name", json_text(json_field(manifest_field, "name"))},
                                       {"offset", offset}, {"type", json_text(json_field(manifest_field, "type"))}});
        }
        if (size != static_cast<long long>(expected_size) || alignment != static_cast<long long>(expected_alignment))
            throw std::runtime_error("ABI manifest aggregate layout contains unsupported padding or alignment");
        verified.emplace_back(JsonObject{{"alignment", alignment}, {"contract", json_text(json_field(*found->second, "contract"))},
                                     {"fields", fields}, {"layout_policy", "provider_verified"}, {"name", name},
                                     {"size", size}, {"status", "verified"}, {"version", flowcontracts::json::Integer{1}}});
    }
    return verified;
}

int verify(const std::string& report, const std::string& policy_path, const std::string& abi_manifest_path) {
#ifdef FLOWBIND_TEST_ALLOCATION_FAILURE
    throw std::bad_alloc();
#endif
    const auto public_header = flowcontracts::require_header(flowcontracts::json::parse(report), "flowanalyst.semantic_report", 1);
    if (public_header.status != "ok") {
        std::cout << "{\n  \"format\": \"flowbind.binding_report\",\n  \"version\": 1,\n  \"status\": \"blocked\",\n  \"reason\": \"semantic report is not ready\"\n}\n";
        return 2;
    }
    const auto needed = requirements(report);
    validate_lowering_plan(report, needed);
    const auto verified_layouts = !abi_manifest_path.empty() ? verified_aggregate_layouts(report, read_path(abi_manifest_path, "ABI manifest")) : JsonArray{};
    const bool aggregate_manifest_verified = !verified_layouts.empty();
    const auto grants = read_policy(policy_path);
    const Json semantic_root = JsonParser{report}.parse();
    std::map<std::string, std::string> declared_representations;
    if (const auto* contracts = json_field(semantic_root, "abi_type_contracts"))
        for (const auto& type : json_array(contracts, "abi_type_contracts"))
            declared_representations[json_text(json_field(type, "name"))] = json_text(json_field(type, "repr"));
    auto supported_type = [&](const std::string& type) {
        if (type == "c_int" || type == "c_long" || type == "c_ulong" || type == "c_size_t" || type == "c_string" || type == "c_pointer" || type == "c_double") return true;
        if (type == "TextOutcome") return true;
        const auto found = declared_representations.find(type);
        return found != declared_representations.end() && (found->second == "void*" || found->second == "const void*" ||
                                                            (type == "Text" && found->second == "const char*"));
    };
    std::set<std::string> verified_aggregate_names;
    for (const auto& layout : verified_layouts) verified_aggregate_names.insert(json_text(json_field(layout, "name")));
    std::set<std::string> graph_aggregate_names;
    if (const auto* plan = json_field(semantic_root, "lowering_plan")) {
        if (const auto* graph = json_field(*plan, "source_graph")) {
            for (const auto& node : json_array(json_field(*graph, "receivers"), "source_graph.receivers")) {
                graph_aggregate_names.insert(json_text(json_field(node, "input_type")));
                graph_aggregate_names.insert(json_text(json_field(node, "output_type")));
            }
            for (const auto& node : json_array(json_field(*graph, "providers"), "source_graph.providers"))
                graph_aggregate_names.insert(json_text(json_field(node, "output_type")));
        }
    }
    using LibraryHandle = std::unique_ptr<void, int (*)(void*)>;
    std::map<std::string, LibraryHandle> handles;
    std::vector<std::string> failures;
    std::map<std::string, std::pair<std::string, std::string>> verified_providers;
    for (const auto& item : needed) {
        if (!granted(grants, item)) failures.push_back(item.library + ": symbol '" + item.symbol + "' denied by capability policy");
        if (item.convention != "c") failures.push_back(item.symbol + ": unsupported calling convention '" + item.convention + "'");
        if (item.return_type == "TextOutcome" &&
            !(item.contract == "text_runtime" && item.library == "libflowtext.so" &&
              item.symbol == "flow_text_concat_value" && item.parameter_types == "Text,Text"))
            failures.push_back(item.symbol + ": TextOutcome is reserved for the atomic text provider contract");
        if (!supported_type(item.return_type) && (!verified_aggregate_names.count(item.return_type) || !graph_aggregate_names.count(item.return_type)))
            failures.push_back(item.symbol + ": unsupported return ABI type '" + item.return_type + "'");
        std::size_t start = 0;
        while (start < item.parameter_types.size()) {
            const auto end = item.parameter_types.find(',', start);
            const auto type = item.parameter_types.substr(start, end == std::string::npos ? std::string::npos : end - start);
            if (!supported_type(type) && (!verified_aggregate_names.count(type) || !graph_aggregate_names.count(type)))
                failures.push_back(item.symbol + ": " + (verified_aggregate_names.count(type) ? "aggregate ABI manifest verified; aggregate call lowering is not implemented" : "unsupported parameter ABI type '" + type + "'"));
            if (end == std::string::npos) break;
            start = end + 1;
        }
        if (!granted(grants, item)) continue;
        if (!handles.count(item.library))
            handles.emplace(item.library, LibraryHandle{dlopen(item.library.c_str(), RTLD_LAZY | RTLD_LOCAL), &dlclose});
        if (!handles.at(item.library)) { failures.push_back(item.library + ": library unavailable"); continue; }
        void* symbol_address = dlsym(handles.at(item.library).get(), item.symbol.c_str());
        if (!symbol_address) { failures.push_back(item.library + ": symbol '" + item.symbol + "' unavailable"); continue; }
        if (!item.evidence.empty()) {
            try {
                const auto expected = item.evidence.substr(item.evidence.size() - 64);
                if (!verified_providers.count(item.library)) {
                    link_map* mapping = nullptr;
                    if (dlinfo(handles.at(item.library).get(), RTLD_DI_LINKMAP, &mapping) != 0 || !mapping || !mapping->l_name || !mapping->l_name[0])
                        throw std::runtime_error("loaded provider path is unavailable");
                    const std::string path = mapping->l_name;
                    verified_providers.emplace(item.library, std::pair{path, provider_digest(path)});
                }
                if (verified_providers.at(item.library).second != expected)
                    throw std::runtime_error("loaded provider SHA-256 does not match authorized evidence");
                Dl_info symbol_provider{};
                if (!dladdr(symbol_address, &symbol_provider) || !symbol_provider.dli_fname ||
                    provider_digest(symbol_provider.dli_fname) != expected)
                    throw std::runtime_error("resolved symbol is outside the authorized provider evidence");
            } catch (const std::exception& error) {
                failures.push_back(item.library + ": " + error.what());
            }
        }
    }
    if (!failures.empty()) {
        const auto classification = classify_blocked(failures);
        std::cout << "{\n  \"format\": \"flowbind.binding_report\",\n  \"version\": 1,\n  \"status\": \"blocked\",\n  \"code\": " << json_string(std::string{classification.code})
                  << ",\n  \"stage\": " << json_string(std::string{classification.stage})
                  << ",\n  \"provider\": \"dlopen+dlsym\",\n  \"failures\": [";
        for (std::size_t i = 0; i < failures.size(); ++i) { if (i) std::cout << ','; std::cout << json_string(failures[i]); }
        std::cout << "]";
        if (aggregate_manifest_verified) std::cout << ",\n  \"aggregate_abi\": \"verified\"";
        if (aggregate_manifest_verified) std::cout << ",\n  \"aggregate_abi_layouts\": " << flowcontracts::json::serialize(verified_layouts);
        std::cout << "\n}\n";
        return 2;
    }
    std::cout << "{\n  \"format\": \"flowbind.binding_report\",\n  \"version\": 1,\n  \"status\": \"ready\",\n  \"provider\": {\"name\": \"dlopen+dlsym\", \"requirements\": " << needed.size() << "},\n  \"symbols\": [";
    for (std::size_t i = 0; i < needed.size(); ++i) { if (i) std::cout << ','; std::cout << '"' << needed[i].symbol << '"'; }
    std::cout << "],\n  \"capabilities\": [";
    for (std::size_t i = 0; i < needed.size(); ++i) {
        if (i) std::cout << ',';
        const auto& item = needed[i];
        std::cout << "{\"contract\":" << json_string(item.contract)
                  << ",\"library\":" << json_string(item.library)
                  << ",\"symbol\":" << json_string(item.symbol)
                  << ",\"convention\":" << json_string(item.convention)
                  << ",\"effect\":" << json_string(item.effect)
                  << ",\"parameter_types\":" << json_string(item.parameter_types)
                  << ",\"return_type\":" << json_string(item.return_type)
                  << ",\"evidence\":" << json_string(item.evidence)
                  << ",\"status\":\"authorized\"}";
    }
    std::cout << "],\n  \"provider_evidence\": [";
    bool first_provider = true;
    for (const auto& [library, evidence] : verified_providers) {
        if (!first_provider) std::cout << ',';
        first_provider = false;
        std::cout << "{\"library\":" << json_string(library)
                  << ",\"path\":" << json_string(evidence.first)
                  << ",\"sha256\":" << json_string(evidence.second)
                  << ",\"status\":\"loaded-bytes-verified\"}";
    }
    std::size_t generic_operation_count = 0;
    try {
        const Json semantic_root = JsonParser{report}.parse();
        if (const auto* plan = json_field(semantic_root, "lowering_plan")) {
            generic_operation_count = json_array(json_field(*plan, "operations"), "lowering_plan.operations").size();
        }
    } catch (const std::exception&) {
        generic_operation_count = 0;
    }
    std::cout << "],\n  \"aggregate_abi_layouts\": " << flowcontracts::json::serialize(verified_layouts)
              << ",\n  \"lowering_plan\": {\"kind\": \"generic\""
              << ",\"contract\":\"flowcore.lowering_plan\",\"operation_count\":" << generic_operation_count;
    std::cout << "},\n  \"policy\": {\"status\": \"authorized\", \"grants\": " << grants.size() << "},\n  \"abi\": {\"convention\": \"c\", \"carrier_types_supported\": true, \"provider_signature_evidence\": \"not-provided\", \"sizeof_int\": " << sizeof(int) << ", \"sizeof_long\": " << sizeof(long) << ", \"sizeof_size_t\": " << sizeof(std::size_t) << ", \"sizeof_pointer\": " << sizeof(void*) << "},\n  \"execution\": \"not-performed\"\n}\n";
    return 0;
}

}

int main(int argc, char** argv) {
    bool structured_diagnostics = false;
    try {
        const auto options = parse_options(argc, argv);
        structured_diagnostics = options.structured_diagnostics;
        if (argc >= 2) {
            const std::string option = argv[1];
            if (option == "-h" || option == "--help" || option == "-?") { std::cout << "flowbind - verify and authorize external provider bindings\n\nUsage: flowbind [--policy policy.conf] [--abi-manifest manifest.json] [semantic-report.json]\n       flowmini ... | flowanalyst | flowbind --policy policy.conf\n\nPolicy: one exact grant per line: allow LIBRARY SYMBOL CONVENTION EFFECT\nABI manifest: provider-owned aggregate layout evidence\n\nOptions: -h, -?, --help  show help\n         -a, --about    show about information\n         -v, --version  print the raw version number\n\nMore help: Flowbind/README.md\n"; return 0; }
            if (option == "-a" || option == "--about") { std::cout << "Flowbind verifies declared external libraries and symbols without executing them.\nMore help: Flowbind/README.md\n"; return 0; }
            if (option == "-v" || option == "--version") { std::cout << VERSION << '\n'; return 0; }
        }
        return verify(read_input(options), options.policy_path, options.abi_manifest_path);
    } catch (const std::bad_alloc&) {
        if (structured_diagnostics) write_structured_failure("FLOWBIND_RESOURCE_EXHAUSTED", "runtime", "allocation failed");
        else std::cerr << "flowbind error: allocation failed\n";
        return 1;
    } catch (const std::exception& error) {
        if (structured_diagnostics) {
            const auto classification = classify_failure(error.what());
            write_structured_failure(classification.code, classification.stage, error.what());
        }
        else std::cerr << "flowbind error: " << error.what() << '\n';
        return 1;
    } catch (...) {
        if (structured_diagnostics) write_structured_failure("FLOWBIND_UNKNOWN_FAILURE", "cli", "unknown non-standard failure");
        else std::cerr << "flowbind error: unknown non-standard failure\n";
        return 1;
    }
}
