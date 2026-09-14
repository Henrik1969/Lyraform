#include <flowcontracts/validate.hpp>
#include <flowcontracts/bounded_input.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {
std::string read(std::string_view path) {
    if (path.empty() || path == "-") return flowcontracts::read_bounded(std::cin, "input");
    std::ifstream file{std::string(path)}; if (!file) throw std::runtime_error("cannot open input");
    return flowcontracts::read_bounded(file, "input");
}
int exit_code(flowcontracts::ValidationClass value) {
    switch (value) {
        case flowcontracts::ValidationClass::valid: return 0;
        case flowcontracts::ValidationClass::invalid: return 1;
        case flowcontracts::ValidationClass::blocked: return 2;
        case flowcontracts::ValidationClass::unsupported: return 3;
    }
    return 1;
}
}

int main(int argc, char** argv) {
    bool structured_diagnostics = false;
    try {
        bool canonical = false, human = false; std::string path;
        for (int index = 1; index < argc; ++index) {
            const std::string argument = argv[index];
            if (argument == "--canonical") canonical = true;
            else if (argument == "--human") human = true;
            else if (argument == "--diagnostics") {
                if (++index >= argc || std::string(argv[index]) != "json") throw std::runtime_error("--diagnostics requires json");
                structured_diagnostics = true;
            }
            else if (!argument.empty() && argument.front() == '-') throw std::runtime_error("unknown option '" + argument + "'");
            else if (path.empty()) path = argument;
            else throw std::runtime_error("too many input paths");
        }
        const auto value = flowcontracts::json::parse(read(path));
        const auto result = flowcontracts::validate(value);
        if (canonical && result.classification == flowcontracts::ValidationClass::valid) std::cout << flowcontracts::json::serialize(value) << '\n';
        else if (human) std::cout << flowcontracts::name(result.classification) << ": " << result.format << " v" << result.version << " " << result.path << ": " << result.reason << (result.source.empty() ? "" : " [source: " + result.source + "]") << '\n';
        else std::cout << flowcontracts::json::serialize(flowcontracts::json::Object{
            {"classification", std::string(flowcontracts::name(result.classification))}, {"format", result.format},
            {"path", result.path}, {"reason", result.reason}, {"source", result.source}, {"version", result.version}}) << '\n';
        return exit_code(result.classification);
    } catch (const flowcontracts::json::Error& error) {
        if (structured_diagnostics) {
            std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWVALIDATE_CONTRACT_FAILURE\",\"stage\":\"contract\",\"message\":"
                      << flowcontracts::json::serialize(std::string(error.what()))
                      << ",\"disposition\":\"no_artifact\"}\n";
            return 1;
        }
        std::cout << flowcontracts::json::serialize(flowcontracts::json::Object{{"classification", "invalid"}, {"format", ""}, {"path", error.path()}, {"reason", error.reason()}, {"version", 0}}) << '\n';
        return 1;
    } catch (const std::exception& error) {
        if (structured_diagnostics) {
            std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWVALIDATE_FAILURE\",\"stage\":\"cli\",\"message\":"
                      << flowcontracts::json::serialize(std::string(error.what()))
                      << ",\"disposition\":\"no_artifact\"}\n";
            return 1;
        }
        std::cerr << "flowvalidate: " << error.what() << '\n'; return 1;
    } catch (...) {
        if (structured_diagnostics) {
            std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWVALIDATE_UNKNOWN_FAILURE\",\"stage\":\"cli\",\"message\":\"unknown non-standard failure\",\"disposition\":\"no_artifact\"}\n";
            return 1;
        }
        std::cerr << "flowvalidate: unknown non-standard failure\n"; return 1;
    }
}
