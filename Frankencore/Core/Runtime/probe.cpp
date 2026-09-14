#include "frankencore/runtime.hpp"

#include <exception>
#include <iostream>
#include <new>

int main() {
    try {
        const auto result = frankencore::runtime::to_json_checked(
            frankencore::runtime::discover());
        if (!result.valid) {
            std::cerr << "frankencore_runtime_probe: " << result.error << '\n';
            return 1;
        }
        std::cout << result.json;
        return 0;
    } catch (const std::bad_alloc&) {
        std::cerr << "frankencore_runtime_probe: runtime capability discovery exhausted memory\n";
    } catch (const std::exception& error) {
        std::cerr << "frankencore_runtime_probe: " << error.what() << '\n';
    } catch (...) {
        std::cerr << "frankencore_runtime_probe: unknown runtime capability discovery failure\n";
    }
    return 1;
}
