#include <flowcontracts/json.hpp>
#include <flowcontracts/diagnostics.hpp>
#include <flowcontracts/scheduling.hpp>

#include <cstdio>
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

std::string diagnostic_text(std::string_view value) {
    std::FILE* file = std::tmpfile();
    require(file != nullptr, "temporary diagnostic file unavailable");
    flowcontracts::write_json_string(file, value);
    require(std::fflush(file) == 0, "diagnostic flush failed");
    require(std::fseek(file, 0, SEEK_SET) == 0, "diagnostic rewind failed");
    std::string result;
    for (int character = std::fgetc(file); character != EOF; character = std::fgetc(file))
        result.push_back(static_cast<char>(character));
    std::fclose(file);
    return result;
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

    const auto neutral_schedule_value = parse(
        R"({"schedule_policy":"parallel_independent_v1","cancellation":"none","async":"none"})");
    const auto& neutral_schedule = object(neutral_schedule_value);
    require(!flowcontracts::scheduling_refusal(neutral_schedule, true),
            "admitted execution scheduling controls were refused");
    require(flowcontracts::scheduling_refusal(neutral_schedule, false)->request == "parallel_independent_v1",
            "planner accepted a requested stronger schedule");
    const auto required_cancellation_value = parse(R"({"cancellation":"required"})");
    const auto& required_cancellation = object(required_cancellation_value);
    require(flowcontracts::scheduling_refusal(required_cancellation, true)->request == "cancellation",
            "unknown cancellation request was ignored");
    const auto reentrant_value = parse(R"({"reentrancy":"requested"})");
    const auto& reentrant = object(reentrant_value);
    require(flowcontracts::scheduling_refusal(reentrant, true)->request == "reentrancy",
            "reentrant request was ignored");
    const auto wrong_control_type_value = parse(R"({"async":true})");
    const auto& wrong_control_type = object(wrong_control_type_value);
    rejects([&] { (void)flowcontracts::scheduling_refusal(wrong_control_type, true); }, "$.async");
    rejects([&] { (void)flowcontracts::scheduling_refusal(wrong_control_type, true, "$.providers[0]"); },
            "$.providers[0].async");

    require(diagnostic_text("quote\" slash\\ newline\n control\x01") ==
                "quote\\\" slash\\\\ newline\\n control\\u0001",
            "diagnostic escaping mismatch");
    std::string oversized(10000, 'x');
    const auto bounded = diagnostic_text(oversized);
    require(bounded.size() <= flowcontracts::max_diagnostic_bytes, "diagnostic bound exceeded");
    require(bounded.size() == flowcontracts::max_diagnostic_bytes && bounded.ends_with("..."),
            "diagnostic truncation mismatch");
    try { (void)parse('"' + bounded + '"'); }
    catch (const Error&) { require(false, "bounded diagnostic is not valid JSON string content"); }
    return 0;
}
