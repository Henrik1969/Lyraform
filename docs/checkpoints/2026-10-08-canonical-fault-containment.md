# Canonical fault containment — Gate 5

Date: 2026-10-08.
Mission: [Canonical Fault Containment — Mission 05](../tasks/canonical-fault-containment-mission-05.md)
Gate result: **PASS for one activation-scope halt-and-quarantine route**

## Baseline and protected scope

The stage began from synchronized `main` at
`760b995ec374d82590a9c2b750dd740ab6ce295f`, equal to `origin/main`. The
unrelated untracked `meta-discusions.md` and generated `output/` tree remain
untouched.

No source syntax, compiler plan, backend, guard runtime, mutation, parallel
execution, `master`, or FlowLFS content was modified.

## Decision and authority

Henrik selected Candidate A on 2026-10-08: a fault halts and quarantines its
exact declared scope with no local continuation. ADR 0065 records that law.

`lyraform.fault_containment_plan/v1` binds one exact producer `Fault<F>`, typed
fault wire, activation-scope containment authority, and versioned policy
selection under `serial_halt_quarantine_v1`. The only admitted action is:

```text
halt exact activation
    + quarantine it
    + suppress normal publication
    + permit no local continuation
    + preserve fault evidence
```

`lyraform.fault_envelope/v1` carries disposition, obligation, producing
operation and attempt, correlation, exact fault type and payload, no-commit
evidence, and provenance. Dispatch uses the exact authority identity.

`lyraform.fault_containment_receipt/v1` records the plan, wire, authority,
obligation, scope, action, resulting scope state, publication and continuation
state, correlation, fault type, origin evidence, containment provenance, and
`contained_no_continuation` transition. It accounts for the route without
claiming recovery or success.

## Hostile evidence

Validation refuses invalid or future plan and receipt versions; wrong producer,
wire, authority, type, scope, action, or policy identities; expected-failure
ports; partial publication; non-activation scope claims; weakened quarantine;
normal publication; resume; evidence loss; implicit termination; unavailable,
wrong-scope, recovering, or throwing authorities; forged envelopes; and
forged receipt scope, publication, or continuation.

## Verification

- Focused GCC containment test: **1/1 PASS**.
- Focused Clang 18 ASan/UBSan test: **1/1 PASS**, with LeakSanitizer disabled
  for the managed ptraced environment.
- Valgrind 3.22.0: **0 errors**, **0 bytes in use at exit**, 851 allocations
  and 851 frees.
- `./igor build`: **PASS**.
- `./igor test`: **181/181 PASS**, 57.59 seconds.
- Canonical-language authority freeze: **PASS**.
- Current-documentation drift guard: **PASS** after this checkpoint was
  installed.
- `git diff --check`: **PASS**.

## Deliberate boundary

This is one serial reference containment route for one activation. It does not
admit fault recovery or reclassification, outer escalation, trusted restart or
repair, implicit or top-level termination, provider/graph/process/distributed
containment, source/compiler/backend integration, durable quarantine, or
concurrent fault composition.

## Next bounded stage

The next mechanical stage is to connect an explicitly faulting response
attempt to this containment authority while proving that the original expected
failure remains unresolved and the affected activation cannot rejoin normal
flow. General evidence-chain depth must still wait for executable ADR 0063
budgets and hard-limit accounting.

## Gate 5

**PASS.** One integrity fault now has an exact, inspectable containment route
whose only result is halted and quarantined activation state, suppressed normal
publication, no local continuation, and preserved evidence.
