#pragma once

#include <frankencore/contracts.hpp>

#include <string>

namespace frankencore::language {

struct Resolution {
    bool resolved = false;
    std::string input;
    std::string canonical;
    std::string diagnostic;
};

// Resolve a local moniker against one already validated language map.
Resolution resolve_moniker(const contracts::LanguageMap& map,
                           const std::string& input) noexcept;

} // namespace frankencore::language
