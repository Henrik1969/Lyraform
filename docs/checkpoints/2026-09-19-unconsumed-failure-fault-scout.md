# Unconsumed failure and fault disposition scout

Date: 2026-09-19.

Gate: **PASS** — exhaustive explicit disposition was accepted on 2026-09-19
and recorded in ADR 0056.

## Baseline and authority

- synchronized branch baseline: `main` at
  `6ba9db303db453af952327069814c3a607b26d03`;
- ADR 0054: failure is explicit flow;
- ADR 0055: refusal is outside execution and each admitted attempt completes
  with exactly one of `Success<T>`, `Failure<E>`, or `Fault<F>`;
- no source syntax, carrier layout, or propagation default is authorized yet;
- `master` and `flowlfs-v0.1-alive` remain outside scope;
- no staging, commit, or push authority.

## Bounded question

What makes a produced failure or fault fully accounted for when no downstream
ordinary consumer handles it?

This scout does not select source syntax, recovery transforms, retries,
cancellation, backpressure, multi-failure composition, or fault-containment
regions.

## Existing evidence

### Ordinary graph outputs

The current bounded native graph permits an unconnected ordinary output and
emits a drop diagnostic. That rule predates the canonical failure/fault
distinction and applies to ordinary graph payloads. It is not evidence that a
failure or fault may be discarded.

### Existing runtime failure behavior

The current native scalar graph turns division faults and explicit source
failure into structured runtime evidence, suppresses normal output and fan-out,
and exits the graph invocation nonzero. Flowparallel reports task and worker
failures as `no_artifact` and does not promote partial success.

These implementations prove fail-closed boundary behavior. They do not define
a general language-level propagation law.

### Existing graph admission behavior

Graph validation already rejects unconnected required sink inputs and terminal
nodes that expose output ports. Runtime node failures retain node, port, wire,
signal, and original-failure provenance while propagating through the bounded
execution boundary.

This supports static accounting but does not settle whether every failure port
must be written explicitly, may propagate lexically, or may terminate through
a declared top-level policy.

## Safety properties required

Any accepted model must preserve:

- no silent loss of `Failure<E>` or `Fault<F>`;
- no implicit conversion of failure or fault to success;
- exactly one semantic completion per admitted attempt;
- complete origin and propagation provenance;
- static refusal when the compiler can prove a disposition has no legal route;
- explicit, inspectable top-level policy;
- faults cannot be consumed by ordinary failure recovery without specific
  authority;
- backends execute the chosen route and do not invent one.

## Resolved by ADR 0056

- Every possible failure and fault requires a statically inspectable route.
- A missing, ambiguous, silently discarded, or implicitly terminated route is
  a **dangling wire error** and causes admission refusal.
- Legal routes recover or transform under contract, propagate through a
  declared enclosing disposition, reach an explicit top-level failure policy,
  or reach an explicit fault-containment authority.
- Diagnostics and audit observations do not consume a disposition.
- The existing optional-success-output drop rule does not apply to failure or
  fault dispositions.

## Still undefined

- the source spelling for declared propagation through enclosing operations;
- how generic forwarding preserves an open set of failure types;
- how fault containment authorities are named and scoped;
- whether a top-level policy sink is part of the graph or an execution profile;
- how dynamically selected providers declare failure and fault families;
- how multiple concurrent unsuccessful outcomes compose.

## Resolved decision

Every possible failure and fault must have one statically inspectable
disposition route. No route defaults to silent discard, implicit termination,
or hidden propagation.

See
[the unconsumed-disposition decision brief](2026-09-19-unconsumed-failure-fault-decision-brief.md).

The next semantic gate is
[the canonical carrier-projection decision](2026-09-19-disposition-carrier-projection-decision-brief.md).

GATE: PASS — NO DANGLING FAILURE OR FAULT WIRES
