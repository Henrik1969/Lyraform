# ADR-0063: Disposition evidence uses bounded epochs

Date: 2026-10-07.

Status: accepted architectural law; source spelling, runtime storage, and
retention-profile formats are not yet admitted.

## Context

ADR 0055 gives every admitted attempt one `Success<T>`, `Failure<E>`, or
`Fault<F>` disposition. ADR 0056 requires every unsuccessful disposition to
have an accountable route. ADR 0062 routes expected failures through typed
envelopes so recovery code can inspect the producer's proof and preserve its
provenance.

Keeping every intermediate event forever would make evidence itself an
unbounded resource leak. Dropping it opportunistically would destroy the proof
needed to explain or validate an unresolved operation. The lifetime therefore
needs semantic ownership and an explicit bound.

## Decision

Crossing an admitted evidence boundary opens a bounded **evidence epoch**. The
epoch carries only the evidence still required by its unresolved disposition,
obligation, policy, audit, or downstream validation dependencies.

```text
boundary crossed
    -> open evidence epoch

operation and response transitions
    -> extend the live evidence

obligation resolved or transferred
    -> close or transfer the epoch

no remaining evidence dependency
    -> release detailed evidence
```

Evidence is live while any canonical consumer still depends on it. It may be
released only after every obligation it proves has been discharged or
transferred and every required durable observation has been published.

## Closure and compaction

A proven closure boundary may replace detailed live evidence with a bounded
closure receipt containing at least:

```text
epoch identity
origin boundary identity
closed disposition and obligation identities
final disposition
commit state
selected route and policy provenance where applicable
closure provenance
```

A later boundary starts a new epoch and may reference that receipt instead of
copying the entire earlier chain. Compaction must preserve every identity and
fact still required by the new epoch; it cannot be inferred from elapsed time,
buffer pressure, or implementation convenience.

Forwarding transfers the current obligation and its required live evidence.
Transformation creates a linked successor. Recovery closes the original
obligation with a declared transition. Retry closes one attempt with a receipt
and creates a new linked attempt only after separate retry-safety and policy
authority admit it.

## Retention layers

The same semantic evidence may have different bounded projections:

- compiler evidence lives while later stages must validate it;
- live runtime evidence lives while an operation or obligation is unresolved;
- durable audit evidence lives only where contract or policy requires it.

Removing a runtime representation after its proof obligations are closed does
not rewrite historical compiler or durable audit evidence.

## Evidence budgets

Policy may impose deterministic limits on live chain depth, retained bytes,
unresolved age, retry ancestry, and nested boundary depth. Policy may select an
already authorized compaction or disposition route; it cannot authorize silent
evidence loss.

Approaching a limit should produce an explanatory diagnostic identifying the
root operation, repeating route or growth source, unresolved obligation, and
available closure or refactoring classes.

At a hard limit:

- perform compaction only when an admitted contract proves it safe;
- otherwise produce a declared expected `Failure<EvidenceBudgetExceeded>` when
  the operation can preserve correctness and route that resource failure;
- produce a `Fault<EvidenceIntegrityLost>` and enter fault containment when
  mandatory proof can no longer be preserved safely.

The concrete failure/fault type names above describe the semantic classes;
this ADR does not admit those source types or spellings.

## Consequences

- provenance remains available for as long as it proves live behavior;
- failure handling can explain what happened and what was attempted;
- evidence chains cannot grow without deterministic policy visibility;
- resource pressure never converts an unresolved failure into success;
- optimizers and runtimes need evidence-liveness accounting before erasure;
- diagnostics may be personable in human mode, but structured output remains
  deterministic and professional.

## Deferred

- concrete envelope and closure-receipt serialization;
- source-visible evidence inspection;
- compiler evidence-liveness analysis;
- runtime storage and reclamation mechanisms;
- durable audit provider contracts;
- retention policy schema, defaults, and authorization;
- exact expected-failure and integrity-fault type declarations;
- distributed evidence compaction and trust anchors.
