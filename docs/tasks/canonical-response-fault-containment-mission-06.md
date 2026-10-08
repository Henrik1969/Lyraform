# Canonical Response Fault Containment — Mission 06

Date: 2026-10-08.

## Objective

Connect one selected failure-response function's declared `Fault<F>` result to
the exact activation-scope containment authority established by Mission 05.

## Baseline

Mission 05 Gate 5 on synchronized `main` baseline
`760b995ec374d82590a9c2b750dd740ab6ce295f` plus the uncommitted Mission 05
working set.

## Bounded question

Which identities and evidence prove that a response attempt faulted, its fault
received a distinct obligation and exact containment route, the original
expected-failure obligation remains unresolved within the quarantined
activation, and no normal rejoin occurred?

## Scope

- one original selected recovering response;
- one declared response fault type;
- one distinct response-fault obligation;
- one exact Mission 05 containment plan;
- preserved correlation, no-commit evidence, and provenance;
- original-obligation state `unresolved_contained`;
- suppressed normal publication and no continuation or rejoin;
- one independently validated composite receipt;
- positive, negative, and hostile evidence.

## Explicit non-goals

No recovery or reclassification of the fault, original-obligation closure,
local continuation, escalation, restart, repair, implicit termination,
multiple fault types, recursive containment failure, broader containment
scopes, source/compiler/backend integration, or concurrent composition.

## Gate 6

PASS only when a response fault cannot masquerade as response success or
expected failure, cannot alias or erase the original obligation, and cannot
escape its exact containment scope or rejoin normal flow.
