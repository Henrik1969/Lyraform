# Canonical response-attempt failure — Gate 4

Date: 2026-10-07.
Mission: [Canonical Executable Failure Flow — Mission 04](../tasks/canonical-executable-failure-flow-mission-04.md)
Gate result: **PASS for one bounded expected response-failure recovery and rejoin**

## Baseline and protected scope

The stage extends Mission 03 Gate 3 from synchronized `main` baseline
`9de06b6896dfba1346c553fca22d53e0fe0c7576` and its uncommitted Mission 02/03
working set. The unrelated untracked `meta-discusions.md` and generated
`output/` tree remain untouched.

No source syntax, compiler plan, backend, guard runtime, mutation, parallel
execution, `master`, or FlowLFS content was modified.

## Bounded authority

`lyraform.failure_response_attempt_plan/v1` closes one expected-failure branch
of one selected recovering response:

```text
original Failure<E1>, obligation q1
    -> selected response attempt a2
    -> declared Failure<E2>, distinct obligation q2
    -> exact closed-set recovery route
    -> Success<T>
    -> exact typed rejoin
    -> close q2, then close q1
```

The original obligation remains open when the response attempt fails. The
response failure receives a distinct obligation, exact producer disposition
and operation, preserved correlation, no-commit evidence, and response
provenance. Its recovery may rejoin only when the successful payload type
equals the original response transition's promised result type. The final
receipt independently proves the nested recovery and rejoin before recording
original-obligation closure.

## Refusal boundary

The contract refuses obligation aliasing; identity, type, correlation, commit,
or provenance drift; an inferred or type-changing rejoin; cycles through the
same response function; undeclared result classes; fault results; and C++
exceptions escaping without a declared disposition. A host exception is a
contract-boundary failure, not a Lyraform expected failure.

The bounded recovery response is not permitted to fail recursively. Fault
containment remains a separate authority under ADR 0055 and ADR 0056.

## Evidence

Positive evidence exercises `ReadFailure -> use_cache -> CacheUnavailable ->
fallback -> Reading`. It proves that `q1` remains identifiable, `q2` is
distinct, the second response receives the exact nested envelope, and only its
validated `Reading` result rejoins the original recovery continuation.

Hostile evidence covers plan, route, function, transition, disposition,
operation, failure-type, output-type, obligation-law, correlation, commit,
provenance, and rejoin drift; obligation reuse; unexpected success on the
failure-only executor; wrong failure type; fault; undeclared host exception;
forged receipt state; changed final payload; and future artifact versions.

## Verification

- Focused GCC response-attempt test: **1/1 PASS**.
- Focused Clang 18 ASan/UBSan test: **1/1 PASS**, with LeakSanitizer disabled
  for the managed ptraced environment.
- Valgrind 3.22.0: **0 errors**, **0 bytes in use at exit**, 6,363 allocations
  and 6,363 frees.
- `./igor build`: **PASS**.
- `./igor test`: **180/180 PASS**, 59.10 seconds.
- Canonical-language authority freeze: **PASS**.
- Current-documentation drift guard: **PASS** after this checkpoint was
  installed.
- `git diff --check`: **PASS**.

## Deliberate boundary

This is not general recursive failure handling. It does not admit multiple
response-failure types, a response-failure recovery that itself fails, fault
containment, arbitrary chain depth, evidence-budget profiles or compaction,
propagation, retry, sinks, source/compiler/backend integration, mutation, or
parallel composition.

## Next decision boundary

The next safe expansion is no longer another fixed nesting level. General
depth requires an executable evidence-epoch budget and hard-limit disposition
contract under ADR 0063. Fault results require the independently defined fault
containment authority. Either must be established before recurrence can be
admitted.

## Gate 4

**PASS.** One response attempt's expected failure is now explicit flow with a
distinct obligation and exact recovery route; it cannot erase its original
failure or rejoin without type- and identity-proven closure.
