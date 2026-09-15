#include "frankencore/runtime.hpp"

#include <dlfcn.h>
#include <exception>
#include <fstream>
#include <charconv>
#include <limits>
#include <new>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <unistd.h>

namespace frankencore::runtime {
namespace {

std::string quote(std::string_view value) {
    std::string result = "\"";
    for (const char character : value) {
        if (character == '\\' || character == '"') result.push_back('\\');
        if (character == '\n') result += "\\n";
        else if (character == '\r') result += "\\r";
        else result.push_back(character);
    }
    result.push_back('"');
    return result;
}

std::uint64_t parse_kibibytes(const std::string& line) {
    std::istringstream input(line);
    std::string label;
    std::string value_text;
    std::string unit;
    input >> label >> value_text >> unit;
    if (label.empty() || unit != "kB" || value_text.empty()) return 0;
    std::uint64_t value = 0;
    const auto parsed = std::from_chars(value_text.data(),
                                        value_text.data() + value_text.size(),
                                        value);
    if (parsed.ec != std::errc{} ||
        parsed.ptr != value_text.data() + value_text.size()) return 0;
    constexpr auto multiplier = std::uint64_t{1024};
    if (value > std::numeric_limits<std::uint64_t>::max() / multiplier)
        return std::numeric_limits<std::uint64_t>::max();
    return value * multiplier;
}

MemoryFacts read_memory() {
    MemoryFacts result;
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    while (std::getline(meminfo, line)) {
        if (line.rfind("MemTotal:", 0) == 0) result.total_bytes = parse_kibibytes(line);
        else if (line.rfind("MemAvailable:", 0) == 0) result.available_bytes = parse_kibibytes(line);
    }
    return result;
}

CudaFacts discover_cuda() {
    CudaFacts result;
    void* library = dlopen(
#ifdef FRANKENCORE_RUNTIME_CUDA_CLOSE_FAULT
        "libc.so.6",
#else
        "libcuda.so.1",
#endif
        RTLD_LAZY | RTLD_LOCAL);
    if (!library) {
        result.status = "unavailable";
        result.diagnostic = "libcuda.so.1 was not available";
        return result;
    }
    using init_fn = int (*)(unsigned int);
    using count_fn = int (*)(int*);
    const auto init = reinterpret_cast<init_fn>(dlsym(library, "cuInit"));
    const auto count = reinterpret_cast<count_fn>(dlsym(library, "cuDeviceGetCount"));
    const auto close_library = [&] {
        const int close_status = dlclose(library);
#ifdef FRANKENCORE_RUNTIME_CUDA_CLOSE_FAULT
        (void)close_status;
        const int reported_status = -1;
#else
        const int reported_status = close_status;
#endif
        if (reported_status != 0) {
            result.status = "unknown";
            result.diagnostic = "CUDA driver library cleanup failed";
        }
    };
    if (!init || !count) {
        result.status = "unknown";
        result.diagnostic = "CUDA driver symbols were incomplete";
        close_library();
        return result;
    }
    if (init(0) != 0) {
        result.status = "unknown";
        result.diagnostic = "CUDA driver initialization failed";
        close_library();
        return result;
    }
    int devices = 0;
    if (count(&devices) != 0) {
        result.status = "unknown";
        result.diagnostic = "CUDA device enumeration failed";
        close_library();
        return result;
    }
    result.status = devices > 0 ? "available" : "unavailable";
    result.device_count = devices > 0 ? static_cast<std::uint64_t>(devices) : 0;
    if (devices == 0) result.diagnostic = "CUDA driver loaded but no devices were reported";
    close_library();
    return result;
}

} // namespace

Capabilities discover() {
#ifdef FRANKENCORE_RUNTIME_DISCOVERY_TEST_FAULT
    throw std::runtime_error("injected runtime capability discovery failure");
#endif
    Capabilities result;
    const long processors = sysconf(_SC_NPROCESSORS_ONLN);
    result.cpu.logical_processors = processors > 0 ? static_cast<std::uint64_t>(processors) : 0;
    result.memory = read_memory();
    result.cuda = discover_cuda();
    return result;
}

DiscoveryResult discover_checked() noexcept {
    try {
        return {true, discover(), {}};
    } catch (const std::bad_alloc&) {
        return {false, {}, "runtime capability discovery exhausted memory"};
    } catch (const std::exception&) {
        return {false, {}, "runtime capability discovery failed"};
    } catch (...) {
        return {false, {}, "runtime capability discovery failed with unknown non-standard failure"};
    }
}

std::string to_json(const Capabilities& capabilities) {
    return "{\n"
           "  \"format\": " + quote(capabilities.format) + ",\n"
           "  \"version\": " + std::to_string(capabilities.version) + ",\n"
           "  \"cpu\": {\"logical_processors\": " + std::to_string(capabilities.cpu.logical_processors) + "},\n"
           "  \"memory\": {\"total_bytes\": " + std::to_string(capabilities.memory.total_bytes) +
           ", \"available_bytes\": " + std::to_string(capabilities.memory.available_bytes) + "},\n"
           "  \"cuda\": {\"status\": " + quote(capabilities.cuda.status) +
           ", \"driver\": " + quote(capabilities.cuda.driver) +
           ", \"device_count\": " + std::to_string(capabilities.cuda.device_count) +
           ", \"diagnostic\": " + quote(capabilities.cuda.diagnostic) + "}\n"
           "}\n";
}

JsonResult to_json_checked(const Capabilities& capabilities) noexcept {
    try {
        return {true, to_json(capabilities), {}};
    } catch (const std::bad_alloc&) {
        return {false, {}, "runtime capability serialization exhausted memory"};
    } catch (const std::exception&) {
        return {false, {}, "runtime capability serialization failed"};
    } catch (...) {
        return {false, {}, "unknown non-standard runtime capability serialization failure"};
    }
}

} // namespace frankencore::runtime
