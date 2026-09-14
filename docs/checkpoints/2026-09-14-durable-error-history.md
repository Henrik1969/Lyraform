# Durable error-state history checkpoint

**Date:** 2026-09-14  
**Status:** implemented and verified on `main`

Frankencore Provenance now provides `ErrorStateHistory` for the project-local
JSONL boundary described by ADR-0002. Despite its compatibility name, the
store accepts committed mutation records, rejected mutation attempts, and
error-state lifecycle events. It uses an exclusive lock file, validates
the complete valid prefix before publication, appends and flushes one complete
event per line, deduplicates exact replay by `event_id`, rejects conflicting
duplicate content, and exposes an ordered `read_records()` projection only
after the history prefix validates. `find_event(event_id)` provides exact
identity lookup with explicit `found`, `not_found`, and rejected-ID outcomes.

Successful append and repair operations now flush both the history file and
its parent directory, so file creation/truncation durability is part of the
reported success condition.

An incomplete final line makes the history non-publishable. Appends refuse to
continue until the operator explicitly calls `repair_incomplete_tail()`. That
operation writes the incomplete bytes to a non-overwriting `.quarantine` file,
truncates only to the known valid prefix, flushes the repaired history, and
reports the quarantine path.

Verification covers:

```text
append and valid-prefix inspection: PASS
mutation and rejection event append: PASS
exact duplicate replay: PASS
conflicting duplicate event: PASS
torn-tail refusal: PASS
explicit quarantine and repair: PASS
post-repair append: PASS
ordered validated read/replay projection: PASS
exact identity lookup and invalid-ID refusal: PASS
```

This is a durable error-state boundary, not a complete history subsystem.
Full JSON parsing, semantic event replay, retention, and cross-branch history
reconciliation remain open Gate 5 work.
