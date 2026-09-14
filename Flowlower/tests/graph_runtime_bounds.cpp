#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string>
#include <unistd.h>

extern "C" void flow_graph_parallel_run(
    void (*const* workers)(std::int64_t, std::int64_t*), const std::int64_t* inputs,
    std::int64_t* outputs, std::int64_t count);
extern "C" void flow_graph_drop(const char* value);

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

    char path[] = "/tmp/flowgraph-diagnostic-bound-XXXXXX";
    const int descriptor = mkstemp(path);
    assert(descriptor >= 0);
    const int saved_stderr = dup(STDERR_FILENO);
    assert(saved_stderr >= 0);
    assert(dup2(descriptor, STDERR_FILENO) >= 0);
    const std::string large(1024U * 1024U, 'x');
    for (int index = 0; index < 20; ++index) flow_graph_drop(large.c_str());
    const std::string oversized(17U * 1024U * 1024U, 'y');
    flow_graph_drop(oversized.c_str());
    std::fflush(stderr);
    assert(dup2(saved_stderr, STDERR_FILENO) >= 0);
    close(saved_stderr);
    struct stat details{};
    assert(fstat(descriptor, &details) == 0);
    close(descriptor);
    unlink(path);
    assert(static_cast<std::uintmax_t>(details.st_size) <= 16U * 1024U * 1024U + 128U);
    return 0;
}
