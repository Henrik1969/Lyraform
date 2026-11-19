# ADR 0068: Complete ordinary record construction

## Status and provenance

Accepted language decision on 2026-10-17 by Henrik Sørensen, responding
“Adopt complete record construction (recommended)” to the
[concrete decision brief](../../development/ordinary-record-construction-decision.md).

## Context

ADR 0052 establishes independent aggregate values. Record readiness probes
exposed permissive parsing without executable construction support. This decision
settles the bounded construction rules needed for general application data.
It applies to ordinary records, independently of application or fixture names.

## Decision

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

## Consequences and limits

Semantic analysis must reject invalid construction before executable admission.
Backends must preserve declared field identity and independent values regardless
of their private representation. A parsed record or accepted rule is not evidence
of implementation. Existing unsupported records remain refused until their full
execution contract is implemented and verified.

Member writes, new root write authority, recursive/reference/resource-bearing
records, resource lifetimes and effectful initializer ordering require their own
accepted authority. This decision does not grant an application-specific compiler
exception or any public layout/foreign ABI guarantee.
