# Canonical source-operation coverage — Gate 1

Date: 2026-09-26.

## Result

The first compiler-island bridge is connected without adding syntax or language
meaning. Callable lowering-plan v2 now fails closed unless every structural
source statement has an explicit disposition.

The versioned `lyraform.source_operation_coverage/v1` evidence classifies each
statement as one of:

```text
lowered
static_semantic
declaration_only
elided_pure
graph_projection
refused
```

Only `complete` coverage may enter Igor's bounded canonical execution path.
The shared Flowcontracts validator checks identities, counts, dispositions, and
operation references. Flowparallel and Flowoptimize preserve the lowering plan
unchanged.

## Correctness defect closed

Reconnaissance found that an accepted program containing `print x` for an
integer could produce `status: ok` while the staged lowering plan silently
omitted the print statement. The new negative fixture proves that callable
plan v2 now reports `FLOWANALYST_SOURCE_OPERATION_GAP` at the original source
location and emits no executable plan.

Deprecated legacy plan v1 retains source-operation evidence for inspection but
is oracle evidence only, never canonical execution authority. Historical
behavior has not been silently reclassified as canonical semantics.

## Igor bridge

`igor run-canonical` now drives the existing bounded chain:

```text
structural frontend
    -> Flowanalyst callable plan v2
    -> complete source-operation coverage gate
    -> Flowparallel
    -> Flowoptimize
    -> Flowlower / LLVM
    -> native executable
```

`igor run` is the canonical command; `igor run-canonical` remains an explicit
alias. `igor run-legacy` isolates the deprecated behavior oracle. Canonical
execution does not fall back when analysis or lowering refuses source.

## Aggregate reconstruction correction

Aggregate member reconstruction was rejected as the first executable bridge.
Ordinary record construction, layout, and construction completeness remain
deliberately undefined; implementing reconstruction first would have invented
prerequisite semantics during the freeze. The authority map now records that
blocked prerequisite.

## Verification

- shared-contract unit test: PASS;
- canonical positive execution: PASS, native exit status 42;
- omitted-operation negative fixture: PASS, refused before execution;
- focused convergence/compiler tests: **5/5 PASS**, 2.49 seconds;
- hostile contradictory coverage at Flowparallel and Flowlower: PASS, refused;
- accumulated-tree canonical suite: **169/169 PASS**, 80.12 seconds;
- fresh GCC 13.3 Debug build and suite: **169/169 PASS**, 80.23 seconds;
- Clang 18.1.3 ASan/UBSan focused tests: **2/2 PASS**;
- Valgrind focused probes: **4/4 clean**;
- repeated semantic analysis: byte-identical PASS;
- `git diff --check`: PASS.

LeakSanitizer cannot operate under the host's ptrace wrapper. ASan/UBSan was
rerun with leak detection disabled, and leak checking was covered independently
by Valgrind.

A separate fresh GCC Release run reached **168/169**: the unrelated
`frankencore_error_state_history` test performs setup inside `assert(...)`, so
`-DNDEBUG` removes that setup and the test later aborts on a missing file. The
normal assertion-enabled fresh GCC suite is green. This release-test defect is
recorded as a remaining infrastructure risk and was not changed under this
compiler-convergence stage.

## Gate

Gate 1: **PASS**. The full canonical and fresh assertion-enabled GCC suites are
green, supported source executes through the bounded canonical chain, and
unaccounted source fails closed. The next stage may connect only facts whose
meaning is already admitted; aggregate reconstruction, general failure routing,
owned parameters, and other decision-bound semantics remain excluded.

## Remaining convergence inventory

The 98-file historical pass corpus currently contains 41 programs with one or
more callable-plan-v2 source-operation gaps. They remain legacy oracle inputs
but are explicitly refused as canonical executables. Each must be
migrated by generic semantic family; fixture-specific compiler dispatch is
forbidden.
