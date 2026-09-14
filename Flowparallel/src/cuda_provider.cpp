#include <dlfcn.h>
#include <flowcontracts/artifacts.hpp>
#include <flowcontracts/json.hpp>

#include <charconv>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

struct Options { std::string plan_path; unsigned matrix_size = 512; bool structured_diagnostics = false; };
constexpr unsigned MAX_MATRIX_SIZE = 4096;

std::string quote(std::string_view value) { return "\"" + std::string(value) + "\""; }

unsigned parse_unsigned(std::string_view text, const char* option) {
    unsigned value = 0;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size())
        throw std::runtime_error(std::string(option) + " requires a complete non-negative integer");
    return value;
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

int reject_unsupported(std::string_view request, std::string_view reason) {
    std::cout << "{\n  \"format\": \"flowparallel.cuda_selection\",\n"
                 "  \"version\": 1,\n  \"status\": \"unsupported\",\n"
                 "  \"request\": " << quote(request) << ",\n"
                 "  \"reason\": " << quote(reason) << ",\n"
                 "  \"fallback\": {\"emitted\": false}\n}\n";
    return 2;
}

std::string input(const Options& options) {
    std::ostringstream stream;
    if (!options.plan_path.empty()) { std::ifstream file(options.plan_path); if (!file) throw std::runtime_error("cannot open execution plan"); stream << file.rdbuf(); }
    else stream << std::cin.rdbuf();
    return stream.str();
}

struct CudaProbe { std::string status = "unknown"; std::string diagnostic; unsigned devices = 0; };

CudaProbe probe() {
    CudaProbe result;
    void* library = dlopen("libcuda.so.1", RTLD_LAZY | RTLD_LOCAL);
    if (!library) { result.status = "unavailable"; result.diagnostic = "libcuda.so.1 was not available"; return result; }
    using init_fn = int (*)(unsigned int);
    using count_fn = int (*)(int*);
    const auto init = reinterpret_cast<init_fn>(dlsym(library, "cuInit"));
    const auto count = reinterpret_cast<count_fn>(dlsym(library, "cuDeviceGetCount"));
    if (!init || !count) { result.diagnostic = "CUDA driver symbols were incomplete"; dlclose(library); return result; }
    if (init(0) != 0) { result.diagnostic = "CUDA driver initialization failed"; dlclose(library); return result; }
    int devices = 0;
    if (count(&devices) != 0) { result.diagnostic = "CUDA device enumeration failed"; dlclose(library); return result; }
    result.devices = devices > 0 ? static_cast<unsigned>(devices) : 0;
    result.status = devices > 0 ? "available" : "unavailable";
    result.diagnostic = devices > 0 ? "CUDA device discovered; kernel execution remains provider-deferred" : "CUDA driver loaded but no devices were reported";
    dlclose(library);
    return result;
}

Options parse(int argc, char** argv) {
    Options options;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--plan") { if (++index >= argc) throw std::runtime_error("--plan requires a value"); options.plan_path = argv[index]; }
        else if (argument == "--matrix-size") { if (++index >= argc) throw std::runtime_error("--matrix-size requires a value"); options.matrix_size = parse_unsigned(argv[index], "--matrix-size"); }
        else if (argument == "--diagnostics") { if (++index >= argc || std::string(argv[index]) != "json") throw std::runtime_error("--diagnostics requires json"); options.structured_diagnostics = true; }
        else if (argument == "-h" || argument == "--help" || argument == "-?") { std::cout << "flowparallel_cuda - optional CUDA provider probe\n\nOptions: --plan plan.json --matrix-size N --diagnostics json\n         -h, -?, --help  show help\n         -a, --about    show about information\n         -v, --version  print the raw version number\n"; std::exit(0); }
        else if (argument == "-a" || argument == "--about") { std::cout << "Flowparallel CUDA probes linear-algebra provider availability and preserves CPU fallback.\n"; std::exit(0); }
        else if (argument == "-v" || argument == "--version") { std::cout << "0.1.0\n"; std::exit(0); }
        else throw std::runtime_error("unknown option '" + argument + "'");
    }
    if (options.matrix_size == 0 || options.matrix_size > MAX_MATRIX_SIZE)
        throw std::runtime_error("--matrix-size must be between 1 and 4096");
    return options;
}

