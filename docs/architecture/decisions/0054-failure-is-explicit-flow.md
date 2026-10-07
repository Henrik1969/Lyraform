# ADR 0054: Failure is explicit flow

## Status

Accepted architectural law on 2026-09-19. The result-disposition algebra was
subsequently resolved by
[ADR 0055](0055-refusal-failure-fault-dispositions.md), contextual projections
by [ADR 0058](0058-canonical-disposition-fact-projections.md), and the typed
consumer/policy boundary by
[ADR 0062](0062-typed-failure-consumers-and-policy-routing.md). Concrete source
spelling and executable general routing remain unresolved.

## Decision

Lyraform failures are explicit, typed information in the computational graph.
They are not exceptions, implicit unwinding, silent status loss, or a hidden
secondary control system.

The architecture recognizes distinct semantic channels:

```text
normal data flow
failure/error flow
control flow
event flow
```

Recovery, propagation, logging, retry, transformation and termination are
graph behavior or declared execution policy. They must not occur through
invisible language control transfer.

The source-level law is:

```text
provably correct
    ordinary admitted flow

provably invalid
    compile-time semantic refusal

runtime-dependent failure
    explicit failure flow, once its carrier and routing contract are admitted
```

Until a required failure-flow contract exists, compilation fails closed. A
feature must not invent its own private exception, implicit status, trap,
termination rule, or bespoke recovery channel merely to become executable.

## Existing evidence

- The graph design states that failure is flow and diagnostics travel with the
  envelope.
- The exception-containment boundary forbids exceptions and implicit unwinding
  in Lyraform language semantics.
- `Outcome<Text,TextFailure>` is a bounded backend-neutral tagged outcome, not
  yet a general language carrier.
- `MutationRejection`, `ErrorStateEvent`, and committed `MutationRecord` are
  deliberately separate semantic families.
- Durable error-state history records operational lifecycle; it does not
  define source-language failure routing.

## Guard consequence

Runtime guard overwatch must eventually produce ordinary failure flow carrying
guard identity, predicate identity, candidate-state evidence, affected
dependencies, source operation and provenance.

It must not throw `GuardException`, commit before checking, silently discard
the candidate, or make every input violation a programmer assertion.

For the current bounded guard stage, runtime-dependent transitions remain
compile-time refusals. This is a deliberate fail-closed disposition, not a
claim that runtime guards should remain impossible.

## Required distinctions

These remain semantically distinct:

```text
rejected attempt
operational error state
committed state transition
diagnostic observation
```

A failure may cause or be recorded by another family, but serialization or
transport convenience must not collapse their meanings.

## Subsequent resolution

ADR 0055 establishes that construction/admission refusal remains outside
execution and that each admitted attempt has exactly one semantic completion:
`Success<T>`, `Failure<E>`, or `Fault<F>`. A tagged value, graph ports,
provider response, and execution record may be projections of that same
discriminated fact. Diagnostics and events may accompany the completion but do
not constitute additional completions.

[ADR 0056](0056-no-dangling-failure-or-fault-wires.md) subsequently establishes
that every failure and fault requires an explicit, statically inspectable
disposition route. A missing route is a dangling wire error and causes
admission refusal.

[ADR 0062](0062-typed-failure-consumers-and-policy-routing.md) subsequently
establishes that expected failures route through typed consumers to ordinary
statically resolved developer functions. Policy may select only among routes
already authorized by semantic and consumer contracts; it cannot create
language meaning or implicit control transfer.

## Deferred decisions

This ADR deliberately does not decide:

- the universal failure carrier or source spelling;
- typed failure-category hierarchy;
- concrete wiring and propagation syntax;
- recovery-transform syntax;
- retry, timeout, cancellation or backpressure semantics;
- top-level execution policy;
- concrete projection forms for tagged values and graph lanes.

Those questions require bounded decisions and evidence before general runtime
failure flow is admitted.
