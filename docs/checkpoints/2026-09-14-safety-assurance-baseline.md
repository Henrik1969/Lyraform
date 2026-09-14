# Safety-assurance baseline — 2026-09-14

## Scope

Gate 0 of the [Lyraform pre-self-hosting safety-assurance mission](../tasks/pre-self-hosting-safety-assurance.md).

This records the starting evidence for safety hardening. It is not a safety
certification, production-readiness claim, or assurance claim beyond the
tested boundaries listed here.

## Repository state

```text
branch: main
HEAD: c3efe2c5aae3e2095083c29f7ecf0ecb65ef1a31
upstream: origin/main
working tree: clean before this checkpoint
FlowLFS branch: remotes/origin/flowlfs-v0.1-alive, separate
historical branch: remotes/origin/master, separate
force push: not used
```

The environment rejected `git fetch --all --prune` because `.git/FETCH_HEAD`
is read-only. No repository history or remote-tracking ref was changed. The
local branch was already synchronized with `origin/main` before the audit.

## Commands and results

```text
./igor doctor  PASS
./igor check   PASS — 107 registered tests
./igor test    PASS — 107/107 tests
git diff --check PASS
```

The test run rebuilt 150 targets and completed in 58.73 seconds. The tested
surface includes malformed and hostile artifact rejection, exact capability
authorization, ABI/resource boundaries, graph activation, cleanup, LLVM/
TinyVM parity, TinyVM artifact validation, provenance, runtime capability
probes, target policy, and Flowkernel probes.

Previously recorded independent evidence remains:

```text
Clang 18.1.3 ASan/UBSan: 107/107 — ptrace-compatible leak settings
Valgrind native-chain boundary: 57/57
Flowbind fuzz gate: passing in canonical suite
```

## Confirmed controls at baseline

- malformed, unsupported, unauthorized, and mutated artifacts fail explicitly;
- provider discovery is not authorization;
- ABI/effect/provider policy must match the admitted tuple;
- portable TinyVM artifacts contain no host pointers or computed labels;
- receiver activation is fresh, bounded, and provenance-carrying;
- fan-out does not re-execute receivers;
- resource cleanup and double-cleanup refusal are tested;
- Text construction has an explicit outcome path for exhaustion;
- target selection has no silent fallback;
- LLVM and TinyVM switch/computed execution have differential coverage.

## Known open safety boundaries

- no formal safety certification or independent safety case yet;
- general cancellation, async execution, backpressure, and effectful parallel
  execution remain outside the admitted contract;
- generalized mutation provenance and durable error-state storage remain
  incomplete;
- isolation providers, signed profiles, stronger trust anchors, and broad
  multi-platform assurance remain future work;
- arbitrary native ABI/FFI remains refused;
- self-hosting has not started.

## Next gate

Build the safety-case inventory: map each admitted operation, provider,
artifact, resource, and lifecycle to its hazard, contract, enforcement point,
diagnostic, cleanup/recovery disposition, tests, and residual risk.
