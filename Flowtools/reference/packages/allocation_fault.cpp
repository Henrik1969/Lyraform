#include <frankencore/packages.hpp>

#include <cassert>
#include <string>

int main() {
    frankencore::packages::Inventory inventory;
    const auto result = frankencore::packages::to_json_checked(inventory);
    assert(!result.valid);
    assert(result.json.empty());
    assert(result.error.find("exhausted memory") != std::string::npos);
    return 0;
}
