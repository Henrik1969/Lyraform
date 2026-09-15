#pragma once

#include <string>

namespace frankencore::requirements {

struct VersionResult {
    bool valid = false;
    std::string diagnostic;
};

struct MatchResult {
    bool valid = false;
    bool matches = false;
    std::string diagnostic;
};

// Deliberately small initial grammar: dotted non-negative integers compared
// by whitespace-separated operators: >= <= > < =. Unknown syntax is rejected.
VersionResult validate_version(const std::string& version) noexcept;
MatchResult satisfies(const std::string& version, const std::string& expression) noexcept;

} // namespace frankencore::requirements
