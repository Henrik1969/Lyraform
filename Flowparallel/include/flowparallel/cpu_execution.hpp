#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace flowparallel::cpu {

inline constexpr unsigned max_workers = 256;

struct Task {
    std::function<void()> execute;
};

struct ExecutionResult {
    std::string status = "ok";
    std::string code;
    std::string disposition = "no_artifact";
    std::size_t completed = 0;
    std::size_t failed_task = static_cast<std::size_t>(-1);
    std::string error;
};

// Executes only the supplied, already-approved independent tasks. The caller
// remains responsible for proving that tasks do not share mutable state.
ExecutionResult execute_independent(const std::vector<Task>& tasks, unsigned workers);

} // namespace flowparallel::cpu
