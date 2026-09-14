#include "frankencore/requirements.hpp"

#include <charconv>
#include <sstream>
#include <vector>

namespace frankencore::requirements {
namespace {

constexpr std::size_t max_version_bytes = 4096;
constexpr std::size_t max_version_components = 128;

struct ParsedVersion { std::vector<unsigned long long> parts; };

bool parse(const std::string& text, ParsedVersion& result) {
    if (text.empty() || text.size() > max_version_bytes) return false;
    std::size_t start = 0;
    while (start < text.size()) {
        if (result.parts.size() >= max_version_components) return false;
        const auto end = text.find('.', start);
        const auto stop = end == std::string::npos ? text.size() : end;
        if (stop == start) return false;
        unsigned long long value = 0;
        const auto conversion = std::from_chars(text.data() + start,
                                                 text.data() + stop, value);
        if (conversion.ec != std::errc{} || conversion.ptr != text.data() + stop) return false;
        result.parts.push_back(value);
        if (end == std::string::npos) break;
        start = end + 1;
    }
    return !result.parts.empty();
}

int compare(const ParsedVersion& left, const ParsedVersion& right) {
    const auto count = left.parts.size() > right.parts.size()
                           ? left.parts.size() : right.parts.size();
    for (std::size_t index = 0; index < count; ++index) {
        const auto lhs = index < left.parts.size() ? left.parts[index] : 0;
        const auto rhs = index < right.parts.size() ? right.parts[index] : 0;
        if (lhs < rhs) return -1;
        if (lhs > rhs) return 1;
    }
    return 0;
}

bool operator_matches(const int comparison, const std::string& operation) {
    if (operation == ">=") return comparison >= 0;
    if (operation == "<=") return comparison <= 0;
    if (operation == ">") return comparison > 0;
    if (operation == "<") return comparison < 0;
    if (operation == "=") return comparison == 0;
    return false;
}

} // namespace

VersionResult validate_version(const std::string& version) {
    ParsedVersion parsed;
    if (!parse(version, parsed)) return {false, "version must be dotted non-negative integers"};
    return {true, {}};
}

MatchResult satisfies(const std::string& version, const std::string& expression) {
    ParsedVersion actual;
    if (!parse(version, actual)) return {false, false, "invalid actual version"};
    if (expression.size() > max_version_bytes) return {false, false, "version range exceeds the 4096-byte limit"};
    std::istringstream input(expression);
    std::string token;
    bool found = false;
    while (input >> token) {
        std::string operation;
        std::string required_text;
        if (token.rfind(">=", 0) == 0 || token.rfind("<=", 0) == 0) {
            operation = token.substr(0, 2);
            required_text = token.substr(2);
        } else if (token.front() == '>' || token.front() == '<' || token.front() == '=') {
            operation = token.substr(0, 1);
            required_text = token.substr(1);
        } else {
            return {false, false, "version range requires comparison operators"};
        }
        ParsedVersion required;
        if (!parse(required_text, required)) return {false, false, "invalid required version"};
        found = true;
        if (!operator_matches(compare(actual, required), operation)) {
            return {true, false, {}};
        }
    }
    if (!found) return {false, false, "empty version range"};
    return {true, true, {}};
}

} // namespace frankencore::requirements
