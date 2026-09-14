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

struct HistoryResult {
    bool valid = false;
    bool changed = false;
    std::size_t records = 0;
    std::string status;
    std::string error;
    std::string quarantine_path;
};

struct HistoryReadResult {
    bool valid = false;
    std::size_t records = 0;
    std::vector<std::string> json_records;
    std::string status;
    std::string error;
};

struct HistoryLookupResult {
    bool valid = false;
    bool found = false;
    std::string json;
    std::string status;
    std::string error;
};

struct HistoryReconciliationResult {
    bool valid = false;
    std::size_t left_records = 0;
    std::size_t right_records = 0;
    std::size_t common_events = 0;
    std::size_t left_only_events = 0;
    std::size_t right_only_events = 0;
    std::size_t conflicting_events = 0;
    std::string status;
    std::string error;
};

struct MutationReplayState {
    std::string entity_identity;
    std::uint64_t revision = 0;
    std::string state_reference;
};

struct MutationReplayResult {
    bool valid = false;
    std::size_t records = 0;
    std::vector<MutationReplayState> states;
    std::string status;
    std::string error;
};

// Project-local append-only error-state history. The file is JSONL with one
// complete ErrorStateEvent per line; callers must explicitly repair a torn
// final line before appending again.
class ErrorStateHistory {
public:
    explicit ErrorStateHistory(std::string path, std::size_t max_line_bytes = 1024 * 1024,
                               std::size_t max_history_bytes = 64 * 1024 * 1024);

    HistoryResult inspect() const noexcept;
    HistoryReadResult read_records() const noexcept;
    HistoryLookupResult find_event(const std::string& event_id) const noexcept;
    HistoryResult append(const MutationRecord& record) const noexcept;
    HistoryResult append(const MutationRejection& rejection) const noexcept;
    HistoryResult append(const ErrorStateEvent& event) const noexcept;
    HistoryResult repair_incomplete_tail() const noexcept;
    MutationReplayResult replay_mutations() const noexcept;

private:
    std::string path_;
    std::size_t max_line_bytes_;
    std::size_t max_history_bytes_;
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

// Read-only comparison of two independently produced project-local histories.
// Both inputs are validated before comparison; neither file is rewritten.
HistoryReconciliationResult reconcile_histories(const std::string& left_path,
                                                const std::string& right_path) noexcept;

} // namespace frankencore::provenance
