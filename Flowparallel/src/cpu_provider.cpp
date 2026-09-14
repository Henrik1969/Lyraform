#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unistd.h>

namespace {

constexpr std::string_view VERSION = "0.1.0";

struct Options { std::string plan_path; double observed_speedup = 0.0; double minimum_speedup = 1.25; unsigned requested_workers = 0; };

std::string read_input(const Options& options) {
    std::ostringstream input;
    if (!options.plan_path.empty()) { std::ifstream file(options.plan_path); if (!file) throw std::runtime_error("cannot open execution plan"); input << file.rdbuf(); }
    else input << std::cin.rdbuf();
    return input.str();
}

bool has_top_level_format(std::string_view input, std::string_view value) {
    const auto first = input.find("\"format\"");
    if (first == std::string_view::npos) return false;
    return input.find("\"format\": \"" + std::string(value) + "\"", first) == first ||
           input.find("\"format\":\"" + std::string(value) + "\"", first) == first;
}

bool has_field(std::string_view input, std::string_view field, std::string_view value) {
    return input.find("\"" + std::string(field) + "\": \"" + std::string(value) + "\"") != std::string_view::npos ||
           input.find("\"" + std::string(field) + "\":\"" + std::string(value) + "\"") != std::string_view::npos;
}

std::string quote(std::string_view value) { return "\"" + std::string(value) + "\""; }

int reject_unsupported_request(std::string_view request, std::string_view reason) {
    std::cout << "{\n  \"format\": \"flowparallel.cpu_selection\",\n"
                 "  \"version\": 1,\n  \"status\": \"unsupported\",\n"
                 "  \"request\": " << quote(request) << ",\n"
                 "  \"reason\": " << quote(reason) << ",\n"
                 "  \"fallback\": {\"emitted\": false}\n}\n";
    return 2;
}

std::uint64_t number_after(std::string_view input, std::string_view field) {
    const auto position = input.find("\"" + std::string(field) + "\":");
    if (position == std::string_view::npos) return 0;
    auto begin = position + field.size() + 3;
    while (begin < input.size() && (input[begin] == ' ' || input[begin] == '\t')) ++begin;
    std::uint64_t value = 0;
    while (begin < input.size() && input[begin] >= '0' && input[begin] <= '9') value = value * 10 + static_cast<unsigned>(input[begin++] - '0');
    return value;
}

Options parse(int argc, char** argv) {
    Options options;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        auto next = [&](const char* name) { if (++index >= argc) throw std::runtime_error(std::string(name) + " requires a value"); return std::string(argv[index]); };
        if (argument == "--plan") options.plan_path = next("--plan");
        else if (argument == "--observed-speedup") options.observed_speedup = std::stod(next("--observed-speedup"));
        else if (argument == "--minimum-speedup") options.minimum_speedup = std::stod(next("--minimum-speedup"));
        else if (argument == "--workers") options.requested_workers = static_cast<unsigned>(std::stoul(next("--workers")));
        else if (argument == "-h" || argument == "--help" || argument == "-?") { std::cout << "flowparallel_cpu - policy-resolved CPU provider selection\n\nUsage: flowparallel_cpu [--plan plan.json] [--observed-speedup N]\n\nOptions: -h, -?, --help  show help\n         -a, --about    show about information\n         -v, --version  print the raw version number\n"; std::exit(0); }
        else if (argument == "-a" || argument == "--about") { std::cout << "Flowparallel CPU provider selects serial or thread-pool execution from a plan, runtime capacity, and policy.\n"; std::exit(0); }
        else if (argument == "-v" || argument == "--version") { std::cout << VERSION << '\n'; std::exit(0); }
        else throw std::runtime_error("unknown option '" + argument + "'");
    }
    return options;
}

int resolve(const std::string& plan, const Options& options) {
    if (!has_top_level_format(plan, "flowparallel.execution_plan")) throw std::runtime_error("input is not a Flowparallel execution plan");
    if (has_field(plan, "schedule_policy", "parallel_effectful_v1"))
        return reject_unsupported_request("parallel_effectful_v1", "effectful parallel scheduling is not admitted");
    if (has_field(plan, "cancellation", "requested"))
        return reject_unsupported_request("cancellation", "cancellation is not admitted by this provider");
    if (has_field(plan, "async", "requested"))
        return reject_unsupported_request("async", "asynchronous execution is not admitted by this provider");
    if (has_field(plan, "backpressure", "requested"))
        return reject_unsupported_request("backpressure", "backpressure is not admitted by this provider");
    if (!has_field(plan, "status", "ready")) { std::cout << "{\n  \"format\": \"flowparallel.cpu_selection\",\n  \"version\": 1,\n  \"status\": \"blocked\",\n  \"reason\": \"execution plan is not ready\"\n}\n"; return 2; }
    const auto candidates = number_after(plan, "parallel_candidates");
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
    try { const auto options = parse(argc, argv); return resolve(read_input(options), options); }
    catch (const std::exception& error) { std::cerr << "flowparallel_cpu error: " << error.what() << '\n'; return 1; }
}
