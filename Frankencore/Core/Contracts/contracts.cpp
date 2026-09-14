#include "frankencore/contracts.hpp"

namespace frankencore::contracts {
namespace {

constexpr std::size_t max_contract_text = 4096;
constexpr std::size_t max_contract_items = 100000;

ValidationResult valid() { return {true, {}}; }
ValidationResult invalid(const char* message) { return {false, message}; }
bool nonempty(const std::string& value) { return !value.empty(); }
bool bounded(const std::string& value) { return value.size() <= max_contract_text; }

bool valid_key_state(const std::string& value) {
    return value == "trusted" || value == "unknown" || value == "expired" ||
           value == "revoked" || value == "invalid" || value == "unavailable";
}

bool valid_integrity(const std::string& value) {
    return value == "matched" || value == "mismatched" || value == "absent" ||
           value == "not_applicable";
}

bool valid_authenticity(const std::string& value) {
    return value == "supplier_authenticated" || value == "owner_attested" ||
           value == "unverified" || value == "unknown";
}

bool valid_policy_outcome(const PolicyOutcome value) {
    switch (value) {
    case PolicyOutcome::allowed:
    case PolicyOutcome::allowed_with_isolation:
    case PolicyOutcome::requires_confirmation:
    case PolicyOutcome::quarantined:
    case PolicyOutcome::rejected:
    case PolicyOutcome::unresolved:
        return true;
    }
    return false;
}

bool valid_assurance(const std::string& value) {
    return value == "none" || value == "constrained" || value == "isolated" || value == "hardened";
}

bool valid_enforcement(const std::string& value) {
    return value == "self_report" || value == "locally_verified" ||
           value == "independently_verified" || value == "unavailable" || value == "unknown";
}

bool valid_failure_policy(const std::string& value) {
    return value == "diagnose_and_stop" || value == "allow_partial_targets";
}

} // namespace

const char* to_string(const PolicyOutcome outcome) {
    switch (outcome) {
    case PolicyOutcome::allowed: return "allowed";
    case PolicyOutcome::allowed_with_isolation: return "allowed_with_isolation";
    case PolicyOutcome::requires_confirmation: return "requires_confirmation";
    case PolicyOutcome::quarantined: return "quarantined";
    case PolicyOutcome::rejected: return "rejected";
    case PolicyOutcome::unresolved: return "unresolved";
    }
    return "unresolved";
}

ValidationResult validate(const VerificationEvidence& evidence) {
    const std::string* text_fields[] = {
        &evidence.artifact_identity, &evidence.artifact_version, &evidence.artifact_kind,
        &evidence.artifact_platform, &evidence.artifact_digest, &evidence.substrate_provider,
        &evidence.substrate_version, &evidence.substrate_method, &evidence.source, &evidence.signer,
        &evidence.key_state, &evidence.integrity, &evidence.authenticity,
    };
    for (const auto* field : text_fields)
        if (!bounded(*field)) return invalid("verification evidence field exceeds the 4096-byte limit");
    if (evidence.provenance.size() > max_contract_items || evidence.diagnostics.size() > max_contract_items)
        return invalid("verification evidence collection exceeds the 100000-entry limit");
    for (const auto* collection : {&evidence.provenance, &evidence.diagnostics})
        for (const auto& value : *collection)
            if (!bounded(value)) return invalid("verification evidence entry exceeds the 4096-byte limit");
    if (!nonempty(evidence.artifact_identity)) return invalid("artifact identity is required");
    if (!nonempty(evidence.substrate_provider)) return invalid("substrate provider is required");
    if (!nonempty(evidence.substrate_method)) return invalid("substrate method is required");
    if (!valid_key_state(evidence.key_state)) return invalid("invalid key state");
    if (!valid_integrity(evidence.integrity)) return invalid("invalid integrity state");
    if (!valid_authenticity(evidence.authenticity)) return invalid("invalid authenticity state");
    if (!valid_policy_outcome(evidence.policy_outcome)) return invalid("invalid policy outcome");
    if (evidence.policy_outcome == PolicyOutcome::allowed &&
        (evidence.key_state != "trusted" || evidence.integrity != "matched" ||
         (evidence.authenticity != "supplier_authenticated" &&
          evidence.authenticity != "owner_attested")))
        return invalid("allowed outcome requires trusted, integrity-matched, authenticated evidence");
    if (evidence.policy_outcome == PolicyOutcome::allowed_with_isolation &&
        evidence.integrity != "matched")
        return invalid("isolated allowance requires integrity-matched evidence");
    if (evidence.operator_override && evidence.authenticity == "owner_attested") {
        return invalid("operator override cannot become owner attestation");
    }
    return valid();
}

ValidationResult validate(const IsolationClaim& claim) {
    const std::pair<const char*, const std::string*> fields[] = {
        {"requested boundary", &claim.requested_boundary},
        {"assurance level", &claim.assurance_level},
        {"enforcement", &claim.enforcement},
        {"provider identity", &claim.provider_identity},
        {"provider version", &claim.provider_version},
        {"resources", &claim.resources},
        {"identity", &claim.identity},
        {"filesystem", &claim.filesystem},
        {"network", &claim.network},
        {"privilege", &claim.privilege},
        {"teardown", &claim.teardown},
        {"verification method", &claim.verification_method},
    };
    for (const auto& [name, value] : fields)
        if (!bounded(*value)) return invalid("isolation claim field exceeds the 4096-byte limit");
    for (const auto& [name, value] : fields)
        if (value->empty()) return invalid((std::string(name) + " is required").c_str());
    if (!valid_assurance(claim.assurance_level)) return invalid("invalid isolation assurance level");
    if (!valid_enforcement(claim.enforcement)) return invalid("invalid isolation enforcement state");
    if (claim.assurance_level == "none" && claim.enforcement != "self_report")
        return invalid("none isolation assurance requires self-report enforcement");
    if (claim.assurance_level == "constrained" &&
        claim.enforcement != "locally_verified" && claim.enforcement != "independently_verified")
        return invalid("constrained isolation requires local or independent verification");
    if ((claim.assurance_level == "isolated" || claim.assurance_level == "hardened") &&
        claim.enforcement != "independently_verified")
        return invalid("isolated or hardened claims require independent verification");
    return valid();
}

ValidationResult validate(const LanguageMap& map) {
    if (map.format != "frankencore.language-map") return invalid("invalid language-map format");
    if (map.version != 1) return invalid("unsupported language-map version");
    for (const auto* field : {&map.format, &map.id, &map.revision, &map.parser, &map.parent})
        if (!bounded(*field)) return invalid("language-map field exceeds the 4096-byte limit");
    if (!nonempty(map.id) || !nonempty(map.revision) || !nonempty(map.parser) ||
        !nonempty(map.parent)) return invalid("language-map identity fields are required");
    if (map.monikers.size() > max_contract_items) return invalid("language-map exceeds the 100000-entry limit");
    if (map.monikers.empty()) return invalid("language-map monikers are required");
    for (const auto& [canonical, aliases] : map.monikers) {
        if (canonical.empty() || aliases.empty()) return invalid("language-map contains an empty mapping");
        if (!bounded(canonical) || aliases.size() > max_contract_items)
            return invalid("language-map entry exceeds its contract bound");
        for (const auto& alias : aliases) {
            if (alias.empty()) return invalid("language-map contains an empty moniker");
            if (!bounded(alias)) return invalid("language-map moniker exceeds the 4096-byte limit");
        }
    }
    return valid();
}

ValidationResult validate(const ChainPolicy& policy) {
    if (policy.format != "frankencore.chain-policy") return invalid("invalid chain-policy format");
    if (policy.version != 1) return invalid("unsupported chain-policy version");
    for (const auto* field : {&policy.format, &policy.name, &policy.language_map, &policy.dialect,
                              &policy.profile, &policy.failure_policy})
        if (!bounded(*field)) return invalid("chain-policy field exceeds the 4096-byte limit");
    if (!nonempty(policy.name) || !nonempty(policy.language_map)) return invalid("chain-policy identity is required");
    if (policy.targets.empty()) return invalid("chain-policy requires a target");
    if (policy.prerequisites.size() > max_contract_items || policy.targets.size() > max_contract_items)
        return invalid("chain-policy collection exceeds the 100000-entry limit");
    if (!valid_failure_policy(policy.failure_policy)) return invalid("invalid chain failure policy");
    for (const auto& requirement : policy.prerequisites) {
        if (!nonempty(requirement.capability) || !nonempty(requirement.version)) return invalid("invalid prerequisite");
        if (!bounded(requirement.capability) || !bounded(requirement.version)) return invalid("prerequisite field exceeds the 4096-byte limit");
    }
    for (const auto& target : policy.targets) {
        if (!nonempty(target.name) || !nonempty(target.substrate) || !nonempty(target.version) ||
            !nonempty(target.optimizer) || !nonempty(target.lowering)) return invalid("invalid target profile");
        for (const auto* field : {&target.name, &target.substrate, &target.version, &target.optimizer, &target.lowering})
            if (!bounded(*field)) return invalid("target profile field exceeds the 4096-byte limit");
    }
    return valid();
}

ValidationResult validate(const FacadeInvocation& invocation) {
    if (invocation.format != "frankencore.facade_invocation") return invalid("invalid facade format");
    if (invocation.version != 1) return invalid("unsupported facade version");
    for (const auto* field : {&invocation.format, &invocation.facade, &invocation.backend,
                              &invocation.backend_version, &invocation.policy, &invocation.schema})
        if (!bounded(*field)) return invalid("facade field exceeds the 4096-byte limit");
    if (invocation.arguments.size() > max_contract_items || invocation.diagnostics.size() > max_contract_items ||
        invocation.provenance.size() > max_contract_items)
        return invalid("facade collection exceeds the 100000-entry limit");
    if (!nonempty(invocation.facade) || !nonempty(invocation.backend)) return invalid("facade and backend are required");
    return valid();
}

} // namespace frankencore::contracts
