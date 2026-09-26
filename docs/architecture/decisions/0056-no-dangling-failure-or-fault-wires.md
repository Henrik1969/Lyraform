# ADR 0056: No dangling failure or fault wires

## Status

Accepted on 2026-09-19.

## Context

ADR 0055 establishes that every admitted attempt completes as exactly one of
`Success<T>`, `Failure<E>`, or `Fault<F>`. A program remains unsafe if a failure
or fault can be produced but has no accountable destination.

Such programs often appear to work for their intended path while relying on
developer memory not to trigger an unhandled condition. That memory is not a
contract, does not survive maintenance, and cannot be verified by the compiler.

## Decision

Every possible failure and fault must have exactly one statically inspectable
disposition route at each semantic boundary.

A route may:

```text
recover or transform under a compatible declared contract
propagate through the enclosing operation's declared disposition
reach an explicitly declared top-level failure-policy sink
reach an explicitly declared fault-containment authority
```

If the compiler can prove that a failure or fault route is missing, ambiguous,
silently discarded, converted implicitly to success, or sent to an implicit
termination mechanism, the program is refused with a **dangling wire error**.

This is an admission error, not a runtime fallback. A backend may not repair a
dangling route by choosing termination, logging, dropping the disposition, or
inventing propagation.

## Meaning of “wire”

“Wire” names the semantic route, not necessarily a required surface token or a
physical graph edge. Future concise syntax may declare propagation at a
function, graph, contract, or policy boundary. Its canonical semantic facts
must still expose the producer, disposition type, destination, authority, and
provenance so the route can be inspected and validated.

## Diagnostics are not destinations

Logging, metrics, tracing, audit events, and durable error-state observations
may accompany a disposition. They do not consume it. A program that only logs
a failure or fault still has a dangling wire unless another legal disposition
route exists.

A dangling-wire diagnostic should identify at least:

```text
producing operation and source origin
failure or fault type
unaccounted output/route identity
enclosing contract or graph boundary
legal destination classes
```

Exact diagnostic codes and source wording remain an implementation decision.
Their human and debugger projections must follow
[ADR 0057](0057-explanatory-diagnostics-from-canonical-evidence.md).

## Failure and fault authority

An expected `Failure<E>` may be recovered, transformed, propagated, or routed
to an explicit top-level failure policy according to declared contracts.

A `Fault<F>` must reach a declared containment authority. It may not enter
ordinary failure recovery unless an explicitly authorized transition proves
that reclassification or continued execution is safe.

## Optional successful outputs

This law does not automatically prohibit every unused successful value. The
current bounded graph may diagnose and drop an unconnected optional ordinary
output. Dropping optional success data and losing the only unsuccessful
completion are different semantics.

Whether a successful output is required, optional, or intentionally ignored is
defined by its contract. Failure and fault dispositions are never optional
merely because no consumer was connected.

## Guard consequence

Runtime-dependent guard overwatch remains refused until its carrier and routing
projection exist. Once admitted, every possible `Failure<GuardViolation>` must
have a complete route. Printing a diagnostic or relying on an operator to avoid
the violating input does not satisfy this law.

## Consequences

- Silent unsuccessful paths become structurally inadmissible.
- Maintenance cannot unknowingly turn an undocumented operational convention
  into data loss or false success.
- Interface review can see all unsuccessful dispositions and their owners.
- Top-level termination remains possible, but only through an explicit policy
  sink with a declared scope.
- Fault containment remains distinct from ordinary domain recovery.
- Generic and provider-selected operations must expose enough failure/fault
  contract information for route completeness to be checked.

## Deferred

This ADR does not define:

- concrete carrier or graph-port syntax;
- concise propagation syntax;
- failure and fault type hierarchies;
- top-level policy-sink and fault-containment declarations;
- generic open failure sets;
- concurrent multi-failure composition;
- retry, timeout, cancellation, or backpressure semantics.
