#include "frankencore/provenance.hpp"
#include <flowcontracts/json.hpp>

#include <sstream>
#include <new>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>
#include <utility>

namespace frankencore::provenance {
namespace {

struct HistoryScan {
    HistoryResult result;
    std::unordered_map<std::string, std::string> events;
    std::unordered_map<std::string, std::string> error_states;
    std::vector<std::string> records;
    std::size_t valid_prefix = 0;
    bool incomplete_tail = false;
    std::string tail;
};

bool valid_error_state_transition(const std::string& previous, const std::string& next) {
    if (previous == "opened") return next == "diagnosed" || next == "recovery_attempted" || next == "escalated";
    if (previous == "diagnosed") return next == "recovery_attempted" || next == "resolved" || next == "escalated";
    if (previous == "recovery_attempted") return next == "diagnosed" || next == "resolved" || next == "escalated";
    if (previous == "escalated") return next == "resolved" || next == "reopened";
    if (previous == "resolved") return next == "reopened";
    if (previous == "reopened") return next == "diagnosed" || next == "escalated";
    return false;
}

bool valid_json_record(const std::string& line, std::string& error) {
    try {
        using namespace flowcontracts::json;
        const auto root = object(parse(line));
        const auto& format = string(required(root, "format"), "$.format");
        if (integer(required(root, "version"), "$.version") != 1)
            throw Error("$.version", "unsupported history record version");
        const auto& status = string(required(root, "status"), "$.status");
        const auto& event_id = string(required(root, "event_id"), "$.event_id");
        if (!is_valid_ulid(event_id)) throw Error("$.event_id", "must be a valid ULID");
        if (format == "frankencore.error_state_event") {
            if (status != "opened" && status != "diagnosed" && status != "recovery_attempted" &&
                status != "resolved" && status != "escalated" && status != "reopened")
                throw Error("$.status", "invalid error-state status");
            for (const auto field : {"error_state_id", "attempt_id", "correlation_id"}) {
                const auto& value = string(required(root, field), std::string("$.") + field);
                if (!is_valid_ulid(value)) throw Error(std::string("$.") + field, "must be a valid ULID");
            }
            string(required(root, "diagnosis"), "$.diagnosis");
            string(required(root, "recovery"), "$.recovery");
            boolean(required(root, "operator_action_required"), "$.operator_action_required");
            return true;
        }
        if (format == "frankencore.mutation_record") {
            if (status != "committed") throw Error("$.status", "mutation record must be committed");
            integer(required(root, "old_revision"), "$.old_revision");
            integer(required(root, "new_revision"), "$.new_revision");
            if (integer(required(root, "new_revision"), "$.new_revision") <=
                integer(required(root, "old_revision"), "$.old_revision"))
                throw Error("$.new_revision", "must be greater than old_revision");
            for (const auto field : {"attempt_id", "correlation_id"}) {
                const auto& value = string(required(root, field), std::string("$.") + field);
                if (!is_valid_ulid(value)) throw Error(std::string("$.") + field, "must be a valid ULID");
            }
            for (const auto field : {"entity_identity", "actor_identity", "provider_identity",
                                     "authorizing_policy", "before_state_reference", "after_state_reference",
                                     "operation", "atomicity", "recoverability"})
                if (string(required(root, field), std::string("$.") + field).empty())
                    throw Error(std::string("$.") + field, "must not be empty");
            if (string(required(root, "before_state_reference"), "$.before_state_reference") ==
                string(required(root, "after_state_reference"), "$.after_state_reference"))
                throw Error("$.after_state_reference", "must differ from before_state_reference");
            const auto& causes = array(required(root, "causes"), "$.causes");
            if (causes.empty()) throw Error("$.causes", "must not be empty");
            for (std::size_t index = 0; index < causes.size(); ++index)
                string(causes[index], "$.causes[" + std::to_string(index) + "]");
            return true;
        }
        if (format == "frankencore.mutation_event") {
            if (status != "rejected") throw Error("$.status", "mutation event must be rejected");
            for (const auto field : {"attempt_id", "correlation_id"}) {
                const auto& value = string(required(root, field), std::string("$.") + field);
                if (!is_valid_ulid(value)) throw Error(std::string("$.") + field, "must be a valid ULID");
            }
            for (const auto field : {"entity_identity", "actor_identity", "provider_identity",
                                     "authorizing_policy", "operation", "rejection_domain", "rejection_reason"})
                if (string(required(root, field), std::string("$.") + field).empty())
                    throw Error(std::string("$.") + field, "must not be empty");
            boolean(required(root, "retryable"), "$.retryable");
            if (const auto* observed = optional(root, "observed_revision"); observed != nullptr &&
                !std::holds_alternative<std::nullptr_t>(*observed))
                integer(*observed, "$.observed_revision");
            return true;
        }
        throw Error("$.format", "unsupported history record format");
    } catch (const flowcontracts::json::Error& exception) {
        error = exception.what();
    } catch (const std::exception& exception) {
        error = exception.what();
    } catch (...) {
        error = "unknown JSON validation failure";
    }
    return false;
}

bool valid_replay_record(const std::string& line,
                         const std::unordered_map<std::string, std::string>& states,
                         std::string& error) {
    try {
        using namespace flowcontracts::json;
        const auto root = object(parse(line));
        if (string(required(root, "format"), "$.format") != "frankencore.error_state_event") return true;
        const auto state_id = string(required(root, "error_state_id"), "$.error_state_id");
        const auto status = string(required(root, "status"), "$.status");
        const auto previous = states.find(state_id);
        if (previous == states.end()) {
            if (status != "opened") throw Error("$.status", "error-state replay must begin with opened");
        } else if (!valid_error_state_transition(previous->second, status)) {
            throw Error("$.status", "illegal error-state lifecycle transition");
        }
    } catch (const std::exception& exception) {
        error = exception.what();
        return false;
    } catch (...) {
        error = "unknown error-state replay validation failure";
        return false;
    }
    return true;
}

int lock_history(const std::string& path, int operation) {
    const auto lock_path = path + ".lock";
    const int descriptor = ::open(lock_path.c_str(), O_CREAT | O_RDWR, 0644);
    if (descriptor < 0) return -1;
    if (::flock(descriptor, operation) != 0) {
        const int saved = errno;
        ::close(descriptor);
        errno = saved;
        return -1;
    }
    return descriptor;
}

int sync_parent_directory(const std::string& path) {
    const auto parent = std::filesystem::path(path).parent_path();
    const auto directory = parent.empty() ? std::string{"."} : parent.string();
    const int descriptor = ::open(directory.c_str(), O_RDONLY | O_DIRECTORY);
    if (descriptor < 0) return -1;
    const int status = ::fsync(descriptor);
    const int saved = errno;
    ::close(descriptor);
    errno = saved;
    return status;
}

bool write_all(int descriptor, const char* bytes, std::size_t length) {
    while (length != 0) {
        const auto written = ::write(descriptor, bytes, length);
        if (written < 0 && errno == EINTR) continue;
        if (written <= 0) return false;
        bytes += written;
        length -= static_cast<std::size_t>(written);
    }
    return true;
}

HistoryScan scan_history(const std::string& path, std::size_t max_line_bytes,
                         std::size_t max_history_bytes) {
    HistoryScan scan;
    const int descriptor = ::open(path.c_str(), O_RDONLY);
    if (descriptor < 0) {
        if (errno == ENOENT) {
            scan.result = {true, false, 0, "empty", {}, {}};
            return scan;
        }
        scan.result = {false, false, 0, "error", std::string("cannot open history: ") + std::strerror(errno), {}};
        return scan;
    }
    std::string contents;
    char buffer[8192];
    while (true) {
        const auto count = ::read(descriptor, buffer, sizeof(buffer));
        if (count < 0 && errno == EINTR) continue;
        if (count < 0) {
            const int saved = errno;
            ::close(descriptor);
            scan.result = {false, false, 0, "error", std::string("cannot read history: ") + std::strerror(saved), {}};
            return scan;
        }
        if (count == 0) break;
        const auto bytes = static_cast<std::size_t>(count);
        if (contents.size() > max_history_bytes || bytes > max_history_bytes - contents.size()) {
            ::close(descriptor);
            scan.result = {false, false, scan.events.size(), "exhausted", "history exceeds configured byte bound", {}};
            return scan;
        }
        contents.append(buffer, bytes);
    }
    ::close(descriptor);

    std::size_t offset = 0;
    while (offset < contents.size()) {
        const auto newline = contents.find('\n', offset);
        if (newline == std::string::npos) {
            scan.incomplete_tail = true;
            scan.tail = contents.substr(offset);
            break;
        }
        const auto line_length = newline - offset;
        if (line_length == 0 || line_length > max_line_bytes) {
            scan.result = {false, false, scan.events.size(), "invalid", "history contains an empty or oversized record", {}};
            return scan;
        }
        const auto line = contents.substr(offset, line_length);
        std::string json_error;
        if (!valid_json_record(line, json_error)) {
            scan.result = {false, false, scan.events.size(), "invalid", "history contains an invalid record: " + json_error, {}};
            return scan;
        }
        if (!valid_replay_record(line, scan.error_states, json_error)) {
            scan.result = {false, false, scan.events.size(), "invalid", "history replay validation failed: " + json_error, {}};
            return scan;
        }
        using namespace flowcontracts::json;
        const auto root = object(parse(line));
        if (string(required(root, "format"), "$.format") == "frankencore.error_state_event")
            scan.error_states[string(required(root, "error_state_id"), "$.error_state_id")] =
                string(required(root, "status"), "$.status");
        constexpr std::string_view marker = "\"event_id\":\"";
        const auto event_begin = line.find(marker);
        if (event_begin == std::string::npos) {
            scan.result = {false, false, scan.events.size(), "invalid", "history record has no event_id", {}};
            return scan;
        }
        const auto id_begin = event_begin + marker.size();
        const auto id_end = line.find('"', id_begin);
        if (id_end == std::string::npos) {
            scan.result = {false, false, scan.events.size(), "invalid", "history record has an unterminated event_id", {}};
            return scan;
        }
        const auto event_id = line.substr(id_begin, id_end - id_begin);
        if (!is_valid_ulid(event_id)) {
            scan.result = {false, false, scan.events.size(), "invalid", "history record has an invalid event_id", {}};
            return scan;
        }
        if (!scan.events.emplace(event_id, line).second) {
            scan.result = {false, false, scan.events.size(), "invalid", "history contains a duplicate event_id", {}};
            return scan;
        }
        scan.records.push_back(line);
        offset = newline + 1;
        scan.valid_prefix = offset;
    }
    scan.result = {!scan.incomplete_tail, false, scan.events.size(), scan.incomplete_tail ? "incomplete" : "valid", {}, {}};
    return scan;
}

ValidationResult required(const MutationRecord& record) {
    const std::pair<const char*, const std::string*> fields[] = {
        {"event_id", &record.event_id},
        {"attempt_id", &record.attempt.attempt_id},
        {"correlation_id", &record.attempt.correlation_id},
        {"entity_identity", &record.attempt.entity_identity},
        {"actor_identity", &record.attempt.actor_identity},
        {"provider_identity", &record.attempt.provider_identity},
        {"authorizing_policy", &record.attempt.authorizing_policy},
        {"before_state_reference", &record.before_state_reference},
        {"after_state_reference", &record.after_state_reference},
        {"operation", &record.attempt.operation},
        {"atomicity", &record.atomicity},
        {"recoverability", &record.recoverability},
    };
    for (const auto& [name, value] : fields)
        if (value->empty()) return {false, std::string(name) + " must not be empty"};
    const std::pair<const char*, const std::string*> identities[] = {
        {"event_id", &record.event_id},
        {"attempt_id", &record.attempt.attempt_id},
        {"correlation_id", &record.attempt.correlation_id},
    };
    for (const auto& [name, value] : identities)
        if (!is_valid_ulid(*value)) return {false, std::string(name) + " must be a valid ULID"};
    if (record.attempt.causes.empty()) return {false, "causes must not be empty"};
    return {true, {}};
}

std::string escape(const std::string& value) {
    std::ostringstream out;
    for (const unsigned char character : value) {
        if (character == '"') out << "\\\"";
        else if (character == '\\') out << "\\\\";
        else if (character == '\n') out << "\\n";
        else if (character == '\r') out << "\\r";
        else if (character == '\t') out << "\\t";
        else if (character < 0x20) {
            out << "\\u00" << "0123456789abcdef"[(character >> 4) & 0xf]
                << "0123456789abcdef"[character & 0xf];
        } else out << character;
    }
    return out.str();
}

void quoted(std::ostringstream& out, const std::string& value) {
    out << '"' << escape(value) << '"';
}

void string_array(std::ostringstream& out, const std::vector<std::string>& values) {
    out << '[';
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index != 0) out << ',';
        quoted(out, values[index]);
    }
    out << ']';
}

} // namespace

