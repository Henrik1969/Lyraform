# Canonical Failure Consumer Authority — Mission 01

Date: 2026-09-27.

## Mission type

Post-convergence semantic foundation / expected-failure routing.

## Authority

- ADR 0054 through ADR 0059;
- ADR 0062;
- the Stage Execution Protocol;
- synchronized `main` baseline `ecdac7184236dab24423ca7495e882f771cdf287`;
- existing accumulated canonical-convergence work remains unstaged and
  preserved in the current worktree.

No staging, commit, push, branch mutation, `master`, FlowLFS, or
`meta-discusions.md` modification is authorized by this mission.

## Bounded question

What is the smallest compiler-stage-independent contract that proves:

```text
one typed failure consumer
    owns one closed expected-failure set
    exposes only exact ordinary developer-function identities
    gives policy only those routes to select
```

without claiming source syntax, runtime routing, recovery completion, retry
execution, or runtime guard admission?

## Objective

Add a versioned declarative `lyraform.failure_consumer` contract and a matching
declarative policy-selection contract. Establish exact identity, type, closure,
and authorization checks independently of Flowanalyst, source spelling, and
backends.

This stage proves the consumer declaration and policy-selection boundary. It
does not make a selected response executable because current ordinary function
signatures do not yet establish the response function's resulting canonical
disposition.

## Required contract

The consumer must retain:

```text
consumer identity
declaration scope identity
closed expected-failure payload-type set
route identities
exact failure type for each route
exact ordinary function symbol identity for each route
declarative/non-executable status
```

Each referenced function must:

- be an ordinary available definition;
- have exactly one semantic input projection;
- accept the canonical `failure_envelope` projection for the exact failure
  payload type named by the route;
- have a non-empty, non-void result type;
- be referenced by semantic identity, never textual lookup.

The policy-selection contract must retain:

```text
consumer identity
failure payload type
selected route identity
policy identity and revision
declarative/non-executable status
```

It must reject a route not authorized by that consumer for that exact failure
type.

## Required refusals

At minimum refuse:

- empty or open failure sets;
- duplicate failure types, route identities, or function identities;
- an accepted failure with no route;
- a route outside the accepted failure set;
- unknown, declaration-only, naked-payload, wrong-envelope-payload, void, or
  otherwise incompatible functions;
- malformed or unsupported contract versions/status;
- policy selection for the wrong consumer, failure type, or route;
- empty policy identity or revision.

## Explicit non-goals

Do not implement or choose:

- failure-consumer source spelling;
- nested functions, captures, dynamic handlers, or nearest-handler lookup;
- producer-to-consumer source association;
- executable response-function result dispositions;
- runtime-dependent guard lowering;
- general Outcome types or exhaustive match syntax;
- graph failure ports or policy sinks;
- retries, clocks, waiting, timeout, cancellation, backpressure, or async;
- fault containment;
- Flowanalyst emission or backend execution.

The standalone contract must remain visibly declarative. No current ready plan
may gain executable authority from its presence.

## Verification

- deterministic serialization and round-trip;
- positive carrier-independent consumer and policy-selection cases;
- hostile identity, closure, type, function, status, and selection mutations;
- focused Flowcontracts test;
- canonical documentation and authority drift guards;
- `git diff --check`.

## Gate 1

PASS only when the declarative consumer and policy-selection contracts are
versioned, independently validated, carrier-independent, hostile-tested, and
explicitly incapable of authorizing execution. Stop after Gate 1 and design
the response-function disposition contract before integrating source or runtime
guard behavior.
