#include <dlfcn.h>
#include <flowparallel/cuda_resources.hpp>
#include <flowparallel/dynamic_library.hpp>

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using error_t = int;
using handle_t = void*;
constexpr int success = 0;
constexpr int host_to_device = 1;
constexpr int device_to_host = 2;

using Library = flowparallel::DynamicLibrary;

void check(error_t value, const char* operation) { if (value != success) throw std::runtime_error(std::string(operation) + " failed: " + std::to_string(value)); }
struct Options { int size = 512; int iterations = 5; bool structured_diagnostics = false; };

std::string json_escape(std::string_view value) { std::string escaped; for (const char character : value) { if (character == '\\' || character == '"') escaped.push_back('\\'); if (character == '\n') escaped += "\\n"; else if (character == '\r') escaped += "\\r"; else if (character == '\t') escaped += "\\t"; else escaped.push_back(character); } return escaped; }

int parse_integer(std::string_view text, const char* option) { int value = 0; const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value); if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size()) throw std::runtime_error(std::string(option) + " requires a complete integer"); return value; }

Options parse(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--size" || arg == "--iterations") {
            if (++i >= argc) throw std::runtime_error(arg + " requires a value");
            int value = parse_integer(argv[i], arg.c_str());
            if (arg == "--size") options.size = value; else options.iterations = value;
        } else if (arg == "-h" || arg == "-?" || arg == "--help") {
            std::cout << "flowparallel_matrix_benchmark - CPU/CUDA matrix benchmark\n\nOptions: --size N --iterations N --diagnostics json\n         -h, -?, --help  show help\n         -a, --about    show about information\n         -v, --version  print the raw version number\n"; std::exit(0);
        } else if (arg == "-a" || arg == "--about") { std::cout << "Flowparallel compares a single-thread CPU matrix baseline with CUDA cuBLAS.\n"; std::exit(0); }
        else if (arg == "-v" || arg == "--version") { std::cout << "0.1.0\n"; std::exit(0); }
        else if (arg == "--diagnostics") { if (++i >= argc || std::string(argv[i]) != "json") throw std::runtime_error("--diagnostics requires json"); options.structured_diagnostics = true; }
        else throw std::runtime_error("unknown option '" + arg + "'");
    }
    if (options.size < 32 || options.size > 2048 || options.iterations < 2 || options.iterations > 100) throw std::runtime_error("benchmark dimensions are outside safe bounds");
    return options;
}

double checksum(const std::vector<float>& values) { double result = 0.0; for (float value : values) result += value; return result; }

