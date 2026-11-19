#include <cassert>
#include <cstdint>
#include <limits>

// Include the implementation so this test can exercise the private parser
// without widening the public runtime API solely for test access.
#include "runtime.cpp"

namespace frankencore::runtime {

int runtime_memory_parser_test() {
    assert(parse_kibibytes("MemTotal:       4096 kB") == 4096U * 1024U);
    assert(parse_kibibytes("MemTotal:       4096 MB") == 0);
    assert(parse_kibibytes("MemTotal:       4junk kB") == 0);
    assert(parse_kibibytes("MemTotal:       ") == 0);
    assert(parse_kibibytes("MemTotal:       " +
                           std::to_string(std::numeric_limits<std::uint64_t>::max()) +
                           " kB") == std::numeric_limits<std::uint64_t>::max());
    const auto serialization = to_json_checked(Capabilities{});
    assert(!serialization.valid);
    assert(serialization.json.empty());
    assert(serialization.error.find("exhausted memory") != std::string::npos);
    return 0;
}

} // namespace frankencore::runtime

int main() { return frankencore::runtime::runtime_memory_parser_test(); }
