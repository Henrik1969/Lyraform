# ADR-0012: Public-key trust for profile declarations

**Status:** accepted direction, implementation future
**Date:** 2026-08-20
**Scope:** Frankencore signed profiles and authority resolution

## Decision

Frankencore uses public-key signatures and explicit trust policy for
authoritative operating-profile declarations.

The model is compatible with established public-key ecosystems such as SSH,
but does not require reuse of an SSH private key or silently inherit an SSH
trust decision. Compatibility is an adapter and policy choice.

```text
private key
    signs profile content

public key
    verifies content and identifies signer

trust policy
    decides whether signer is authoritative for scope
```

## Trust requirements

The future implementation must support explicit handling of:

- trust anchors and trusted public keys;
- key identity and owner metadata;
- key rotation and successor relationships;
- revocation and compromise response;
- delegated authority and scope restrictions;
- expiry and validity periods;
- offline verification from locally available trust material;
- audit evidence for the key and policy used.

Possessing a valid key proves control of that key, not unlimited authority.
Trust policy must still authorize the key for the requested project, action,
disclosure level, or system scope.

## Default behavior

The default profile requires a valid signature and a trusted key for
authoritative claims. Missing, unknown, invalid, revoked, or expired keys
produce diagnostics and non-authoritative results. An explicit owner policy
may relax verification for a defined scope, but the relaxation is itself
policy-controlled and recorded.

## Key handling law

Frankencore must never require private keys to be embedded in projects,
profiles, catalogs, artifacts, or provenance records. Only public verification
material and references to approved trust stores may travel with those
artifacts.

## Revisit triggers

The current contracts validator enforces the non-authoritative default at the
evidence boundary: an `allowed` outcome requires trusted key state, matched
integrity, and supplier authentication or owner attestation. An
`allowed_with_isolation` outcome still requires matched integrity. This is a
claim-consistency guard, not a signature verifier or trust-store
implementation; signed profiles, expiry, revocation, rotation, and downgrade
handling remain future work.

Revisit when selecting the signature encoding, cryptographic implementation,
trust-store format, SSH interoperability profile, or multi-administrator
delegation model.
