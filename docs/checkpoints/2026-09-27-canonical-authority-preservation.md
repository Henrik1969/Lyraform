# Canonical authority preservation — Stage 4A

Date: 2026-09-27.

## Objective

Audit every active artifact route carrying callable-plan-v2 source-call and
deferred independence authority. Close silent discard, mutable-summary, and
validation-bypass gaps without creating scheduling or concurrency meaning.

## Route inventory

| Route | Before Stage 4A | Stage 4A result |
|---|---|---|
| Flowanalyst → Flowparallel | exact semantic validation | retained |
| Flowparallel execution plan | lowering plan and call projection retained; effect/candidate evidence collapsed to counts | exact evidence preserved and revalidated |
| semantic report → Flowoptimize | semantic input validated; emitted report discarded effect/candidate evidence | exact evidence preserved |
| execution plan → Flowoptimize | lowering plan validated without the source-call/effect/candidate relationship | full relationship revalidated |
| Flowoptimize → Flowlower | lowering plan carried; exact evidence absent | exact evidence preserved and revalidated |
| Flowoptimize → Flowprepare → backend artifact → LLVM/TinyVM | backend artifact discarded exact evidence | exact evidence preserved and revalidated by both backends |
| Flowbind side authorization | consumes provider requirements but does not carry call/candidate semantic authority | classified as a non-carrying authorization branch; later pairing remains checked against lowering operations |
| lowering-plan v1 | historical compatibility | unchanged; no Stage 4A claim |

## Closed defects

Callable-plan-v2 carrying artifacts now retain, byte-for-byte:

- `external_operations`;
- `effect_facts`;
- `parallel_candidates`;
- `lowering_plan`.

Shared validation recomputes source-call and candidate relationships at the
execution-plan, optimization-report, and prepared-backend boundaries.
Flowparallel's candidate and pure-callable summary counts must agree with the
exact preserved evidence. Missing evidence is a contract failure, not an empty
or inferred default.

The direct semantic-report input accepted by Flowoptimize produces the same
preserved authority as the ordinary Flowparallel route.

## Explicit non-claims

Candidate status remains `deferred`. Preservation does not create execution
waves, scheduling policy, concurrency guarantees, async behavior,
backpressure, cancellation, retry, or effectful parallelism.

## Initial evidence

- fresh configure/build: PASS;
- focused preservation, candidate, provider, identity, Flowparallel,
  Flowoptimize, backend-artifact, and Flowlower gates: PASS;
- direct and ordinary Flowoptimize routes preserve identical authority;
- wrong call projection, peer operation, effect fact, operation conflict,
  summary count, and missing-evidence attacks: refused at each applicable
  consumer.

## Final verification

- full canonical suite: **172/172 PASS**, 77.45 seconds;
- ASan/UBSan end-to-end hostile preservation gate: PASS (`detect_leaks=0`
  because the host ptrace wrapper prevents LeakSanitizer operation);
- Valgrind Flowoptimize consumer: **0 errors**, 0 bytes in 0 blocks at exit;
- Valgrind Flowprepare consumer: **0 errors**, 0 bytes in 0 blocks at exit;
- Valgrind LLVM Flowlower consumer: **0 errors**, 0 bytes in 0 blocks at exit;
- Valgrind TinyVM lowering consumer: **0 errors**, 0 bytes in 0 blocks at exit;
- repeated execution-plan, optimization-report, and backend-artifact emission:
  byte-identical;
- LLVM and TinyVM both reject hostile backend carrying-authority mutations;
- current-documentation, canonical-authority, and legacy-deprecation drift
  guards: PASS;
- Igor doctor: PASS;
- repository diff whitespace check: PASS.

## Gate 4A

**PASS.** Every active callable-plan-v2 route carrying source-call and deferred
independence authority now preserves the exact evidence and revalidates it
before further transformation or LLVM/TinyVM lowering. Summary counts cannot
contradict exact evidence. Direct optimization and prepared-backend routes do
not bypass the contract. No scheduling or runtime-concurrency meaning changed.

Recommended next stage: inventory the remaining canonical fact families across
the same carrying boundaries and prove that each is either preserved inside
the validated lowering plan, deliberately consumed into a separately validated
authorization artifact, or explicitly absent from non-carrying compatibility
routes. Do not remove plan-v1 compatibility until its users and replacement
evidence are classified.
