#include "flowparallel/cpu_execution.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <exception>
#include <mutex>
#include <new>
#include <stdexcept>
#include <thread>

namespace flowparallel::cpu {

ExecutionResult execute_independent(const std::vector<Task>& tasks, unsigned workers) {
    ExecutionResult result;
    if (workers == 0) { result.status = "error"; result.code = "INVALID_WORKER_COUNT"; result.error = "workers must be greater than zero"; return result; }
    if (workers > max_workers) { result.status = "error"; result.code = "WORKER_COUNT_EXCEEDED"; result.error = "workers exceed the 256-worker limit"; return result; }
    if (tasks.empty()) return result;
    const auto actual_workers = std::min<unsigned>(workers, static_cast<unsigned>(tasks.size()));
    std::atomic<std::size_t> next{0};
    std::atomic<std::size_t> completed{0};
    std::atomic<bool> failed{false};
    std::mutex failure_mutex;
    std::size_t failed_task = static_cast<std::size_t>(-1);
    std::string failure_code;
    std::array<char, 256> failure_message{};

    const auto capture_failure = [&](const char* message) {
        const auto length = std::min<std::size_t>(std::char_traits<char>::length(message),
                                                  failure_message.size() - 1);
        std::copy_n(message, length, failure_message.data());
        failure_message[length] = '\0';
    };

    auto worker = [&] {
        while (!failed.load(std::memory_order_acquire)) {
            const auto index = next.fetch_add(1, std::memory_order_relaxed);
            if (index >= tasks.size()) return;
            try {
                if (!tasks[index].execute) throw std::runtime_error("task has no executable body");
                tasks[index].execute();
                completed.fetch_add(1, std::memory_order_relaxed);
            } catch (const std::exception& error) {
                if (!failed.exchange(true, std::memory_order_acq_rel)) {
                    std::lock_guard lock(failure_mutex);
                    failed_task = index;
                    failure_code = "TASK_FAILURE";
                    capture_failure(error.what());
                }
                return;
            } catch (...) {
                if (!failed.exchange(true, std::memory_order_acq_rel)) {
                    std::lock_guard lock(failure_mutex);
                    failed_task = index;
                    failure_code = "TASK_UNKNOWN_FAILURE";
                    capture_failure("task failed with a non-standard exception");
                }
                return;
            }
        }
    };

    // jthread joins already-started workers if vector growth or a later
    // worker launch fails. This keeps partial initialization from leaking a
    // joinable thread or terminating the process during vector unwinding.
    {
        std::vector<std::jthread> threads;
        try {
#ifdef FLOWPARALLEL_CPU_EXECUTION_TEST_LAUNCH_FAILURE
            throw std::bad_alloc();
#endif
            threads.reserve(actual_workers);
            for (unsigned index = 0; index < actual_workers; ++index) threads.emplace_back(worker);
        } catch (const std::bad_alloc& error) {
            result.status = "error";
            result.code = "WORKER_LAUNCH_RESOURCE_EXHAUSTED";
            result.completed = completed.load(std::memory_order_relaxed);
            result.error = std::string("worker launch exhausted resources: ") + error.what();
            return result;
        } catch (const std::exception& error) {
            result.status = "error";
            result.code = "WORKER_LAUNCH_FAILURE";
            result.completed = completed.load(std::memory_order_relaxed);
            result.error = std::string("worker launch failed: ") + error.what();
            return result;
        } catch (...) {
            result.status = "error";
            result.code = "WORKER_LAUNCH_UNKNOWN_FAILURE";
            result.completed = completed.load(std::memory_order_relaxed);
            result.error = "worker launch failed with a non-standard exception";
            return result;
        }
    }
    result.completed = completed.load(std::memory_order_relaxed);
    if (failed.load(std::memory_order_acquire)) {
        result.status = "error";
        result.code = failure_code;
        result.failed_task = failed_task;
        result.error = failure_message.data();
    }
    return result;
}

} // namespace flowparallel::cpu
