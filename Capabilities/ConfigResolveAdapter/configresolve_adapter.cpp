#include "frankencore/policy.hpp"

#include <configresolve/configresolve.h>

#include <exception>
#include <new>

namespace frankencore::policy {
namespace {

configresolve_source_kind source_kind(const FactSource source) {
    switch (source) {
    case FactSource::internal_default: return CONFIGRESOLVE_SOURCE_INTERNAL_DEFAULT;
    case FactSource::file: return CONFIGRESOLVE_SOURCE_FILE;
    case FactSource::environment: return CONFIGRESOLVE_SOURCE_ENVIRONMENT;
    case FactSource::cli: return CONFIGRESOLVE_SOURCE_CLI;
    case FactSource::runtime: return CONFIGRESOLVE_SOURCE_RUNTIME;
    }
    return CONFIGRESOLVE_SOURCE_UNKNOWN;
}

contracts::PolicyOutcome outcome_from(const char* value) {
    if (value == nullptr) return contracts::PolicyOutcome::unresolved;
    const std::string text(value);
    if (text == "allowed") return contracts::PolicyOutcome::allowed;
    if (text == "allowed_with_isolation") return contracts::PolicyOutcome::allowed_with_isolation;
    if (text == "requires_confirmation") return contracts::PolicyOutcome::requires_confirmation;
    if (text == "quarantined") return contracts::PolicyOutcome::quarantined;
    if (text == "rejected") return contracts::PolicyOutcome::rejected;
    return contracts::PolicyOutcome::unresolved;
}

} // namespace

Decision resolve_with_configresolve(const std::vector<Fact>& facts,
                                    const std::string& outcome_key) noexcept {
    decltype(configresolve_create()) context = nullptr;
    try {
        Decision decision;
        context = configresolve_create();
        if (context == nullptr) {
            decision.diagnostics = "ConfigResolve context creation failed";
            return decision;
        }

        auto cleanup = [&] {
            configresolve_destroy(context);
            context = nullptr;
        };
        if (configresolve_require(context, outcome_key.c_str(),
                                  CONFIGRESOLVE_VALUE_STRING) != CONFIGRESOLVE_OK) {
            decision.diagnostics = "ConfigResolve could not require policy outcome";
            cleanup();
            return decision;
        }
        for (const char* value : {"allowed", "allowed_with_isolation",
                                  "requires_confirmation", "quarantined", "rejected"}) {
            if (configresolve_allowed_string(context, outcome_key.c_str(), value) != CONFIGRESOLVE_OK) {
                decision.diagnostics = "ConfigResolve could not register policy outcome vocabulary";
                cleanup();
                return decision;
            }
        }
        for (const auto& fact : facts) {
            if (configresolve_add_string(context, fact.key.c_str(), fact.value.c_str(),
                                         source_kind(fact.source), fact.source_name.c_str()) != CONFIGRESOLVE_OK) {
                decision.diagnostics = "ConfigResolve rejected a policy fact";
                cleanup();
                return decision;
            }
        }

        configresolve_result* result = nullptr;
        const auto status = configresolve_resolve(context, &result);
        if (status != CONFIGRESOLVE_OK || result == nullptr || !configresolve_result_ok(result)) {
            decision.diagnostics = result != nullptr && configresolve_result_diagnostics(result) != nullptr
                                      ? configresolve_result_diagnostics(result)
                                      : "ConfigResolve resolution failed";
            configresolve_result_destroy(result);
            cleanup();
            return decision;
        }

        const char* outcome = nullptr;
        if (configresolve_result_get_string(result, outcome_key.c_str(), &outcome) != CONFIGRESOLVE_OK) {
            decision.diagnostics = "ConfigResolve produced no policy outcome";
        } else {
            decision.outcome = outcome_from(outcome);
            decision.resolved = decision.outcome != contracts::PolicyOutcome::unresolved;
            const char* explanation = configresolve_result_explain(result, outcome_key.c_str());
            if (explanation != nullptr) decision.explanation = explanation;
            if (!decision.resolved) decision.diagnostics = "ConfigResolve returned an unknown policy outcome";
        }
        configresolve_result_destroy(result);
        cleanup();
        return decision;
    } catch (const std::bad_alloc&) {
        if (context != nullptr) configresolve_destroy(context);
        return {contracts::PolicyOutcome::unresolved, false,
                "ConfigResolve policy resolution exhausted memory", {}};
    } catch (const std::exception&) {
        if (context != nullptr) configresolve_destroy(context);
        return {contracts::PolicyOutcome::unresolved, false,
                "ConfigResolve policy resolution failed", {}};
    } catch (...) {
        if (context != nullptr) configresolve_destroy(context);
        return {contracts::PolicyOutcome::unresolved, false,
                "ConfigResolve policy resolution failed with unknown non-standard failure", {}};
    }
}

} // namespace frankencore::policy
