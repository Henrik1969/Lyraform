#pragma once

#include <frankencore/contracts.hpp>

#include <string>
#include <vector>

namespace frankencore::policy {

enum class FactSource { internal_default, file, environment, cli, runtime };

struct Fact {
    std::string key;
    std::string value;
    FactSource source = FactSource::runtime;
    std::string source_name;
};

struct Decision {
    contracts::PolicyOutcome outcome = contracts::PolicyOutcome::unresolved;
    bool resolved = false;
    std::string diagnostics;
    std::string explanation;
};

// ConfigResolve is the default provider when this optional adapter is built.
// The provider receives facts and returns a normalized Frankencore decision;
// it does not invent policy semantics or bypass ConfigResolve.
Decision resolve_with_configresolve(const std::vector<Fact>& facts,
                                    const std::string& outcome_key) noexcept;

} // namespace frankencore::policy
