#include <dlfcn.h>
#include <flowparallel/cuda_resources.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using cuda_error_t = int;
using cublas_status_t = int;
using cublas_handle_t = void*;

constexpr cuda_error_t cuda_success = 0;
constexpr int cuda_memcpy_host_to_device = 1;
constexpr int cuda_memcpy_device_to_host = 2;
constexpr int cublas_op_n = 0;

struct Library {
    void* handle = nullptr;
    explicit Library(const char* name) : handle(dlopen(name, RTLD_NOW | RTLD_LOCAL)) {
        if (!handle) throw std::runtime_error(std::string("cannot load ") + name);
    }
    ~Library() { if (handle) dlclose(handle); }
    Library(const Library&) = delete;
    Library& operator=(const Library&) = delete;

    template <typename Function>
    Function symbol(const char* name) const {
        auto* value = dlsym(handle, name);
        if (!value) throw std::runtime_error(std::string("missing CUDA symbol: ") + name);
        return reinterpret_cast<Function>(value);
    }
};

void require_cuda(cuda_error_t status, const char* operation) {
    if (status != cuda_success) throw std::runtime_error(std::string(operation) + " failed with CUDA error " + std::to_string(status));
}

void require_cublas(cublas_status_t status, const char* operation) {
    if (status != 0) throw std::runtime_error(std::string(operation) + " failed with cuBLAS status " + std::to_string(status));
}

struct Options { int size = 64; };

Options parse(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--size") {
            if (++i >= argc) throw std::runtime_error("--size requires a value");
            options.size = std::stoi(argv[i]);
            if (options.size < 2 || options.size > 4096) throw std::runtime_error("--size must be between 2 and 4096");
        } else if (arg == "-h" || arg == "-?" || arg == "--help") {
            std::cout << "flowparallel_cuda_execute - verified CUDA matrix multiplication\n\n"
                         "Options: --size N\n"
                         "         -h, -?, --help  show help\n"
                         "         -a, --about    show about information\n"
                         "         -v, --version  print the raw version number\n";
            std::exit(0);
        } else if (arg == "-a" || arg == "--about") {
            std::cout << "Flowparallel CUDA executes and verifies a small cuBLAS matrix multiplication.\n";
            std::exit(0);
        } else if (arg == "-v" || arg == "--version") {
            std::cout << "0.1.0\n";
            std::exit(0);
        } else {
            throw std::runtime_error("unknown option '" + arg + "'");
        }
    }
    return options;
}