int run(const std::string& plan, const Options& options) {
    if (options.matrix_size == 0 || options.matrix_size > MAX_MATRIX_SIZE)
        throw std::runtime_error("--matrix-size must be between 1 and 4096");
    const auto plan_value = flowcontracts::json::parse(plan);
    const auto& root = flowcontracts::json::object(plan_value);
    const auto requested = [&](std::string_view field, std::string_view value) {
        const auto* item = flowcontracts::json::optional(root, field);
        return item != nullptr && flowcontracts::json::string(*item, "$." + std::string(field)) == value;
    };
    if (requested("schedule_policy", "parallel_effectful_v1"))
        return reject_unsupported("parallel_effectful_v1", "effectful parallel scheduling is not admitted by this provider");
    if (requested("cancellation", "requested"))
        return reject_unsupported("cancellation", "cancellation is not admitted by this provider");
    if (requested("async", "requested"))
        return reject_unsupported("async", "asynchronous execution is not admitted by this provider");
    if (requested("backpressure", "requested"))
        return reject_unsupported("backpressure", "backpressure is not admitted by this provider");
    const auto artifact = flowcontracts::execution_plan(plan_value);
    if (artifact.artifact.status != "ready") { std::cout << "{\n  \"format\": \"flowparallel.cuda_selection\",\n  \"version\": 1,\n  \"status\": \"blocked\",\n  \"reason\": \"execution plan is not ready\"\n}\n"; return 2; }
    const auto cuda = probe();
    const std::uint64_t matrix_elements = static_cast<std::uint64_t>(options.matrix_size) * options.matrix_size;
    const std::uint64_t matrix_bytes = matrix_elements * sizeof(float);
    std::cout << "{\n  \"format\": \"flowparallel.cuda_selection\",\n"
                 "  \"version\": 1,\n  \"status\": \"ready\",\n"
                 "  \"provider\": \"cuda\",\n"
                 "  \"cuda\": {\"status\": " << quote(cuda.status) << ", \"devices\": " << cuda.devices << ", \"diagnostic\": " << quote(cuda.diagnostic) << "},\n"
                 "  \"workload\": {\"operation\": \"matrix_multiply\", \"matrix_size\": " << options.matrix_size << ", \"matrix_bytes_per_operand\": " << matrix_bytes << "},\n"
                 "  \"transfer_cost\": {\"host_to_device\": \"included\", \"device_to_host\": \"included\", \"calibration\": \"runtime\"},\n"
                 "  \"execution\": \"not-performed\",\n"
                 "  \"fallback\": {\"required\": true, \"provider\": \"cpu.serial\"}\n}\n";
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    bool structured_diagnostics = false;
    for (int index = 1; index < argc; ++index)
        if (std::string(argv[index]) == "--diagnostics" && index + 1 < argc && std::string(argv[index + 1]) == "json")
            structured_diagnostics = true;
    try {
        const auto options = parse(argc, argv);
        structured_diagnostics = options.structured_diagnostics;
        return run(input(options), options);
    } catch (const std::exception& error) {
        if (structured_diagnostics)
            std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWPARALLEL_CUDA_FAILURE\",\"message\":\"" << json_escape(error.what()) << "\",\"disposition\":\"no_artifact\"}\n";
        else std::cerr << "flowparallel_cuda error: " << error.what() << '\n';
        return 1;
    } catch (...) {
        if (structured_diagnostics)
            std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWPARALLEL_CUDA_UNKNOWN_FAILURE\",\"message\":\"unknown non-standard failure\",\"disposition\":\"no_artifact\"}\n";
        else std::cerr << "flowparallel_cuda error: unknown non-standard failure\n";
        return 1;
    }
}
