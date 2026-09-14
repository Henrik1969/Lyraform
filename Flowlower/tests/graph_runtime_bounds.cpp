#include <cassert>
#include <cstdint>
#include <sys/wait.h>
#include <unistd.h>

extern "C" void flow_graph_parallel_run(
    void (*const* workers)(std::int64_t, std::int64_t*), const std::int64_t* inputs,
    std::int64_t* outputs, std::int64_t count);

namespace {
void worker(std::int64_t input, std::int64_t* output) { *output = input + 1; }
}

int main() {
    void (*workers[1]) (std::int64_t, std::int64_t*) = {worker};
    const std::int64_t input[1] = {41};
    std::int64_t output[1] = {0};
    flow_graph_parallel_run(workers, input, output, 1);
    assert(output[0] == 42);

    const pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        flow_graph_parallel_run(workers, input, output, 257);
        _exit(0);
    }
    int status = 0;
    assert(waitpid(child, &status, 0) == child);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 70);
    return 0;
}