ValidationResult validate(const MutationRecord& record) {
    if (record.new_revision <= record.old_revision)
        return {false, "new_revision must be greater than old_revision"};
    const auto result = required(record);
    if (!result.valid) return result;
    if (record.before_state_reference == record.after_state_reference)
        return {false, "before and after state references must differ"};
    return {true, {}};
}

std::string to_json(const MutationRecord& record) {
    const auto result = validate(record);
    if (!result.valid) throw std::invalid_argument(result.error);

    std::ostringstream out;
    out << "{\"format\":\"frankencore.mutation_record\",\"version\":1,"
           "\"status\":\"committed\",\"event_id\":";
    quoted(out, record.event_id);
    out << ",\"attempt_id\":";
    quoted(out, record.attempt.attempt_id);
    out << ",\"correlation_id\":";
    quoted(out, record.attempt.correlation_id);
    out << ",\"entity_identity\":";
    quoted(out, record.attempt.entity_identity);
    out << ",\"old_revision\":" << record.old_revision
        << ",\"new_revision\":" << record.new_revision
        << ",\"actor_identity\":";
    quoted(out, record.attempt.actor_identity);
    out << ",\"provider_identity\":";
    quoted(out, record.attempt.provider_identity);
    out << ",\"authorizing_policy\":";
    quoted(out, record.attempt.authorizing_policy);
    out << ",\"before_state_reference\":";
    quoted(out, record.before_state_reference);
    out << ",\"after_state_reference\":";
    quoted(out, record.after_state_reference);
    out << ",\"operation\":";
    quoted(out, record.attempt.operation);
    out << ",\"atomicity\":";
    quoted(out, record.atomicity);
    out << ",\"recoverability\":";
    quoted(out, record.recoverability);
    out << ",\"rollback_reference\":";
    if (record.rollback_reference) quoted(out, *record.rollback_reference);
    else out << "null";
    out << ",\"causes\":";
    string_array(out, record.attempt.causes);
    out << ",\"derived_entities\":";
    string_array(out, record.derived_entities);
    out << ",\"evidence\":{\"before\":{\"value\":";
    quoted(out, record.before.value);
    out << "},\"after\":{\"value\":";
    quoted(out, record.after.value);
    out << "}}}";
    return out.str();
}

