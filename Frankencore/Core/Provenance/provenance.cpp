#include "frankencore/provenance.hpp"

#include <sstream>
#include <new>
#include <stdexcept>
#include <utility>

namespace frankencore::provenance {
namespace {

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

} // namespace frankencore::provenance
