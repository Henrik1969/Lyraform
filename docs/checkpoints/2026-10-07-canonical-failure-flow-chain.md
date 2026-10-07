# Canonical executable failure flow — Gate 3

Date: 2026-10-07.
Mission: [Canonical Executable Failure Flow — Mission 03](../tasks/canonical-executable-failure-flow-mission-03.md)
Gate result: **PASS for one transform-to-recovery successor chain**

## Baseline and protected scope

The stage extends Mission 02 Gate 2 from synchronized `main` baseline
`9de06b6896dfba1346c553fca22d53e0fe0c7576`. The unrelated untracked
`meta-discusions.md` and generated `output/` tree remain untouched.

No source syntax, compiler plan, backend, guard runtime, mutation, parallel
execution, `master`, or FlowLFS content was modified.

## Bounded authority

`lyraform.failure_flow_chain/v1` composes exactly two already-valid closed-set
plans:

```text
Failure<E1>
    -> exact transform transition
    -> explicit successor handoff
    -> Failure<E2> with linked successor obligation
    -> exact recovery transition
    -> Success<T> and closure
```

The handoff identifies the source plan, source failure type, source transition,
destination plan, successor disposition, successor operation, and successor
failure type. It also fixes the evidence laws for correlation, payload,
no-commit state, and provenance.

The successor envelope is constructed only from the validated transformation
receipt. It carries the response attempt as the producing attempt, the linked
successor obligation, the response payload, unchanged correlation and
no-commit evidence, and response provenance as its new origin.

## Evidence

Positive evidence proves one `ReadFailure -> SensorUnavailable -> Reading`
chain and verifies the exact successor envelope observed by the recovering
function.

Negative and hostile evidence refuses invalid chain identity or schedule;
plan/disposition identity cycles; wrong source plan, transition, destination
plan, successor disposition, operation, or type; changed correlation, payload,
commit, or provenance laws; a first stage that does not transform; a second
stage that does not recover; changed successor-obligation closure; wrong
origin type; and future chain versions.

## Verification

- Focused GCC chain test: **1/1 PASS**.
- Focused Clang 18 ASan/UBSan chain test: **1/1 PASS** with LeakSanitizer
  disabled for the managed ptraced environment.
- Valgrind 3.22.0: **0 errors**, **0 bytes in use at exit**, 5,317 allocations
  and 5,317 frees.
- `./igor build`: **PASS**.
- `./igor test`: **179/179 PASS**, 58.81 seconds.
- Canonical-language authority freeze: **PASS**.
- Current-documentation drift guard: **PASS**.
- `git diff --check`: **PASS**.

## Deliberate boundary

This is a fixed two-stage reference contract, not general propagation. It does
not define arbitrary depth, retention-policy syntax, compaction, retry,
response-attempt failure, sinks, faults, source/compiler/backend integration,
mutation, or parallel composition.

## Next decision boundary

Further depth is no longer a mechanical repetition. General chains require a
canonical evidence-budget and hard-limit disposition contract under ADR 0063;
response functions that fail require their own `Failure<E>`/`Fault<F>`
accounting; compiler integration requires an authoritative producer/consumer
projection. Those should be selected as separate stages rather than hidden in
an unbounded loop.

## Gate 3

**PASS.** One transformation successor is now carried into and closed by one
exact recovery stage without inferred routing or evidence loss.