std::string to_json(const MutationRejection& rejection) {
    const auto& attempt = rejection.attempt;
    const std::pair<const char*, const std::string*> fields[] = {
        {"event_id", &rejection.event_id},
        {"attempt_id", &attempt.attempt_id},
        {"correlation_id", &attempt.correlation_id},
        {"entity_identity", &attempt.entity_identity},
        {"actor_identity", &attempt.actor_identity},
        {"provider_identity", &attempt.provider_identity},
        {"authorizing_policy", &attempt.authorizing_policy},
        {"operation", &attempt.operation},
        {"rejection_domain", &rejection.rejection_domain},
        {"rejection_reason", &rejection.rejection_reason},
    };
    for (const auto& [name, value] : fields)
        if (value->empty()) throw std::invalid_argument(std::string(name) + " must not be empty");
    const std::pair<const char*, const std::string*> identities[] = {
        {"event_id", &rejection.event_id},
        {"attempt_id", &attempt.attempt_id},
        {"correlation_id", &attempt.correlation_id},
    };
    for (const auto& [name, value] : identities)
        if (!is_valid_ulid(*value)) throw std::invalid_argument(std::string(name) + " must be a valid ULID");
    if (attempt.causes.empty()) throw std::invalid_argument("causes must not be empty");

    std::ostringstream out;
    out << "{\"format\":\"frankencore.mutation_event\",\"version\":1,"
           "\"status\":\"rejected\",\"event_id\":";
    quoted(out, rejection.event_id);
    out << ",\"attempt_id\":";
    quoted(out, attempt.attempt_id);
    out << ",\"correlation_id\":";
    quoted(out, attempt.correlation_id);
    out << ",\"entity_identity\":";
    quoted(out, attempt.entity_identity);
    out << ",\"actor_identity\":";
    quoted(out, attempt.actor_identity);
    out << ",\"provider_identity\":";
    quoted(out, attempt.provider_identity);
    out << ",\"authorizing_policy\":";
    quoted(out, attempt.authorizing_policy);
    out << ",\"operation\":";
    quoted(out, attempt.operation);
    out << ",\"causes\":";
    string_array(out, attempt.causes);
    out << ",\"rejection_domain\":";
    quoted(out, rejection.rejection_domain);
    out << ",\"rejection_reason\":";
    quoted(out, rejection.rejection_reason);
    out << ",\"retryable\":" << (rejection.retryable ? "true" : "false")
        << ",\"observed_revision\":";
    if (rejection.observed_revision) out << *rejection.observed_revision;
    else out << "null";
    out << "}";
    return out.str();
}

