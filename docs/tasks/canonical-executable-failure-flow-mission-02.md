# Canonical Executable Failure Flow — Mission 02

Date: 2026-10-07.

## Objective

Generalize the Gate 1 serial reference executor from one expected failure type
to one producer's complete finite closed failure set without changing the
meaning of recovery, transformation, policy, envelopes, or receipts.

## Baseline

Synchronized `main` at `9de06b6896dfba1346c553fca22d53e0fe0c7576`,
after Canonical Executable Failure Flow Mission 01 Gate 1.

## Bounded question

How can one producer declare several expected `Failure<E>` types while proving
before execution that every type has exactly one explicit wire and exactly one
policy-selected authorized route, with deterministic dispatch for each
concrete envelope?

## Scope

- `lyraform.failure_flow_plan/v2` for a finite closed producer failure set;
- exact producer-order correspondence among declared failure types, wires, and
  policy selections;
- complete equality between producer and consumer closed sets;
- one explicit typed wire and one selected route for every declared type;
- multiple authorized response routes per type where policy selects exactly
  one;
- complete declarative response-transition coverage for every authorized
  route;
- exact serial dispatch of each concrete envelope through its selected route;
- reuse of the version-1 envelope and receipt formats;
- deterministic round-trip, negative, and hostile-artifact evidence.

## Explicit non-goals

No source spelling, compiler-plan integration, backend execution, dynamic
handler lookup, open failure sets, wildcard routes, subtype matching,
propagation, retry, sinks, response-attempt failure, faults, mutation,
parallelism, cancellation, or durable storage is admitted.

## Gate 2

PASS only when every member of a finite producer failure set has one exact
selected accountable route, no undeclared or uncovered type can execute, an
unselected authorized alternative cannot be invoked, and recovery or
transformation receipts remain valid under the Gate 1 laws.
