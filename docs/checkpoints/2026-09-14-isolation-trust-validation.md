# Isolation and trust claim validation checkpoint

**Date:** 2026-09-14  
**Status:** implemented boundary; provider enforcement remains provisional

The Frankencore contracts layer now validates `IsolationClaim` declarations.
Every claim must identify its requested boundary, provider and version,
resource/identity/filesystem/network/privilege/teardown limits, enforcement
state, and verification method. A constrained claim requires local or
independent verification; isolated and hardened claims require independent
verification. The validator does not pretend to create or certify isolation.

Verification evidence now rejects an `allowed` policy outcome unless the key
is trusted, integrity is matched, and the source is supplier-authenticated or
owner-attested. `allowed_with_isolation` still requires matched integrity.
Unknown, mismatched, or unverified evidence therefore remains conditional or
non-authoritative.

Evidence:

```text
focused contracts probe: PASS
normal CTest: 111/111 PASS
worktree: clean
FlowLFS touched: NO
master touched: NO
force push: NO
```

Remaining Gate 6 work is provider execution enforcement, signed profile and
trust-store implementation, key expiry/revocation/rotation, and the explicit
Linux platform assurance matrix.