std::string to_json(const ErrorStateEvent& event) {
    const std::pair<const char*, const std::string*> fields[] = {
        {"error_state_id", &event.error_state_id},
        {"event_id", &event.event_id},
        {"attempt_id", &event.attempt_id},
        {"correlation_id", &event.correlation_id},
        {"status", &event.status},
        {"diagnosis", &event.diagnosis},
        {"recovery", &event.recovery},
    };
    for (const auto& [name, value] : fields)
        if (value->empty()) throw std::invalid_argument(std::string(name) + " must not be empty");
    const std::pair<const char*, const std::string*> identities[] = {
        {"error_state_id", &event.error_state_id},
        {"event_id", &event.event_id},
        {"attempt_id", &event.attempt_id},
        {"correlation_id", &event.correlation_id},
    };
    for (const auto& [name, value] : identities)
        if (!is_valid_ulid(*value)) throw std::invalid_argument(std::string(name) + " must be a valid ULID");

    std::ostringstream out;
    out << "{\"format\":\"frankencore.error_state_event\",\"version\":1,"
           "\"error_state_id\":";
    quoted(out, event.error_state_id);
    out << ",\"event_id\":";
    quoted(out, event.event_id);
    out << ",\"attempt_id\":";
    quoted(out, event.attempt_id);
    out << ",\"correlation_id\":";
    quoted(out, event.correlation_id);
    out << ",\"status\":";
    quoted(out, event.status);
    out << ",\"diagnosis\":";
    quoted(out, event.diagnosis);
    out << ",\"recovery\":";
    quoted(out, event.recovery);
    out << ",\"operator_action_required\":"
        << (event.operator_action_required ? "true" : "false") << "}";
    return out.str();
}

