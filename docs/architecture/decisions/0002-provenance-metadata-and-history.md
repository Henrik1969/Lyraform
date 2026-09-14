# ADR-0002: Provenance metadata and historical truth

**Status:** accepted direction, provisional storage
**Date:** 2026-08-20
**Scope:** Frankencore identity and mutation provenance

## Decision

Frankencore uses a hybrid provenance model:

```text
project metadata
    current descriptions and declared capabilities

project-local history file
    historical identity and decision evidence

artifacts
    stable ULIDs and compact provenance references
```

Project metadata is authoritative for current descriptions. The project-local
append-only history file is authoritative for what happened and what was known
at the time.
Artifacts carry stable ULIDs and enough snapshot information to remain
minimally inspectable when the project metadata or history is unavailable.

No artifact may require mutable current metadata to interpret its basic
identity, revision, status, or provenance relationship.

## Rationale

Embedding all descriptive metadata in every artifact duplicates data and can
make historical records contradictory. Using only a mutable project registry
makes old artifacts change meaning over time. The hybrid preserves both
offline inspectability and historical accuracy.

## Storage direction

The history is generated as a file inside the project and travels with that
project. It must not silently create or depend on a global user or system
history store. A project may explicitly export or replicate the file later,
but that is a project policy rather than an implicit side effect.

The canonical project-local path is:

```text
frankencore/provenance/history.jsonl
```

The explicit visible path is intentional: provenance must be visible in source
reviews, archives, backups, and project inventories. JSON Lines encoding
details and rotation remain provisional until the first durable
implementation; the initial locking policy is defined below.

The first durable implementation will use one JSONL file with:

- exclusive local writer locking;
- one complete JSON object per line;
- atomic append and flush before lock release;
- validation of existing content before append;
- quarantine or explicit recovery handling for an incomplete final line;
- ULID ordering as the primary event order;
- `event_id` deduplication;
- an explicit reconciliation tool for histories produced on different Git
  branches.

The history is append-only semantically, but writers must validate and commit
each complete record atomically. Blind concurrent appends are not permitted.

## Recovery policy

History handling is diagnostic and fail-closed:

1. Inspect and diagnose the file before attempting repair.
2. Report the failure, location, suspected cause, and available recovery.
3. Automatically recover only when the damage is provably limited to an
   incomplete final append and the valid prefix can be preserved exactly.
4. Record the recovery action as an operational diagnostic; recovery must not
   erase the explanation of what happened.
5. Never silently rewrite or discard complete committed records.
6. If recovery is uncertain or would alter valid history, stop and require an
   explicit operator repair action.

Rollback therefore means restoring a known-valid file boundary, not deleting
historical mutations by convenience.

## Error-state artifacts

Diagnostics, recovery plans, quarantine fragments, and repair transcripts are
temporary artifacts belonging to a specific unresolved error state. They are
not part of the permanent mutation history and must not pollute the normal
project record after the error state is resolved.

While unresolved, they must remain visible and identify the relevant
`event_id`, `attempt_id`, and `correlation_id`. Resolution must be explicit and
must record the outcome before those temporary artifacts may be removed.
Permanent history retains the fact and outcome of the error state, not the
temporary repair material itself.

## Current implementation boundary

The C++ core provides ULID generation, mutation-event structures, and a
project-local `ErrorStateHistory` boundary. The boundary uses an exclusive
writer lock, validates the complete JSONL prefix before append, deduplicates
identical event IDs, rejects conflicting IDs, flushes successful appends, and
requires explicit quarantine/repair for an incomplete final line. The current
implementation performs full JSON syntax validation and known-record
type/status validation, plus bounded inspection, ordered reads, and exact
identity lookup, error-state lifecycle replay, and linear mutation
revision/state replay. Retention, crash fault injection, and cross-branch
reconciliation remain future work.

## Revisit triggers

Revisit when implementing the first durable history file, especially before
choosing retention, compaction, replay, locking refinements, or export rules.
