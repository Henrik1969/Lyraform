#include <algorithm>
#include <charconv>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <flowcontracts/artifacts.hpp>
#include <flowparallel/bounded_input.hpp>
#include <flowparallel/diagnostics.hpp>
#include <fstream>
#include <iostream>
#include <cmath>
#include <new>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unistd.h>

namespace {

constexpr std::string_view VERSION = "0.1.0";

void write_structured_failure(std::string_view code, std::string_view stage, std::string_view message) noexcept {
    std::fputs("{\"status\":\"failed\",\"code\":\"", stderr);
    flowparallel::write_json_string(stderr, code);
    std::fputs("\",\"stage\":\"", stderr);
    flowparallel::write_json_string(stderr, stage);
    std::fputs("\",\"message\":\"", stderr);
    flowparallel::write_json_string(stderr, message);
    std::fputs("\",\"disposition\":\"no_artifact\"}\n", stderr);
}

struct Options { std::string plan_path; double observed_speedup = 0.0; double minimum_speedup = 1.25; unsigned requested_workers = 0; bool structured_diagnostics = false; };

std::string read_input(const Options& options) {
    if (!options.plan_path.empty()) { std::ifstream file(options.plan_path); if (!file) throw std::runtime_error("cannot open execution plan"); return flowparallel::read_bounded(file, "execution plan"); }
    return flowparallel::read_bounded(std::cin, "execution plan");
}

std::string quote(std::string_view value) { return "\"" + std::string(value) + "\""; }

double parse_number(std::string_view text, const char* option) {
    std::size_t consumed = 0;
    double value = 0.0;
    try { value = std::stod(std::string(text), &consumed); }
    catch (...) { throw std::runtime_error(std::string(option) + " requires a complete number"); }
    if (consumed != text.size() || !std::isfinite(value)) throw std::runtime_error(std::string(option) + " requires a complete finite number");
    return value;
}

unsigned parse_unsigned(std::string_view text, const char* option) {
    unsigned value = 0;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size()) throw std::runtime_error(std::string(option) + " requires a complete non-negative integer");
    return value;
}

int reject_unsupported_request(std::string_view request, std::string_view reason) {
    std::cout << "{\n  \"format\": \"flowparallel.cpu_selection\",\n"
                 "  \"version\": 1,\n  \"status\": \"unsupported\",\n"
                 "  \"request\": " << quote(request) << ",\n"
                 "  \"reason\": " << quote(reason) << ",\n"
                 "  \"fallback\": {\"emitted\": false}\n}\n";
    return 2;
}

Options parse(int argc, char** argv) {
    Options options;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        auto next = [&](const char* name) { if (++index >= argc) throw std::runtime_error(std::string(name) + " requires a value"); return std::string(argv[index]); };
        if (argument == "--plan") options.plan_path = next("--plan");
        else if (argument == "--observed-speedup") options.observed_speedup = parse_number(next("--observed-speedup"), "--observed-speedup");
        else if (argument == "--minimum-speedup") options.minimum_speedup = parse_number(next("--minimum-speedup"), "--minimum-speedup");
        else if (argument == "--workers") options.requested_workers = parse_unsigned(next("--workers"), "--workers");
        else if (argument == "--diagnostics") { if (next("--diagnostics") != "json") throw std::runtime_error("--diagnostics requires json"); options.structured_diagnostics = true; }
        else if (argument == "-h" || argument == "--help" || argument == "-?") { std::cout << "flowparallel_cpu - policy-resolved CPU provider selection\n\nUsage: flowparallel_cpu [--plan plan.json] [--observed-speedup N]\n\nOptions: -h, -?, --help  show help\n         -a, --about    show about information\n         -v, --version  print the raw version number\n"; std::exit(0); }
        else if (argument == "-a" || argument == "--about") { std::cout << "Flowparallel CPU provider selects serial or thread-pool execution from a plan, runtime capacity, and policy.\n"; std::exit(0); }
        else if (argument == "-v" || argument == "--version") { std::cout << VERSION << '\n'; std::exit(0); }
        else throw std::runtime_error("unknown option '" + argument + "'");
    }
    return options;
}

