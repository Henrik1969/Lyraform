# Decision adopted: ordinary record construction

Status: option A accepted by Henrik on 2026-10-17.
Canonical authority: [ADR 0068](../architecture/decisions/0068-complete-ordinary-record-construction.md).
Decision provenance: explicit response, “Adopt complete record construction (recommended)”.
Purpose: enable general Fact/Result-like values for the ConfigResolve port without
application-specific compiler code. Governed by ADR 0052 and the stage protocol.

## Evidence

ADR 0052 accepts independent aggregate values and reconstruction/rebinding for
member updates, but explicitly defers declaration syntax, write authority,
ownership and backend layout. Structural `record` declarations and named-field
literals already parse. Nine readiness probes show name lookup rejects unknown
types/members; construction with missing, extra or wrong-type fields currently
gets a ready plan and fails only at native lowering. That is not semantic support.

## Accepted bounded rule

For an ordinary finite record with declared fields:

1. Construction names every declared field exactly once; missing, duplicate or
   unknown fields are rejected. No implicit defaults or omitted-field values.
2. Field values must have the declared type under already accepted conversions;
   this decision adds no implicit conversions.
3. Named initializer order does not change which field receives a value.
   For the first executable slice initializer expressions must be pure; effectful
   field evaluation order remains explicitly unadmitted pending a separate rule.
4. Field projection yields its declared typed value. Copying/passing/returning
   preserves ADR 0052 independent-value semantics.
5. Backend layout is private, with no C ABI or source-visible offset promise.
   The first slice contains only already-admitted scalar values and finite nested
   records; recursive/reference/resource-bearing fields remain unadmitted.
6. This does not admit member writes, new root write authority, references,
   shared identity, borrowing, resource lifecycle or generic collections.

## Alternatives

A. Adopt these explicit complete-construction rules (recommended). This supports
ordinary data modeling with strong validation and no hidden initialization.
B. Allow partial construction with defaults or later initialization. This needs
additional defaulting/definite-initialization and failure laws before execution.
C. Defer ordinary records. Continue independent disposition work, but the full
port remains dependent on a data-model decision.

Existing historical permissiveness is not canon. Under A, formerly parsed but
unsupported invalid constructions receive early diagnostics. No previously
claimed executable ordinary-record behavior is intentionally changed.

Decision: A. The owner explicitly adopted complete record construction on
2026-10-17. The alternatives above are retained as decision history. Acceptance
establishes semantics; it does not establish compiler support or an executable
record implementation. Member writes and resource lifetimes remain separate.
