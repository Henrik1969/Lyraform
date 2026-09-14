#include <flowcontracts/artifacts.hpp>

#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
constexpr std::string_view VERSION = "0.1.0";
struct Options { std::string input_path; std::string provider_decision_path; bool structured_diagnostics = false; };

Options parse_options(int argc, char** argv) {
    Options options;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--diagnostics") { if (++index >= argc || std::string(argv[index]) != "json") throw std::runtime_error("--diagnostics requires json"); options.structured_diagnostics = true; }
        else if (argument == "--provider-decision") { if (++index >= argc) throw std::runtime_error("--provider-decision requires a file"); options.provider_decision_path = argv[index]; }
        else if (!argument.empty() && argument[0] == '-') throw std::runtime_error("unknown option: " + argument);
        else if (options.input_path.empty()) options.input_path = argument;
        else throw std::runtime_error("only one input report is accepted");
    }
    return options;
}
std::string json_escape(std::string_view value) {
    std::string escaped;
    for (const unsigned char c : value) {
        if (c == '"') escaped += "\\\"";
        else if (c == '\\') escaped += "\\\\";
        else if (c == '\n') escaped += "\\n";
        else if (c == '\r') escaped += "\\r";
        else if (c == '\t') escaped += "\\t";
        else if (c < 0x20) { const char* digits = "0123456789abcdef"; escaped += "\\u00"; escaped += digits[c >> 4]; escaped += digits[c & 0x0f]; }
        else escaped.push_back(static_cast<char>(c));
    }
    return escaped;
}
void write_structured_failure(std::string_view code, std::string_view stage, std::string_view message) {
    std::cerr << "{\"status\":\"failed\",\"code\":\"" << json_escape(code)
              << "\",\"stage\":\"" << json_escape(stage)
              << "\",\"message\":\"" << json_escape(message)
              << "\",\"disposition\":\"no_artifact\"}\n";
}
std::string read_path_or_stdin(const std::string& path) {
    std::ostringstream input;
    if (!path.empty()) { std::ifstream file(path); if (!file) throw std::runtime_error("cannot open artifact: " + path); input << file.rdbuf(); }
    else input << std::cin.rdbuf();
    return input.str();
}
flowcontracts::json::Value text(std::string value) { return flowcontracts::json::Value{std::move(value)}; }

