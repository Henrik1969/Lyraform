#pragma once

#include <cerrno>

namespace flowparallel {

using CudaFree = int (*)(void*);
using CublasDestroy = int (*)(void*);

// Owns the native resources acquired by an admitted CUDA operation. The
// cleanup operation is repeatable and returns the first provider failure so a
// caller can preserve that evidence in its explicit result.
struct CudaDeviceResources {
    CudaFree cuda_free = nullptr;
    CublasDestroy cublas_destroy = nullptr;
    void* device_a = nullptr;
    void* device_b = nullptr;
    void* device_c = nullptr;
    void* handle = nullptr;

    template <typename Callback>
    static int invoke(Callback callback, void* resource) noexcept {
        if (!callback) return EINVAL;
        try {
            return callback(resource);
        } catch (...) {
            return EFAULT;
        }
    }

    int cleanup() noexcept {
        int first_failure = 0;
        if (handle) {
            const int status = invoke(cublas_destroy, handle);
            if (first_failure == 0 && status != 0) first_failure = status;
            handle = nullptr;
        }
        if (device_c) {
            const int status = invoke(cuda_free, device_c);
            if (first_failure == 0 && status != 0) first_failure = status;
            device_c = nullptr;
        }
        if (device_b) {
            const int status = invoke(cuda_free, device_b);
            if (first_failure == 0 && status != 0) first_failure = status;
            device_b = nullptr;
        }
        if (device_a) {
            const int status = invoke(cuda_free, device_a);
            if (first_failure == 0 && status != 0) first_failure = status;
            device_a = nullptr;
        }
        return first_failure;
    }
};

} // namespace flowparallel
