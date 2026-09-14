#include <frankencore/requirements.hpp>

#include <cassert>
#include <string>

int main() {
    const auto validation = frankencore::requirements::validate_version("1.0");
    assert(!validation.valid);
    assert(validation.diagnostic.find("validation failed") != std::string::npos);
    const auto matching = frankencore::requirements::satisfies("1.0", ">=1");
    assert(!matching.valid);
    assert(matching.diagnostic.find("evaluation failed") != std::string::npos);
    return 0;
}
