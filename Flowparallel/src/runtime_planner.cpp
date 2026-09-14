#include <flowcontracts/artifacts.hpp>
#include <flowparallel/bounded_input.hpp>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <new>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
constexpr std::string_view version = "0.1.0";

struct Options {
    std::string plan_path;
    std::string capabilities_path;
    std::string calibration_path;
    double minimum_speedup = 1.25;
    bool structured_diagnostics = false;
};

std::string read_file(const std::string& path, const char* label) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error(std::string("cannot open ") + label);
    return flowparallel::read_bounded(file, label);
}

std::string quote(std::string_view value) {
    std::string result = "\"";
    for (char character : value) {
        if (character == '\\' || character == '"') result.push_back('\\');
        result.push_back(character);
    }
    result.push_back('"');
    return result;
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

double parse_number(std::string_view text, const char* option) {
    std::size_t consumed = 0;
    double value = 0.0;
    try { value = std::stod(std::string(text), &consumed); }
    catch (...) { throw std::runtime_error(std::string(option) + " requires a complete number"); }
    if (consumed != text.size() || !std::isfinite(value)) throw std::runtime_error(std::string(option) + " requires a complete finite number");
    return value;
}

int reject_unsupported(std::string_view request, std::string_view reason) {
    std::cout << "{\n  \"format\": \"flowparallel.provider_decision\",\n"
                 "  \"version\": 1,\n  \"status\": \"unsupported\",\n"
                 "  \"request\": " << quote(request) << ",\n"
                 "  \"reason\": " << quote(reason) << ",\n"
                 "  \"fallback\": {\"emitted\": false}\n}\n";
    return 2;
}

Options parse(int argc, char** argv) {
    Options options;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        auto value = [&](const char* name) -> std::string {
            if (++index >= argc) throw std::runtime_error(std::string(name) + " requires a value");
            return argv[index];
        };
        if (argument == "--plan") options.plan_path = value("--plan");
        else if (argument == "--capabilities") options.capabilities_path = value("--capabilities");
        else if (argument == "--calibration") options.calibration_path = value("--calibration");
        else if (argument == "--min-speedup") options.minimum_speedup = parse_number(value("--min-speedup"), "--min-speedup");
        else if (argument == "--diagnostics") { if (value("--diagnostics") != "json") throw std::runtime_error("--diagnostics requires json"); options.structured_diagnostics = true; }
        else if (argument == "-h" || argument == "-?" || argument == "--help") {
            std::cout << "flowparallel_runtime_planner - explainable CPU/CUDA provider decision\n\n"
                         "Options: --plan plan.json --capabilities capabilities.json\n"
                         "         [--calibration benchmark.json] [--min-speedup N] [--diagnostics json]\n"
                         "         -h, -?, --help  show help\n"
                         "         -a, --about    show about information\n"
                         "         -v, --version  print the raw version number\n";
            std::exit(0);
        } else if (argument == "-a" || argument == "--about") {
            std::cout << "Flowparallel resolves a runtime CPU/CUDA choice from plan, facts, policy, and calibration.\n";
            std::exit(0);
        } else if (argument == "-v" || argument == "--version") {
            std::cout << version << '\n';
            std::exit(0);
        } else throw std::runtime_error("unknown option '" + argument + "'");
    }
    if (options.plan_path.empty() || options.capabilities_path.empty()) throw std::runtime_error("--plan and --capabilities are required");
    if (options.minimum_speedup <= 0.0) throw std::runtime_error("--min-speedup must be positive");
    return options;
}