namespace {

template <typename Value>
JsonResult checked_json(const Value& value) {
    try {
        return {true, to_json(value), {}};
    } catch (const std::invalid_argument& error) {
        return {false, {}, error.what()};
    } catch (const std::bad_alloc&) {
        return {false, {}, "serialization allocation failed"};
    } catch (const std::exception& error) {
        return {false, {}, std::string{"serialization failed: "} + error.what()};
    } catch (...) {
        return {false, {}, "serialization failed with unknown non-standard failure"};
    }
}

} // namespace

JsonResult to_json_checked(const MutationRecord& record) {
    return checked_json(record);
}

JsonResult to_json_checked(const MutationRejection& rejection) {
    return checked_json(rejection);
}

JsonResult to_json_checked(const ErrorStateEvent& event) {
    return checked_json(event);
}

namespace {

HistoryResult append_serialized(const std::string& path, std::size_t max_line_bytes,
                                std::size_t max_history_bytes,
                                const std::string& json) noexcept {
    try {
        std::string json_error;
        if (!valid_json_record(json, json_error))
            return {false, false, 0, "rejected", "serialized event is invalid: " + json_error, {}};
        const int lock = lock_history(path, LOCK_EX);
        if (lock < 0)
            return {false, false, 0, "error", std::string("cannot lock history: ") + std::strerror(errno), {}};
        const auto scan = scan_history(path, max_line_bytes, max_history_bytes);
        if (!scan.result.valid || scan.incomplete_tail) {
            ::flock(lock, LOCK_UN);
            ::close(lock);
            return {false, false, scan.result.records, scan.incomplete_tail ? "incomplete" : scan.result.status,
                    scan.incomplete_tail ? "history has an incomplete final record; explicit repair is required" : scan.result.error, {}};
        }
        constexpr std::string_view marker = "\"event_id\":\"";
        const auto event_id_begin = json.find(marker);
        if (event_id_begin == std::string::npos) {
            ::flock(lock, LOCK_UN);
            ::close(lock);
            return {false, false, scan.result.records, "rejected", "serialized event has no event_id", {}};
        }
        const auto id_begin = event_id_begin + marker.size();
        const auto id_end = json.find('"', id_begin);
        if (id_end == std::string::npos) {
            ::flock(lock, LOCK_UN);
            ::close(lock);
            return {false, false, scan.result.records, "rejected", "serialized event has an unterminated event_id", {}};
        }
        const auto event_id = json.substr(id_begin, id_end - id_begin);
        if (const auto existing = scan.events.find(event_id); existing != scan.events.end()) {
            ::flock(lock, LOCK_UN);
            ::close(lock);
            if (existing->second == json) return {true, false, scan.result.records, "duplicate", {}, {}};
            return {false, false, scan.result.records, "conflict", "event_id already exists with different content", {}};
        }
        if (!valid_replay_record(json, scan.error_states, json_error)) {
            ::flock(lock, LOCK_UN);
            ::close(lock);
            return {false, false, scan.result.records, "rejected", "serialized event violates replay rules: " + json_error, {}};
        }
        const int descriptor = ::open(path.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644);
        if (descriptor < 0) {
            const int saved = errno;
            ::flock(lock, LOCK_UN);
            ::close(lock);
            return {false, false, scan.result.records, "error", std::string("cannot open history for append: ") + std::strerror(saved), {}};
        }
        const std::string line = json + '\n';
        if (scan.valid_prefix > max_history_bytes || line.size() > max_history_bytes - scan.valid_prefix) {
            ::close(descriptor);
            ::flock(lock, LOCK_UN);
            ::close(lock);
            return {false, false, scan.result.records, "exhausted", "history append exceeds configured byte bound", {}};
        }
        bool written = write_all(descriptor, line.data(), line.size()) && ::fsync(descriptor) == 0;
        int saved = written ? 0 : errno;
        ::close(descriptor);
        if (written && sync_parent_directory(path) != 0) {
            written = false;
            saved = errno;
        }
        ::flock(lock, LOCK_UN);
        ::close(lock);
        if (!written) return {false, false, scan.result.records, "error", std::string("history append durability failed: ") + std::strerror(saved == 0 ? EIO : saved), {}};
        return {true, true, scan.result.records + 1, "appended", {}, {}};
    } catch (const std::exception& error) {
        return {false, false, 0, "error", error.what(), {}};
    } catch (...) {
        return {false, false, 0, "error", "history append failed with an unknown non-standard failure", {}};
    }
}

} // namespace

