# ADR 0058: One canonical disposition fact, many projections

## Status

Accepted on 2026-09-19.

## Decision

Lyraform has one canonical operation-disposition fact. Tagged outcomes,
function signatures, graph ports and wires, provider responses, execution
records, diagnostics, and debugger views are contextual projections of that
fact rather than independent failure mechanisms.

The canonical fact establishes at least:

```text
operation identity
exclusive possible dispositions
success, failure, and fault payload type identities where declared
commit/no-commit law
origin and provenance
disposition route identities where routing exists
```

Every projection must preserve the discriminant, declared type identities,
route completeness, commit law, and provenance relevant to that projection. A
consumer that cannot represent the fact refuses it; it must not weaken,
reclassify, terminate, or silently discard a disposition.

## Projection law

The same semantic completion may appear as:

```text
sequential/value context    tagged outcome projection
function boundary           declared result/disposition signature
graph context               typed disposition ports and wires
provider boundary           versioned response contract
execution/audit boundary    disposition record and observations
```

Those forms need not share one memory layout or surface spelling. Their
equivalence is established by versioned contracts and validators, not by
convention.

## Diagnostics and debugging

Human diagnostics and debugger/machine views follow ADR 0057. They project
canonical evidence but are not semantic destinations and do not close a
dangling wire.

## Consequences

- Backends cannot invent target-specific failure meaning.
- Dangling-wire validation has one semantic authority.
- Value-oriented and graph-oriented source can retain appropriate forms.
- Existing `Outcome<Text,TextFailure>`, graph-failure evidence, and provider
  responses remain bounded projections until validated against this authority.
- A projection mismatch is a contract refusal, never compatibility fallback.

## Deferred

This ADR does not define:

- concrete source syntax;
- the complete serialized schema;
- failure/fault type hierarchies;
- recovery syntax;
- top-level policy declarations;
- runtime guard routing;
- retry, cancellation, timeout, backpressure, or concurrent failure
  composition.

The first bounded implementation is governed by
[Canonical Disposition Authority — Mission 01](../../tasks/canonical-disposition-authority-mission-01.md).
