# Decision brief — unconsumed failure and fault disposition

Date: 2026-09-19.

Status: **DECIDED — Candidate A accepted on 2026-09-19.**

Durable authority:
[ADR 0056](../architecture/decisions/0056-no-dangling-failure-or-fault-wires.md).

## Problem

ADR 0055 defines an exclusive `Success<T> | Failure<E> | Fault<F>` completion.
It does not yet define what makes the unsuccessful branches fully accounted
for.

For example, if an operation can produce `Failure<GuardViolation>` but no
recovery node consumes it, Lyraform must decide whether to refuse the graph,
propagate automatically, terminate automatically, or permit a drop.

## Candidate A — exhaustive explicit disposition

Every possible `Failure<E>` and `Fault<F>` must have exactly one statically
inspectable disposition route at each boundary. A route may:

```text
propagate as part of the enclosing operation's declared disposition
transform or recover under a compatible declared contract
reach an explicitly declared top-level failure-policy sink
reach an explicitly declared fault-containment authority
```

Silent discard, implicit success conversion, implicit termination, and magical
unwinding are illegal. A compiler-proven missing or ambiguous route is an
admission refusal.

Faults may not enter ordinary failure recovery unless a specific authority
defines a safe reclassification or containment transition. Observation-only
diagnostics do not consume a disposition.

The exact source spelling and whether a forwarding route can be synthesized
from explicit function/graph signatures remain later representation decisions.
The route itself must still be present in inspectable semantic facts.

**Pros**

- makes silent failure structurally impossible;
- gives compiler and graph validators a deterministic completeness rule;
- preserves provenance across every boundary;
- permits policy-controlled termination without making termination implicit;
- keeps integrity faults out of ordinary catch-all recovery;
- fits the existing fail-closed admission posture.

**Cons**

- requires effect/result summaries before general fallible operations scale;
- can be verbose without principled forwarding syntax;
- generic and provider-selected operations need open or parameterized failure
  sets;
- top-level policy and fault-containment contracts must eventually be defined.

**Guard consequence**

A runtime-dependent guard is admitted only when its `GuardViolation` route is
complete. Merely printing a diagnostic or dropping an unwired failure port does
not satisfy the rule.

## Candidate B — implicit propagation to the enclosing operation

An unhandled failure or fault automatically becomes the enclosing operation's
corresponding disposition until a handler or process boundary is reached.

**Pros**

- concise source;
- familiar propagation ergonomics;
- fewer explicit forwarding wires.

**Cons**

- introduces hidden control/data routing;
- enclosing signatures may change through transitive implementation details;
- faults can cross trust boundaries without an explicit containment decision;
- makes graph inspection and stable interface review harder.

## Candidate C — implicit fail-stop termination

An unconsumed failure or fault emits structured evidence and terminates the
current activation, graph, or process according to a runtime default.

**Pros**

- fail closed;
- simple initial runtime implementation;
- no silent continuation after failure.

**Cons**

- the termination region and policy are hidden semantics;
- expected recoverable failures become operational crashes;
- creates an out-of-graph control mechanism resembling the hidden channel the
  architecture rejects;
- distributed execution cannot infer a universally correct boundary.

## Candidate D — diagnostic and drop

An unconsumed failure or fault is recorded diagnostically and then discarded,
as current bounded graphs may do for unconnected ordinary outputs.

**Pros**

- smallest graph requirement;
- superficially matches the existing ordinary-output drop mechanism.

**Cons**

- permits silent semantic failure after diagnostics are ignored;
- loses completion accountability;
- can leave callers believing an operation succeeded;
- treats integrity faults as disposable telemetry;
- incompatible with the safety objective and explicit-failure law.

The ordinary-output rule is not a precedent: dropping optional successful data
and losing the only unsuccessful completion have different semantics.

## Candidate E — defer

Keep all runtime-dependent fallible language features refused until another
prerequisite is defined.

This remains safe but does not advance general failure-flow semantics.

## Recommendation

Recommend **Candidate A — exhaustive explicit disposition**.

The core law would be:

> Every admitted failure and fault path is total, explicit, and inspectable.
> No unsuccessful disposition may disappear merely because no ordinary
> consumer is connected.

This does not require every forwarding token to be handwritten. A future
surface may provide concise propagation, but the resulting semantic route must
be explicit in canonical facts and declared at the enclosing boundary.

Candidate A is also the smallest model that permits both expected recovery and
strict fault containment without exceptions or hidden termination.

## Decision

Candidate A was accepted on 2026-09-19. A failure or fault route that has no
accountable destination is a **dangling wire error** and causes admission
refusal.

The term is intentionally concrete: the unsafe condition is not merely an
unused value but an unsuccessful semantic completion whose route ends without
recovery, propagation, or governed containment.

## Historical choices presented

```text
A. Every failure/fault requires an explicit, statically inspectable route
B. Unhandled failure/fault implicitly propagates to the enclosing operation
C. Unhandled failure/fault implicitly terminates a runtime-selected region
D. An emitted diagnostic permits the failure/fault to be dropped
E. Defer and identify another prerequisite
```
