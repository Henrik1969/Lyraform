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
reported success condition. Inspection and append are also bounded by a
64 MiB total-history default and a 1 MiB per-record default; exceeding either
bound yields an explicit `exhausted` result without publishing a record.

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
abrupt-process-exit with torn tail recovery: PASS
explicit quarantine and repair: PASS
quarantine collision / uncertain-recovery refusal: PASS
post-repair append: PASS
read-only branch divergence reconciliation: PASS
same-identity conflicting branch reconciliation: PASS
typed mutation final-state replay projection: PASS
ordered validated read/replay projection: PASS
exact identity lookup and invalid-ID refusal: PASS
total-history bound and no-publication exhaustion: PASS
full JSON syntax and known-record type validation: PASS
error-state lifecycle replay refusal: PASS
linear mutation revision/state replay refusal: PASS
```

Partial writes are fault-injected as well: if a write changes the file but
does not complete, append returns `uncertain` with `changed=true`, and the
incomplete tail remains visible for explicit repair. This prevents a torn
record from being reported as an ordinary no-change error. Zero-progress
after a partial write follows the same path with a deterministic I/O error;
the returned record count includes only complete records, never torn bytes.
The parent-directory synchronization barrier is fault-injected independently;
when file `fsync` succeeds but directory `fsync` fails, append also returns
`uncertain` while the complete record remains inspectable.

This is a durable error-state boundary, not a complete history subsystem.
Retention and deeper crash fault injection remain open Gate 5 work. The history test
now uses a child process that exits immediately after writing an incomplete
record, then proves the parent refuses publication until explicit quarantine
and repair. It also compares two independently copied histories without
rewriting either input: common events are counted and branch-only events are
reported explicitly. Typed mutation final-state replay now exposes only
continuity-validated state; negative revisions are rejected before projection.
The storage-boundary test also writes a hostile negative-revision record and
confirms that inspection rejects it as invalid before replay.
Error-state lifecycle replay
now rejects records that do not begin with `opened` or that violate the
documented transition graph. Linear mutation replay also requires each
subsequent record's old revision and before-state reference to match the
previous committed state for that entity.

The focused provenance trio (`frankencore_revision_contract`,
`frankencore_provenance_api`, and `frankencore_error_state_history`) also
passes under Clang 18.1.3 ASan/UBSan with the documented leak exclusions.
The complete sanitizer CTest matrix is not credited here: 24 subprocess tests
were rejected by the environment's incompatible-ASan-runtime supervision
boundary, while the remaining tests passed.

The history and provenance API executables pass independently under Valgrind
3.22.0 Memcheck with full leak checking and zero errors.
After the per-record bound change, the history executable and the injected Text
allocation-failure executable were rerun independently under the same gate;
both completed with zero errors and zero leaks. The partial-write injection was
also included in the history executable's current Memcheck run and completed
with zero errors and zero leaks.

The current project-facing smoke gates also pass:

```text
./igor doctor: PASS
./igor build: PASS
./igor test: PASS (115/115)
```

The updated history test also passes under the existing Clang 18.1.3
ASan/UBSan build:

```text
ASAN_OPTIONS=detect_leaks=0:verify_asan_link_order=0 \\
UBSAN_OPTIONS=halt_on_error=1 \\
/tmp/lyraform-asan-20260914/flowtools/reference/revision/frankencore_error_state_history_test: PASS
```
