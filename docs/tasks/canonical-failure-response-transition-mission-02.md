# Canonical Failure Response Transition — Mission 02

Date: 2026-10-07.

## Objective

Establish the smallest carrier-independent semantic contract that gives the
successful completion of every already-authorized failure-consumer route one
exact declarative meaning: bounded recovery to `Success<T>` or transformation
to a linked successor `Failure<E2>`.

## Baseline

Synchronized `main` at `16a1cfff6e560ed343286af0f9f604edbb3d02e5`, after
Mission 01 Gate 1 and the accepted Candidate A response-transition decision.

## Scope

- a versioned `lyraform.failure_response_transition/v1` shared contract;
- exact consumer, route, function, input failure, and output type identity;
- complete one-to-one transition coverage of a consumer's routes;
- `recover` with original-obligation closure;
- `transform` with a parent-linked successor obligation;
- immutable origin commit evidence and linked provenance;
- separate obligations for failures or faults of the response attempt itself;
- deterministic round-trip, negative, and hostile-artifact evidence.

## Explicit non-goals

No source spelling, Flowanalyst emission, producer association, graph port,
runtime execution, propagation, retry, sink, evidence storage, guard routing,
new carrier type, or backend behavior is admitted by this mission.

## Gate 2

PASS only when every route in the bounded contract set has exactly one valid,
declarative recover or transform transition, contradictory identity/type/law
claims fail closed, and no contract can claim executable status.