int run(const Options& options) {
    Library runtime("libcudart.so.12");
    Library blas("libcublas.so.12");

    using get_device_count_fn = cuda_error_t (*)(int*);
    using malloc_fn = cuda_error_t (*)(void**, std::size_t);
    using memcpy_fn = cuda_error_t (*)(void*, const void*, std::size_t, int);
    using synchronize_fn = cuda_error_t (*)();
    using create_fn = cublas_status_t (*)(cublas_handle_t*);
    using sgemm_fn = cublas_status_t (*)(cublas_handle_t, int, int, int, int, int,
                                         const float*, const float*, int,
                                         const float*, int, const float*, float*, int);

    const auto get_device_count = runtime.symbol<get_device_count_fn>("cudaGetDeviceCount");
    const auto cuda_malloc = runtime.symbol<malloc_fn>("cudaMalloc");
    const auto cuda_free = runtime.symbol<flowparallel::CudaFree>("cudaFree");
    const auto cuda_memcpy = runtime.symbol<memcpy_fn>("cudaMemcpy");
    const auto cuda_synchronize = runtime.symbol<synchronize_fn>("cudaDeviceSynchronize");
    const auto cublas_create = blas.symbol<create_fn>("cublasCreate_v2");
    const auto cublas_destroy = blas.symbol<flowparallel::CublasDestroy>("cublasDestroy_v2");
    const auto cublas_sgemm = blas.symbol<sgemm_fn>("cublasSgemm_v2");

    int device_count = 0;
    require_cuda(get_device_count(&device_count), "cudaGetDeviceCount");
    if (device_count == 0) throw std::runtime_error("CUDA reported no devices");

    const std::size_t elements = static_cast<std::size_t>(options.size) * options.size;
    const std::size_t bytes = elements * sizeof(float);
    std::vector<float> host_a(elements), host_b(elements), host_c(elements, 0.0F);
    for (int row = 0; row < options.size; ++row) {
        for (int column = 0; column < options.size; ++column) {
            host_a[static_cast<std::size_t>(column) * options.size + row] = row == column ? 2.0F : 1.0F;
            host_b[static_cast<std::size_t>(column) * options.size + row] = row == column ? 3.0F : 1.0F;
        }
    }

    flowparallel::CudaDeviceResources resources{cuda_free, cublas_destroy};
    try {
        require_cuda(cuda_malloc(&resources.device_a, bytes), "cudaMalloc(A)");
        require_cuda(cuda_malloc(&resources.device_b, bytes), "cudaMalloc(B)");
        require_cuda(cuda_malloc(&resources.device_c, bytes), "cudaMalloc(C)");
        require_cuda(cuda_memcpy(resources.device_a, host_a.data(), bytes, cuda_memcpy_host_to_device), "cudaMemcpy(A)");
        require_cuda(cuda_memcpy(resources.device_b, host_b.data(), bytes, cuda_memcpy_host_to_device), "cudaMemcpy(B)");
        require_cublas(cublas_create(&resources.handle), "cublasCreate");
        const float alpha = 1.0F;
        const float beta = 0.0F;
        require_cublas(cublas_sgemm(resources.handle, cublas_op_n, cublas_op_n, options.size, options.size, options.size,
                                    &alpha, static_cast<const float*>(resources.device_a), options.size,
                                    static_cast<const float*>(resources.device_b), options.size,
                                    &beta, static_cast<float*>(resources.device_c), options.size), "cublasSgemm");
        require_cuda(cuda_synchronize(), "cudaDeviceSynchronize");
        require_cuda(cuda_memcpy(host_c.data(), resources.device_c, bytes, cuda_memcpy_device_to_host), "cudaMemcpy(C)");
    } catch (...) {
        const int cleanup_status = resources.cleanup();
        if (cleanup_status != 0) throw std::runtime_error("CUDA operation failed and cleanup failed with status " + std::to_string(cleanup_status));
        throw;
    }
    const int cleanup_status = resources.cleanup();
    if (cleanup_status != 0) throw std::runtime_error("CUDA cleanup failed with status " + std::to_string(cleanup_status));

    float max_error = 0.0F;
    for (int column = 0; column < options.size; ++column) {
        for (int row = 0; row < options.size; ++row) {
            const float expected = static_cast<float>(options.size + (row == column ? 5 : 3));
            max_error = std::max(max_error, std::fabs(host_c[static_cast<std::size_t>(column) * options.size + row] - expected));
        }
    }
    const bool verified = max_error < 0.001F;
    double result_sum = 0.0;
    for (const float value : host_c) result_sum += value;
    std::cout << std::setprecision(17) << "{\n"
                 "  \"format\": \"flowparallel.cuda_execution\",\n"
                 "  \"version\": 1,\n"
                 "  \"status\": \"" << (verified ? "verified" : "failed") << "\",\n"
                 "  \"provider\": \"cuda.cublas\",\n"
                 "  \"matrix_size\": " << options.size << ",\n"
                 "  \"max_error\": " << max_error << ",\n"
                 "  \"result_sum\": " << result_sum << ",\n"
                 "  \"device_count\": " << device_count << "\n"
                 "}\n";
    return verified ? 0 : 2;
}

} // namespace

int main(int argc, char** argv) {
    try { return run(parse(argc, argv)); }
    catch (const std::exception& error) { std::cerr << "flowparallel_cuda_execute error: " << error.what() << '\n'; return 1; }
}
