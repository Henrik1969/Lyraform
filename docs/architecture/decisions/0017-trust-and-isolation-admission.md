# ADR-0017: Trust and isolation admission paths

**Status:** accepted direction, provisional profiles
**Date:** 2026-08-20
**Scope:** Frankencore external material admission

## Decision

Frankencore provides two normal admission paths for acquired material:

```text
verified provenance
    trust according to explicit policy

insufficient provenance
    isolate according to explicit policy
```

Users are not required to be expert source auditors before they can inspect or
use useful material. The system reduces uncertainty through provenance and
reduces consequences through isolation.

## Admission outcomes

```text
trusted
    provenance, integrity, authority, and policy requirements satisfied

isolated
    provenance is insufficient, but the requested operation is confined by a
    verified isolation boundary

quarantined
    material may be inspected, but neither trust nor adequate isolation exists

rejected
    policy, integrity, provenance, or safety requirements fail decisively
```

Trust and isolation are independent dimensions. A signed project may still be
isolated by local policy, and an unsigned project may be safely inspected or
executed inside an approved isolation boundary without becoming trusted.

## Default decision law

The system should prefer, in order:

1. trusted execution when provenance and policy establish the required trust;
2. isolated execution when trust is insufficient but the boundary is verified;
3. quarantine when neither trust nor adequate isolation is available;
4. rejection when the material or requested operation violates a hard law.

Every outcome records the provenance, verification, isolation, policy, and
responsibility that produced it.

The current C++ contracts implementation admits only the first path. Trusted,
integrity-matched, authenticated evidence with an `allowed` policy outcome can
pass execution admission. `allowed_with_isolation` remains fail-closed even
with a structurally valid isolation claim because no independent enforcement
provider is connected to the admission boundary. Quarantined, rejected,
confirmation-required, and unresolved outcomes never authorize execution.

## No universal trust score

Frankencore does not reduce trust to one numeric score. A score creates false
precision, hides critical failures, and can allow a manipulated reputation
signal to outweigh missing integrity or authorization evidence.

Trust is represented as independent evidence, for example:

```text
origin_known
integrity_verified
signature_valid
signer_authorized
dependencies_known
review_status
isolation_available
policy_compatible
```

Policy derives the admission outcome from those facts. A catalog may rank
entries for discovery, but ranking is never authorization. Consumers should be
able to see both the evidence and the reason an item was trusted, isolated,
quarantined, or rejected.

## Revisit triggers

Revisit when defining trust scoring, source auditing, sandbox escape response,
remote execution, or automated promotion from isolated to trusted state.
