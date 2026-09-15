#include "flowparallel/cpu_execution.hpp"

#include <charconv>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

std::uint64_t mix(std::uint64_t value) {
    value ^= value >> 30;
    value *= 0xbf58476d1ce4e5b9ULL;
    value ^= value >> 27;
    value *= 0x94d049bb133111ebULL;
    return value ^ (value >> 31);
}

struct Options { unsigned workers = 2; unsigned tasks = 2; };

unsigned parse_unsigned(const char* text, const char* option) {
    unsigned value = 0;
    const std::string_view input{text};
    const auto parsed = std::from_chars(input.data(), input.data() + input.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != input.data() + input.size())
        throw std::runtime_error(std::string(option) + " requires a complete non-negative integer");
    return value;
}

Options parse_options(int argc, char** argv) {
    Options options;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--workers") {
            if (++index >= argc) throw std::runtime_error("--workers requires a value");
            options.workers = parse_unsigned(argv[index], "--workers");
        } else if (argument == "--tasks") {
            if (++index >= argc) throw std::runtime_error("--tasks requires a value");
            options.tasks = parse_unsigned(argv[index], "--tasks");
        } else if (argument == "-h" || argument == "--help" || argument == "-?") {
            std::cout << "flowparallel_execution_smoketest - approved-task CPU execution test\n\n"
                         "Options: --workers N --tasks N\n"
                         "         -h, -?, --help  show help\n"
                         "         -a, --about      show about information\n"
                         "         -v, --version    print the raw version number\n";
            std::exit(0);
        } else if (argument == "-a" || argument == "--about") { std::cout << "Tests execution of independent approved CPU tasks and failure propagation.\n"; std::exit(0); }
        else if (argument == "-v" || argument == "--version") { std::cout << "0.1.0\n"; std::exit(0); }
        else throw std::runtime_error("unknown option '" + argument + "'");
    }
    return options;
}

std::vector<flowparallel::cpu::Task> make_tasks(std::vector<std::uint64_t>& results, bool fail) {
    std::vector<flowparallel::cpu::Task> tasks;
    for (std::size_t task = 0; task < results.size(); ++task) tasks.push_back({[&, task, fail] {
        if (fail && task == 1) throw std::runtime_error("intentional smoke-test task failure");
        std::uint64_t value = 0;
        for (std::uint64_t index = task * 100000; index < (task + 1) * 100000; ++index) value += mix(index);
        results[task] = value;
    }});
    return tasks;
}

std::uint64_t total(const std::vector<std::uint64_t>& values) { std::uint64_t result = 0; for (const auto value : values) result += value; return result; }

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        if (options.tasks == 0) throw std::runtime_error("tasks must be greater than zero");
        const auto workers = options.workers;
        std::vector<std::uint64_t> serial_values(options.tasks), parallel_values(options.tasks);
        const auto serial = flowparallel::cpu::execute_independent(make_tasks(serial_values, false), 1);
        const auto parallel = flowparallel::cpu::execute_independent(make_tasks(parallel_values, false), workers);
        std::vector<std::uint64_t> failed_values(options.tasks);
        const auto failed = flowparallel::cpu::execute_independent(make_tasks(failed_values, true), workers);
        const auto long_failure = flowparallel::cpu::execute_independent(
            std::vector<flowparallel::cpu::Task>{{[] { throw std::runtime_error(std::string(4096, 'x')); }}}, 1);
        const auto invalid_workers = flowparallel::cpu::execute_independent({}, 0);
        const auto excessive_workers = flowparallel::cpu::execute_independent({}, flowparallel::cpu::max_workers + 1);
        const bool matching = serial.status == "ok" && parallel.status == "ok" && total(serial_values) == total(parallel_values);
        const bool failure_propagated = failed.status == "error" && failed.code == "TASK_FAILURE" && failed.disposition == "no_artifact" && failed.failed_task != static_cast<std::size_t>(-1);
        const bool long_failure_bounded = long_failure.status == "error" && long_failure.code == "TASK_FAILURE" && long_failure.error.size() == 255;
        const bool invalid_worker_rejected = invalid_workers.status == "error" && invalid_workers.code == "INVALID_WORKER_COUNT" && invalid_workers.disposition == "no_artifact";
        const bool excessive_worker_rejected = excessive_workers.status == "error" && excessive_workers.code == "WORKER_COUNT_EXCEEDED" && excessive_workers.disposition == "no_artifact";
        std::cout << "{\n  \"format\": \"flowparallel.cpu_execution_smoketest\",\n"
                     "  \"version\": 1,\n  \"status\": \"" << (matching && failure_propagated ? "ok" : "error") << "\",\n"
                     "  \"serial_result\": " << total(serial_values) << ",\n"
                     "  \"parallel_result\": " << total(parallel_values) << ",\n"
                     "  \"workers\": " << workers << ",\n"
                     "  \"correctness\": {\"matching_result\": " << (matching ? "true" : "false") << ", \"failure_propagated\": " << (failure_propagated ? "true" : "false") << ", \"long_failure_bounded\": " << (long_failure_bounded ? "true" : "false") << ", \"invalid_worker_rejected\": " << (invalid_worker_rejected ? "true" : "false") << ", \"excessive_worker_rejected\": " << (excessive_worker_rejected ? "true" : "false") << "}\n}\n";
        return matching && failure_propagated && long_failure_bounded && invalid_worker_rejected && excessive_worker_rejected ? 0 : 1;
    } catch (const std::exception& error) { std::cerr << "flowparallel_execution_smoketest error: " << error.what() << '\n'; return 2; }
}
