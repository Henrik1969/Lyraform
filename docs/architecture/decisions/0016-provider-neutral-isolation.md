# ADR-0016: Provider-neutral layered isolation

**Status:** accepted direction, implementation future
**Date:** 2026-08-20
**Scope:** Frankencore quarantine and execution safety

## Decision

Frankencore requires a mechanism for policy-directed isolation of untrusted or
high-risk material, but does not prescribe a particular enforcement substrate.

Possible providers include:

```text
OS permissions
namespaces / sandboxing
containers
virtual machines
remote or disposable workers
hardware or provider-specific isolation
```

These may be composed in layers according to the consuming entity's policy.
The contract describes the requested boundary and the provider's verified
enforcement claim; it does not hardcode Docker, a VM, namespaces, or any other
implementation.

## Isolation contract

An isolation provider must declare:

- requested boundary and scope;
- capabilities and limitations;
- resources exposed or denied;
- identity and provider version;
- policy used to authorize it;
- whether enforcement was verified, partial, unavailable, or unknown;
- diagnostics and provenance for setup and teardown.

If a provider cannot enforce a requested boundary, the system must report the
limitation and must not describe the material as isolated to that level.

## Default profile

The sane default is conservative:

- unknown or untrusted material is not executed directly on the trusted host;
- inspection is separated from installation and execution;
- the strongest available compatible isolation is selected;
- missing or uncertain isolation fails closed for execution;
- the owner may explicitly relax the policy and accept the declared risk.

The default is a policy profile, not a constitutional requirement for one
specific substrate.

## Assurance selection

Isolation providers report a common assurance level plus detailed claims:

```text
none          no meaningful isolation claim
constrained   selected resources restricted; host remains materially exposed
isolated      execution boundary enforced for the declared resources
hardened      isolated boundary plus defense-in-depth and verified limits
```

The default policy selects the highest assurance level that is both available
and compatible with the requested operation. It must not infer a higher level
from a provider name or marketing label. The provider must substantiate the
level with explicit resource, identity, filesystem, network, privilege, and
lifecycle claims.

If no provider meets the default assurance requirement, execution fails
closed. The consuming user, administrator, or root may consciously relax the
requirement, but the resulting lower assurance, authorizing policy, and
responsibility are recorded.

## Verification model

Assurance uses layered verification:

```text
none:
    provider self-report

constrained:
    self-report + local sanity checks

isolated:
    self-report + mandatory local verifier

hardened:
    self-report + independent verification + defense-in-depth evidence
```

Provider self-report is never sufficient for `hardened`. If provider claims,
local checks, or independent evidence conflict, the resolver selects the lower
assurance level and records the conflict. It never averages or guesses between
claims.

## Layer composition

The resolver may select several providers, for example:

```text
project policy
    ↓
container / namespace
    ↓
restricted filesystem and network
    ↓
disposable VM or remote worker
```

Each layer must report its own boundary. A composition is only as strong as
the verified claims and policy decisions that make it up.

## Revisit triggers

The C++ contracts layer now validates the shape and consistency of an
`IsolationClaim`. It rejects constrained claims without local or independent
verification and rejects isolated or hardened claims without independent
verification. Its execution-admission boundary admits directly trusted
evidence, but deliberately refuses `allowed_with_isolation` even when claim
text says `independently_verified`: no enforcement provider is wired to that
decision yet. This validates declarations and prevents labels from becoming
authority; it does not itself create an isolation boundary or certify a
provider.

Revisit when defining the first isolation-provider ABI, default Linux profile,
remote worker contract, or security-critical execution policy.
