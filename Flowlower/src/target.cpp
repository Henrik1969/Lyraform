#include <flowcontracts/validate.hpp>
#include <flowcontracts/bounded_input.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace flowcontracts;

std::string read(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error("target policy is unavailable: " + path.string());
    return flowcontracts::read_bounded(file, "target policy");
}

bool valid_name(const std::string& name) {
    if (name.empty() || name.front() == '.' || name.back() == '.') return false;
    for (const unsigned char character : name)
        if (!(std::isalnum(character) || character == '-' || character == '_' || character == '.')) return false;
    return true;
}
}

int main(int argc, char** argv) {
    bool structured_diagnostics = false;
    try {
        std::vector<std::string> arguments;
        for (int index = 1; index < argc; ++index) {
            if (std::string(argv[index]) == "--diagnostics") {
                if (++index >= argc || std::string(argv[index]) != "json") throw std::runtime_error("--diagnostics requires json");
                structured_diagnostics = true;
            } else arguments.emplace_back(argv[index]);
        }
        if (arguments.size() == 1 && arguments[0] == "--version") { std::cout << "0.1.0\n"; return 0; }
        if (arguments.size() != 3 || arguments[0] != "--policy-root")
            throw std::runtime_error("usage: flowtarget --policy-root DIRECTORY TARGET-NAME");
        const std::string name = arguments[2];
        if (!valid_name(name)) throw std::runtime_error("invalid target policy name");
        const auto value = json::parse(read(std::filesystem::path(arguments[1]) / (name + ".json")));
        validate_target_policy(value);
        const auto& root = json::object(value);
        if (json::string(json::required(root, "name"), "$.name") != name)
            throw json::Error("$.name", "resolved policy identity does not match requested target name");
        std::cout << json::serialize(value) << '\n';
        return 0;
    } catch (const flowcontracts::json::Error& error) {
        if (structured_diagnostics) { std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWTARGET_CONTRACT_FAILURE\",\"stage\":\"contract\",\"message\":" << json::serialize(std::string(error.what())) << ",\"disposition\":\"no_artifact\"}\n"; return 1; }
        std::cerr << "flowtarget contract error: " << error.what() << '\n'; return 1;
    } catch (const std::exception& error) {
        if (structured_diagnostics) { std::cerr << "{\"status\":\"failed\",\"code\":\"FLOWTARGET_FAILURE\",\"stage\":\"cli\",\"message\":" << json::serialize(std::string(error.what())) << ",\"disposition\":\"no_artifact\"}\n"; return 1; }
        std::cerr << "flowtarget error: " << error.what() << '\n'; return 1;
    }
}
