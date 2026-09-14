#include <frankencore/provenance.hpp>

#include <cassert>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <fcntl.h>
#include <iterator>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {

bool fail_fsync = false;
bool fail_parent_fsync = false;
bool fail_write_after_partial = false;
bool fail_zero_write_after_partial = false;
bool fail_close = false;
bool fail_directory_close = false;
bool fail_truncate = false;
bool crash_on_file_fsync = false;

extern "C" int __real_ftruncate(int, off_t);
extern "C" int __wrap_ftruncate(int descriptor, off_t length) {
    if (fail_truncate) {
        errno = EIO;
        return -1;
    }
    return __real_ftruncate(descriptor, length);
}

extern "C" int __real_close(int);
extern "C" int __wrap_close(int descriptor) {
    struct stat details{};
    const bool is_directory = ::fstat(descriptor, &details) == 0 && S_ISDIR(details.st_mode);
    const int result = __real_close(descriptor);
    if (fail_close || (fail_directory_close && is_directory)) {
        errno = EIO;
        return -1;
    }
    return result;
}

extern "C" int __real_fsync(int);
extern "C" int __wrap_fsync(int descriptor) {
    if (crash_on_file_fsync) ::_exit(137);
    if (fail_fsync) {
        errno = EIO;
        return -1;
    }
    static unsigned call_count = 0;
    if (fail_parent_fsync && call_count++ == 1) {
        errno = EIO;
        return -1;
    }
    if (!fail_parent_fsync) call_count = 0;
    return __real_fsync(descriptor);
}

