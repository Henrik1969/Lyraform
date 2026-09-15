#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace frankencore::contracts {

enum class PolicyOutcome {
    allowed,
    allowed_with_isolation,
    requires_confirmation,
    quarantined,
    rejected,
    unresolved
};

const char* to_string(PolicyOutcome outcome);

struct ValidationResult {
    bool valid = false;
    std::string error;
};

struct VerificationEvidence {
    std::string artifact_identity;
    std::string artifact_version;
    std::string artifact_kind;
    std::string artifact_platform;
    std::string artifact_digest;
    std::string substrate_provider;
    std::string substrate_version;
    std::string substrate_method;
    std::string source;
    std::string signer;
    std::string key_state;
    std::string integrity;
    std::string authenticity;
    bool operator_override = false;
    std::vector<std::string> provenance;
    std::vector<std::string> diagnostics;
    PolicyOutcome policy_outcome = PolicyOutcome::unresolved;
};

struct IsolationClaim {
    std::string requested_boundary;
    std::string assurance_level;
    std::string enforcement;
    std::string provider_identity;
    std::string provider_version;
    std::string resources;
    std::string identity;
    std::string filesystem;
    std::string network;
    std::string privilege;
    std::string teardown;
    std::string verification_method;
};

struct LanguageMap {
    std::string format = "frankencore.language-map";
    std::uint32_t version = 1;
    std::string id;
    std::string revision;
    std::string parser;
    std::string parent;
    std::map<std::string, std::vector<std::string>> monikers;
};

struct Requirement {
    std::string capability;
    std::string version;
};

struct TargetProfile {
    std::string name;
    std::string substrate;
    std::string version;
    std::string optimizer;
    std::string lowering;
};

struct ChainPolicy {
    std::string format = "frankencore.chain-policy";
    std::uint32_t version = 1;
    std::string name;
    std::string language_map;
    std::string dialect;
    std::string profile;
    std::vector<Requirement> prerequisites;
    std::vector<TargetProfile> targets;
    std::string failure_policy;
};

struct FacadeInvocation {
    std::string format = "frankencore.facade_invocation";
    std::uint32_t version = 1;
    std::string facade;
    std::string backend;
    std::string backend_version;
    std::vector<std::string> arguments;
    std::string policy;
    std::string schema;
    int exit_status = 0;
    PolicyOutcome policy_outcome = PolicyOutcome::unresolved;
    std::vector<std::string> diagnostics;
    std::vector<std::string> provenance;
};

ValidationResult validate(const VerificationEvidence& evidence);
ValidationResult validate(const IsolationClaim& claim);
ValidationResult validate(const LanguageMap& map);
ValidationResult validate(const ChainPolicy& policy);
ValidationResult validate(const FacadeInvocation& invocation);

// Non-throwing public validation boundary for language/runtime consumers.
ValidationResult validate_checked(const VerificationEvidence& evidence) noexcept;
ValidationResult validate_checked(const IsolationClaim& claim) noexcept;
ValidationResult validate_checked(const LanguageMap& map) noexcept;
ValidationResult validate_checked(const ChainPolicy& policy) noexcept;
ValidationResult validate_checked(const FacadeInvocation& invocation) noexcept;

} // namespace frankencore::contracts
