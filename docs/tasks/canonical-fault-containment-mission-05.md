# Canonical Fault Containment — Mission 05

Date: 2026-10-08.

## Objective

Implement the first bounded reference projection of ADR 0065: one declared
`Fault<F>` route halts and quarantines one exact activation scope without
ordinary recovery, local continuation, normal publication, or implicit
termination.

## Baseline

Synchronized `main` at
`760b995ec374d82590a9c2b750dd740ab6ce295f`.

## Bounded question

Which identities and evidence prove that one produced fault reached its exact
containment authority and left its declared activation halted, quarantined,
non-publishing, and unable to continue?

## Scope

- one versioned fault-containment plan;
- one declared fault type and producer disposition;
- one exact typed fault wire;
- one activation-scope containment authority;
- one explicit policy selection of `halt_and_quarantine`;
- one read-only fault envelope;
- exact authority dispatch;
- one independently serializable and validated containment receipt;
- positive, negative, and hostile evidence.

## Explicit non-goals

No ordinary recovery, fault-to-failure reclassification, continuation,
escalation, restart, repair, retry, implicit or top-level termination,
provider/graph/process containment, source/compiler/backend integration,
concurrent faults, or durable quarantine provider.

## Gate 5

PASS only when a fault cannot disappear, become success or expected failure,
reach the wrong scope, publish normal output, continue locally, or claim
containment without an exact authority result and validated receipt.
