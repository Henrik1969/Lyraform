#include <dlfcn.h>
#include <flowcontracts/artifacts.hpp>
#include <flowcontracts/json.hpp>
#include <flowparallel/bounded_input.hpp>
#include <flowparallel/dynamic_library.hpp>
#include <flowparallel/diagnostics.hpp>

#include <charconv>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <new>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

struct Options { std::string plan_path; unsigned matrix_size = 512; bool structured_diagnostics = false; };
constexpr unsigned MAX_MATRIX_SIZE = 4096;

void write_structured_failure(std::string_view code, std::string_view stage, std::string_view message) noexcept {
    std::fputs("{\"status\":\"failed\",\"code\":\"", stderr);
    flowparallel::write_json_string(stderr, code);
    std::fputs("\",\"stage\":\"", stderr);
    flowparallel::write_json_string(stderr, stage);
    std::fputs("\",\"message\":\"", stderr);
    flowparallel::write_json_string(stderr, message);
    std::fputs("\",\"disposition\":\"no_artifact\"}\n", stderr);
}

std::string quote(std::string_view value) { return "\"" + std::string(value) + "\""; }

unsigned parse_unsigned(std::string_view text, const char* option) {
    unsigned value = 0;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size())
        throw std::runtime_error(std::string(option) + " requires a complete non-negative integer");
    return value;
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
    if (!options.plan_path.empty()) { std::ifstream file(options.plan_path); if (!file) throw std::runtime_error("cannot open execution plan"); return flowparallel::read_bounded(file, "execution plan"); }
    return flowparallel::read_bounded(std::cin, "execution plan");
}

struct CudaProbe { std::string status = "unknown"; std::string diagnostic; unsigned devices = 0; };

CudaProbe probe() {
    CudaProbe result;
    void* raw_library = dlopen("libcuda.so.1", RTLD_LAZY | RTLD_LOCAL);
    if (!raw_library) { result.status = "unavailable"; result.diagnostic = "libcuda.so.1 was not available"; return result; }
    flowparallel::DynamicLibrary library{raw_library, ::dlclose};
    const auto finish = [&] {
        if (library.close() != 0) {
            result.status = "unknown";
            result.diagnostic = "CUDA driver library cleanup failed";
        }
        return result;
    };
    using init_fn = int (*)(unsigned int);
    using count_fn = int (*)(int*);
    const auto init = reinterpret_cast<init_fn>(dlsym(library.handle(), "cuInit"));
    const auto count = reinterpret_cast<count_fn>(dlsym(library.handle(), "cuDeviceGetCount"));
    if (!init || !count) { result.diagnostic = "CUDA driver symbols were incomplete"; return finish(); }
    if (init(0) != 0) { result.diagnostic = "CUDA driver initialization failed"; return finish(); }
    int devices = 0;
    if (count(&devices) != 0) { result.diagnostic = "CUDA device enumeration failed"; return finish(); }
    result.devices = devices > 0 ? static_cast<unsigned>(devices) : 0;
    result.status = devices > 0 ? "available" : "unavailable";
    result.diagnostic = devices > 0 ? "CUDA device discovered; kernel execution remains provider-deferred" : "CUDA driver loaded but no devices were reported";
    return finish();
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
    if (const auto refusal = flowcontracts::scheduling_refusal(root, true))
        return reject_unsupported(refusal->request, refusal->reason);
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
    try {
        for (int index = 1; index + 1 < argc; ++index)
            if (std::strcmp(argv[index], "--diagnostics") == 0 && std::strcmp(argv[index + 1], "json") == 0)
                structured_diagnostics = true;
        const auto options = parse(argc, argv);
        structured_diagnostics = options.structured_diagnostics;
#ifdef FLOWPARALLEL_CUDA_TEST_ALLOCATION_FAILURE
        throw std::bad_alloc();
#endif
        return run(input(options), options);
    } catch (const std::bad_alloc&) {
        if (structured_diagnostics) write_structured_failure("FLOWPARALLEL_CUDA_RESOURCE_EXHAUSTED", "runtime", "allocation failed");
        else std::cerr << "flowparallel_cuda error: allocation failed\n";
        return 1;
    } catch (const flowcontracts::json::Error& error) {
        if (structured_diagnostics) write_structured_failure("FLOWPARALLEL_CUDA_CONTRACT_FAILURE", "contract", error.what());
        else std::cerr << "flowparallel_cuda contract error: " << error.what() << '\n';
        return 1;
    } catch (const std::exception& error) {
        if (structured_diagnostics) write_structured_failure("FLOWPARALLEL_CUDA_INPUT_INVALID", "input", error.what());
        else std::cerr << "flowparallel_cuda input error: " << error.what() << '\n';
        return 1;
    } catch (...) {
        if (structured_diagnostics) write_structured_failure("FLOWPARALLEL_CUDA_UNKNOWN_FAILURE", "runtime", "unknown non-standard failure");
        else std::cerr << "flowparallel_cuda error: unknown non-standard failure\n";
        return 1;
    }
}
