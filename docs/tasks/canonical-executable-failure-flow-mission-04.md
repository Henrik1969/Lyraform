# Canonical Executable Failure Flow — Mission 04

Date: 2026-10-07.

## Objective

Close the bounded response-attempt failure hole: one selected recovering
response may itself produce one declared expected `Failure<E2>`, whose exact
route may recover a value that rejoins the original response transition.

## Baseline

Mission 03 Gate 3 on synchronized `main` baseline
`9de06b6896dfba1346c553fca22d53e0fe0c7576` plus the uncommitted Mission 02 and
Mission 03 working set.

## Bounded question

Which identities and receipts prove that a response attempt's expected failure
creates a distinct obligation, leaves the original obligation open, and may
close both only through an explicitly typed recovery-and-rejoin path?

## Scope

- one versioned serial response-attempt failure/recovery plan;
- one original recovery transition;
- one declared expected failure type from its selected response function;
- one distinct linked response-failure obligation;
- one exact closed-set plan that recovers that failure;
- one explicit rejoin whose output type equals the original transition result;
- preserved correlation, no-commit evidence, and provenance;
- independently validated final receipt;
- positive, negative, and hostile evidence.

## Explicit non-goals

No faults or fault containment, multiple response-failure types, recursively
failing recovery responses, arbitrary chain depth, evidence-budget profile,
compaction, propagation, retry, sink, source/compiler/backend integration,
mutation, or parallel execution.

## Gate 4

PASS only when a failed response cannot be mistaken for successful recovery,
its new obligation cannot erase or alias the original obligation, and original
closure requires an exact successful recovery receipt plus a type-correct,
identity-bound rejoin.
