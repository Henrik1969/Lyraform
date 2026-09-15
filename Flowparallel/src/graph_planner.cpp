#include <flowcontracts/artifacts.hpp>
#include <flowparallel/bounded_input.hpp>
#include <flowparallel/diagnostics.hpp>

#include <cmath>
#include <fstream>
#include <iostream>
#include <new>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
constexpr std::string_view version = "0.1.0";
std::string read_file(const std::string& path) { std::ifstream file(path); if (!file) throw std::runtime_error("cannot open " + path); return flowparallel::read_bounded(file, "graph planner input"); }
std::string quote(std::string_view value) { std::string result = "\""; for (const char c : value) { if (c == '\\' || c == '"') result.push_back('\\'); result.push_back(c); } result.push_back('"'); return result; }
struct JsonEscaped { std::string_view value; };
JsonEscaped json_escape(std::string_view value) { return {value}; }
std::ostream& operator<<(std::ostream& output, const JsonEscaped escaped) {
    flowparallel::write_json_string(stderr, escaped.value);
    return output;
}
double parse_number(std::string_view text, const char* option) { std::size_t consumed = 0; double value = 0.0; try { value = std::stod(std::string(text), &consumed); } catch (...) { throw std::runtime_error(std::string(option) + " requires a complete finite number"); } if (consumed != text.size() || !std::isfinite(value)) throw std::runtime_error(std::string(option) + " requires a complete finite number"); return value; }
struct Options { std::string graph, capabilities, calibration; double density = 0.25; double min_speedup = 1.25; bool structured_diagnostics = false; };
Options parse(int argc, char** argv) { Options o; for (int i = 1; i < argc; ++i) { const std::string arg = argv[i]; auto req = [&](const char* name) { if (++i >= argc) throw std::runtime_error(std::string(name) + " requires a value"); return std::string(argv[i]); }; if (arg == "--graph") o.graph = req("--graph"); else if (arg == "--capabilities") o.capabilities = req("--capabilities"); else if (arg == "--calibration") o.calibration = req("--calibration"); else if (arg == "--density-threshold") o.density = parse_number(req("--density-threshold"), "--density-threshold"); else if (arg == "--min-speedup") o.min_speedup = parse_number(req("--min-speedup"), "--min-speedup"); else if (arg == "--diagnostics") { if (req("--diagnostics") != "json") throw std::runtime_error("--diagnostics requires json"); o.structured_diagnostics = true; } else throw std::runtime_error("unknown option: " + arg); } if (o.graph.empty() || o.capabilities.empty()) throw std::runtime_error("--graph and --capabilities are required"); if (o.density < 0.0 || o.density > 1.0 || o.min_speedup <= 0.0) throw std::runtime_error("invalid policy threshold"); return o; }