extern "C" ssize_t __real_write(int, const void*, size_t);
extern "C" ssize_t __wrap_write(int descriptor, const void* bytes, size_t length) {
    static bool partial_write_emitted = false;
    if ((fail_write_after_partial || fail_zero_write_after_partial) && !partial_write_emitted) {
        partial_write_emitted = true;
        const auto partial = length > 3 ? 3 : length;
        return __real_write(descriptor, bytes, partial);
    }
    if (fail_zero_write_after_partial) return 0;
    if (fail_write_after_partial) {
        errno = EIO;
        return -1;
    }
    if (!fail_write_after_partial && !fail_zero_write_after_partial) partial_write_emitted = false;
    return __real_write(descriptor, bytes, length);
}

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

    const auto close_failure_path = base / "close-failure.jsonl";
    ErrorStateHistory close_failure_history(close_failure_path.string());
    fail_close = true;
    const auto close_failure = close_failure_history.append(event("opened"));
    fail_close = false;
    assert(!close_failure.valid && close_failure.status == "uncertain");

    const auto directory_close_failure_path = base / "directory-close-failure.jsonl";
    ErrorStateHistory directory_close_failure_history(directory_close_failure_path.string());
    fail_directory_close = true;
    const auto directory_close_failure = directory_close_failure_history.append(event("opened"));
    fail_directory_close = false;
    assert(!directory_close_failure.valid && directory_close_failure.status == "uncertain");

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
    const auto replay = history.replay_mutations();
    assert(replay.valid && replay.status == "replayed");
    assert(replay.records == 4 && replay.states.size() == 1);
    assert(replay.states.front().entity_identity == "entity");
    assert(replay.states.front().revision == 2);
    assert(replay.states.front().state_reference == "after");
    const auto found = history.find_event(first.event_id);
    assert(found.valid && found.found && found.status == "found");
    assert(history.find_event(generate_ulid()).status == "not_found");
    assert(!history.find_event("not-a-ulid").valid);
    const auto bounded_path = base / "bounded.jsonl";
    ErrorStateHistory bounded(bounded_path.string(), 1024 * 1024, 1);
    assert(bounded.append(first).status == "exhausted");
    const auto bounded_inspection = bounded.inspect();
    assert(bounded_inspection.valid && bounded_inspection.records == 0);
    const auto line_bounded_path = base / "line-bounded.jsonl";
    ErrorStateHistory line_bounded(line_bounded_path.string(), 32, 1024 * 1024);
    const auto line_bounded_result = line_bounded.append(first);
    assert(!line_bounded_result.valid && line_bounded_result.status == "exhausted");
    assert(line_bounded_result.error == "history record exceeds configured line bound");
    assert(line_bounded.inspect().valid && line_bounded.inspect().records == 0);
    ErrorStateHistory invalid_bounds(base / "invalid-bounds.jsonl", 0, 1024);
    assert(!invalid_bounds.inspect().valid);
    assert(invalid_bounds.inspect().status == "rejected");
    assert(invalid_bounds.append(first).status == "rejected");

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
    const auto negative_path = base / "negative-revision.jsonl";
    auto negative_json = to_json(mutation);
    const auto old_revision_marker = negative_json.find("\"old_revision\":1");
    assert(old_revision_marker != std::string::npos);
    negative_json.replace(old_revision_marker, std::string("\"old_revision\":1").size(), "\"old_revision\":-1");
    {
        std::ofstream output(negative_path, std::ios::binary);
        output << negative_json << '\n';
    }
    ErrorStateHistory negative(negative_path.string());
    const auto negative_inspection = negative.inspect();
    assert(!negative_inspection.valid);
    assert(negative_inspection.status == "invalid");

    const auto crash_pid = ::fork();
    assert(crash_pid >= 0);
    if (crash_pid == 0) {
        const int descriptor = ::open(path.c_str(), O_WRONLY | O_APPEND);
        if (descriptor < 0) ::_exit(126);
        const std::string torn = "{\"format\":\"frankencore.error_state_event\",\"event_id\":\"" +
                                 generate_ulid() + "\"";
        const auto written = ::write(descriptor, torn.data(), torn.size());
        ::close(descriptor);
        ::_exit(written == static_cast<ssize_t>(torn.size()) ? 137 : 126);
    }
    int crash_status = 0;
    assert(::waitpid(crash_pid, &crash_status, 0) == crash_pid);
    assert(WIFEXITED(crash_status) && WEXITSTATUS(crash_status) == 137);
    const auto incomplete = history.inspect();
    assert(!incomplete.valid);
    assert(incomplete.status == "incomplete");
    assert(incomplete.records == 4);
    assert(history.append(third).status == "incomplete");

    const auto truncate_failure_path = base / "truncate-failure.jsonl";
    ErrorStateHistory truncate_failure_history(truncate_failure_path.string());
    assert(truncate_failure_history.append(event("opened")).status == "appended");
    const int truncate_descriptor = ::open(truncate_failure_path.c_str(), O_WRONLY | O_APPEND);
    assert(truncate_descriptor >= 0);
    const std::string truncate_torn = "{\"format\":\"frankencore.error_state_event\",\"event_id\":\"" + generate_ulid();
    assert(::write(truncate_descriptor, truncate_torn.data(), truncate_torn.size()) == static_cast<ssize_t>(truncate_torn.size()));
    assert(::close(truncate_descriptor) == 0);
    fail_truncate = true;
    const auto truncate_failure = truncate_failure_history.repair_incomplete_tail();
    fail_truncate = false;
    assert(!truncate_failure.valid && truncate_failure.status == "error");
    assert(std::filesystem::exists(truncate_failure.quarantine_path));
    std::filesystem::remove(truncate_failure.quarantine_path);
    const auto truncate_repair = truncate_failure_history.repair_incomplete_tail();
    assert(truncate_repair.valid && truncate_repair.status == "repaired");

    const auto quarantine_close_failure_path = base / "quarantine-close-failure.jsonl";
    ErrorStateHistory quarantine_close_failure_history(quarantine_close_failure_path.string());
    assert(quarantine_close_failure_history.append(event("opened")).status == "appended");
    const int quarantine_close_descriptor = ::open(quarantine_close_failure_path.c_str(), O_WRONLY | O_APPEND);
    assert(quarantine_close_descriptor >= 0);
    const std::string quarantine_close_torn = "{\"format\":\"frankencore.error_state_event\",\"event_id\":\"" + generate_ulid();
    assert(::write(quarantine_close_descriptor, quarantine_close_torn.data(), quarantine_close_torn.size()) == static_cast<ssize_t>(quarantine_close_torn.size()));
    assert(::close(quarantine_close_descriptor) == 0);
    fail_close = true;
    const auto quarantine_close_failure = quarantine_close_failure_history.repair_incomplete_tail();
    fail_close = false;
    assert(!quarantine_close_failure.valid && quarantine_close_failure.status == "error");
    assert(std::filesystem::exists(quarantine_close_failure.quarantine_path));
    std::filesystem::remove(quarantine_close_failure.quarantine_path);
    assert(quarantine_close_failure_history.repair_incomplete_tail().status == "repaired");

    const auto repaired = history.repair_incomplete_tail();
    assert(repaired.valid);
    assert(repaired.changed);
    assert(repaired.status == "repaired");
    assert(std::filesystem::exists(repaired.quarantine_path));
    assert(history.inspect().status == "valid");
    assert(history.read_records().json_records.size() == 4);
    assert(history.append(third).status == "appended");
    assert(history.inspect().records == 5);

    const auto fsync_crash_path = base / "fsync-crash.jsonl";
    ErrorStateHistory fsync_crash_history(fsync_crash_path.string());
    const auto fsync_crash_opened = event("opened");
    assert(fsync_crash_history.append(fsync_crash_opened).status == "appended");
    const auto fsync_crash_pid = ::fork();
    assert(fsync_crash_pid >= 0);
    if (fsync_crash_pid == 0) {
        crash_on_file_fsync = true;
        auto fsync_crash_diagnosed = fsync_crash_opened;
        fsync_crash_diagnosed.event_id = generate_ulid();
        fsync_crash_diagnosed.status = "diagnosed";
        (void)fsync_crash_history.append(fsync_crash_diagnosed);
        ::_exit(126);
    }
    int fsync_crash_status = 0;
    assert(::waitpid(fsync_crash_pid, &fsync_crash_status, 0) == fsync_crash_pid);
    assert(WIFEXITED(fsync_crash_status) && WEXITSTATUS(fsync_crash_status) == 137);
    crash_on_file_fsync = false;
    const auto fsync_crash_inspection = fsync_crash_history.inspect();
    assert(fsync_crash_inspection.valid);
    assert(fsync_crash_inspection.records == 2);

    const auto left_path = base / "branch-left.jsonl";
    const auto right_path = base / "branch-right.jsonl";
    std::filesystem::copy_file(path, left_path);
    std::filesystem::copy_file(path, right_path);
    ErrorStateHistory right_history(right_path.string());
    auto branch_event = event("reopened");
    branch_event.error_state_id = first.error_state_id;
    assert(right_history.append(branch_event).status == "appended");
    const auto reconciliation = reconcile_histories(left_path.string(), right_path.string());
    assert(reconciliation.valid);
    assert(reconciliation.status == "diverged");
    assert(reconciliation.left_records == 5);
    assert(reconciliation.right_records == 6);
    assert(reconciliation.common_events == 5);
    assert(reconciliation.left_only_events == 0);
    assert(reconciliation.right_only_events == 1);
    assert(reconciliation.conflicting_events == 0);

    const auto conflict_left_path = base / "conflict-left.jsonl";
    const auto conflict_right_path = base / "conflict-right.jsonl";
    std::filesystem::copy_file(path, conflict_left_path);
    std::filesystem::copy_file(path, conflict_right_path);
    {
        std::ifstream input(conflict_right_path);
        std::string first_line;
        std::getline(input, first_line);
        const std::string remainder((std::istreambuf_iterator<char>(input)), {});
        const auto marker = first_line.find("controlled history test");
        assert(marker != std::string::npos);
        first_line.replace(marker, std::string("controlled history test").size(), "different branch fact");
        std::ofstream output(conflict_right_path, std::ios::binary | std::ios::trunc);
        output << first_line << '\n' << remainder;
    }
    const auto read_bytes = [](const auto& file_path) {
        std::ifstream input(file_path, std::ios::binary);
        return std::string((std::istreambuf_iterator<char>(input)), {});
    };
    const auto conflict_left_before = read_bytes(conflict_left_path);
    const auto conflict_right_before = read_bytes(conflict_right_path);
    const auto conflict_reconciliation = reconcile_histories(conflict_left_path.string(), conflict_right_path.string());
    assert(conflict_reconciliation.valid);
    assert(conflict_reconciliation.status == "conflict");
    assert(conflict_reconciliation.left_records == 5);
    assert(conflict_reconciliation.right_records == 5);
    assert(conflict_reconciliation.common_events == 4);
    assert(conflict_reconciliation.left_only_events == 0);
    assert(conflict_reconciliation.right_only_events == 0);
    assert(conflict_reconciliation.conflicting_events == 1);
    assert(read_bytes(conflict_left_path) == conflict_left_before);
    assert(read_bytes(conflict_right_path) == conflict_right_before);

    const auto durability_path = base / "durability.jsonl";
    ErrorStateHistory durability(durability_path.string());
    const auto durability_opened = event("opened");
    assert(durability.append(durability_opened).status == "appended");
    auto durability_diagnosed = durability_opened;
    durability_diagnosed.event_id = generate_ulid();
    durability_diagnosed.status = "diagnosed";
    fail_fsync = true;
    const auto uncertain_append = durability.append(durability_diagnosed);
    fail_fsync = false;
    assert(!uncertain_append.valid && uncertain_append.changed);
    assert(uncertain_append.status == "uncertain");
    assert(uncertain_append.records == 2);
    assert(durability.inspect().valid && durability.inspect().records == 2);

    const auto partial_path = base / "partial-write.jsonl";
    ErrorStateHistory partial_history(partial_path.string());
    const auto partial_opened = event("opened");
    assert(partial_history.append(partial_opened).status == "appended");
    auto partial_diagnosed = partial_opened;
    partial_diagnosed.event_id = generate_ulid();
    partial_diagnosed.status = "diagnosed";
    fail_write_after_partial = true;
    const auto partial_append = partial_history.append(partial_diagnosed);
    fail_write_after_partial = false;
    assert(!partial_append.valid && partial_append.changed);
    assert(partial_append.status == "uncertain");
    assert(partial_append.records == 1);
    assert(partial_history.inspect().status == "incomplete");
    assert(partial_history.repair_incomplete_tail().status == "repaired");
    assert(partial_history.inspect().valid && partial_history.inspect().records == 1);

    const auto zero_write_path = base / "zero-write.jsonl";
    ErrorStateHistory zero_write_history(zero_write_path.string());
    const auto zero_write_opened = event("opened");
    assert(zero_write_history.append(zero_write_opened).status == "appended");
    auto zero_write_diagnosed = zero_write_opened;
    zero_write_diagnosed.event_id = generate_ulid();
    zero_write_diagnosed.status = "diagnosed";
    fail_zero_write_after_partial = true;
    const auto zero_write_append = zero_write_history.append(zero_write_diagnosed);
    fail_zero_write_after_partial = false;
    assert(!zero_write_append.valid && zero_write_append.changed);
    assert(zero_write_append.status == "uncertain");
    assert(zero_write_append.records == 1);
    assert(zero_write_history.inspect().status == "incomplete");

    const auto parent_sync_path = base / "parent-sync.jsonl";
    ErrorStateHistory parent_sync_history(parent_sync_path.string());
    const auto parent_sync_opened = event("opened");
    assert(parent_sync_history.append(parent_sync_opened).status == "appended");
    auto parent_sync_diagnosed = parent_sync_opened;
    parent_sync_diagnosed.event_id = generate_ulid();
    parent_sync_diagnosed.status = "diagnosed";
    fail_parent_fsync = true;
    const auto parent_sync_append = parent_sync_history.append(parent_sync_diagnosed);
    fail_parent_fsync = false;
    assert(!parent_sync_append.valid && parent_sync_append.changed);
    assert(parent_sync_append.status == "uncertain");
    assert(parent_sync_append.records == 2);
    assert(parent_sync_history.inspect().valid && parent_sync_history.inspect().records == 2);

    {
        std::ofstream output(path, std::ios::binary | std::ios::app);
        output << "{\"format\":\"frankencore.error_state_event\",\"event_id\":\""
               << generate_ulid() << "\"";
    }
    const auto uncertain = history.repair_incomplete_tail();
    assert(!uncertain.valid);
    assert(uncertain.status == "error");
    assert(history.inspect().status == "incomplete");
    assert(history.inspect().records == 5);

    std::filesystem::remove_all(base);
    return 0;
}