int resolve(const std::string& plan, const Options& options) {
#ifdef FLOWPARALLEL_CPU_TEST_ALLOCATION_FAILURE
    (void)plan;
    (void)options;
    throw std::bad_alloc();
#endif
    using namespace flowcontracts::json;
    const auto root = object(flowcontracts::json::parse(plan));
    if (string(required(root, "format"), "$.format") != "flowparallel.execution_plan")
        throw Error("$.format", "input is not a Flowparallel execution plan");
    if (integer(required(root, "version"), "$.version") != 1)
        throw Error("$.version", "unsupported Flowparallel execution plan version");
    const auto has_field = [&](std::string_view field, std::string_view value) {
        const auto* item = optional(root, field);
        return item != nullptr && string(*item, std::string("$.") + std::string(field)) == value;
    };
    if (const auto refusal = flowcontracts::scheduling_refusal(root, true))
        return reject_unsupported_request(refusal->request, refusal->reason);
    if (!has_field("status", "ready")) { std::cout << "{\n  \"format\": \"flowparallel.cpu_selection\",\n  \"version\": 1,\n  \"status\": \"blocked\",\n  \"reason\": \"execution plan is not ready\"\n}\n"; return 2; }
    const auto& dependency = object(required(root, "dependency_analysis"), "$.dependency_analysis");
    std::uint64_t candidates = 0;
    if (const auto* item = optional(dependency, "parallel_candidates"); item != nullptr) {
        const auto value = integer(*item, "$.dependency_analysis.parallel_candidates");
        if (value < 0) throw Error("$.dependency_analysis.parallel_candidates", "must not be negative");
        candidates = static_cast<std::uint64_t>(value);
    }
    const long local_processors = sysconf(_SC_NPROCESSORS_ONLN);
    const unsigned available = local_processors > 0 ? static_cast<unsigned>(local_processors) : 1;
    const unsigned requested = options.requested_workers == 0 ? available : options.requested_workers;
    const unsigned workers = candidates == 0 ? 1 : std::max(1U, std::min({available, requested, static_cast<unsigned>(candidates)}));
    const bool worthwhile = workers > 1 && options.observed_speedup >= options.minimum_speedup;
    const auto provider = worthwhile ? "cpu.threadpool" : "cpu.serial";
    std::cout << "{\n  \"format\": \"flowparallel.cpu_selection\",\n"
                 "  \"version\": 1,\n  \"status\": \"ready\",\n"
                 "  \"provider\": " << quote(provider) << ",\n"
                 "  \"workers\": " << (worthwhile ? workers : 1) << ",\n"
                 "  \"available_logical_processors\": " << available << ",\n"
                 "  \"parallel_candidates\": " << candidates << ",\n"
                 "  \"observed_speedup\": " << options.observed_speedup << ",\n"
                 "  \"minimum_speedup\": " << options.minimum_speedup << ",\n"
                 "  \"decision\": " << quote(worthwhile ? "parallel" : "serial") << ",\n"
                 "  \"execution\": \"not-performed\"\n}\n";
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    bool structured_diagnostics = false;
    try {
        for (int index = 1; index + 1 < argc; ++index)
            if (std::strcmp(argv[index], "--diagnostics") == 0 && std::strcmp(argv[index + 1], "json") == 0)
                structured_diagnostics = true;
        const auto options = parse(argc, argv);
        return resolve(read_input(options), options);
    } catch (const std::bad_alloc&) {
        if (structured_diagnostics) write_structured_failure("FLOWPARALLEL_CPU_RESOURCE_EXHAUSTED", "runtime", "allocation failed");
        else std::cerr << "flowparallel_cpu error: allocation failed\n";
        return 1;
    } catch (const flowcontracts::json::Error& error) {
        if (structured_diagnostics) write_structured_failure("FLOWPARALLEL_CPU_CONTRACT_FAILURE", "contract", error.what());
        else std::cerr << "flowparallel_cpu contract error: " << error.what() << '\n';
        return 1;
    } catch (const std::exception& error) {
        if (structured_diagnostics) write_structured_failure("FLOWPARALLEL_CPU_INPUT_INVALID", "input", error.what());
        else std::cerr << "flowparallel_cpu input error: " << error.what() << '\n';
        return 1;
    } catch (...) {
        if (structured_diagnostics) write_structured_failure("FLOWPARALLEL_CPU_UNKNOWN_FAILURE", "runtime", "unknown non-standard failure");
        else std::cerr << "flowparallel_cpu error: unknown non-standard failure\n";
        return 1;
    }
}
