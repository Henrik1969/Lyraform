# Frankencore Mutation-Provenance Contract

**Status:** contract with reference evidence producer; storage mechanism intentionally unspecified

This contract defines the evidence required when a published semantic entity
changes. The canonical in-process representation is the C++20
`frankencore::provenance::MutationRecord` API in the
`Frankencore::Provenance` target. It does not prescribe a
database, snapshot format, persistent map, copy-on-write implementation, or
runtime storage API.

Successful publications use `MutationRecord`. Failed attempts use the
separate `MutationRejection` type, sharing `MutationAttempt` metadata. Their
JSON projections use a common mutation-event family while preserving the
semantic distinction between rejected work and published state.

Every emitted result carries a mandatory `event_id`. Every execution attempt
carries mandatory `attempt_id` and `correlation_id` values. A retry receives a
new attempt and event identity while retaining the correlation identity.

## Required record

Every recorded mutation must identify:

```text
entity_identity
old_revision
new_revision
actor_identity
provider_identity
authorizing_policy
before_state_reference
after_state_reference
operation
atomicity
recoverability
rollback_reference
causes
derived_entities
```

## Laws

1. A published revision is immutable.
2. A mutation creates a new revision or an explicitly rejected result.
3. The old revision remains addressable while retention policy permits it.
4. The actor/provider responsible for the mutation is recorded.
5. The policy authorizing the mutation is recorded, or the record states that
   no policy authorization was required.
6. Before/after evidence is sufficient to identify the changed state.
7. Atomicity and recoverability are explicit rather than inferred.
8. Failure must not publish a partially applied semantic state.
9. Derived diagnostics and projections identify the revision they describe.
10. Rollback is a policy/provider operation, not an implicit rewrite of
    history.

## Initial implementation boundary

The first implementation is the C++20 API plus the versioned JSON evidence record emitted by
`Flowtools/reference/revision/frankencore_revision_probe`. It is a conformance
fixture and reference producer only; JSON is an inspectable projection and
does not force the future runtime storage model. Its contract is checked by
`tools/check-frankencore-mutation-provenance.sh`.

The promoted core currently lives at `Frankencore/Core/Provenance` and is
exposed to CMake consumers as `Frankencore::Provenance`.

The intended durable history is a generated file kept inside the project. It
must follow the project and must not silently pollute a global user or system
store. Its canonical visible path is
`frankencore/provenance/history.jsonl`; encoding evolution, rotation, and
locking remain provisional.

The initial storage policy is one JSONL file with exclusive local writer
locking, complete-line atomic appends, ULID ordering, event-ID deduplication,
and explicit branch-history reconciliation. Blind concurrent appends are not
permitted.

History recovery is diagnostic and fail-closed. A writer must report the
failure and location first, may automatically restore only a provably valid
prefix after an incomplete final append, and must preserve the diagnosis.
Complete committed records may never be silently rewritten or discarded. Any
uncertain repair requires explicit operator action.

Diagnostics, quarantine fragments, recovery plans, and repair transcripts are
temporary error-state artifacts. They identify the related event, attempt, and
correlation ULIDs while the error remains unresolved. Once resolution is
explicitly recorded, they may be removed; permanent history retains the error
and its outcome, not temporary repair material.

Error-state lifecycle is a separate semantic family, `ErrorStateEvent`, with
statuses such as `opened`, `diagnosed`, `recovery_attempted`, `resolved`,
`escalated`, and `reopened`. Resolving an error state is not automatically a
mutation. Every error state has a mandatory `error_state_id` ULID; lifecycle
events also carry event, attempt, and correlation ULIDs.

The core validates JSON syntax, event shape, field types, and status
vocabulary. The history boundary also validates legal error-state transitions
and linear mutation revision/state continuity. Policy authorizes exceptional
transitions, which must record their authorization and reason; history records
the resulting fact immutably.

The status vocabulary is additive rather than a closed enum. Extensions are
permitted when they preserve the constitutional laws and applicable policies;
the resolver must understand or explicitly reject them before authoritative
publication. Extensions must not redefine existing status meanings.

Project metadata defines an extension status's meaning and lifecycle rules;
providers declare support; the resolver validates the match before publication.
A provider cannot define project semantics by itself.

Declaration scope follows capability scope. Project-specific statuses remain
project metadata; shared provider capabilities use integration metadata; only
genuinely universal capabilities become system-level contracts. Promotion to
a broader scope requires independent consumers and explicit authority,
versioning, and compatibility rules.
