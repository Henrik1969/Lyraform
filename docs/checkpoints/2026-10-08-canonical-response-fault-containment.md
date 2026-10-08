# Canonical response fault containment — Gate 6

Date: 2026-10-08.
Mission: [Canonical Response Fault Containment — Mission 06](../tasks/canonical-response-fault-containment-mission-06.md)
Gate result: **PASS for one response-fault-to-containment link**

## Baseline and protected scope

The stage extends Mission 05 Gate 5 from synchronized `main` baseline
`760b995ec374d82590a9c2b750dd740ab6ce295f` and its uncommitted Mission 05
working set. The unrelated untracked `meta-discusions.md` and generated
`output/` tree remain untouched.

No source syntax, compiler plan, backend, guard runtime, mutation, parallel
execution, `master`, or FlowLFS content was modified.

## Bounded authority

`lyraform.failure_response_fault_plan/v1` connects one selected recovering
response to one exact Mission 05 containment plan:

```text
original Failure<E>, obligation q1
    -> selected response attempt
    -> declared Fault<F>, distinct obligation q2
    -> exact fault-containment authority
    -> halt and quarantine activation
    -> suppress normal publication
    -> no continuation and no rejoin

q1 remains unresolved_contained
q2 is accounted for by the containment receipt
```

The link fixes the original plan, failure type, route, function, and transition;
the containment plan, fault disposition, operation, and type; and the laws for
distinct obligations, correlation, no-transition commit state, provenance, and
halt/quarantine without rejoin.

The composite receipt records the original and fault obligations, response
attempt, correlation, exact fault envelope, validated containment receipt,
original `unresolved_contained` state, final `contained_fault_no_rejoin` state,
suppressed publication, and absent continuation/rejoin.

## Hostile evidence

Validation refuses wrong plan, route, function, transition, containment plan,
fault disposition, operation, type, producer function, obligation,
correlation, commit, provenance, or containment law; obligation aliasing;
response success or expected-failure substitution; forged original closure,
rejoin, or fault envelope; and future versions.

## Verification

- Focused GCC response-fault test: **1/1 PASS**.
- Focused Clang 18 ASan/UBSan test: **1/1 PASS**, with LeakSanitizer disabled
  for the managed ptraced environment.
- Valgrind 3.22.0: **0 errors**, **0 bytes in use at exit**, 3,429 allocations
  and 3,429 frees.
- `./igor build`: **PASS**.
- `./igor test`: **182/182 PASS**, 58.13 seconds.
- Canonical-language authority freeze: **PASS**.
- Current-documentation drift guard: **PASS** after this checkpoint was
  installed.
- `git diff --check`: **PASS**.

## Deliberate boundary

This link neither resolves the original expected failure nor permits recovery
from the integrity fault. It does not admit multiple response fault types,
containment escalation or failure, trusted restart or repair, top-level
termination, broader scopes, source/compiler/backend integration, durable
quarantine, or concurrent composition.

## Next decision boundary

General failure-chain depth now depends on executable ADR 0063 evidence epochs,
closure receipts, deterministic budgets, and hard-limit disposition routing.
That stage must keep safe compaction separate from expected budget failure and
integrity-loss fault containment.

## Gate 6

**PASS.** A response attempt fault now reaches one exact containment authority
without erasing or closing its original expected-failure obligation and without
publishing, continuing, recovering, or rejoining normal flow.
