# ADR-0062: Typed failure consumers with policy-selected routing

Date: 2026-09-27.

Status: accepted architectural law; source spelling and executable support are
not yet admitted.

## Context

ADR 0054 establishes that failure is explicit flow. ADR 0055 distinguishes
`Failure<E>` from admission refusal and `Fault<F>`. ADR 0056 requires every
failure and fault to have exactly one accountable route, and ADR 0058 gives
those routes one canonical semantic authority with contextual projections.

The remaining architectural question is how expected runtime failure reaches
developer-defined recovery behavior without introducing exception-style
unwinding, implicit handler lookup, a separate keyword for every response, or
source-visible names for compiler operations that already have canonical
identity and provenance.

## Decision

A Lyraform failure consumer owns a closed, declared set of expected
`Failure<E>` dispositions and maps each admitted failure type to one or more
statically resolved ordinary functions. The developer names and implements
those functions. Examples of developer intent include `pollnetwork`, `reload`,
`use_cache`, `request_credentials`, and `reject_candidate`; these names do not
become compiler dispatch rules.

The selected function receives a typed, read-only failure-envelope projection,
not merely the naked payload `E`. The envelope preserves the originating
disposition and obligation identities, producer and attempt identities,
payload type and value, commit/no-commit proof, and provenance needed to
account for the route. Ordinary code may inspect admitted projections; it may
not rewrite the producer's evidence.

The semantic layers are:

```text
language semantics
    establish the producing operation, possible dispositions, payload types,
    commit law, provenance, and legal response constraints

failure-consumer contract
    establish the closed accepted failure set and the exact compatible
    developer-function identities available for each failure type

policy
    select one authorized response and its permitted bounded parameters

runtime
    execute the selected canonical route without changing its meaning
```

Policy selects within the graph established by semantics and consumer
contracts. Policy does not create language meaning.

## Operation identity

No `attempt` keyword or developer-authored operation name is required by this
decision. Calls, placements, guard checks, and other admitted operations retain
their compiler-established operation, statement, expression, scope, function,
guard, provider, and provenance identities.

Source syntax may later expose an operation identity where useful, but failure
routing must not depend on developers inventing names such as `update_x` merely
to make an existing canonical operation addressable.

## Function and scope law

The first consumer projection should resolve ordinary declared Lyraform
functions by semantic identity. A consumer may organize those identities into
a declared scope, but it does not search the dynamic call stack, select the
nearest matching handler, dispatch by textual function name, or acquire
closure captures implicitly.

An explicit function, graph, guard, or policy-boundary association must connect
the producing disposition to the selected consumer. The canonical facts expose
the producer, failure type, consumer, selected function, route, policy decision,
and provenance even when future source syntax provides a concise declaration.

Nested function definitions, inner-function capture, dynamic handler
installation, and ambient global consumers are not implied. A bounded first
implementation may reference existing ordinary functions without adding those
features.

## Consumer completeness

For every admitted expected failure:

- the producer's declared `Failure<E>` must be accepted by exactly one
  accountable consumer route at each semantic boundary;
- dispatch is based on canonical failure type and contract facts, never a
  diagnostic string or incidental numeric status;
- every selected function has a compatible declared input and resulting
  disposition contract;
- the route preserves producer, attempt, guard/provider, commit, and provenance
  evidence;
- a response function that itself can fail creates another explicit obligation;
- missing, ambiguous, incompatible, cyclic-without-bounds, or unauthorized
  routing is an admission refusal;
- logging, metrics, tracing, and durable observation do not consume the
  failure.

A response function may recover, transform, request an authorized retry,
propagate, or reach an explicit failure-policy sink. It may not return `void`
or an ordinary success merely to erase the originating obligation. Recovery
must produce a declared valid result or state transition with its own
provenance.

Passing an envelope onward transfers its existing must-account obligation;
ordinary copying does not create a second independent failure. Transformation
creates a successor disposition linked to the original evidence. Recovery
closes the original obligation only through a declared response transition.
Retry closes the failed attempt with a retry-request receipt and, when later
authorized, begins a distinct linked attempt.

## Policy authority and limits

Policy may choose among response functions and parameters already authorized
by semantics and the consumer contract. For example, different profiles may
select bounded polling, an admitted cache fallback, or immediate propagation.

Policy must not:

- convert failure to success without a declared recovery operation;
- select a function outside the consumer's declared set;
- change the producer's failure type, commit law, or provenance without an
  explicit validated transformation;
- discard an unsuccessful disposition or treat observation as consumption;
- retry an operation lacking canonical retry-safety authority;
- make an unbounded retry or wait policy executable;
- route `Fault<F>` into ordinary expected-failure recovery;
- invent propagation, termination, or a default consumer.

Policy identity, version, authority, selected route, parameters, and provenance
must be inspectable wherever policy affects execution.

## Retry consequence

Retry is a policy-governed new attempt, not a property that makes the original
failure disappear. Before retry can be admitted, the operation contract must
establish the applicable safety condition, such as idempotence, no commit,
compensation, or another explicit retry authority. Retry policy must be bounded
and must route exhaustion explicitly. Delay requires declared clock and
scheduling capabilities.

General retry, timeout, cancellation, backpressure, and effectful parallel
composition remain separate decisions and are not admitted by this ADR.

## Evidence lifetime

Failure evidence is proof-carrying but not immortal. ADR 0063 defines bounded
evidence epochs, proven closure receipts, safe compaction, and policy-governed
retention budgets. Required evidence may never be silently truncated to stay
within a budget.

## Failure and fault separation

Failure consumers handle expected `Failure<E>` flow. `Fault<F>` remains routed
to a separately authorized fault-containment authority under ADR 0055 and ADR
0056. A broad consumer or policy cannot reclassify a fault as an expected
failure without an explicit trusted transition proving that continued
execution is safe.

## Projection law

The same consumer relationship may later project contextually:

```text
sequential context    must-account value/owner and exhaustive dispatch
function boundary     declared disposition transfer to a consumer
graph context          typed failure port and explicit wire
policy boundary        versioned selection among authorized functions
runtime/debugger       executed route and preserved evidence
```

These are projections of one canonical disposition and route, not independent
failure systems.

## Current implementation boundary

This decision does not claim that general failure consumers are implemented.
The current executable language remains limited to the bounded
`Outcome<Text,TextFailure>` accounting and unique-transfer projections already
documented by ADR 0059 and ADR 0060. Runtime-dependent guard transitions,
general propagation, failure-consumer declarations, policy sinks, retry, and
fault containment remain refused or unsupported until separately bounded
missions establish source representation, contracts, validation, execution,
and backend parity.

The first declarative contract implementation accepts only functions whose
single semantic input projection is `failure_envelope` with the exact routed
payload type. It remains non-executable because the response-transition
contract is not yet integrated with source or runtime execution.

## Deferred decisions

- concrete failure-consumer and route source spelling;
- whether consumer organization receives namespace-like syntax;
- function-signature spelling for accepted and produced dispositions;
- failure type hierarchy and generic/open failure sets;
- exhaustive local dispatch syntax;
- policy artifact schema and authorization model for this language surface;
- recovery-result and retry-request carrier spelling;
- graph failure ports, function-boundary propagation, policy sinks, and fault
  containment implementation order;
- concurrent and distributed disposition composition.
