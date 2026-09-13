#include <flowcontracts/artifacts.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
constexpr std::string_view VERSION = "0.1.0";

std::string read_input(int argc, char** argv) {
    if (argc > 2) throw std::runtime_error("usage: flowparallel [semantic-report.json]");
    std::ostringstream input;
    if (argc == 2) { std::ifstream file(argv[1]); if (!file) throw std::runtime_error("cannot open semantic report"); input << file.rdbuf(); }
    else input << std::cin.rdbuf();
    return input.str();
}

flowcontracts::json::Value text(std::string value) { return flowcontracts::json::Value{std::move(value)}; }

int analyze(std::string_view input) {
    using namespace flowcontracts;
    using namespace flowcontracts::json;
    const auto report = semantic_report(parse(input));
    if (report.artifact.status != "ok") {
        std::cout << serialize(Object{{"format", text("flowparallel.execution_plan")},
                                     {"reason", text("semantic report is not accepted")},
                                     {"status", text("blocked")}, {"version", Integer{1}}}) << '\n';
        return 2;
    }
    const auto& matrix = report.dependency_matrix;
    Object output{
        {"abi_type_contracts", report.abi_type_contracts},
        {"aggregate_abi_layouts", report.aggregate_abi_layouts},
        {"cost_model", Object{{"calibration", text("runtime")}, {"minimum_duration_ns", text("policy")},
                              {"minimum_speedup", 1.25}, {"status", text("deferred")}, {"work_units", text("runtime")}}},
        {"dependency_analysis", Object{{"candidate_kind", text("pure-callee-disjoint-inputs")},
                                       {"parallel_candidates", Integer{static_cast<Integer>(report.independent_candidate_count)}},
                                       {"pure_callables", Integer{static_cast<Integer>(report.proven_pure_count)}}, {"status", text("available")}}},
        {"external_operations", report.external_operations},
        {"fallback", Object{{"provider", text("cpu.serial")}, {"required", true}}},
        {"format", text("flowparallel.execution_plan")},
        {"graph_projection", Object{{"columns", matrix.columns}, {"entries", matrix_entries(matrix)},
                                    {"kind", text("graph_to_matrix")}, {"name", text(matrix.name)}, {"rows", matrix.rows},
                                    {"semiring", text(matrix.semiring)}, {"status", text("available")}, {"storage", text(matrix.storage)}}},
        {"input", Object{{"format", text("flowanalyst.semantic_report")}, {"version", Integer{1}}}},
        {"lowering_plan", report.lowering_plan},
        {"message", text("parallel execution is policy- and runtime-deferred; no unsafe candidates emitted")},
        {"provider_selection", Object{{"policy", text("runtime")}, {"status", text("deferred")}}},
        {"runtime", Object{{"capabilities_format", text("frankencore.runtime_capabilities")}, {"required", true}}},
        {"source", Object{{"path", text(report.source_path)}}}, {"status", text("ready")},
        {"targets", report.targets}, {"version", Integer{1}}
    };
    if (const auto* graph = optional(object(report.lowering_plan), "source_graph"))
        output.emplace("graph_schedule", graph_schedule(*graph));
    std::cout << serialize(output) << '\n';
    return 0;
}
} // namespace

int main(int argc, char** argv) {
    try {
        if (argc == 2) {
            const std::string option = argv[1];
            if (option == "-h" || option == "--help" || option == "-?") { std::cout << "flowparallel - runtime-deferred parallel execution planning\n\nUsage: flowparallel [semantic-report.json]\n       flowmini ... | flowanalyst | flowparallel\n\nOptions: -h, -?, --help  show help\n         -a, --about    show about information\n         -v, --version  print the raw version number\n"; return 0; }
            if (option == "-a" || option == "--about") { std::cout << "Flowparallel derives a conservative, inspectable execution plan after semantic analysis.\n"; return 0; }
            if (option == "-v" || option == "--version") { std::cout << VERSION << '\n'; return 0; }
        }
        return analyze(read_input(argc, argv));
    } catch (const flowcontracts::json::Error& error) {
        std::cerr << "flowparallel contract error: " << error.what() << '\n'; return 1;
    } catch (const std::exception& error) { std::cerr << "flowparallel error: " << error.what() << '\n'; return 1; }
}
