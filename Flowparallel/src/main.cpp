#include <flowcontracts/artifacts.hpp>
#include <flowparallel/bounded_input.hpp>

#include <fstream>
#include <iostream>
#include <new>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
constexpr std::string_view VERSION = "0.1.0";

std::string read_input(int argc, char** argv) {
    if (argc > 2 && !(argc == 3 && std::string_view(argv[1]) == "--diagnostics" && std::string_view(argv[2]) == "json"))
        throw std::runtime_error("usage: flowparallel [semantic-report.json]");
    if (argc == 2) { std::ifstream file(argv[1]); if (!file) throw std::runtime_error("cannot open semantic report"); return flowparallel::read_bounded(file, "semantic report"); }
    return flowparallel::read_bounded(std::cin, "semantic report");
}

std::string json_escape(std::string_view value) {
    std::string escaped;
    for (const char character : value) {
        if (character == '\\' || character == '"') escaped.push_back('\\');
        if (character == '\n') escaped += "\\n";
        else if (character == '\r') escaped += "\\r";
        else if (character == '\t') escaped += "\\t";
        else escaped.push_back(character);
    }
    return escaped;
}

flowcontracts::json::Value text(std::string value) { return flowcontracts::json::Value{std::move(value)}; }

int reject_unsupported(std::string_view request, std::string_view reason) {
    std::cout << "{\n  \"format\": \"flowparallel.execution_plan\",\n"
                 "  \"version\": 1,\n  \"status\": \"unsupported\",\n"
                 "  \"request\": \"" << json_escape(request) << "\",\n"
                 "  \"reason\": \"" << json_escape(reason) << "\",\n"
                 "  \"fallback\": {\"emitted\": false}\n}\n";
    return 2;
}

int analyze(std::string_view input) {
#ifdef FLOWPARALLEL_TEST_ALLOCATION_FAILURE
    (void)input;
    throw std::bad_alloc();
#endif
    using namespace flowcontracts;
    using namespace flowcontracts::json;
    const auto parsed = parse(input);
    const auto& parsed_root = object(parsed);
    const auto requested = [&](std::string_view field, std::string_view value) {
        const auto* item = optional(parsed_root, field);
        return item != nullptr && string(*item, "$." + std::string(field)) == value;
    };
    if (requested("schedule_policy", "parallel_effectful_v1"))
        return reject_unsupported("parallel_effectful_v1", "effectful parallel scheduling is not admitted");
    if (requested("cancellation", "requested"))
        return reject_unsupported("cancellation", "cancellation is not admitted by this planner");
    if (requested("async", "requested"))
        return reject_unsupported("async", "asynchronous execution is not admitted by this planner");
    if (requested("backpressure", "requested"))
        return reject_unsupported("backpressure", "backpressure is not admitted by this planner");
    const auto report = semantic_report(parsed);
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
    bool structured_diagnostics = argc == 3 && std::string_view(argv[1]) == "--diagnostics" && std::string_view(argv[2]) == "json";
    try {
        if (argc == 2) {
            const std::string option = argv[1];
            if (option == "-h" || option == "--help" || option == "-?") { std::cout << "flowparallel - runtime-deferred parallel execution planning\n\nUsage: flowparallel [semantic-report.json]\n       flowmini ... | flowanalyst | flowparallel\n\nOptions: -h, -?, --help  show help\n         -a, --about    show about information\n         -v, --version  print the raw version number\n"; return 0; }
            if (option == "-a" || option == "--about") { std::cout << "Flowparallel derives a conservative, inspectable execution plan after semantic analysis.\n"; return 0; }
            if (option == "-v" || option == "--version") { std::cout << VERSION << '\n'; return 0; }
        }
        return analyze(read_input(argc, argv));
    } catch (const std::bad_alloc&) {
        if (structured_diagnostics)
            std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWPARALLEL_RESOURCE_EXHAUSTED\",\"stage\":\"runtime\",\"message\":\"allocation failed\",\"disposition\":\"no_artifact\"}\n";
        else std::cerr << "flowparallel error: allocation failed\n";
        return 1;
    } catch (const flowcontracts::json::Error& error) {
        if (structured_diagnostics)
            std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWPARALLEL_CONTRACT_FAILURE\",\"message\":\"" << json_escape(error.what()) << "\",\"disposition\":\"no_artifact\"}\n";
        else std::cerr << "flowparallel contract error: " << error.what() << '\n';
        return 1;
    } catch (const std::exception& error) {
        if (structured_diagnostics)
            std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWPARALLEL_FAILURE\",\"message\":\"" << json_escape(error.what()) << "\",\"disposition\":\"no_artifact\"}\n";
        else std::cerr << "flowparallel error: " << error.what() << '\n';
        return 1;
    } catch (...) {
        if (structured_diagnostics)
            std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWPARALLEL_UNKNOWN_FAILURE\",\"message\":\"unknown non-standard failure\",\"disposition\":\"no_artifact\"}\n";
        else std::cerr << "flowparallel error: unknown non-standard failure\n";
        return 1;
    }
}
