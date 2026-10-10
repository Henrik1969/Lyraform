#pragma once
#include <flowcontracts/json.hpp>

namespace flowcontracts {
enum class DispositionDiagnosticKind {
    missing_route, wrong_type, ambiguous_route, fault_to_recovery,
    unsupported_source, forged_artifact
};
struct DispositionDiagnosticContext {
    std::string source;
    long long line = 0, column = 0, operation = -1, disposition = -1;
    std::string lane, payload_type, route, boundary, cause;
    bool redacted = false;
    bool source_verified = false;
};
class DispositionProjectionError : public json::Error {
public:
    DispositionProjectionError(std::string path, std::string reason, DispositionDiagnosticKind kind)
        : json::Error(std::move(path), std::move(reason)), kind(kind) {}
    DispositionDiagnosticKind kind;
};
inline json::Value disposition_diagnostic(DispositionDiagnosticKind kind,
                                          const DispositionDiagnosticContext& c) {
    using namespace json;
    const char* code = "LYRAFORM_DISPOSITION_FORGED_ARTIFACT";
    const char* explanation = "Artifact evidence contradicts disposition authority.";
    Array repairs{"regenerate_from_canonical_authority"};
    switch (kind) {
    case DispositionDiagnosticKind::missing_route:
        code = "LYRAFORM_DISPOSITION_MISSING_ROUTE";
        explanation = "An unsuccessful disposition has no accountable typed destination.";
        repairs = c.lane == "fault"
            ? Array{"regenerate_with_existing_declarative_containment_route"}
            : Array{"regenerate_with_existing_declarative_recovery_route"};
        break;
    case DispositionDiagnosticKind::wrong_type:
        code = "LYRAFORM_DISPOSITION_WRONG_DESTINATION_TYPE";
        explanation = "The destination does not accept the exact disposition payload type.";
        repairs = Array{"regenerate_with_exact_declared_payload_type"}; break;
    case DispositionDiagnosticKind::ambiguous_route:
        code = "LYRAFORM_DISPOSITION_AMBIGUOUS_ROUTE";
        explanation = "More than one route claims the same unsuccessful disposition.";
        repairs = Array{"regenerate_with_one_existing_declarative_route"}; break;
    case DispositionDiagnosticKind::fault_to_recovery:
        code = "LYRAFORM_DISPOSITION_FAULT_TO_RECOVERY";
        explanation = "An integrity fault cannot enter ordinary recovery.";
        repairs = Array{"regenerate_with_existing_declarative_containment_route"}; break;
    case DispositionDiagnosticKind::unsupported_source:
        code = "LYRAFORM_DISPOSITION_UNSUPPORTED_SOURCE";
        explanation = "The source form does not establish the required disposition route authority.";
        repairs = Array{"source_disposition_association_not_yet_supported"}; break;
    case DispositionDiagnosticKind::forged_artifact: break;
    }
    for (const auto* field : {&c.source, &c.lane, &c.payload_type, &c.route, &c.boundary, &c.cause})
        if (field->size() > 4096) throw Error("$.diagnostic", "diagnostic field exceeds admission bound; evidence was not truncated");
    if (c.line < 0 || c.column < 0 || (c.source_verified && (c.source.empty() || c.line < 1 || c.column < 1)))
        throw Error("$.diagnostic", "invalid original source location");
    return Object{{"format", "lyraform.disposition_diagnostic"}, {"version", 1},
        {"code", code}, {"phase", kind == DispositionDiagnosticKind::unsupported_source ? "semantic" : "contract"},
        {"classification", "refusal"}, {"runtime_attempt_created", false},
        {"primary", Object{{"source", c.redacted ? "[redacted]" : c.source},
            {"line", Integer(c.line)}, {"column", Integer(c.column)}, {"verified", c.source_verified}}},
        {"operation_id", Integer(c.operation)}, {"disposition_id", Integer(c.disposition)},
        {"lane", c.lane}, {"payload_type", c.payload_type}, {"route_or_missing_endpoint", c.route},
        {"boundary", c.boundary}, {"related_cause", c.redacted ? "[redacted]" : c.cause},
        {"redacted", c.redacted}, {"explanation", explanation}, {"legal_repair_classes", repairs}};
}
inline std::string render_disposition_diagnostic(const json::Value& diagnostic) {
    using namespace json;
    const auto& d = object(diagnostic, "$.diagnostic");
    const auto& p = object(required(d, "primary", "$"), "$.primary");
    // Render fields from the same fact; never infer a repair or new disposition.
    return string(required(p, "source", "$"), "$") + ":" +
        std::to_string(integer(required(p, "line", "$"), "$")) + ":" +
        std::to_string(integer(required(p, "column", "$"), "$")) + ": " +
        string(required(d, "code", "$"), "$") + ": " +
        string(required(d, "explanation", "$"), "$") + "\n  evidence: " + serialize(diagnostic);
}
} // namespace flowcontracts
