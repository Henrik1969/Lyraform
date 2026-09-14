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

    {
        std::ofstream output(path, std::ios::binary | std::ios::app);
        output << "{\"format\":\"frankencore.error_state_event\",\"event_id\":\"";
        output << generate_ulid() << "\"";
    }
    const auto incomplete = history.inspect();
    assert(!incomplete.valid);
    assert(incomplete.status == "incomplete");
    assert(incomplete.records == 2);
    assert(history.append(third).status == "incomplete");

    const auto repaired = history.repair_incomplete_tail();
    assert(repaired.valid);
    assert(repaired.changed);
    assert(repaired.status == "repaired");
    assert(std::filesystem::exists(repaired.quarantine_path));
    assert(history.inspect().status == "valid");
    assert(history.append(third).status == "appended");
    assert(history.inspect().records == 3);

    std::filesystem::remove_all(base);
    return 0;
}
