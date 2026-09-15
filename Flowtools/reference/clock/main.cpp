#include "../diagnostics.hpp"

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <time.h>

namespace {

constexpr std::string_view VERSION = "0.1.0";

struct ClockReading {
    clockid_t clock_id;
    std::string name;
};

void write_structured_failure(std::string_view code, std::string_view message) {
    std::fputs("{\"status\":\"failed\",\"code\":\"", stderr);
    flowtools::reference::write_json_string(stderr, code);
    std::fputs("\",\"message\":\"", stderr);
    flowtools::reference::write_json_string(stderr, message);
    std::fputs("\",\"disposition\":\"no_artifact\"}\n", stderr);
}

ClockReading select_clock(std::string_view name) {
    if (name == "monotonic") return {CLOCK_MONOTONIC, "monotonic"};
    if (name == "realtime") return {CLOCK_REALTIME, "realtime"};
    throw std::runtime_error("unknown clock '" + std::string(name) + "'");
}

std::int64_t nanoseconds(const timespec& value) {
    return static_cast<std::int64_t>(value.tv_sec) * 1000000000LL + value.tv_nsec;
}

int run(std::string_view clock_name) {
    const auto selected = select_clock(clock_name);
    timespec value{};
    timespec resolution{};
    if (clock_gettime(selected.clock_id, &value) != 0) {
        const int error = errno;
        std::cout << "{\"format\":\"frankencore.clock_report\",\"version\":1,"
                     "\"status\":\"error\",\"error\":{\"domain\":\"linux.clock\","
                     "\"code\":" << error << ",\"message\":\"" << std::strerror(error)
                  << "\",\"recoverable\":true}}\n";
        return 2;
    }
    if (clock_getres(selected.clock_id, &resolution) != 0) {
        const int error = errno;
        std::cout << "{\"format\":\"frankencore.clock_report\",\"version\":1,"
                     "\"status\":\"error\",\"error\":{\"domain\":\"linux.clock\","
                     "\"code\":" << error << ",\"message\":\"" << std::strerror(error)
                  << "\",\"recoverable\":true}}\n";
        return 2;
    }
    std::cout << "{\"format\":\"frankencore.clock_report\",\"version\":1,"
                 "\"status\":\"ok\",\"type\":\"Clock\",\"identity\":\"clock:"
              << selected.name << "\",\"properties\":{\"clock\":\"" << selected.name
              << "\",\"resolution_ns\":" << nanoseconds(resolution)
              << "},\"capabilities\":[{\"id\":\"frankencore.clock.read\","
                 "\"version\":1}],\"provider\":{\"id\":\"linux.clock_gettime\","
                 "\"backend\":\"clock_gettime\"},\"policy\":{\"decision\":\"allow\","
                 "\"reason\":\"reference-default\"},\"value_ns\":"
              << nanoseconds(value) << "}\n";
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    bool structured_diagnostics = false;
    try {
        std::string clock_name = "monotonic";
        for (int index = 1; index < argc; ++index) {
            const std::string argument = argv[index];
            if (argument == "--clock") {
                if (++index >= argc) throw std::runtime_error("--clock requires monotonic or realtime");
                clock_name = argv[index];
            } else if (argument == "--diagnostics") {
                if (++index >= argc || std::string_view(argv[index]) != "json")
                    throw std::runtime_error("--diagnostics requires json");
                structured_diagnostics = true;
            } else if (argument == "-h" || argument == "-?" || argument == "--help") {
                std::cout << "frankencore_clock - expose a typed Clock reference object\n\n"
                             "Usage: frankencore_clock [--clock monotonic|realtime]\n"
                             "More help: Flowtools/reference/clock/README.md\n";
                return 0;
            } else if (argument == "-a" || argument == "--about") {
                std::cout << "Reference Frankencore Clock provider using a Linux adapter.\n"
                             "More help: Flowtools/reference/clock/README.md\n";
                return 0;
            } else if (argument == "-v" || argument == "--version") {
                std::cout << VERSION << '\n';
                return 0;
            } else {
                throw std::runtime_error("unknown option '" + argument + "'");
            }
        }
        return run(clock_name);
    } catch (const std::exception& error) {
        if (structured_diagnostics) write_structured_failure("FRANKENCORE_CLOCK_FAILURE", error.what());
        else std::cerr << "frankencore_clock error: " << error.what() << '\n';
        return 1;
    } catch (...) {
        if (structured_diagnostics) write_structured_failure("FRANKENCORE_CLOCK_UNKNOWN_FAILURE", "unknown non-standard failure");
        else std::cerr << "frankencore_clock error: unknown non-standard failure\n";
        return 1;
    }
}
