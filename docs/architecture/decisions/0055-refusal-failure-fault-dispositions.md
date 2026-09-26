# ADR 0055: Refusal, failure, and fault are distinct dispositions

## Status

Accepted on 2026-09-19.

## Context

Lyraform forbids exception-style hidden control transfer and treats expected
runtime failure as explicit flow. A single undifferentiated “error” category
would nevertheless collapse three materially different states:

```text
source or semantics cannot be admitted
an admitted operation cannot succeed for this value or environment
execution integrity can no longer be trusted
```

The distinction is required before guards, division, bounds, providers, and
other fallible operations can share one semantic authority.

## Decision

Construction and admission **refusal** remain outside execution. Refused
source produces no executable operation.

Every admitted runtime attempt completes with exactly one semantic
disposition:

```text
Success<T>
Failure<E>
Fault<F>
```

- `Success<T>` reports completion according to the operation's contract.
- `Failure<E>` is an expected, typed runtime outcome declared by the
  operation's contract. It is ordinary explicit flow and may be propagated,
  transformed, recovered, or retried only where contracts and policy admit it.
- `Fault<F>` reports a breach of execution, artifact, runtime-invariant, or
  trusted-boundary integrity. It is explicit evidence governed by a separate
  containment policy and must not be silently consumed as an ordinary domain
  failure.

The three dispositions are mutually exclusive for one attempt. Diagnostics,
metrics, audit events, and provenance may accompany any disposition, but they
are observations rather than additional semantic completions.

A tagged value, graph ports, provider response, and execution record may be
different projections of this same discriminated completion. A backend may
implement or refuse an already established semantic operation; it may not
reclassify an invalid program as valid or invent another completion law.

## Preconditions

For a semantic precondition `P`:

```text
P provably true
    admit without a runtime check

P provably false
    refuse before execution

P unknown until runtime
    lower declared enforcement that produces Failure<E> when P is false
```

If the enforcement mechanism discovers contradictory trusted facts, corrupt
artifacts, or an inability to preserve its own contract, it produces
`Fault<F>` rather than the operation's expected failure.

Only conditions represented by the semantic model—through types, contracts,
guards, invariants, provider contracts, or another admitted mechanism—can be
proved or enforced by this law.

## Commit and partial-work law

Failure and fault do not silently publish a successful normal-state commit.
Any partial or degraded work must be represented explicitly inside the chosen
disposition, including whether a separately governed commit occurred. An
observation that work was attempted is not itself a successful completion.

## Guard consequence

For a runtime-dependent guarded transition:

```text
guard satisfied
    Success<CommittedTransition>

guard violated
    Failure<GuardViolation>, with no guarded-state commit

contradictory validated guard artifact or impossible enforcement state
    Fault<...>
```

The current bounded guard stage still refuses runtime-dependent transitions
because the carrier and routing contract have not yet been admitted. This ADR
does not authorize a private guard exception, trap, or termination path.

## Resource exhaustion

Resource exhaustion is contract-sensitive. Exhaustion exposed by a declared
bounded provider may be an expected `Failure<E>`. Exhaustion that prevents the
runtime from preserving execution integrity may be a `Fault<F>`.

## Consequences

- Compile-time diagnostics cannot masquerade as runtime failures.
- Ordinary recovery cannot accidentally swallow integrity corruption.
- Generic compiler and graph stages share one completion cardinality.
- Existing bounded binary outcomes remain valid projections where faults are
  outside the admitted bounded contract.
- Assertions and provider boundaries must classify their unsuccessful states
  deliberately rather than relying on one catch-all category.

## Deferred

Routing completeness was subsequently resolved by
[ADR 0056](0056-no-dangling-failure-or-fault-wires.md): every possible failure
and fault requires an explicit, statically inspectable disposition route.

This ADR does not define:

- source spelling or concrete carrier layout;
- failure and fault type hierarchies;
- graph-port syntax;
- concrete propagation syntax;
- recovery-transform syntax;
- fault-containment regions and top-level policy;
- retry, timeout, cancellation, backpressure, or multi-failure composition.

Those require separate bounded decisions and evidence.
