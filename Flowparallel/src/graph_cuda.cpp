#include <dlfcn.h>
#include <flowcontracts/artifacts.hpp>
#include <flowparallel/bounded_input.hpp>
#include <flowparallel/cuda_resources.hpp>

#include <cstddef>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
using error_t = int;
using handle_t = void*;
constexpr int success = 0;
constexpr int host_to_device = 1;
constexpr int device_to_host = 2;

struct Library {
    void* handle;
    explicit Library(const char* name) : handle(dlopen(name, RTLD_NOW | RTLD_LOCAL)) { if (!handle) throw std::runtime_error(std::string("cannot load ") + name); }
    ~Library() { dlclose(handle); }
    template <typename Function> Function get(const char* name) const { auto* symbol = dlsym(handle, name); if (!symbol) throw std::runtime_error(std::string("missing CUDA symbol: ") + name); return reinterpret_cast<Function>(symbol); }
};

void check(error_t value, const char* operation) { if (value != success) throw std::runtime_error(std::string(operation) + " failed: " + std::to_string(value)); }

std::string read_input(int argc, char** argv) {
    if (argc > 2 && !(argc == 3 && std::string_view(argv[1]) == "--diagnostics" && std::string_view(argv[2]) == "json"))
        throw std::runtime_error("usage: flowparallel_graph_cuda [semantic-report.json]");
    if (argc == 2) { std::ifstream file(argv[1]); if (!file) throw std::runtime_error("cannot open semantic report"); return flowparallel::read_bounded(file, "semantic report"); }
    return flowparallel::read_bounded(std::cin, "semantic report");
}

std::string quote(std::string_view value) { std::string result = "\""; for (char character : value) { if (character == '\\' || character == '"') result.push_back('\\'); result.push_back(character); } result.push_back('"'); return result; }
std::string json_escape(std::string_view value) { std::string escaped; for (const char character : value) { if (character == '\\' || character == '"') escaped.push_back('\\'); if (character == '\n') escaped += "\\n"; else if (character == '\r') escaped += "\\r"; else if (character == '\t') escaped += "\\t"; else escaped.push_back(character); } return escaped; }

