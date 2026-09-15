#include <frankencore/packages.hpp>

#include <cassert>
#include <string>

int main() {
    frankencore::packages::Inventory inventory;
    const auto read_result = frankencore::packages::read_dpkg_status_checked(
        "/path/that/does/not/exist");
    assert(!read_result.valid);
    assert(read_result.inventory.packages.empty());
    assert(read_result.error.find("exhausted memory") != std::string::npos);
    const auto result = frankencore::packages::to_json_checked(inventory);
    assert(!result.valid);
    assert(result.json.empty());
    assert(result.error.find("exhausted memory") != std::string::npos);
    return 0;
}