int run(const Options& o) {
#ifdef FLOWPARALLEL_GRAPH_PLANNER_TEST_ALLOCATION_FAILURE
    (void)o;
    throw std::bad_alloc();
#endif
    const auto semantic = flowcontracts::semantic_report(flowcontracts::json::parse(read_file(o.graph)));
    if (semantic.artifact.status != "ok") throw std::runtime_error("graph is not an accepted semantic report");
    const auto rows = semantic.dependency_matrix.rows, columns = semantic.dependency_matrix.columns;
    if (rows == 0 || rows != columns) throw std::runtime_error("unsupported graph dimensions");
    const auto edges = semantic.dependency_matrix.entries.size();
    const double density = static_cast<double>(edges) / static_cast<double>(rows * columns);

    const auto capabilities_value = flowcontracts::json::parse(read_file(o.capabilities));
    const auto& capabilities = flowcontracts::json::object(capabilities_value);
    const auto capability_format = flowcontracts::json::string(flowcontracts::json::required(capabilities, "format"), "$.format");
    if (flowcontracts::json::integer(flowcontracts::json::required(capabilities, "version"), "$.version") != 1) throw flowcontracts::json::Error("$.version", "unsupported runtime capability version");
    bool cuda = false;
    if (capability_format == "flowcore.runtime_capabilities") {
        cuda = flowcontracts::json::string(flowcontracts::json::required(capabilities, "status"), "$.status") == "available" && flowcontracts::json::integer(flowcontracts::json::required(capabilities, "device_count"), "$.device_count") > 0;
    } else if (capability_format == "frankencore.runtime_capabilities") {
        const auto& cuda_capability = flowcontracts::required_object(capabilities, "cuda");
        cuda = flowcontracts::json::string(flowcontracts::json::required(cuda_capability, "status", "$.cuda"), "$.cuda.status") == "available" && flowcontracts::json::integer(flowcontracts::json::required(cuda_capability, "device_count", "$.cuda"), "$.cuda.device_count") > 0;
    } else throw flowcontracts::json::Error("$.format", "unsupported runtime capability format");

    bool calibrated = false; double speedup = 0.0;
    if (!o.calibration.empty()) {
        const auto calibration_value = flowcontracts::json::parse(read_file(o.calibration));
        const auto& calibration = flowcontracts::json::object(calibration_value);
        const auto format = flowcontracts::json::string(flowcontracts::json::required(calibration, "format"), "$.format");
        const auto header = flowcontracts::require_header(calibration_value, format, 1);
        if (format != "flowparallel.matrix_benchmark" && format != "flowparallel.graph_cuda") throw flowcontracts::json::Error("$.format", "unsupported calibration format");
        calibrated = header.status == "verified";
        if (const auto* measured = flowcontracts::json::optional(calibration, "end_to_end_speedup")) speedup = flowcontracts::json::number(*measured, "$.end_to_end_speedup");
    }
    const bool dense = density >= o.density;
    const bool select_cuda = dense && cuda && calibrated && speedup >= o.min_speedup;
    const std::string provider = select_cuda ? "cuda.cublas.boolean_threshold" : "cpu.reference";
    const std::string reason = select_cuda ? "dense graph, CUDA available, and verified calibration exceeds threshold" : !dense ? "sparse graph below density threshold; retain CPU reference provider" : !cuda ? "CUDA is unavailable; retain required CPU fallback" : !calibrated ? "no verified CUDA calibration; retain required CPU fallback" : "verified CUDA calibration is below configured speedup threshold";
    std::cout << "{\n  \"format\": \"flowparallel.graph_provider_decision\",\n  \"version\": 1,\n  \"status\": \"verified\",\n  \"policy\": {\"density_threshold\": " << o.density << ", \"minimum_end_to_end_speedup\": " << o.min_speedup << "},\n  \"graph\": {\"rows\": " << rows << ", \"columns\": " << columns << ", \"edges\": " << edges << ", \"density\": " << density << "},\n  \"provider\": " << quote(provider) << ",\n  \"representation\": " << quote(dense ? "dense" : "sparse") << ",\n  \"reason\": " << quote(reason) << ",\n  \"fallback\": \"cpu.reference\"\n}\n";
    return 0;
}
}
int main(int argc, char** argv) { bool structured_diagnostics = false; for (int i = 1; i < argc; ++i) if (std::string(argv[i]) == "--diagnostics" && i + 1 < argc && std::string(argv[i + 1]) == "json") structured_diagnostics = true; try { if (argc == 2 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "-?" || std::string(argv[1]) == "--help")) { std::cout << "flowparallel_graph_planner - choose graph representation and provider\n"; return 0; } if (argc == 2 && (std::string(argv[1]) == "-a" || std::string(argv[1]) == "--about")) { std::cout << "Flowparallel applies explicit sparse/dense and runtime-provider policy to graph projections.\n"; return 0; } if (argc == 2 && (std::string(argv[1]) == "-v" || std::string(argv[1]) == "--version")) { std::cout << version << '\n'; return 0; } const auto options = parse(argc, argv); structured_diagnostics = options.structured_diagnostics; return run(options); } catch (const std::bad_alloc&) { if (structured_diagnostics) std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWPARALLEL_GRAPH_PLANNER_RESOURCE_EXHAUSTED\",\"message\":\"allocation failed\",\"disposition\":\"no_artifact\"}\n"; else std::cerr << "flowparallel_graph_planner error: allocation failed\n"; return 1; } catch (const std::exception& error) { if (structured_diagnostics) std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWPARALLEL_GRAPH_PLANNER_FAILURE\",\"message\":\"" << json_escape(error.what()) << "\",\"disposition\":\"no_artifact\"}\n"; else std::cerr << "flowparallel_graph_planner error: " << error.what() << '\n'; return 1; } catch (...) { if (structured_diagnostics) std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWPARALLEL_GRAPH_PLANNER_UNKNOWN_FAILURE\",\"message\":\"unknown non-standard failure\",\"disposition\":\"no_artifact\"}\n"; else std::cerr << "flowparallel_graph_planner error: unknown non-standard failure\n"; return 1; } }