ErrorStateHistory::ErrorStateHistory(std::string path, std::size_t max_line_bytes,
                                     std::size_t max_history_bytes)
    : path_(std::move(path)), max_line_bytes_(max_line_bytes),
      max_history_bytes_(max_history_bytes) {}

HistoryResult ErrorStateHistory::inspect() const noexcept {
    try {
        const int lock = lock_history(path_, LOCK_SH);
        if (lock < 0)
            return {false, false, 0, "error", std::string("cannot lock history: ") + std::strerror(errno), {}};
        const auto scan = scan_history(path_, max_line_bytes_, max_history_bytes_);
        ::flock(lock, LOCK_UN);
        ::close(lock);
        return scan.result;
    } catch (const std::exception& error) {
        return {false, false, 0, "error", error.what(), {}};
    } catch (...) {
        return {false, false, 0, "error", "history inspection failed with an unknown non-standard failure", {}};
    }
}

HistoryReadResult ErrorStateHistory::read_records() const noexcept {
    try {
        const int lock = lock_history(path_, LOCK_SH);
        if (lock < 0)
            return {false, 0, {}, "error", std::string("cannot lock history: ") + std::strerror(errno)};
        const auto scan = scan_history(path_, max_line_bytes_, max_history_bytes_);
        ::flock(lock, LOCK_UN);
        ::close(lock);
        if (!scan.result.valid)
            return {false, scan.result.records, scan.records, scan.result.status, scan.result.error};
        return {true, scan.result.records, scan.records, scan.result.status, {}};
    } catch (const std::exception& error) {
        return {false, 0, {}, "error", error.what()};
    } catch (...) {
        return {false, 0, {}, "error", "history read failed with an unknown non-standard failure"};
    }
}

