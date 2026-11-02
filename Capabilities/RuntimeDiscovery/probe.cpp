#include "frankencore/runtime.hpp"

#include <exception>
#include <iostream>
#include <new>
#include <type_traits>
#include <utility>

int main() {
    static_assert(noexcept(frankencore::runtime::discover_checked()));
    const auto discovery = frankencore::runtime::discover_checked();
    if (!discovery.valid) {
        std::cerr << "frankencore_runtime_probe: " << discovery.error << '\n';
        return 1;
    }
    const auto result = frankencore::runtime::to_json_checked(discovery.capabilities);
    if (!result.valid) {
        std::cerr << "frankencore_runtime_probe: " << result.error << '\n';
        return 1;
    }
    std::cout << result.json;
    return 0;
}
