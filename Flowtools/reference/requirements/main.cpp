#include <frankencore/requirements.hpp>

#include <cassert>
#include <string>
#include <type_traits>
#include <utility>

int main() {
    static_assert(noexcept(frankencore::requirements::validate_version(
        std::declval<const std::string&>())));
    static_assert(noexcept(frankencore::requirements::satisfies(
        std::declval<const std::string&>(), std::declval<const std::string&>())));
    using namespace frankencore::requirements;
    assert(validate_version("2.8.3").valid);
    assert(!validate_version("v2").valid);
    assert(satisfies("2.8.3", ">=2 <3").valid);
    assert(satisfies("2.8.3", ">=2 <3").matches);
    assert(satisfies("3.0", ">=2 <3").valid);
    assert(!satisfies("3.0", ">=2 <3").matches);
    assert(!satisfies("2.8", "latest").valid);
    assert(!validate_version(std::string(4097, '1')).valid);
    assert(!satisfies("1", std::string(4097, ' ')).valid);
    std::string too_many_components = "1";
    for (int index = 0; index < 128; ++index) too_many_components += ".1";
    assert(!validate_version(too_many_components).valid);
    return 0;
}