int run(std::string_view report) {
    const auto semantic = flowcontracts::semantic_report(flowcontracts::json::parse(report));
    if (semantic.artifact.status != "ok") { std::cout << "{\"format\":\"flowparallel.graph_cuda\",\"version\":1,\"status\":\"blocked\"}\n"; return 2; }
    const auto rows = static_cast<std::size_t>(semantic.dependency_matrix.rows);
    const auto columns = static_cast<std::size_t>(semantic.dependency_matrix.columns);
    if (rows == 0 || rows != columns || rows > 1024) throw std::runtime_error("unsupported graph matrix dimensions");
    const std::size_t elements = rows * columns; std::vector<float> adjacency(elements, 0.0F);
    for (const auto& entry : semantic.dependency_matrix.entries)
        adjacency[static_cast<std::size_t>(entry.column) * rows + static_cast<std::size_t>(entry.row)] = entry.value ? 1.0F : 0.0F;
    const auto cpu_start = std::chrono::steady_clock::now();
    std::vector<unsigned char> cpu_reach(elements, 0);
    for (std::size_t i = 0; i < elements; ++i) cpu_reach[i] = adjacency[i] > 0.5F;
    for (std::size_t pivot = 0; pivot < rows; ++pivot)
        for (std::size_t row = 0; row < rows; ++row)
            if (cpu_reach[pivot * rows + row])
                for (std::size_t column = 0; column < columns; ++column)
                    cpu_reach[column * rows + row] = static_cast<unsigned char>(cpu_reach[column * rows + row] || cpu_reach[column * rows + pivot]);
    const double cpu_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - cpu_start).count();
    const auto cuda_start = std::chrono::steady_clock::now();
    Library runtime("libcudart.so.12"); Library blas("libcublas.so.12");
    using count_fn = error_t (*)(int*); using malloc_fn = error_t (*)(void**, std::size_t); using memcpy_fn = error_t (*)(void*, const void*, std::size_t, int); using sync_fn = error_t (*)();
    using create_fn = error_t (*)(handle_t*); using gemm_fn = error_t (*)(handle_t, int, int, int, int, int, const float*, const float*, int, const float*, int, const float*, float*, int);
    const auto count = runtime.get<count_fn>("cudaGetDeviceCount"); int devices = 0; check(count(&devices), "cudaGetDeviceCount");
    if (devices < 1) { std::cout << "{\"format\":\"flowparallel.graph_cuda\",\"version\":1,\"status\":\"unavailable\",\"provider\":\"cuda.cublas\",\"device_count\":0}\n"; return 2; }
    const auto cuda_malloc = runtime.get<malloc_fn>("cudaMalloc"); const auto cuda_free = runtime.get<flowparallel::CudaFree>("cudaFree"); const auto cuda_memcpy = runtime.get<memcpy_fn>("cudaMemcpy"); const auto cuda_sync = runtime.get<sync_fn>("cudaDeviceSynchronize");
    const auto create = blas.get<create_fn>("cublasCreate_v2"); const auto destroy = blas.get<flowparallel::CublasDestroy>("cublasDestroy_v2"); const auto gemm = blas.get<gemm_fn>("cublasSgemm_v2");
    std::vector<float> power = adjacency, next(elements), reach = adjacency; const std::size_t bytes = elements * sizeof(float);
    flowparallel::CudaDeviceResources resources{cuda_free, destroy};
    try {
        check(cuda_malloc(&resources.device_a, bytes), "cudaMalloc(A)"); check(cuda_malloc(&resources.device_b, bytes), "cudaMalloc(B)"); check(cuda_malloc(&resources.device_c, bytes), "cudaMalloc(C)"); check(cuda_memcpy(resources.device_b, adjacency.data(), bytes, host_to_device), "cudaMemcpy(B)"); check(create(&resources.handle), "cublasCreate");
    const float alpha = 1.0F; const float beta = 0.0F;
    for (std::size_t step = 1; step < rows; ++step) {
        check(cuda_memcpy(resources.device_a, power.data(), bytes, host_to_device), "cudaMemcpy(A)");
        check(gemm(resources.handle, 0, 0, static_cast<int>(rows), static_cast<int>(columns), static_cast<int>(rows), &alpha, static_cast<const float*>(resources.device_a), static_cast<int>(rows), static_cast<const float*>(resources.device_b), static_cast<int>(rows), &beta, static_cast<float*>(resources.device_c), static_cast<int>(rows)), "cublasSgemm");
        check(cuda_sync(), "cudaDeviceSynchronize"); check(cuda_memcpy(next.data(), resources.device_c, bytes, device_to_host), "cudaMemcpy(C)");
        for (std::size_t index = 0; index < elements; ++index) { power[index] = next[index] > 0.5F ? 1.0F : 0.0F; reach[index] = (reach[index] > 0.5F || power[index] > 0.5F) ? 1.0F : 0.0F; }
    }
    } catch (...) {
        const int cleanup_status = resources.cleanup();
        if (cleanup_status != success) throw std::runtime_error("CUDA operation failed and cleanup failed with status " + std::to_string(cleanup_status));
        throw;
    }
    const int cleanup_status = resources.cleanup();
    if (cleanup_status != success) throw std::runtime_error("CUDA cleanup failed with status " + std::to_string(cleanup_status));
    const double cuda_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - cuda_start).count();
    std::size_t reachable_pairs = 0; for (float value : reach) reachable_pairs += value > 0.5F;
    std::size_t cpu_reachable_pairs = 0; for (unsigned char value : cpu_reach) cpu_reachable_pairs += value != 0;
    std::cout << "{\n  \"format\": \"flowparallel.graph_cuda\",\n  \"version\": 1,\n  \"status\": \"verified\",\n  \"source\": {\"path\": " << quote(semantic.source_path) << "},\n  \"operation\": \"reachability\",\n  \"semiring\": \"boolean\",\n  \"reachable_pairs\": " << reachable_pairs << ",\n  \"cpu_reachable_pairs\": " << cpu_reachable_pairs << ",\n  \"cpu_reference_ms\": " << cpu_ms << ",\n  \"cuda_end_to_end_ms\": " << cuda_ms << ",\n  \"end_to_end_speedup\": " << cpu_ms / cuda_ms << ",\n  \"provider\": \"cuda.cublas.boolean_threshold\",\n  \"device_count\": " << devices << "\n}\n";
    return 0;
}
}

int main(int argc, char** argv) {
    const bool structured_diagnostics = argc == 3 && std::string_view(argv[1]) == "--diagnostics" && std::string_view(argv[2]) == "json";
    try {
        if (argc == 2 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "-?" || std::string(argv[1]) == "--help")) { std::cout << "flowparallel_graph_cuda - CUDA Boolean graph reachability\n\nOptions: --diagnostics json\n         -h, -?, --help  show help\n         -a, --about    show about information\n         -v, --version  print the raw version number\n"; return 0; }
        if (argc == 2 && (std::string(argv[1]) == "-a" || std::string(argv[1]) == "--about")) { std::cout << "Flowparallel computes Boolean graph reachability through CUDA cuBLAS with CPU differential verification.\n"; return 0; }
        if (argc == 2 && (std::string(argv[1]) == "-v" || std::string(argv[1]) == "--version")) { std::cout << "0.1.0\n"; return 0; }
        return run(read_input(argc, argv));
    } catch (const std::exception& error) {
        if (structured_diagnostics) std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWPARALLEL_GRAPH_CUDA_FAILURE\",\"message\":\"" << json_escape(error.what()) << "\",\"disposition\":\"no_artifact\"}\n";
        else std::cerr << "flowparallel_graph_cuda error: " << error.what() << '\n';
        return 1;
    } catch (...) {
        if (structured_diagnostics) std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWPARALLEL_GRAPH_CUDA_UNKNOWN_FAILURE\",\"message\":\"unknown non-standard failure\",\"disposition\":\"no_artifact\"}\n";
        else std::cerr << "flowparallel_graph_cuda error: unknown non-standard failure\n";
        return 1;
    }
}
