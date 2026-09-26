# Gate 2 — bounded execution and rejection evidence hardening

## Objective

Make the existing local `Outcome<Text,TextFailure>` accounting proof sound
against the 2026-09-26 review's execution-order, mutation, exit, and disposal
counterexamples. Repair backend hostile tests so they exercise accepted input
formats and assert a disposition rejection.

## Baseline and scope

- `main`, HEAD `6ba9db303db453af952327069814c3a607b26d03`.
- Existing unstaged Mission 01/02 work: 15 modified and 27 untracked paths at
  entry. Preserve it and all unrelated work.
- User explicitly authorized implementing the Gate 2 review findings.
- No staging, commit, push, branch changes, or FlowLFS work.
- Existing bounded accounting semantics remain authoritative; unknown routes
  are refused rather than given new ownership or propagation semantics.

## Required evidence

- Refuse overwritten tag/zero operands, early exits, value use after disposal,
  payload aliases, and extra cleanup operations.
- Reconstruct the execution relations from canonical operation, statement,
  block, function, and provider identities at producer and consumer boundaries.
- Retain positive success/failure execution, literal-zero comparisons, and
  reversed sibling branch order.
- Prove valid controls before hostile attacks on semantic, optimized, and
  lowering artifacts; assert outcome/disposition rejection diagnostics.
- Preserve disposition facts through all carrying stages.
- Canonical Igor doctor/build/test, focused ASan/UBSan, Valgrind producer and
  consumer probes, shell syntax, and diff checks.

## Non-goals

General Outcome types, copy/move/borrow language syntax, propagation, containers,
concurrency, runtime guards, and general path-sensitive ownership inference.

## Gate

PASS when the regressions refuse, valid controls execute on both backends,
hostile artifacts fail for the intended contract reason, and required checks
pass. Record a new checkpoint without rewriting historical evidence.
