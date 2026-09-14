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
    const auto second = event("diagnosed");
    const auto third = event("resolved");
    assert(history.append(first).status == "appended");
    assert(history.append(first).status == "duplicate");

    auto conflict = first;
    conflict.diagnosis = "different content";
    assert(history.append(conflict).status == "conflict");
    assert(history.append(second).status == "appended");

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
