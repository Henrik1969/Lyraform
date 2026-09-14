#include <flowparallel/cuda_resources.hpp>

#include <cassert>

namespace {

int free_calls = 0;
int destroy_calls = 0;
int free_status = 0;
int destroy_status = 0;

int fake_free(void*) {
    ++free_calls;
    return free_status;
}

int fake_destroy(void*) {
    ++destroy_calls;
    return destroy_status;
}

void reset() {
    free_calls = 0;
    destroy_calls = 0;
    free_status = 0;
    destroy_status = 0;
}

} // namespace

int main() {
    reset();
    flowparallel::CudaDeviceResources resources{fake_free, fake_destroy};
    resources.device_a = reinterpret_cast<void*>(1);
    resources.device_b = reinterpret_cast<void*>(2);
    resources.device_c = reinterpret_cast<void*>(3);
    resources.handle = reinterpret_cast<void*>(4);
    assert(resources.cleanup() == 0);
    assert(destroy_calls == 1);
    assert(free_calls == 3);
    assert(resources.cleanup() == 0);
    assert(destroy_calls == 1);
    assert(free_calls == 3);

    reset();
    flowparallel::CudaDeviceResources partial{fake_free, fake_destroy};
    partial.device_b = reinterpret_cast<void*>(2);
    assert(partial.cleanup() == 0);
    assert(destroy_calls == 0);
    assert(free_calls == 1);

    reset();
    flowparallel::CudaDeviceResources failed{fake_free, fake_destroy};
    failed.device_a = reinterpret_cast<void*>(1);
    failed.handle = reinterpret_cast<void*>(4);
    destroy_status = 7;
    free_status = 9;
    assert(failed.cleanup() == 7);
    assert(destroy_calls == 1);
    assert(free_calls == 1);
    assert(failed.cleanup() == 0);
    assert(destroy_calls == 1);
    assert(free_calls == 1);

    flowparallel::CudaDeviceResources missing_callbacks{};
    missing_callbacks.device_a = reinterpret_cast<void*>(1);
    missing_callbacks.handle = reinterpret_cast<void*>(2);
    assert(missing_callbacks.cleanup() == EINVAL);
    assert(missing_callbacks.device_a == nullptr);
    assert(missing_callbacks.handle == nullptr);
    return 0;
}
