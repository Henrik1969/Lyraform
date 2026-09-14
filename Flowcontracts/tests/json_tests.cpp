#include <flowcontracts/json.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

namespace {
void require(bool condition, const char* message) {
    if (!condition) { std::cerr << "flowcontracts test failed: " << message << '\n'; std::exit(1); }
}
template <class Callable> void rejects(Callable callable, std::string_view path) {
    try { callable(); } catch (const flowcontracts::json::Error& error) {
        require(error.path() == path, "diagnostic path mismatch"); return;
    }
    require(false, "hostile JSON was accepted");
}
template <class Callable> void rejects_any(Callable callable) {
    try { callable(); } catch (const flowcontracts::json::Error&) { return; }
    require(false, "bounded JSON hostile input was accepted");
}
}

int main() {
    using namespace flowcontracts::json;
    const auto parsed = parse(" { \"z\" : [true,null,-7,1.5], \"a\":\"A\\u00df\\u6771\\ud834\\udd1e\" } ");
    const auto canonical = serialize(parsed);
    require(canonical == "{\"a\":\"Aß東𝄞\",\"z\":[true,null,-7,1.5]}", "canonical serialization mismatch");
    require(serialize(parse(canonical)) == canonical, "canonical round trip is unstable");
    rejects([] { (void)parse("{\"format\":1,\"format\":2}"); }, "$.format");
    rejects([] { (void)parse("[1] trailing"); }, "$");
    rejects([] { (void)parse("9223372036854775808"); }, "$");
    rejects([] { (void)parse("01"); }, "$");
    rejects([] { (void)parse("\"\\ud800\""); }, "$");
    std::string deep;
    for (int index = 0; index < 257; ++index) deep += '[';
    deep += '0';
    for (int index = 0; index < 257; ++index) deep += ']';
    rejects_any([&] { (void)parse(deep); });
    std::string wide_array = "[";
    for (int index = 0; index < 100001; ++index) { if (index != 0) wide_array += ','; wide_array += '0'; }
    wide_array += ']';
    rejects_any([&] { (void)parse(wide_array); });
    std::string many_nodes = "[";
    for (int index = 0; index < 500001; ++index) { if (index != 0) many_nodes += ','; many_nodes += "[0]"; }
    many_nodes += ']';
    rejects_any([&] { (void)parse(many_nodes); });
    const auto& root = object(parsed);
    rejects([&] { (void)required(root, "missing"); }, "$.missing");
    rejects([&] { (void)integer(required(root, "a"), "$.a"); }, "$.a");
    return 0;
}
