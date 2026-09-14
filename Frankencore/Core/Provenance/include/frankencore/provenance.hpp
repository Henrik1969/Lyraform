#pragma once

#include <array>
#include <cstdint>
#include <mutex>
#include <optional>
#include <random>
#include <string>
#include <vector>

namespace frankencore::provenance {

using Ulid = std::string;

class UlidGenerator {
public:
    UlidGenerator();
    Ulid generate();

private:
    std::mutex mutex_;
    std::uint64_t last_timestamp_ms_ = 0;
    std::array<std::uint8_t, 10> last_random_{};
    std::random_device random_device_;
};

Ulid generate_ulid();
bool is_valid_ulid(const std::string& value);

struct StateEvidence {
    std::string value;
};

struct MutationAttempt {
    std::string attempt_id;
    std::string correlation_id;
    std::string entity_identity;
    std::string actor_identity;
    std::string provider_identity;
    std::string authorizing_policy;
    std::string operation;
    std::vector<std::string> causes;
};

struct MutationRecord {
    MutationAttempt attempt;
    std::string event_id;
    std::uint64_t old_revision = 0;
    std::uint64_t new_revision = 0;
    std::string before_state_reference;
    std::string after_state_reference;
    std::string atomicity;
    std::string recoverability;
    std::optional<std::string> rollback_reference;
    std::vector<std::string> derived_entities;
    StateEvidence before;
    StateEvidence after;
};

struct MutationRejection {
    MutationAttempt attempt;
    std::string event_id;
    std::string rejection_domain;
    std::string rejection_reason;
    bool retryable = false;
    std::optional<std::uint64_t> observed_revision;
};

struct ErrorStateEvent {
    std::string error_state_id;
    std::string event_id;
    std::string attempt_id;
    std::string correlation_id;
    std::string status;
    std::string diagnosis;
    std::string recovery;
    bool operator_action_required = false;
};

struct ValidationResult {
    bool valid = false;
    std::string error;
};

struct JsonResult {
    bool valid = false;
    std::string json;
    std::string error;
};

ValidationResult validate(const MutationRecord& record);

// JSON is an inspectable projection of the canonical C++ record.
std::string to_json(const MutationRecord& record);
std::string to_json(const MutationRejection& rejection);
std::string to_json(const ErrorStateEvent& event);

// Non-throwing public serialization boundary for language/runtime consumers.
JsonResult to_json_checked(const MutationRecord& record);
JsonResult to_json_checked(const MutationRejection& rejection);
JsonResult to_json_checked(const ErrorStateEvent& event);

} // namespace frankencore::provenance