int run(const Options& options) {
#ifdef FLOWPARALLEL_RUNTIME_PLANNER_TEST_ALLOCATION_FAILURE
    (void)options;
    throw std::bad_alloc();
#endif
    const auto plan = read_file(options.plan_path, "execution plan");
    const auto capabilities = read_file(options.capabilities_path, "runtime capabilities");
    const auto calibration = options.calibration_path.empty() ? std::string{} : read_file(options.calibration_path, "calibration report");
    const auto plan_value = flowcontracts::json::parse(plan);
    const auto& plan_root = flowcontracts::json::object(plan_value);
    const auto requested = [&](std::string_view field, std::string_view value) {
        const auto* item = flowcontracts::json::optional(plan_root, field);
        return item != nullptr && flowcontracts::json::string(*item, "$." + std::string(field)) == value;
    };
    if (requested("schedule_policy", "parallel_effectful_v1"))
        return reject_unsupported("parallel_effectful_v1", "effectful parallel scheduling is not admitted");
    if (requested("cancellation", "requested"))
        return reject_unsupported("cancellation", "cancellation is not admitted by this provider");
    if (requested("async", "requested"))
        return reject_unsupported("async", "asynchronous execution is not admitted by this provider");
    if (requested("backpressure", "requested"))
        return reject_unsupported("backpressure", "backpressure is not admitted by this provider");
    const auto execution = flowcontracts::execution_plan(plan_value);
    if (execution.artifact.status != "ready") {
        std::cout << "{\n  \"format\": \"flowparallel.provider_decision\",\n  \"version\": 1,\n  \"status\": \"blocked\",\n  \"reason\": \"execution plan is not ready\"\n}\n";
        return 2;
    }
    const auto capability_value = flowcontracts::json::parse(capabilities);
    const auto& capability_root = flowcontracts::json::object(capability_value);
    if (flowcontracts::json::string(flowcontracts::json::required(capability_root, "format"), "$.format") != "frankencore.runtime_capabilities") throw flowcontracts::json::Error("$.format", "unsupported runtime capability format");
    if (flowcontracts::json::integer(flowcontracts::json::required(capability_root, "version"), "$.version") != 1) throw flowcontracts::json::Error("$.version", "unsupported runtime capability version");
    const auto& cuda = flowcontracts::required_object(capability_root, "cuda");
    const auto devices = flowcontracts::json::integer(flowcontracts::json::required(cuda, "device_count", "$.cuda"), "$.cuda.device_count");
    const bool cuda_available = flowcontracts::json::string(flowcontracts::json::required(cuda, "status", "$.cuda"), "$.cuda.status") == "available" && devices >= 1;
    bool calibration_verified = false; double measured_speedup = 0.0;
    if (!calibration.empty()) {
        const auto calibration_value = flowcontracts::json::parse(calibration);
        const auto& calibration_root = flowcontracts::json::object(calibration_value);
        const auto format = flowcontracts::json::string(flowcontracts::json::required(calibration_root, "format"), "$.format");
        if (format != "flowparallel.matrix_benchmark" && format != "flowparallel.graph_cuda") throw flowcontracts::json::Error("$.format", "unsupported calibration format");
        const auto calibration_header = flowcontracts::require_header(calibration_value, format, 1);
        if (const auto* speedup = flowcontracts::json::optional(calibration_root, "end_to_end_speedup")) {
            measured_speedup = flowcontracts::json::number(*speedup, "$.end_to_end_speedup");
            calibration_verified = calibration_header.status == "verified";
        }
    }
    const bool cost_beneficial = calibration_verified && measured_speedup >= options.minimum_speedup;
    const bool cuda_selected = cuda_available && cost_beneficial;
    std::string reason;
    if (cuda_selected) reason = "CUDA available and calibrated end-to-end benefit meets policy";
    else if (!cuda_available) reason = "CUDA is unavailable or reported no device";
    else if (!calibration_verified) reason = "CUDA availability exists but verified calibration is missing";
    else reason = "measured CUDA benefit is below the active minimum-speedup policy";

    std::cout << std::setprecision(10)
              << "{\n"
              << "  \"format\": \"flowparallel.provider_decision\",\n"
              << "  \"version\": 1,\n"
              << "  \"status\": \"selected\",\n"
              << "  \"plan\": {\"format\": \"flowparallel.execution_plan\", \"version\": 1},\n"
              << "  \"policy\": {\"minimum_speedup\": " << options.minimum_speedup << "},\n"
              << "  \"evidence\": {\"cuda_available\": " << (cuda_available ? "true" : "false")
              << ", \"calibration_verified\": " << (calibration_verified ? "true" : "false")
              << ", \"measured_end_to_end_speedup\": " << measured_speedup << "},\n"
              << "  \"selection\": {\"provider\": " << quote(cuda_selected ? "cuda.cublas" : "cpu.serial")
              << ", \"reason\": " << quote(reason) << "},\n"
              << "  \"fallback\": {\"provider\": \"cpu.serial\", \"required\": true}\n"
              << "}\n";
    return 0;
}
}

int main(int argc, char** argv) {
    bool structured_diagnostics = false;
    for (int index = 1; index + 1 < argc; ++index)
        if (std::string(argv[index]) == "--diagnostics" && std::string(argv[index + 1]) == "json") structured_diagnostics = true;
    try {
        const auto options = parse(argc, argv);
        return run(options);
    } catch (const std::bad_alloc&) {
        if (structured_diagnostics)
            std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWPARALLEL_RUNTIME_PLANNER_RESOURCE_EXHAUSTED\",\"stage\":\"runtime\",\"message\":\"allocation failed\",\"disposition\":\"no_artifact\"}\n";
        else std::cerr << "flowparallel_runtime_planner error: allocation failed\n";
        return 1;
    } catch (const std::exception& error) {
        if (structured_diagnostics)
            std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWPARALLEL_RUNTIME_PLANNER_FAILURE\",\"message\":\"" << json_escape(error.what()) << "\",\"disposition\":\"no_artifact\"}\n";
        else std::cerr << "flowparallel_runtime_planner error: " << error.what() << '\n';
        return 1;
    } catch (...) {
        if (structured_diagnostics)
            std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWPARALLEL_RUNTIME_PLANNER_UNKNOWN_FAILURE\",\"message\":\"unknown non-standard failure\",\"disposition\":\"no_artifact\"}\n";
        else std::cerr << "flowparallel_runtime_planner error: unknown non-standard failure\n";
        return 1;
    }
}
