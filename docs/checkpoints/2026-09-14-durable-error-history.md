# Durable error-state history checkpoint

**Date:** 2026-09-14  
**Status:** implemented and verified on `main`

Frankencore Provenance now provides `ErrorStateHistory` for the project-local
JSONL boundary described by ADR-0002. It uses an exclusive lock file, validates
the complete valid prefix before publication, appends and flushes one complete
event per line, deduplicates exact replay by `event_id`, and rejects conflicting
duplicate content.

An incomplete final line makes the history non-publishable. Appends refuse to
continue until the operator explicitly calls `repair_incomplete_tail()`. That
operation writes the incomplete bytes to a non-overwriting `.quarantine` file,
truncates only to the known valid prefix, flushes the repaired history, and
reports the quarantine path.

Verification covers:

```text
append and valid-prefix inspection: PASS
exact duplicate replay: PASS
conflicting duplicate event: PASS
torn-tail refusal: PASS
explicit quarantine and repair: PASS
post-repair append: PASS
```

This is a durable error-state boundary, not a complete history subsystem.
Full JSON parsing, event lookup/replay, retention, and cross-branch history
reconciliation remain open Gate 5 work.
