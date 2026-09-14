# Safety-assurance baseline — 2026-09-14

## Scope

Gate 0 of the [Lyraform pre-self-hosting safety-assurance mission](../tasks/pre-self-hosting-safety-assurance.md).

This records the starting evidence for safety hardening. It is not a safety
certification, production-readiness claim, or assurance claim beyond the
tested boundaries listed here.

## Repository state

```text
branch: main
HEAD at baseline: c3efe2c5aae3e2095083c29f7ecf0ecb65ef1a31
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
./igor test    PASS — 107/107 tests at baseline
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
- the later CLI slice adds structured JSON failure output without contaminating
  artifact stdout; its canonical result is recorded in the next checkpoint.

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

## Graph failure-activation bound — 2026-09-14

The graph runtime's process-boundary failure diagnostic now bounds the scan of
the raw activation fragment before it can reach `stderr`. An oversized or
unterminated activation is represented as `null`, preserving the deterministic
structured failure disposition without allowing an attacker-controlled string
to exhaust diagnostic output. `flowgraph_runtime_bounds` forks a hostile
17 MiB activation through `flow_graph_fail` and verifies that the resulting
failure record remains bounded.

The safety state remains `CONTINUE`: this closes one current graph-runtime
output boundary but does not add cancellation, async scheduling, generalized
mutation durability, isolation, trust-anchor, or arbitrary-FFI semantics.

## Requirement parser bound — 2026-09-14

The public Frankencore version-requirement grammar now refuses version strings
and range expressions above 4096 bytes and refuses more than 128 dotted
components. The existing requirement probe covers oversized text and hostile
component cardinality, keeping parser allocation and comparison work within
an explicit contract.

The safety state remains `CONTINUE`: this is a bounded parser boundary, not a
general claim that every policy grammar or provider API is fully contained.

## Durable-history descriptor ownership — 2026-09-14

History scanning now uses scoped descriptor ownership. Read, bound-refusal,
normal, and exceptional parser paths all release the opened history file
descriptor, preventing an allocation or validation failure from bypassing
cleanup. The existing `frankencore_error_state_history` fault and recovery
suite remains green.

The safety state remains `CONTINUE`: deeper crash fault injection and retention
semantics remain open before broader mutation expansion.

## Durable-history bound validation — 2026-09-14

The history scanner now rejects zero line or total-history bounds with a
deterministic `rejected` result before opening the history file. This prevents
an invalid caller configuration from being interpreted as an empty or
unbounded history policy. The history probe covers inspection and append under
invalid bounds.

The safety state remains `CONTINUE`: crash-depth fault injection and retention
semantics remain open.

## Public contract collection bounds — 2026-09-14

Language maps, chain policies, and facade invocations now reject oversized
collections before their contents are admitted to downstream policy or
execution consumers. The conformance probe covers hostile chain-policy target
and facade-argument cardinality.

The safety state remains `CONTINUE`; serialized consumers still retain their
own schema-specific limits and the broader isolation provider is future work.

## Public contract text bounds — 2026-09-14

All scalar text fields in the current language-map, chain-policy, target,
facade, verification-evidence, and isolation-claim contracts now reject values
larger than 4096 bytes. The conformance probe covers a hostile language-map
identity in addition to the collection cases.

The safety state remains `CONTINUE`; the bounds do not create an isolation
provider or signed trust profile.

## Isolation assurance consistency — 2026-09-14

Isolation validation now rejects a `none` assurance claim paired with unknown
or stronger enforcement. This keeps the lowest assurance label consistent with
the documented self-report-only model; stronger claims retain their existing
local or independent-verification requirements.

The safety state remains `CONTINUE`: no isolation provider or independent
verifier is claimed.

## Recovery-lock ownership — 2026-09-14

Incomplete-tail repair now owns its history lock through a scoped guard. Normal
repair, refusal, quarantine failure, truncation failure, and exceptional scan
paths all release the lock, while the existing quarantine and durable-prefix
rules remain unchanged. The recovery fault suite remains green.

The safety state remains `CONTINUE`: deeper crash fault injection and retention
semantics remain open.

## Quarantine close-error disposition — 2026-09-14

Incomplete-tail repair now verifies close success for both the quarantine file
and the truncated history file. An injected quarantine close failure leaves
repair explicitly failed and the tail quarantined for operator handling; no
durability success is reported.

The safety state remains `CONTINUE`: deeper crash fault injection and retention
semantics remain open.

## Valid-prefix truncation fault — 2026-09-14

The recovery probe now injects a valid-prefix `ftruncate()` failure. Repair
returns an explicit error while retaining the incomplete history and its
quarantine; after the quarantine is handled, a second repair succeeds. This
proves that a failed recovery barrier does not silently promote partial history
or prevent deterministic operator retry.

The safety state remains `CONTINUE`: deeper crash fault injection and retention
semantics remain open.

## Parent-directory close-error disposition — 2026-09-14

The parent-directory durability barrier now treats a failed directory
`close()` as a failed synchronization barrier. The fault-injection probe
separately exercises file-close and directory-close failures, and both return
`uncertain` without claiming durable publication.

The safety state remains `CONTINUE`: deeper crash fault injection and retention
semantics remain open.

## Replay and reconciliation lock ownership — 2026-09-14

Mutation replay and two-history reconciliation now use scoped shared-lock
ownership, including the second-lock acquisition failure path. Exceptional
allocation, parsing, and comparison paths therefore release all acquired
history locks before returning their structured error result.

The safety state remains `CONTINUE`: deeper crash fault injection and
retention semantics remain open.

## Language resolver boundary — 2026-09-14

The Frankencore language resolver now refuses moniker inputs above 4096 bytes
and translates internal standard or non-standard failures into an unresolved
diagnostic. The language probe covers the hostile oversized-input case.

The safety state remains `CONTINUE`: broader Frankencore/provider exception
containment and allocation-fault injection remain open.

## Requirement parser exception boundary — 2026-09-14

The public version validator and range evaluator now translate internal
standard or non-standard failures into invalid structured results. Their
bounded grammar and hostile-input probe remain unchanged; no exception crosses
the current Frankencore requirement API.

The safety state remains `CONTINUE`: broader provider exception containment and
allocation-fault injection remain open.

## Recovery and append descriptor ownership — 2026-09-14

Append, quarantine, and valid-prefix truncation descriptors now use the scoped
descriptor guard. Exceptions during write, sync, or validation cannot bypass
descriptor cleanup, while the existing uncertain, quarantine, and repair
results remain unchanged.

The safety state remains `CONTINUE`: deeper crash fault injection and
retention semantics remain open.

## Append close-error disposition — 2026-09-14

Durable history append now treats a failed `close()` after writing and syncing
as non-durable, returning the existing `uncertain` disposition rather than
claiming a committed append. The fault-injection probe verifies this on an
isolated history file, preserving the documented possibility that bytes may
have changed while durability remains unknown.

The safety state remains `CONTINUE`: deeper crash fault injection and retention
semantics remain open.

## Read-path lock ownership — 2026-09-14

History inspection, record reads, and event lookup now use scoped shared-lock
ownership. Exceptional parsing and allocation paths therefore release the
history lock just like normal returns; the public read results and append-only
semantics are unchanged.

The safety state remains `CONTINUE`: append-path fault injection and retention
semantics remain open.

## Append-path lock ownership — 2026-09-14

The append-only history path now owns its exclusive lock through a scoped guard.
Duplicate, conflict, validation, bound-refusal, open failure, durability
failure, normal append, and exceptional paths all release the lock without
altering the existing `uncertain` and recovery dispositions.

The safety state remains `CONTINUE`: deeper crash fault injection and retention
semantics remain open.

## Trust and isolation evidence bounds — 2026-09-14

Verification evidence and isolation claims now reject text fields larger than
4096 bytes, and verification provenance/diagnostic collections larger than
100000 entries. The contract conformance probe covers hostile oversized fields
and confirms fail-closed validation before policy or provider selection.

This bounds the contract surface only; it does not claim that an isolation
provider, signed profile, or independent verifier exists.

## Current Tier 3 assurance rerun — 2026-09-14

The published head `e7f28c7a777200dc9b954b9f3432164a0a8cdae4` was rebuilt in
the independent Clang 18.1.3 sanitizer tree
`/tmp/lyraform-asan-20260914` with AddressSanitizer and UndefinedBehaviorSanitizer:

```text
cmake -S . -B /tmp/lyraform-asan-20260914 -G Ninja \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_CXX_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer'
cmake --build /tmp/lyraform-asan-20260914
ASAN_OPTIONS=detect_leaks=0:verify_asan_link_order=0 \
UBSAN_OPTIONS=halt_on_error=1 \
ctest --test-dir /tmp/lyraform-asan-20260914 --output-on-failure
```

The build completed all 169 targets and CTest passed 122/122 in 54.04
seconds. LeakSanitizer remains unavailable under the managed traced host;
leak detection was disabled as documented, while address and undefined
behavior checks remained enabled.

Independent Valgrind 3.22.0 runs with
`--error-exitcode=99 --leak-check=full
--errors-for-leak-kinds=definite,indirect` passed for the current history,
contract, language, requirements, and graph-runtime safety probes. The graph
fault fixture can report intentional `possibly lost` child-thread allocations;
these are excluded from the failure threshold and are not definite or
indirect leaks.

The same revision passed `./igor doctor`, `./igor build`, and `./igor test`
(122/122, 55.53 seconds), plus `git diff --check`. The worktree remains clean.

The safety state remains `CONTINUE`: this evidence strengthens implementation
and memory-tooling assurance but does not close the explicitly future
cancellation/async/backpressure, isolation/trust, platform-assurance, or
deeper crash/retention boundaries.