HistoryLookupResult ErrorStateHistory::find_event(const std::string& event_id) const noexcept {
    if (!is_valid_ulid(event_id))
        return {false, false, {}, "rejected", "event_id must be a valid ULID"};
    try {
        const int lock = lock_history(path_, LOCK_SH);
        if (lock < 0)
            return {false, false, {}, "error", std::string("cannot lock history: ") + std::strerror(errno)};
        const auto scan = scan_history(path_, max_line_bytes_, max_history_bytes_);
        ::flock(lock, LOCK_UN);
        ::close(lock);
        if (!scan.result.valid)
            return {false, false, {}, scan.result.status, scan.result.error};
        const auto found = scan.events.find(event_id);
        if (found == scan.events.end()) return {true, false, {}, "not_found", {}};
        return {true, true, found->second, "found", {}};
    } catch (const std::exception& error) {
        return {false, false, {}, "error", error.what()};
    } catch (...) {
        return {false, false, {}, "error", "history lookup failed with an unknown non-standard failure"};
    }
}

HistoryResult ErrorStateHistory::append(const MutationRecord& record) const noexcept {
    const auto serialized = to_json_checked(record);
    return serialized.valid ? append_serialized(path_, max_line_bytes_, max_history_bytes_, serialized.json)
                            : HistoryResult{false, false, 0, "rejected", serialized.error, {}};
}

HistoryResult ErrorStateHistory::append(const MutationRejection& rejection) const noexcept {
    const auto serialized = to_json_checked(rejection);
    return serialized.valid ? append_serialized(path_, max_line_bytes_, max_history_bytes_, serialized.json)
                            : HistoryResult{false, false, 0, "rejected", serialized.error, {}};
}

HistoryResult ErrorStateHistory::append(const ErrorStateEvent& event) const noexcept {
    const auto serialized = to_json_checked(event);
    return serialized.valid ? append_serialized(path_, max_line_bytes_, max_history_bytes_, serialized.json)
                            : HistoryResult{false, false, 0, "rejected", serialized.error, {}};
}

HistoryResult ErrorStateHistory::repair_incomplete_tail() const noexcept {
    try {
        const int lock = lock_history(path_, LOCK_EX);
        if (lock < 0)
            return {false, false, 0, "error", std::string("cannot lock history: ") + std::strerror(errno), {}};
        const auto scan = scan_history(path_, max_line_bytes_, max_history_bytes_);
        if (!scan.incomplete_tail) {
            ::flock(lock, LOCK_UN);
            ::close(lock);
            if (scan.result.valid) return {true, false, scan.result.records, "no_repair", {}, {}};
            return scan.result;
        }
        const auto quarantine = path_ + ".quarantine";
        const int quarantine_descriptor = ::open(quarantine.c_str(), O_CREAT | O_WRONLY | O_EXCL, 0644);
        if (quarantine_descriptor < 0) {
            const int saved = errno;
            ::flock(lock, LOCK_UN);
            ::close(lock);
            return {false, false, scan.result.records, "error", std::string("cannot create quarantine: ") + std::strerror(saved), quarantine};
        }
        const bool quarantined = write_all(quarantine_descriptor, scan.tail.data(), scan.tail.size()) && ::fsync(quarantine_descriptor) == 0;
        ::close(quarantine_descriptor);
        if (!quarantined) {
            ::flock(lock, LOCK_UN);
            ::close(lock);
            return {false, false, scan.result.records, "error", "cannot persist incomplete tail quarantine", quarantine};
        }
        const int descriptor = ::open(path_.c_str(), O_WRONLY);
        const bool truncated = descriptor >= 0 && ::ftruncate(descriptor, static_cast<off_t>(scan.valid_prefix)) == 0 && ::fsync(descriptor) == 0;
        if (descriptor >= 0) ::close(descriptor);
        ::flock(lock, LOCK_UN);
        ::close(lock);
        if (!truncated || sync_parent_directory(path_) != 0)
            return {false, false, scan.result.records, "error", "cannot durably truncate history to valid prefix", quarantine};
        return {true, true, scan.result.records, "repaired", "incomplete final record quarantined", quarantine};
    } catch (const std::exception& error) {
        return {false, false, 0, "error", error.what(), {}};
    } catch (...) {
        return {false, false, 0, "error", "history repair failed with an unknown non-standard failure", {}};
    }
}

} // namespace frankencore::provenance
