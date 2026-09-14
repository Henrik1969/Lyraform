#include <frankencore/provenance.hpp>

#include <cassert>
#include <filesystem>
#include <fstream>

namespace {

frankencore::provenance::ErrorStateEvent event(const char* status) {
    using namespace frankencore::provenance;
    return {
        .error_state_id = generate_ulid(),
        .event_id = generate_ulid(),
        .attempt_id = generate_ulid(),
        .correlation_id = generate_ulid(),
        .status = status,
        .diagnosis = "controlled history test",
        .recovery = "operator review",
        .operator_action_required = true};
}

} // namespace

int main() {
    using namespace frankencore::provenance;
    const auto base = std::filesystem::temp_directory_path() /
                      "frankencore-error-state-history-test";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);
    const auto path = base / "history.jsonl";
    ErrorStateHistory history(path.string());

    const auto first = event("opened");
    auto second = event("diagnosed");
    auto third = event("resolved");
    second.error_state_id = first.error_state_id;
    third.error_state_id = first.error_state_id;
    assert(history.append(first).status == "appended");
    assert(history.append(first).status == "duplicate");

    auto conflict = first;
    conflict.diagnosis = "different content";
    assert(history.append(conflict).status == "conflict");
    assert(history.append(second).status == "appended");

    const auto lifecycle_path = base / "lifecycle.jsonl";
    ErrorStateHistory lifecycle(lifecycle_path.string());
    const auto lifecycle_opened = event("opened");
    auto lifecycle_diagnosed = lifecycle_opened;
    lifecycle_diagnosed.event_id = generate_ulid();
    lifecycle_diagnosed.status = "diagnosed";
    auto lifecycle_resolved = lifecycle_diagnosed;
    lifecycle_resolved.event_id = generate_ulid();
    lifecycle_resolved.status = "resolved";
    assert(lifecycle.append(lifecycle_opened).status == "appended");
    assert(lifecycle.append(lifecycle_diagnosed).status == "appended");
    assert(lifecycle.append(lifecycle_resolved).status == "appended");
    assert(lifecycle.inspect().status == "valid");
    auto lifecycle_illegal = lifecycle_resolved;
    lifecycle_illegal.event_id = generate_ulid();
    lifecycle_illegal.status = "diagnosed";
    assert(lifecycle.append(lifecycle_illegal).status == "rejected");

    MutationRecord mutation{
        .attempt = {.attempt_id = generate_ulid(), .correlation_id = generate_ulid(),
                    .entity_identity = "entity", .actor_identity = "actor",
                    .provider_identity = "provider", .authorizing_policy = "policy",
                    .operation = "update", .causes = {"controlled test"}},
        .event_id = generate_ulid(), .old_revision = 1, .new_revision = 2,
        .before_state_reference = "before", .after_state_reference = "after",
        .atomicity = "single-publication", .recoverability = "rollback",
        .rollback_reference = std::string{"rollback"}, .derived_entities = {},
        .before = {"old"}, .after = {"new"}};
    assert(history.append(mutation).status == "appended");
    auto stale_mutation = mutation;
    stale_mutation.event_id = generate_ulid();
    stale_mutation.old_revision = 1;
    stale_mutation.new_revision = 3;
    stale_mutation.before_state_reference = "wrong-before";
    stale_mutation.after_state_reference = "wrong-after";
    assert(history.append(stale_mutation).status == "rejected");
    MutationRejection rejection{
        .attempt = mutation.attempt, .event_id = generate_ulid(),
        .rejection_domain = "policy", .rejection_reason = "denied",
        .retryable = false, .observed_revision = 2};
    rejection.attempt.attempt_id = generate_ulid();
    assert(history.append(rejection).status == "appended");
    const auto readable = history.read_records();
    assert(readable.valid);
    assert(readable.records == 4);
    assert(readable.json_records.size() == 4);
    assert(readable.json_records.front().find("frankencore.error_state_event") != std::string::npos);
    const auto found = history.find_event(first.event_id);
    assert(found.valid && found.found && found.status == "found");
    assert(history.find_event(generate_ulid()).status == "not_found");
    assert(!history.find_event("not-a-ulid").valid);
    const auto bounded_path = base / "bounded.jsonl";
    ErrorStateHistory bounded(bounded_path.string(), 1024 * 1024, 1);
    assert(bounded.append(first).status == "exhausted");
    const auto bounded_inspection = bounded.inspect();
    assert(bounded_inspection.valid && bounded_inspection.records == 0);

    const auto malformed_path = base / "malformed.jsonl";
    {
        std::ofstream output(malformed_path, std::ios::binary);
        output << "{\"format\":\"frankencore.error_state_event\",\"version\":1,\"event_id\":\""
               << generate_ulid() << "\",\"status\":\"opened\",\"diagnosis\":\"unterminated\n";
    }
    ErrorStateHistory malformed(malformed_path.string());
    const auto malformed_inspection = malformed.inspect();
    assert(!malformed_inspection.valid);
    assert(malformed_inspection.status == "invalid");
    const auto invalid_path = base / "invalid.jsonl";
    {
        std::ofstream output(invalid_path, std::ios::binary);
        output << "{\"format\":\"frankencore.error_state_event\",\"version\":1,\"event_id\":\""
               << generate_ulid() << "\",\"status\":\"opened\",\"error_state_id\":\"bad\"}\n";
    }
    ErrorStateHistory invalid(invalid_path.string());
    const auto invalid_inspection = invalid.inspect();
    assert(!invalid_inspection.valid);
    assert(invalid_inspection.status == "invalid");

    {
        std::ofstream output(path, std::ios::binary | std::ios::app);
        output << "{\"format\":\"frankencore.error_state_event\",\"event_id\":\"";
        output << generate_ulid() << "\"";
    }
    const auto incomplete = history.inspect();
    assert(!incomplete.valid);
    assert(incomplete.status == "incomplete");
    assert(incomplete.records == 4);
    assert(history.append(third).status == "incomplete");

    const auto repaired = history.repair_incomplete_tail();
    assert(repaired.valid);
    assert(repaired.changed);
    assert(repaired.status == "repaired");
    assert(std::filesystem::exists(repaired.quarantine_path));
    assert(history.inspect().status == "valid");
    assert(history.read_records().json_records.size() == 4);
    assert(history.append(third).status == "appended");
    assert(history.inspect().records == 5);

    std::filesystem::remove_all(base);
    return 0;
}
