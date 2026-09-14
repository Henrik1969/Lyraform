#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string>
#include <stdexcept>
#include <unistd.h>

extern "C" void flow_graph_parallel_run(
    void (*const* workers)(std::int64_t, std::int64_t*), const std::int64_t* inputs,
    std::int64_t* outputs, std::int64_t count);
extern "C" void flow_graph_drop(const char* value);
extern "C" void flow_graph_enter(const char* value);
extern "C" [[noreturn]] void flow_graph_fail(std::uint64_t operation, const char* reason);

namespace {
void worker(std::int64_t input, std::int64_t* output) { *output = input + 1; }
void throwing_worker(std::int64_t, std::int64_t*) { throw std::runtime_error("hostile worker"); }
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

    void (*throwing_workers[1]) (std::int64_t, std::int64_t*) = {throwing_worker};
    const pid_t throwing_child = fork();
    assert(throwing_child >= 0);
    if (throwing_child == 0) {
        flow_graph_parallel_run(throwing_workers, input, output, 1);
        _exit(0);
    }
    assert(waitpid(throwing_child, &status, 0) == throwing_child);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 70);

    const std::string oversized_activation(17U * 1024U * 1024U, 'a');
    char failure_path[] = "/tmp/flowgraph-failure-bound-XXXXXX";
    const int failure_descriptor = mkstemp(failure_path);
    assert(failure_descriptor >= 0);
    const pid_t failure_child = fork();
    assert(failure_child >= 0);
    if (failure_child == 0) {
        assert(dup2(failure_descriptor, STDERR_FILENO) >= 0);
        flow_graph_enter(oversized_activation.c_str());
        flow_graph_fail(7, "hostile failure");
    }
    assert(waitpid(failure_child, &status, 0) == failure_child);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 70);
    struct stat failure_details{};
    assert(fstat(failure_descriptor, &failure_details) == 0);
    close(failure_descriptor);
    unlink(failure_path);
    assert(static_cast<std::uintmax_t>(failure_details.st_size) < 4096U);

    char path[] = "/tmp/flowgraph-diagnostic-bound-XXXXXX";
    const int descriptor = mkstemp(path);
    assert(descriptor >= 0);
    const int saved_stderr = dup(STDERR_FILENO);
    assert(saved_stderr >= 0);
    assert(dup2(descriptor, STDERR_FILENO) >= 0);
    const std::string large(1024U * 1024U, 'x');
    for (int index = 0; index < 20; ++index) flow_graph_drop(large.c_str());
    flow_graph_drop(oversized_activation.c_str());
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
