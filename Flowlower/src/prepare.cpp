#include <flowcontracts/validate.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
using namespace flowcontracts;
using namespace flowcontracts::json;

struct Options { std::string optimization_path, binding_path, target_name, target_policy_path; };

Options options(int argc, char** argv) {
    Options result;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--binding-report") {
            if (++index >= argc) throw std::runtime_error("--binding-report requires a path");
            result.binding_path = argv[index];
        } else if (argument == "--target") {
            if (++index >= argc) throw std::runtime_error("--target requires a name");
            result.target_name = argv[index];
        } else if (argument == "--target-policy") {
            if (++index >= argc) throw std::runtime_error("--target-policy requires a path");
            result.target_policy_path = argv[index];
        } else if (!argument.empty() && argument.front() == '-') throw std::runtime_error("unknown option '" + argument + "'");
        else if (result.optimization_path.empty()) result.optimization_path = argument;
        else throw std::runtime_error("too many input paths");
    }
    return result;
}

std::string read(const std::string& path) {
    std::ostringstream input;
    if (path.empty()) input << std::cin.rdbuf();
    else { std::ifstream file(path); if (!file) throw std::runtime_error("cannot open artifact: " + path); input << file.rdbuf(); }
    return input.str();
}

Value select_target(const Object& root, const std::string& requested) {
    const auto& targets = required_array(root, "targets");
    if (targets.empty()) {
        if (!requested.empty() && requested != "main") throw Error("$.targets", "requested target is not present");
        return Object{{"name", "main"}, {"selection", "implicit-default"}};
    }
    if (requested.empty()) throw Error("$.targets", "multiple targets require explicit --target selection");
    for (const auto& value : targets) {
        const auto& target = object(value, "$.targets[]");
        if (string(required(target, "name", "$.targets[]"), "$.targets[].name") == requested) return value;
    }
    throw Error("$.targets", "requested target is not present");
}

int prepare(const Options& option) {
    const auto optimization = parse(read(option.optimization_path));
    validate_optimization_report(optimization);
    const auto& root = object(optimization);
    if (header(optimization).status != "ready") throw Error("$.status", "optimization artifact is not ready");

    Array capabilities;
    Object binding_provenance{{"format", "none"}, {"status", "not-required"}, {"version", Integer{0}}};
    if (!option.binding_path.empty()) {
        const auto binding = parse(read(option.binding_path));
        validate_binding_report(binding);
        const auto binding_header = header(binding);
        if (binding_header.status != "ready") throw Error("$.status", "binding artifact is not ready");
        capabilities = required_array(object(binding), "capabilities");
        binding_provenance = Object{{"format", binding_header.format}, {"status", binding_header.status}, {"version", binding_header.version}};
    }
    const auto& operations = required_array(object(required(root, "lowering_plan"), "$.lowering_plan"), "operations", "$.lowering_plan");
    bool requires_binding = false;
    for (const auto& value : operations) {
        const auto kind = string(required(object(value, "$.lowering_plan.operations[]"), "kind", "$.lowering_plan.operations[]"), "$.lowering_plan.operations[].kind");
        if (kind == "external_call" || kind == "text_outcome") requires_binding = true;
    }
    if (const auto* graph = optional(object(required(root, "lowering_plan")), "source_graph")) {
        if (!source_graph(*graph).executable) throw Error("$.lowering_plan.source_graph", "source graph execution is not admitted");
        requires_binding = true;
    }
    if (requires_binding && capabilities.empty()) throw Error("$.authorization.capabilities", "external calls require a ready binding report");

    const auto optimization_header = header(optimization);
    Object output{
        {"abi_type_contracts", required(root, "abi_type_contracts")},
        {"authorization", Object{{"capabilities", capabilities}, {"status", requires_binding ? "authorized" : "not-required"}}},
        {"external_operations", required(root, "external_operations")},
        {"format", "flowcore.backend_lowering_artifact"},
        {"lowering_plan", required(root, "lowering_plan")},
        {"provenance", Object{
            {"binding", binding_provenance},
            {"optimization", Object{{"format", optimization_header.format}, {"status", optimization_header.status}, {"version", optimization_header.version}}},
            {"transforms", required(root, "transforms")}}},
        {"source", required(root, "source")},
        {"status", "ready"},
        {"target", select_target(root, option.target_name)},
        {"targets", required(root, "targets")},
        {"version", Integer{option.target_policy_path.empty() ? 1 : 2}}
    };
    if (const auto* schedule = optional(root, "graph_schedule")) output.emplace("graph_schedule", *schedule);
    if (!option.target_policy_path.empty()) {
        const auto target_policy = parse(read(option.target_policy_path));
        validate_target_policy(target_policy);
        if (header(target_policy).status != "ready") throw Error("$.target_policy.status", "target policy is not ready");
        output.emplace("target_policy", target_policy);
    }
    validate_backend_lowering_artifact(output);
    std::cout << serialize(output) << '\n';
    return 0;
}
} // namespace

int main(int argc, char** argv) {
    try {
        if (argc == 2 && std::string(argv[1]) == "--version") { std::cout << "0.1.0\n"; return 0; }
        return prepare(options(argc, argv));
    } catch (const flowcontracts::json::Error& error) {
        std::cerr << "flowprepare contract error: " << error.what() << '\n'; return 1;
    } catch (const std::exception& error) {
        std::cerr << "flowprepare error: " << error.what() << '\n'; return 1;
    }
}
