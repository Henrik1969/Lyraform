#include "frankencore/provenance.hpp"

#include <cassert>
#include <string>

int main() {
    using namespace frankencore::provenance;
    const auto first_ulid = generate_ulid();
    const auto second_ulid = generate_ulid();
    assert(is_valid_ulid(first_ulid));
    assert(is_valid_ulid(second_ulid));
    assert(first_ulid < second_ulid);

    static_assert(noexcept(generate_ulid_checked()));
    const auto checked_ulid = generate_ulid_checked();
    assert(checked_ulid.valid);
    assert(is_valid_ulid(checked_ulid.value));

    const auto event_id = generate_ulid();
    MutationRecord record{
        .attempt = {
            .attempt_id = generate_ulid(),
            .correlation_id = generate_ulid(),
            .entity_identity = "test:entity",
            .actor_identity = "test.actor",
            .provider_identity = "test.provider",
            .authorizing_policy = "test-policy",
            .operation = "replace",
            .causes = {"unit-test"},
        },
        .event_id = event_id,
        .old_revision = 3,
        .new_revision = 4,
        .before_state_reference = "test:entity@3",
        .after_state_reference = "test:entity@4",
        .atomicity = "single-publication",
        .recoverability = "old-revision-addressable",
        .rollback_reference = std::string("test:entity@3"),
        .derived_entities = {},
        .before = {"quote \"safe\""},
        .after = {"current"},
    };
    assert(validate(record).valid);
    const auto json = to_json(record);
    assert(json.find("frankencore.mutation_record") != std::string::npos);
    assert(json.find(event_id) != std::string::npos);
    assert(json.find("quote \\\"safe\\\"") != std::string::npos);

    record.new_revision = record.old_revision;
    assert(!validate(record).valid);
    const auto invalid_json = to_json_checked(record);
    assert(!invalid_json.valid);
    assert(invalid_json.json.empty());
    assert(invalid_json.error.find("new_revision") != std::string::npos);

    MutationRejection rejection{
        .attempt = record.attempt,
        .event_id = generate_ulid(),
        .rejection_domain = "policy",
        .rejection_reason = "mutation not authorized",
        .retryable = false,
        .observed_revision = 3,
    };
    const auto rejected = to_json(rejection);
    assert(rejected.find("\"status\":\"rejected\"") != std::string::npos);
    assert(rejected.find("mutation not authorized") != std::string::npos);
    auto invalid_rejection = rejection;
    invalid_rejection.attempt.causes.clear();
    const auto invalid_rejection_json = to_json_checked(invalid_rejection);
    assert(!invalid_rejection_json.valid);
    assert(invalid_rejection_json.json.empty());
    assert(invalid_rejection_json.error.find("causes") != std::string::npos);

    const ErrorStateEvent error_state{
        .error_state_id = generate_ulid(),
        .event_id = generate_ulid(),
        .attempt_id = generate_ulid(),
        .correlation_id = record.attempt.correlation_id,
        .status = "resolved",
        .diagnosis = "incomplete final append",
        .recovery = "valid prefix restored",
        .operator_action_required = false,
    };
    const auto error_json = to_json(error_state);
    assert(error_json.find("frankencore.error_state_event") != std::string::npos);
    assert(error_json.find("valid prefix restored") != std::string::npos);
    auto invalid_error_state = error_state;
    invalid_error_state.status.clear();
    const auto invalid_error_json = to_json_checked(invalid_error_state);
    assert(!invalid_error_json.valid);
    assert(invalid_error_json.json.empty());
    assert(invalid_error_json.error.find("status") != std::string::npos);
    return 0;
}
