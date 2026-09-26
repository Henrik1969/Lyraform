# Gate 2 — bounded execution hardening

Date: 2026-09-26.

Gate 2 hardening: **PASS**.

## Baseline

Branch `main`, HEAD `6ba9db303db453af952327069814c3a607b26d03`.
The existing Mission 01/02 work was unstaged: 15 modified and 27 untracked
paths at entry. This stage preserves that work and does not stage, commit,
push, modify branches, or touch FlowLFS or `meta-discusions.md`.

Mission: [Gate 2 execution hardening](../tasks/gate-2-execution-hardening.md).

## Why Gate 2 was reopened

The 2026-09-19 checkpoint's existing 165 tests still passed, but review found
four unsafe source variants admitted through both backends: an early return
before handling, overwriting the projected tag, overwriting its zero
comparator, and using the success value after disposal. The independent
disposition validator also admitted a forged second disposal; a later
authorization check happened to reject that particular artifact.

The old hostile backend tests passed semantic reports to backends that require
optimized/lowering artifacts. Even an unmodified report failed for the wrong
format, so those tests did not establish the claimed backend protection.
The historical checkpoint remains unchanged; this report supplies corrected
evidence for the bounded boundary.

## Implementation and authority

`Flowcontracts/outcome_execution.hpp` contains a shared bounded execution
relation check. Flowanalyst projects its resolved operations into that check;
the artifact validator independently reconstructs the same projection from
serialized operations. No backend defines new semantics, and no new fact
format or alternate execution ordering was introduced.

The check uses existing statement ordering within blocks, matching the
structured LLVM and TinyVM consumers. Operation IDs remain identities, not
execution order. It enforces:

- ordered production, direct code projection, and two distinct sibling tests;
- one containing block/function and no additional entries into handling blocks;
- no nested producer control-flow route or unsupported handling exits;
- no return or intervening control flow that bypasses accounting;
- an unchanged projected tag and unchanged named zero comparator;
- no replacement of the tagged owner;
- direct, read-only, call-lifetime borrowed success-value uses;
- exactly one canonical cleanup after every admitted value use.

Cleanup is recognized by its resolved provider contract/signature rather than
the spelling `dispose`. Payload copies into locals are refused so aliases
cannot hide later uses. The sample now puts external-call return values in a
separate `status` local instead of overwriting the tag being tested.

This is conservative bounded admission. The tag and comparator must remain
unchanged in the containing function; handling branches must fall through.
It does not infer general liveness or introduce ownership, propagation, or
early-return handling semantics for unsupported source shapes.

## Regression evidence

The focused Text outcome gate now contains:

- four positive programs executed on both LLVM and TinyVM: ordinary success,
  provider failure, literal-zero comparisons, and reversed sibling branches;
- seventeen source refusals, including prior cases and the new mutation,
  ordering, exit, and payload-copy counterexamples;
- twenty-eight hostile mutations checked independently at three consumers
  (84 expected contract rejections);
- exact disposition preservation through parallel, optimized, and lowering
  plans.

Each hostile consumer first accepts an unmodified control in its own format.
Attacks use semantic reports for Flowvalidate, optimized reports plus bindings
for LLVM, and lowering artifacts for TinyVM. Rejections must identify an
outcome/disposition contract; a nonzero exit by itself is insufficient.

Additional attacks cover forged ordering, branch block/function linkage, extra
control-flow edges, extra disposal, changed borrowing metadata, and an inserted
early return. The independent disposal rejection now occurs before unrelated
authorization checks.

## Verification

- Igor doctor: PASS.
- Canonical build: PASS; the existing Flowparallel unused-function warning
  remains unchanged.
- Focused Text outcome and static guard tests: 2/2 PASS, 2.46 seconds.
- Clang 18 ASan/UBSan focused tests: 2/2 PASS, 5.65 seconds. LeakSanitizer is
  disabled for the established host ptrace-wrapper limitation; leak checking
  is covered separately by Valgrind.
- Valgrind producer and validator probes: 2/2 PASS, zero errors and zero
  outstanding bytes for each.
- Shell syntax and `git diff --check`: PASS.
- Final canonical `./igor test`: **165/165 PASS**, 76.16 seconds.

## Remaining boundary

Generic Outcome types, copy/move/borrow language rules, returning or propagating
outcomes, containers, policy sinks, loops/nested accounting, concurrency, and
runtime guards still require their separate decisions. This stage repairs
the existing local TextOutcome guarantee and does not authorize any of those
extensions.
