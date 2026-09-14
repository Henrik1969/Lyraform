#include "frankencore/runtime.hpp"

#include <iostream>

int main() {
    const auto result = frankencore::runtime::to_json_checked(
        frankencore::runtime::discover());
    if (!result.valid) {
        std::cerr << "frankencore_runtime_probe: " << result.error << '\n';
        return 1;
    }
    std::cout << result.json;
    return 0;
}