int run_with_libraries(const Options& options, Library& runtime, Library& blas) {
#ifdef FLOWPARALLEL_MATRIX_BENCHMARK_TEST_ALLOCATION_FAILURE
    (void)options;
    throw std::bad_alloc();
#endif
    const int n = options.size;
    const std::size_t elements = static_cast<std::size_t>(n) * n;
    const std::size_t bytes = elements * sizeof(float);
    std::vector<float> a(elements), b(elements), cpu(elements, 0.0F), gpu(elements, 0.0F);
    for (int row = 0; row < n; ++row) for (int column = 0; column < n; ++column) {
        a[static_cast<std::size_t>(row) * n + column] = row == column ? 2.0F : 1.0F;
        b[static_cast<std::size_t>(row) * n + column] = row == column ? 3.0F : 1.0F;
    }

    const auto cpu_start = std::chrono::steady_clock::now();
    for (int iteration = 0; iteration < options.iterations; ++iteration) {
        for (int row = 0; row < n; ++row) for (int column = 0; column < n; ++column) {
            float value = 0.0F;
            for (int inner = 0; inner < n; ++inner) value += a[static_cast<std::size_t>(row) * n + inner] * b[static_cast<std::size_t>(inner) * n + column];
            cpu[static_cast<std::size_t>(row) * n + column] = value;
        }
    }
    const double cpu_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - cpu_start).count() / options.iterations;

    using malloc_fn = error_t (*)(void**, std::size_t); using free_fn = error_t (*)(void*); using memcpy_fn = error_t (*)(void*, const void*, std::size_t, int); using sync_fn = error_t (*)();
    using create_fn = error_t (*)(handle_t*); using destroy_fn = error_t (*)(handle_t); using gemm_fn = error_t (*)(handle_t, int, int, int, int, int, const float*, const float*, int, const float*, int, const float*, float*, int);
    const auto cuda_malloc = runtime.symbol<malloc_fn>("cudaMalloc"); const auto cuda_free = runtime.symbol<free_fn>("cudaFree"); const auto cuda_memcpy = runtime.symbol<memcpy_fn>("cudaMemcpy"); const auto cuda_sync = runtime.symbol<sync_fn>("cudaDeviceSynchronize");
    const auto create = blas.symbol<create_fn>("cublasCreate_v2"); const auto destroy = blas.symbol<destroy_fn>("cublasDestroy_v2"); const auto gemm = blas.symbol<gemm_fn>("cublasSgemm_v2");
    flowparallel::CudaDeviceResources resources{cuda_free, destroy};
    double gpu_compute_ms = 0.0;
    double end_to_end_ms = 0.0;
    try {
    check(cuda_malloc(&resources.device_a, bytes), "cudaMalloc(A)" ); check(cuda_malloc(&resources.device_b, bytes), "cudaMalloc(B)"); check(cuda_malloc(&resources.device_c, bytes), "cudaMalloc(C)");
    check(cuda_memcpy(resources.device_a, a.data(), bytes, host_to_device), "cudaMemcpy(A)"); check(cuda_memcpy(resources.device_b, b.data(), bytes, host_to_device), "cudaMemcpy(B)"); check(create(&resources.handle), "cublasCreate");
    const float alpha = 1.0F; const float beta = 0.0F;
    check(gemm(resources.handle, 0, 0, n, n, n, &alpha, static_cast<const float*>(resources.device_a), n, static_cast<const float*>(resources.device_b), n, &beta, static_cast<float*>(resources.device_c), n), "cublasSgemm");
    check(cuda_sync(), "warmup");
    const auto gpu_start = std::chrono::steady_clock::now();
    for (int iteration = 0; iteration < options.iterations; ++iteration) check(gemm(resources.handle, 0, 0, n, n, n, &alpha, static_cast<const float*>(resources.device_a), n, static_cast<const float*>(resources.device_b), n, &beta, static_cast<float*>(resources.device_c), n), "cublasSgemm");
    check(cuda_sync(), "cudaDeviceSynchronize");
    gpu_compute_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - gpu_start).count() / options.iterations;
    const auto end_to_end_start = std::chrono::steady_clock::now();
    for (int iteration = 0; iteration < options.iterations; ++iteration) {
        check(cuda_memcpy(resources.device_a, a.data(), bytes, host_to_device), "cudaMemcpy(A)");
        check(cuda_memcpy(resources.device_b, b.data(), bytes, host_to_device), "cudaMemcpy(B)");
        check(gemm(resources.handle, 0, 0, n, n, n, &alpha, static_cast<const float*>(resources.device_a), n, static_cast<const float*>(resources.device_b), n, &beta, static_cast<float*>(resources.device_c), n), "cublasSgemm");
        check(cuda_sync(), "cudaDeviceSynchronize");
        check(cuda_memcpy(gpu.data(), resources.device_c, bytes, device_to_host), "cudaMemcpy(C)");
    }
    end_to_end_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - end_to_end_start).count() / options.iterations;
    } catch (...) {
        const int cleanup_status = resources.cleanup();
        if (cleanup_status != success) throw std::runtime_error("CUDA benchmark failed and cleanup failed with status " + std::to_string(cleanup_status));
        throw;
    }
    const int cleanup_status = resources.cleanup();
    if (cleanup_status != success) throw std::runtime_error("CUDA benchmark cleanup failed with status " + std::to_string(cleanup_status));
    double max_error = 0.0; for (std::size_t i = 0; i < elements; ++i) max_error = std::max(max_error, std::fabs(static_cast<double>(cpu[i]) - gpu[i]));
    std::cout << std::setprecision(10) << "{\n  \"format\": \"flowparallel.matrix_benchmark\",\n  \"status\": \"verified\",\n  \"matrix_size\": " << n << ",\n  \"iterations\": " << options.iterations << ",\n  \"cpu_single_thread_ms\": " << cpu_ms << ",\n  \"cuda_cublas_compute_ms\": " << gpu_compute_ms << ",\n  \"cuda_end_to_end_ms\": " << end_to_end_ms << ",\n  \"compute_speedup\": " << cpu_ms / gpu_compute_ms << ",\n  \"end_to_end_speedup\": " << cpu_ms / end_to_end_ms << ",\n  \"max_error\": " << max_error << ",\n  \"cpu_checksum\": " << checksum(cpu) << ",\n  \"cuda_checksum\": " << checksum(gpu) << "\n}\n";
    return max_error < 0.001 ? 0 : 2;
}

int run(const Options& options) {
    std::optional<Library> runtime;
    std::optional<Library> blas;
    try {
        runtime.emplace("libcudart.so.12");
        blas.emplace("libcublas.so.12");
        const int result = run_with_libraries(options, *runtime, *blas);
        const int runtime_close = runtime->close();
        const int blas_close = blas->close();
        if (runtime_close != 0 || blas_close != 0)
            throw std::runtime_error("CUDA dynamic-library cleanup failed");
        return result;
    } catch (...) {
        const int runtime_close = runtime ? runtime->close() : 0;
        const int blas_close = blas ? blas->close() : 0;
        if (runtime_close != 0 || blas_close != 0)
            throw std::runtime_error("CUDA benchmark and dynamic-library cleanup failed");
        throw;
    }
}
}

int main(int argc, char** argv) {
    bool structured_diagnostics = false;
    for (int i = 1; i + 1 < argc; ++i)
        if (std::string(argv[i]) == "--diagnostics" && std::string(argv[i + 1]) == "json") structured_diagnostics = true;
    try { return run(parse(argc, argv)); }
    catch (const std::bad_alloc&) {
        if (structured_diagnostics) std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWPARALLEL_MATRIX_BENCHMARK_RESOURCE_EXHAUSTED\",\"message\":\"allocation failed\",\"disposition\":\"no_artifact\"}\n";
        else std::cerr << "flowparallel_matrix_benchmark error: allocation failed\n";
        return 1;
    } catch (const std::exception& error) {
        if (structured_diagnostics) std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWPARALLEL_MATRIX_BENCHMARK_FAILURE\",\"message\":\"" << json_escape(error.what()) << "\",\"disposition\":\"no_artifact\"}\n";
        else std::cerr << "flowparallel_matrix_benchmark error: " << error.what() << '\n';
        return 1;
    } catch (...) {
        if (structured_diagnostics) std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWPARALLEL_MATRIX_BENCHMARK_UNKNOWN_FAILURE\",\"message\":\"unknown non-standard failure\",\"disposition\":\"no_artifact\"}\n";
        else std::cerr << "flowparallel_matrix_benchmark error: unknown non-standard failure\n";
        return 1;
    }
}