int analyze(std::string_view input, const std::optional<flowcontracts::ProviderDecision>& decision) {
    using namespace flowcontracts;
    using namespace flowcontracts::json;
    const auto root = parse(input);
    const auto input_header = header(root);
    ExecutionPlan plan;
    std::string input_format;
    if (input_header.format == "flowparallel.execution_plan") {
        plan = execution_plan(root); input_format = input_header.format;
    } else if (input_header.format == "flowanalyst.semantic_report") {
        const auto semantic = semantic_report(root);
        plan.artifact = semantic.artifact; plan.source_path = semantic.source_path; plan.targets = semantic.targets;
        plan.external_operations = semantic.external_operations; plan.abi_type_contracts = semantic.abi_type_contracts;
        plan.aggregate_abi_layouts = semantic.aggregate_abi_layouts;
        plan.lowering_plan = semantic.lowering_plan; plan.dependency_matrix = semantic.dependency_matrix;
        if (optional(object(plan.lowering_plan), "source_graph")) throw Error("$.input", "native graph requires Flowparallel scheduling");
        input_format = input_header.format;
    } else throw Error("$.format", "input is not a supported semantic report or execution plan");
    const bool accepted = input_format == "flowanalyst.semantic_report" ? plan.artifact.status == "ok" : plan.artifact.status == "ready";
    if (!accepted) {
        std::cout << serialize(Object{{"format", text("flowoptimize.optimization_report")}, {"reason", text("input artifact is not accepted")},
                                     {"status", text("blocked")}, {"transforms", Array{}}, {"version", Integer{1}}}) << '\n';
        return 2;
    }
    if (decision && ((decision->provider != "cpu.reference" && decision->provider != "cuda.cublas.boolean_threshold") ||
                     (decision->representation != "sparse" && decision->representation != "dense")))
        throw Error("$.provider", "unsupported provider or representation pair");

    std::set<std::pair<Integer, Integer>> unique_entries;
    for (const auto& entry : plan.dependency_matrix.entries) unique_entries.emplace(entry.row, entry.column);
    const auto input_entries = static_cast<Integer>(plan.dependency_matrix.entries.size());
    const auto output_entries = static_cast<Integer>(unique_entries.size());
    const bool deduplicated = input_entries != output_entries;
    const auto provider = decision ? decision->provider : "";
    const auto representation = decision ? decision->representation : "";
    const auto reason = decision ? decision->reason : "";

    Object output{
        {"abi_type_contracts", plan.abi_type_contracts}, {"external_operations", plan.external_operations},
        {"aggregate_abi_layouts", plan.aggregate_abi_layouts},
        {"format", text("flowoptimize.optimization_report")},
        {"input", Object{{"format", text(input_format)}, {"version", Integer{1}}}}, {"lowering_plan", plan.lowering_plan},
        {"message", text("optimization boundary reached; derived matrix views are available; provider selection remains runtime policy")},
        {"projections", Array{Object{{"columns", plan.dependency_matrix.columns}, {"kind", text("graph_to_matrix")},
                                     {"name", text(plan.dependency_matrix.name)}, {"rows", plan.dependency_matrix.rows},
                                     {"semiring", text(plan.dependency_matrix.semiring)}, {"status", text("available")},
                                     {"storage", text(plan.dependency_matrix.storage)}}}},
        {"provider_policy", Object{
            {"candidates", Array{text("cpu"), text("cuda")}},
            {"cuda", Object{{"fallback", text("cpu")}, {"requires", Array{text("runtime capability"), text("measured cost benefit"), text("provider contract")}}}},
            {"decision", Object{{"provider", text(provider)}, {"reason", text(reason)}, {"representation", text(representation)},
                                {"status", text(decision ? "verified" : "deferred")}}}, {"selection", text("runtime")}
        }},
        {"source", Object{{"path", text(plan.source_path)}}},
        {"state", Object{{"canonical_graph", text("unchanged")}, {"decision_effect", text("advisory_policy_only")}, {"transformation", text("identity")}}},
        {"status", text("ready")}, {"targets", plan.targets},
        {"transforms", Array{Object{{"input_entries", input_entries}, {"kind", text("coo_deduplicate")},
                                    {"output_entries", output_entries}, {"proof", text("Boolean relation duplicate idempotence")},
                                    {"semantics_preserved", true}, {"status", text(deduplicated ? "applied" : "not-needed")}}}},
        {"version", Integer{1}}
    };
    if (!std::holds_alternative<std::nullptr_t>(plan.graph_schedule)) output.emplace("graph_schedule", plan.graph_schedule);
    std::cout << serialize(output) << '\n'; return 0;
}
} // namespace

int main(int argc, char** argv) {
    bool structured_diagnostics = false;
    try {
        if (argc == 2) {
            const std::string option = argv[1];
            if (option == "-h" || option == "--help" || option == "-?") { std::cout << "flowoptimize - typed optimization boundary\n\nUsage: flowoptimize [--provider-decision FILE] [artifact.json]\n"; return 0; }
            if (option == "-a" || option == "--about") { std::cout << "Flowoptimize consumes typed public artifact contracts and emits attributable transforms.\n"; return 0; }
            if (option == "-v" || option == "--version") { std::cout << VERSION << '\n'; return 0; }
        }
        const auto options = parse_options(argc, argv);
        structured_diagnostics = options.structured_diagnostics;
        std::optional<flowcontracts::ProviderDecision> decision;
        if (!options.provider_decision_path.empty()) decision = flowcontracts::provider_decision(flowcontracts::json::parse(read_path_or_stdin(options.provider_decision_path)));
        return analyze(read_path_or_stdin(options.input_path), decision);
    } catch (const flowcontracts::json::Error& error) {
        if (structured_diagnostics) write_structured_failure("FLOWOPTIMIZE_CONTRACT_FAILURE", "contract", error.what());
        else std::cerr << "flowoptimize contract error: " << error.what() << '\n';
        return 1;
    } catch (const std::exception& error) {
        if (structured_diagnostics) write_structured_failure("FLOWOPTIMIZE_FAILURE", "cli", error.what());
        else std::cerr << "flowoptimize error: " << error.what() << '\n';
        return 1;
    }
}
