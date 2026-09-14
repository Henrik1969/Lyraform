#include "flowparallel/cpu_execution.hpp"

#include <cassert>
#include <iostream>

int main() {
    const auto result = flowparallel::cpu::execute_independent(
        {{[] {}}, {[] {}}}, 2);
    assert(result.status == "error");
    assert(result.code == "WORKER_LAUNCH_RESOURCE_EXHAUSTED");
    assert(result.completed == 0);
    assert(result.failed_task == static_cast<std::size_t>(-1));
    assert(result.disposition == "no_artifact");
    std::cout << "Flowparallel CPU execution allocation-fault cleanup: PASS\n";
}
