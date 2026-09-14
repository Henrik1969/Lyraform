#include <atomic>
#include <cassert>
#include <cstdint>
#include <new>

extern "C" void flow_graph_parallel_run(
    void (*const* workers)(std::int64_t, std::int64_t*), const std::int64_t* inputs,
    std::int64_t* outputs, std::int64_t count);

namespace {
std::atomic<bool> completed{false};
void worker(std::int64_t input, std::int64_t* output) {
    *output = input + 1;
    completed.store(true, std::memory_order_release);
}
}

int main() {
    void (*workers[2]) (std::int64_t, std::int64_t*) = {worker, worker};
    const std::int64_t inputs[2] = {41, 42};
    std::int64_t outputs[2] = {0, 0};
    bool caught = false;
    try {
        flow_graph_parallel_run(workers, inputs, outputs, 2);
    } catch (const std::bad_alloc&) {
        caught = true;
    }
    assert(caught);
    assert(completed.load(std::memory_order_acquire));
    assert(outputs[0] == 42);
    return 0;
}
