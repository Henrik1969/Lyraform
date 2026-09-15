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

## Provenance and ULID checked boundary — 2026-09-15

The Frankencore provenance API now exposes `generate_ulid_checked()` and
`UlidGenerator::generate_checked()` as explicit `noexcept` boundaries. They
translate generator initialization, entropy, allocation, and unknown failures
into `UlidResult` without allowing a C++ exception to cross into a language,
artifact, provider, or embedding consumer. The three provenance
`to_json_checked()` overloads are also explicitly declared and defined
`noexcept`, matching their existing structured failure behavior. The throwing
helpers remain visible as Stage 0 compatibility APIs and are not part of the
Lyraform failure model.

The `frankencore_provenance_api` test now asserts the checked ULID contract and
validates the generated identifier. The focused test passed after the change;
the full normal, sanitizer, and Valgrind gates remain required before this
checkpoint is published. The safety state remains `CONTINUE`.

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

## Public exception-boundary audit — 2026-09-14

The Stage 0 executable audit found seven admitted artifact/tool boundaries
whose `main()` functions translated standard exceptions but had no final
non-standard-failure disposition: Flowbind, Flowanalyst, Flowvalidate,
Flowoptimize, Flowlower, Flowprepare, and Flowtarget. Each now emits the
existing structured `no_artifact` failure shape in diagnostics mode and a
deterministic refusal message otherwise. No language-level exception
semantics were added, and no exception is used as a safety mechanism.

The audit command was:

```text
rg -l 'int main\\(' --glob '*.{cpp,cc,cxx}' \\
  --glob '!Pattern_explored/**' --glob '!subprojects/FlowLFS/**'
```

Current public executable boundaries now have either a standard and
non-standard catch disposition or a deliberately non-throwing C/execv
interface. Historical `_archive/` and `Pattern_explored/` sources, and
test-only throw sites used to inject hostile worker/provider failures, remain
outside the admitted public surface and were not rewritten.

Focused GCC and Clang ASan/UBSan regressions passed 9/9 after this change;
the full canonical sanitizer and normal suites remain the required closure
gates at this checkpoint.

The safety state remains `CONTINUE`: allocation-fault injection across every
public boundary and the broader crash/retention campaign remain open.

## Flowbind allocation-fault boundary — 2026-09-14

Flowbind now has a test-only fault-injected executable that raises
`std::bad_alloc` at the public verification boundary. Its process wrapper
translates that exhaustion into `FLOWBIND_RESOURCE_EXHAUSTED` with
`disposition: no_artifact`; stdout remains empty. The focused test passed in
both the normal GCC tree and the Clang 18.1.3 ASan/UBSan tree.

The new CTest case is `flowbind_allocation_fault`. This closes allocation
exhaustion evidence for the Flowbind process boundary only; it does not claim
that all provider, parser, or Frankencore allocation paths have been injected.
Those broader fault-injection cases remain an open Gate 8 item.

The safety state remains `CONTINUE`.

## Flowlower structured input classification — 2026-09-15

Flowlower now classifies generic malformed or missing input failures at its
structured process boundary as `FLOWLOWER_INPUT_INVALID` with stage `input`.
The hostile malformed-stdin and missing-report cases both keep stdout empty
and identify `disposition: no_artifact` on stderr. The focused
`flowlower_pipeline` test passed after a fresh GCC build and in the Clang
18.1.3 ASan/UBSan tree; the complete canonical suite passed 149/149 in both
trees (GCC 55.76s; ASan/UBSan 103.24s).

This closes the current Flowlower input-classification gap. It does not claim
that all native exceptions, provider failures, or later self-hosting stages
are fully contained. The safety state remains `CONTINUE`.

## Flowoptimize structured input classification — 2026-09-15

Flowoptimize now classifies non-contract exceptions while consuming an input
artifact as `FLOWOPTIMIZE_INPUT_INVALID` at the `input` stage. The structured
path keeps stdout empty and preserves `disposition: no_artifact`; contract and
allocation failures remain distinct. The focused gate passes in GCC and Clang
18.1.3 ASan/UBSan trees.

The safety state remains `CONTINUE`.

## Flowanalyst structured input classification — 2026-09-15

Flowanalyst now classifies non-allocation failures while consuming or
projecting a frontend bundle as `FLOWANALYST_INPUT_INVALID` at the `analysis`
stage. The structured path keeps stdout empty and preserves
`disposition: no_artifact`; allocation and unknown failures remain distinct.
The focused gate passes in GCC and Clang 18.1.3 ASan/UBSan trees.

The safety state remains `CONTINUE`.

## Flowbind per-failure diagnostic bound — 2026-09-15

Blocked binding reports now cap each retained failure at 1,024 bytes and append
an explicit `...` marker when hostile provider identity text exceeds the cap.
A 10,000-byte identity case passes in GCC and Clang 18.1.3 ASan/UBSan, in
addition to the 256-entry aggregate bound.

The safety state remains `CONTINUE`.

## Flowbind bounded-report Memcheck — 2026-09-15

Direct Valgrind Memcheck coverage of the hostile 1,000-failure Flowbind
report completed with zero errors and zero definite or indirect leaks. A
wrapper-level run separately exposed a shell-runtime leak, so it is not
credited as executable evidence; the direct Flowbind result is the relevant
boundary proof. The safety state remains `CONTINUE`.

## Flowbind mixed-failure precedence — 2026-09-15

When one binding request produces multiple failure classes, Flowbind now uses
deterministic precedence: provider, then ABI, then policy. The individual
failures remain in the bounded report, so the summary code cannot hide the
underlying evidence. The full GCC and Clang 18.1.3 ASan/UBSan suites pass at
149/149.

The safety state remains `CONTINUE`.

## Flowbind bounded blocked-report output — 2026-09-15

Flowbind now bounds retained failures in a blocked binding report to 256
entries, while preserving `failure_count` and setting `failures_truncated` when
additional failures exist. A hostile 1,000-failure report proves the bounded
shape and deterministic blocked disposition in both GCC and Clang 18.1.3
ASan/UBSan trees.

This bounds report output only; input, provider hashing, execution, and broader
artifact collection limits remain separately scoped. The safety state remains
`CONTINUE`.

## Flowbind structured failure classification — 2026-09-15

Flowbind now classifies exceptional structured-CLI failures with stable
machine-readable categories: `FLOWBIND_INPUT_INVALID`,
`FLOWBIND_POLICY_FAILURE`, `FLOWBIND_ABI_FAILURE`, and
`FLOWBIND_PROVIDER_FAILURE`. Every classified failure retains the
`no_artifact` disposition. The provider gate covers input and policy examples
alongside the blocked provider-inspection report in both normal GCC and Clang
18.1.3 ASan/UBSan trees.

This is a current inspection-boundary control; it does not create recovery,
replacement, trust-store, or native-execution semantics. The safety state
remains `CONTINUE`.

## Flowbind blocked-report condition codes — 2026-09-15

Blocked binding reports now include stable `code` and `stage` fields. Provider
loss, policy denial, and ABI refusal are machine-distinguishable while the
existing failure text remains available for operators. The focused provider
gate passes in the normal GCC and Clang 18.1.3 ASan/UBSan trees.

This improves classification only; it does not grant execution permission or
admit post-binding replacement, recovery, trust-store, or arbitrary ABI/FFI.
The safety state remains `CONTINUE`.

## Generated-provider replacement evidence recheck — 2026-09-15

The existing `native_binding_generation` gate was re-run in the normal GCC
tree and the Clang 18.1.3 ASan/UBSan tree. It passed in both trees, including
the digest-bound replacement-provider refusal. This confirms the narrow
replacement evidence credited in the safety matrix; it does not admit
post-binding provider replacement or native invocation.

The safety state remains `CONTINUE`.

## Flowbind provider-unavailable boundary — 2026-09-15

Flowbind now has explicit hostile coverage for a policy-authorized library
that is absent at inspection. The `flowbind_provider` gate proves that the
boundary returns a versioned `binding_report` with `status: blocked`, reports
`library unavailable`, and publishes no ready binding artifact.

The focused test passed in both the normal GCC tree and the Clang 18.1.3
ASan/UBSan tree. This closes absence-at-inspection only; provider replacement
after authorization, digest/trust admission, and native execution remain open
safety work.

The safety state remains `CONTINUE`.

## Optional ConfigResolve provider evidence boundary — 2026-09-15

The canonical Lyraform build reports `ConfigResolve adapter disabled` because
no ConfigResolve dependency is configured. The adapter source does expose a
checked `noexcept` fail-closed decision boundary with cleanup on standard and
non-standard failures, but no provider-specific runtime, allocation, or
cleanup test is claimed by this baseline. That evidence must be produced in a
dependency-enabled isolated build before the adapter can influence an
admitted policy decision.

The safety state remains `CONTINUE`.

## Durable-history partial-write crash boundary — 2026-09-15

The history fault matrix now includes a child process that crashes immediately
after a partial record write and before synchronization. The parent observes
an `incomplete` history with only the pre-existing valid record; explicit
repair then quarantines the torn tail and restores that valid prefix without
promoting the crashed append. `frankencore_error_state_history` passes in the
normal GCC and Clang 18.1.3 ASan/UBSan trees. Valgrind 3.22.0 Memcheck reports
zero errors and zero definite/indirect leaks across the forked test.

This strengthens crash-depth evidence but does not close retention or all
possible kernel/filesystem crash points. The safety state remains `CONTINUE`.

## Durable-history parent-directory crash boundary — 2026-09-15

The history fault matrix now also kills a child after the complete record file
has been synchronized but before the parent-directory synchronization barrier
returns. The parent observes both complete records as valid; no torn record is
promoted and no silent retry is introduced. The history test passes in GCC and
Clang 18.1.3 ASan/UBSan; Valgrind 3.22.0 reports zero errors and zero
definite/indirect leaks. Directory durability remains an explicit filesystem
boundary rather than an inferred guarantee.

Retention and additional kernel/filesystem crash points remain open. The
safety state remains `CONTINUE`.

## Shared diagnostic-writer adversarial contract — 2026-09-15

The Flowcontracts diagnostic writer now has direct adversarial unit coverage:
JSON quotes, slashes, newlines, and control bytes are escaped correctly; a
10,000-byte hostile value is capped at 4096 bytes with an atomic `...`
truncation marker; and the bounded output remains valid JSON string content.
The `flowcontracts_json` test passed in the normal GCC tree and under
Valgrind 3.22.0 Memcheck with zero errors and zero bytes still allocated at
exit.

This proves the shared primitive’s local contract, not complete end-to-end
log retention or provider fault coverage. The safety state remains
`CONTINUE`.

## Graph-planner diagnostic boundary correction — 2026-09-15

The Flowparallel graph planner’s structured exception path now uses the
shared bounded diagnostic writer directly. This removes its previous
unbounded exception-text stream path and preserves the existing planner
artifact and refusal semantics. Its normal and allocation-fault focused tests
passed in the GCC tree.

The safety state remains `CONTINUE`.

## Lyraform compiler-driver diagnostic boundary — 2026-09-15

The current Lyraform compiler driver now emits structured failure records via
the shared bounded, allocation-free Flowcontracts writer. Its existing
explicit translation from parser/runtime failures to `DiagnosticError`,
tagged runtime outcomes, and no-artifact process failure remains unchanged;
only the second-allocation hazard in JSON escaping was removed.

The focused integration, pipeline, pass-corpus, stdlib, support, UTF-8,
structured-diagnostic, and allocation-fault gates passed (10/10 selected
tests) in the normal GCC tree. This closes the current compiler-driver
diagnostic formatter only; language-runtime allocation and historical
implementation snapshots remain separately scoped.

The safety state remains `CONTINUE`.

## Kernel and artifact-validator diagnostic boundary — 2026-09-15

Flowkernel and Flowvalidate now use the bounded Flowcontracts writer when
structured process failures include exception text. Flowkernel’s probe
executable is wired to the shared header without changing its probe or
machine-result contract; Flowvalidate’s canonical artifact output is likewise
unchanged.

The focused `flowkernel_readonly`, `flowkernel_tempfs`, `flowkernel_ipc`,
`flowkernel_socket_ipc`, `flowkernel_loopback`, `flowkernel_namespaces`,
`flowkernel_all`, `flowkernel_all_contract`, `flowkernel_require_complete`,
`flowkernel_structured_diagnostics`, `flowvalidate_artifacts`, and
`flowvalidate_allocation_fault` tests passed in the normal GCC tree. This
closes only these two public process boundaries; broader API and provider
fault coverage remains open Gate 8 work.

The safety state remains `CONTINUE`.

## Lowering-tool diagnostic allocation boundary — 2026-09-15

Flowlower, Flowprepare, and Flowtarget now use the shared bounded,
allocation-free Flowcontracts writer for structured process failures. Their
catch paths no longer allocate temporary escaped strings or serialize
attacker-controlled exception text through the normal JSON allocator. The
existing artifact output and compatibility diagnostics are unchanged.

The focused `flowlower_backend_artifact`, `flowlower_pipeline`,
`flowlower_allocation_fault`, `flowprepare_allocation_fault`,
`flowtarget_policy_boundary`, `flowtarget_cross_compile`, and
`flowtarget_allocation_fault` tests passed in the normal GCC tree. This closes
these three lowering-tool process boundaries only; deeper parser/provider
containment and broader allocation-fault coverage remain open Gate 8 work.

The safety state remains `CONTINUE`.

## Toolchain diagnostic allocation boundary — 2026-09-15

Flowanalyst, Flowoptimize, and Flowbind now emit structured failure JSON with
the shared bounded, allocation-free Flowcontracts diagnostic writer. Their
catch paths no longer construct temporary escaped strings or temporary
`std::string` values before reporting a failure. Escape tokens are emitted
atomically within the 4096-byte diagnostic bound, including control-byte and
truncation cases; normal artifact JSON serialization remains unchanged.

The focused `flowanalyst_pipeline`, `flowanalyst_allocation_fault`,
`flowoptimize_pipeline`, `flowoptimize_allocation_fault`,
`flowbind_provider`, `flowbind_fuzz`, and `flowbind_allocation_fault` tests
passed in the normal GCC tree. This closes only these three executable
process boundaries; Flowlower, Flowprepare, Flowtarget, and broader
Frankencore/provider APIs remain open Gate 8 work.

The safety state remains `CONTINUE`.

## Flowparallel process diagnostic allocation boundary — 2026-09-15

The primary Flowparallel CLI and CPU provider now emit exception messages in
structured mode through a bounded, allocation-free JSON writer. Escaping is
performed directly to stderr, control bytes remain valid JSON, and fields are
bounded at 4096 bytes. The focused `flowparallel_pipeline` and
`flowparallel_cpu_provider` gates passed 2/2.

This closes the named process projections only; remaining Flowparallel
providers still require the same audit. The safety state remains `CONTINUE`.

The follow-up slice extends the bounded writer to the CUDA provider, CUDA
execution, runtime planner, and graph-reference process boundaries; their
focused normal provider/allocation gates passed 8/8.

Matrix-benchmark now uses the same bounded writer for its structured exception
diagnostic; its normal and allocation-fault gates passed 2/2. Graph-planner
now uses a bounded non-owning diagnostic view backed by the same writer; its
normal and allocation-fault gates passed 2/2. The current named Flowparallel
catch-path set is covered, while broader process diagnostics remain
provisional.

## Reference CLI diagnostic allocation boundary — 2026-09-15

The clock and provenance reference CLIs now emit structured failure strings
through a bounded, allocation-free C stdio writer. Machine-readable catch
paths cap each encoded field at 4096 bytes, preserve JSON escaping, and retain
the `no_artifact` disposition without constructing temporary strings.
The focused clock/revision gate passed 7/7.

This covers the named reference CLIs only; broader process diagnostic and
platform coverage remains provisional. The safety state remains `CONTINUE`.

## Flowparallel worker-failure capture bound — 2026-09-15

The independent CPU executor now captures worker exception text into a fixed
256-byte buffer while still inside the worker catch boundary. It truncates
hostile messages deterministically and constructs the public diagnostic only
after joining all workers, avoiding an allocation-dependent failure while
handling the original task failure.

The focused CPU execution and launch-allocation-fault tests passed 2/2. This
does not admit cancellation, async scheduling, or a general scheduler. The
safety state remains `CONTINUE`.

## Frankencore runtime projection allocation fault — 2026-09-15

The runtime memory test now injects allocation exhaustion inside the checked
capability JSON projection. It verifies an invalid result, empty serialized
output, and the explicit `exhausted memory` diagnostic. The focused
`frankencore_runtime_memory` gate passed 1/1.

This adds direct projection-fault evidence; it does not claim complete runtime
allocation coverage or broader platform assurance. The safety state remains
`CONTINUE`.

## Frankencore runtime CUDA close-fault evidence — 2026-09-15

Runtime capability discovery now has a hardware-independent cleanup-fault
variant. The test-only build opens a ubiquitous libc handle, executes the real
`dlclose()` path, and forces the reported close result to fail; the resulting
capability snapshot downgrades CUDA evidence to `unknown` with an explicit
diagnostic and emits no stderr failure. `frankencore_runtime_probe_close_fault`
passed alongside the normal runtime probe (2/2 focused tests).

This closes the previously untestable CUDA close-classification case without
claiming CUDA hardware availability or broader platform assurance. The safety
state remains `CONTINUE`.

## Canonical native file-producer inventory gate — 2026-09-15

The new `native_file_producer_inventory` gate scans the canonical production
C/C++ source roots and admits exactly six files with direct file-write
authority: Flowmini and Flowlower, TinyVM v1 and v2 artifact writers,
Frankencore provenance/history, and Flowkernel's isolated temporary-filesystem
probe. It validates the defining compiler publication markers, durable-history
append and parent-sync markers, and the probe's scoped create/unlink/remove
lifecycle. An unclassified writer or missing control marker fails the test.

The focused inventory passed 1/1. Complete suites passed 155/155 under GCC in
60.52 seconds and 155/155 under Clang 18.1.3 ASan/UBSan in 115.00 seconds with
the documented leak setting. Valgrind is not applicable to the source-only
inventory script; the controlled native writers retain their preceding memory
evidence.

This closes the unknown-current-native-producer residual for the present
canonical C/C++ graph. Script-language generators, test-fixture constructors,
historical snapshots, newly added source roots, streamed stdout, and
cross-platform behavior remain separately classified. The safety state remains
`CONTINUE`.

## Frankencore apt-provider pipe cleanup — 2026-09-15

The native `apt-indextargets` package reader now owns its `popen()` stream with
a scoped `pclose()` guard. Allocation or parser failure after process launch
therefore cannot bypass pipe cleanup; normal completion still observes the
provider exit status. The fault variant injects allocation exhaustion after
`popen()` and the checked boundary admits no partial inventory.

The focused package probe and allocation-fault tests passed 2/2 in the normal
tree. This closes one provider cleanup path and does not claim complete native
cleanup or cancellation coverage. The safety state remains `CONTINUE`.

## CUDA execution dynamic-library cleanup — 2026-09-15

The CUDA execution provider now uses the shared `DynamicLibrary` owner. Its
explicit `close()` result is checked after normal execution and on exception
paths; a nonzero or throwing close callback cannot become a successful
provider result. The hardware-independent cleanup test injects a close
failure and a throwing close callback, and verifies idempotent handle clearing; the CUDA allocation-fault
boundary remains green. Graph, matrix benchmark, and provider-probe loaders
now use the same checked dynamic-library owner and explicit close-result
handling. The safety state remains `CONTINUE`.

## CUDA dynamic-library cleanup residual — 2026-09-15

This residual was superseded by the later provider-probe, graph, and matrix
loader migration recorded above; the paragraph below is retained as the audit
trail showing the originally identified gap.

The Gate 3 provider audit identified that the CUDA graph and matrix benchmark
`Library` wrappers ignore nonzero `dlclose` results. This is now explicitly
unclaimed in the safety case; it is not equivalent to the separately tested
device/cuBLAS handle cleanup contract. A future hardware-independent wrapper
test and an explicit close-failure artifact policy are required before those
dynamic-library paths can be considered cleanup-complete. The safety state
remains `CONTINUE`.

## Optional ConfigResolve adapter containment — 2026-09-15

The optional ConfigResolve policy adapter now declares its public resolution
boundary `noexcept`, translates allocation, standard, and unknown failures to
an unresolved `Decision`, and preserves provider-context cleanup when C++
exception paths are taken. ConfigResolve is disabled in the authoritative
build, so this source-level hardening is not presented as compiled-provider
evidence; an enabled-provider build and fault suite remain required. The
safety state remains `CONTINUE`.

## Runtime capability discovery boundary — 2026-09-15

Runtime capability discovery now exposes `discover_checked()` as an explicit
`noexcept` API. Allocation, standard, and unknown discovery failures become an
invalid result with no capability snapshot admitted; the reference probe then
serializes only a valid snapshot through its checked JSON boundary. Normal,
fault-injected, sanitizer, and memory-focused runtime tests pass. Broader
platform assurance remains future work. The safety state remains `CONTINUE`.

The runtime CUDA probe also now checks `dlclose` and downgrades the capability
evidence to `unknown` if driver-library cleanup fails. Hardware-independent
close-fault injection remains open.

## Requirements and language-map non-throwing boundaries — 2026-09-15

The existing structured-result APIs for version requirements and language-map
moniker resolution now declare `noexcept`. Their standard and unknown
exception paths use fixed diagnostics, avoiding secondary allocation while
translating a failure. Reference probes assert both contracts at compile time.
The focused normal and ASan/UBSan tests passed, including the existing
allocation-fault cases. Full-suite verification is recorded with this
checkpoint before publication. The safety state remains `CONTINUE`.

## Frankencore contract validation boundary — 2026-09-15

Frankencore now exposes `validate_checked()` for verification evidence,
isolation claims, language maps, chain policies, and facade invocations. Each
checked overload is explicitly `noexcept` and converts allocation, standard,
and unknown failures into an invalid structured result. The existing
`validate()` overloads remain available as Stage 0 compatibility helpers; they
are not the Lyraform failure model.

The conformance test asserts the checked contract and validates a normal
evidence record through it. Full normal and sanitizer gates remain required
after this slice. The safety state remains `CONTINUE`.

## Provenance checked-boundary verification — 2026-09-15

The provenance checked-boundary slice rebuilt successfully and passed the
focused `frankencore_provenance_api` test. The full normal GCC CTest gate
passed 147/147, the Clang 18.1.3 ASan/UBSan gate passed 147/147, and the
Valgrind provenance API run reported zero errors, zero leaks, and zero bytes
in use at exit. This evidence covers the new checked ULID boundary and the
explicitly `noexcept` serialization declarations; broader Frankencore
allocation-fault injection remains open.

The safety state remains `CONTINUE`.

## TinyVM Text compatibility allocation precedence — 2026-09-15

The TinyVM Text compatibility thunks now preserve allocation exhaustion when
artifact-owned string copying fails; they no longer convert that condition to
`invalid_input` or a provider-unavailable outcome. The focused provider fault
test and complete 147/147 normal and 147/147 ASan/UBSan suites passed, with
the focused Valgrind run reporting zero errors and zero leaks.

The safety state remains `CONTINUE`.

## TinyVM runtime-provider string allocation classification — 2026-09-15

The TinyVM runtime provider now distinguishes artifact-owned string-copy
allocation failure from an invalid string handle and reports the explicit
`runtime provider allocation exhausted` fault. The focused
`tinyvm_runtime_provider_allocation_fault` test covers both string copying and
provider-owned Text-outcome retention; neither path publishes an initialized
result or partial provider state.

The focused test passed under normal GCC, Clang 18.1.3 ASan/UBSan, and
Valgrind 3.22.0 Memcheck with zero errors and zero leaks. The safety state
remains `CONTINUE`.

The subsequent provider-string consumer expansion preserved the complete
canonical result: 147/147 normal GCC tests and 147/147 Clang 18.1.3
ASan/UBSan tests passed.

## TinyVM runtime-provider storage allocation classification — 2026-09-15

The runtime provider now distinguishes activation-local storage allocation
exhaustion from a storage bounds violation. The public resolver returns
`runtime provider allocation exhausted`, leaves the result uninitialized, and
retains no partial storage state. The focused
`tinyvm_runtime_provider_allocation_fault` test covers artifact-string copy,
provider-owned outcome retention, and storage allocation exhaustion. It
passed under normal GCC, Clang 18.1.3 ASan/UBSan, and Valgrind 3.22.0
Memcheck with zero errors and zero leaks.

The complete canonical suite passed 147/147 under normal GCC and 147/147
under Clang 18.1.3 ASan/UBSan. The safety state remains `CONTINUE`.

The subsequent expansion applies the same storage-exhaustion precedence to
file read/write and memory copy, move, and comparison thunks. The complete
canonical result remains 147/147 under normal GCC and 147/147 under Clang
18.1.3 ASan/UBSan; the focused Valgrind run reports zero errors and zero
leaks.

The storage-classification precision pass also confirms that invalid handles
remain bounds failures rather than being mislabeled as exhaustion. The
complete normal and sanitized suites remain 147/147, and the focused provider
Valgrind run remains clean with zero errors and zero leaks.

## TinyVM runtime-provider descriptor ownership exhaustion — 2026-09-15

The TinyVM runtime provider now classifies failure to grow its owned-file-
descriptor table as `runtime provider allocation exhausted`. A descriptor
opened before that failure is closed immediately; no descriptor ownership or
initialized result is retained. The focused
`tinyvm_runtime_provider_allocation_fault` test covers this path alongside
string and storage exhaustion, and passed under normal GCC, Clang 18.1.3
ASan/UBSan, and Valgrind 3.22.0 Memcheck with zero errors and zero leaks.

The complete canonical suite passed 147/147 under normal GCC and 147/147
under Clang 18.1.3 ASan/UBSan. The safety state remains `CONTINUE`.

## TinyVM runtime-provider allocation-fault boundary — 2026-09-15

The TinyVM runtime provider now classifies failure to retain a provider-owned
Text outcome as `runtime provider allocation exhausted`. It leaves the result
uninitialized and retains no partial outcome table. The focused
`tinyvm_runtime_provider_allocation_fault` test injects `realloc` failure and
passed in the normal GCC tree, Clang 18.1.3 ASan/UBSan, and Valgrind 3.22.0
Memcheck configurations; Valgrind reported zero errors and zero leaks.

This closes the tested provider-owned outcome retention path. Other native
provider allocation sites remain separately scoped and are not implied safe by
this boundary.

The safety state remains `CONTINUE`.

## TinyVM runtime-provider cleanup failure — 2026-09-15

The TinyVM runtime provider now treats a failed dynamic-library release as an
explicit `runtime provider library cleanup failed` result and clears any
computed value before returning failure. The injected cleanup test confirms
that teardown failure cannot be accepted as successful provider execution;
the descriptor and allocation cases in the same test retain no partial
ownership.

The focused test passed under normal GCC, Clang 18.1.3 ASan/UBSan, and
Valgrind 3.22.0 Memcheck with zero errors and zero leaks. The complete
canonical suites passed 147/147 under both normal GCC and Clang 18.1.3
ASan/UBSan. The safety state remains `CONTINUE`.

The subsequent complete canonical runs passed 147/147 under normal GCC and
147/147 under Clang 18.1.3 ASan/UBSan with leak detection disabled.

## Igor gate refresh at safety checkpoint — 2026-09-15

At published revision `95d487b`, the user-facing gates were rerun from the
canonical checkout: `./igor doctor` passed, `./igor build` completed, and
`./igor test` passed the complete CTest graph at 146/146. Direct normal GCC
and Clang 18.1.3 ASan/UBSan CTest runs independently passed 146/146 as well.

The safety state remains `CONTINUE`.

## TinyVM artifact v1 allocation-fault boundary — 2026-09-15

The retained TinyVM v1 artifact writer now has a test-only fault-injected
variant that exhausts its serialization buffer allocation. The public write
boundary returns the deterministic `allocation failed` diagnostic and does
not publish an output artifact. The focused
`tinyvm_artifact_v1_allocation_fault` test passed in the normal GCC tree and
the Clang 18.1.3 ASan/UBSan tree with leak detection disabled. The focused
fault-injected writer also passed under Valgrind 3.22.0 Memcheck with
`--error-exitcode=99`, zero errors, and zero bytes still allocated at exit.

This covers the retained v1 compatibility writer only; it does not claim
complete allocation-fault coverage for every legacy reader or runtime/provider
allocation site.

The safety state remains `CONTINUE`.

## TinyVM artifact v2 allocation-fault boundary — 2026-09-15

The TinyVM v2 artifact loader now has a test-only fault-injected variant that
exhausts its owned buffer allocation before parsing. The public read boundary
returns the deterministic `allocation failed` diagnostic and retains no
partially allocated artifact ownership. The focused
`tinyvm_artifact_v2_allocation_fault` test passed in the normal GCC tree and
the Clang 18.1.3 ASan/UBSan tree with leak detection disabled. The same
injected boundary passed under Valgrind 3.22.0 Memcheck with
`--error-exitcode=99`, zero errors, and zero bytes still allocated at exit.

This closes the v2 artifact buffer-allocation refusal at the loader boundary.
It does not claim complete allocation-fault coverage for every C artifact
consumer or runtime/provider allocation site; those remain separately scoped.

The safety state remains `CONTINUE`.

## Durable-history fsync crash-depth test — 2026-09-15

The history fault matrix now includes a child process that continues a valid
error-state lifecycle, writes a complete record, and exits at the append
`fsync()` barrier. The parent then inspects the same history and requires a
valid complete prefix containing exactly the previously committed and newly
written records. A lifecycle-invalid child append is deliberately avoided so
the test reaches the durability barrier.

The crash-depth case passes under the normal GCC build, Clang 18.1.3
ASan/UBSan with the documented leak exclusion, and Valgrind 3.22.0 with
definite/indirect leak failures enabled. This strengthens crash evidence but
does not claim physical-storage guarantees beyond the documented fsync
contract; retention and broader crash points remain open.

The safety state remains `CONTINUE`.

## TinyVM lowerer allocation-fault boundary — 2026-09-15

The portable TinyVM backend lowerer now has a test-only fault-injected target
that raises `std::bad_alloc` before consuming the backend artifact. Its public
boundary returns a nonzero failure, leaves stdout empty, and creates no output
artifact. The focused `tinyvm_backend_lowering_allocation_fault` test is part
of the TinyVM CTest suite.

This covers pre-lowering exhaustion only; TinyVM artifact reader/writer
allocation paths remain bounded C APIs and require separate injected evidence.

The safety state remains `CONTINUE`.

## Flowparallel CPU-execution launch-allocation fault — 2026-09-15

The CPU independent-task executor now has a test-only runtime variant that
injects allocation failure at worker-vector launch. The executor converts it
to `WORKER_LAUNCH_RESOURCE_EXHAUSTED`, reports zero completed tasks and no artifact, and
its `std::jthread` ownership scope guarantees already-started workers are
joined during unwinding. The focused
`flowparallel_cpu_execution_allocation_fault` test is part of the canonical
CTest suite.

This strengthens partial-initialization cleanup evidence; cancellation,
backpressure, and effectful scheduling remain refused by policy.

The safety state remains `CONTINUE`.

## Flowparallel planner unsupported-scheduling refusal — 2026-09-15

The top-level Flowparallel planner now checks scheduling requests before
semantic projection and explicitly refuses `parallel_effectful_v1`,
`cancellation`, `async`, and `backpressure` with an unsupported plan and no
fallback artifact. The pipeline regression covers all four requests and
requires exit status 2 plus `fallback.emitted: false`.

This closes a planner-level fail-closed gap; it does not implement any of
those features. Complete cancellation, queue, ordering, commit, and effect
contracts remain open Gate 4 work.

The safety state remains `CONTINUE`.

## Flowmini compiler allocation-fault boundary — 2026-09-15

The current Flowmini compiler CLI now has a test-only fault-injected target
that raises `std::bad_alloc` after source selection and before source expansion
or compilation. Its existing public structured boundary returns
`FLOW_RESOURCE_EXHAUSTED` with `disposition: no_artifact` and leaves stdout
empty. The focused `flowmini_allocation_fault` test is part of the compiler’s
CTest suite.

This covers the compiler CLI boundary before frontend work begins. It does not
claim allocation-fault coverage for every lexer, parser, AST, runtime, or
projection allocation site; those remain open Gate 8 work.

The safety state remains `CONTINUE`.

## Flowparallel matrix-benchmark allocation-fault boundary — 2026-09-15

The CPU/CUDA matrix-benchmark boundary now has a test-only fault-injected
executable that raises `std::bad_alloc` before workload allocation or CUDA
library access. Its public structured boundary returns
`FLOWPARALLEL_MATRIX_BENCHMARK_RESOURCE_EXHAUSTED` with
`disposition: no_artifact` and leaves stdout empty. The focused
`flowparallel_matrix_benchmark_allocation_fault` test is part of the canonical
CTest suite and is independent of CUDA hardware.

This covers pre-workload exhaustion only; benchmark provider execution and
resource cleanup remain governed by their existing explicit contracts.

The safety state remains `CONTINUE`.

## Flowparallel CUDA-execution allocation-fault boundary — 2026-09-15

The CUDA matrix-execution boundary now has a test-only fault-injected
executable that raises `std::bad_alloc` before dynamic CUDA library access or
resource acquisition. Its public structured boundary returns
`FLOWPARALLEL_CUDA_EXECUTE_RESOURCE_EXHAUSTED` with
`disposition: no_artifact` and leaves stdout empty. The focused
`flowparallel_cuda_execute_allocation_fault` test is part of the canonical
CTest suite and is independent of CUDA hardware.

This covers pre-acquisition exhaustion only. CUDA operation failure and
cleanup paths remain governed by the existing explicit resource-cleanup
contract and tests.

The safety state remains `CONTINUE`.

## Flowparallel graph-CUDA allocation-fault boundary — 2026-09-15

The optional CUDA graph-reachability boundary now has a test-only
fault-injected executable that raises `std::bad_alloc` before semantic
analysis or CUDA library access. Its public structured boundary returns
`FLOWPARALLEL_GRAPH_CUDA_RESOURCE_EXHAUSTED` with `disposition: no_artifact`
and leaves stdout empty. The focused
`flowparallel_graph_cuda_allocation_fault` test is part of the canonical CTest
suite and is independent of CUDA hardware.

This covers exhaustion at the graph-CUDA process boundary only. It does not
claim CUDA availability, kernel execution, or effectful parallelism.

The safety state remains `CONTINUE`.

## Flowparallel graph-reference allocation-fault boundary — 2026-09-15

The CPU graph-reference reachability boundary now has a test-only
fault-injected executable that raises `std::bad_alloc` before consuming the
semantic report. Its public structured boundary returns
`FLOWPARALLEL_GRAPH_REFERENCE_RESOURCE_EXHAUSTED` with
`disposition: no_artifact` and leaves stdout empty. The focused
`flowparallel_graph_reference_allocation_fault` test is part of the canonical
CTest suite.

This covers exhaustion at the graph-reference process boundary only. It does
not claim unbounded graph execution or effectful parallelism; matrix and
execution limits remain explicit.

The safety state remains `CONTINUE`.

## Flowparallel graph-planner allocation-fault boundary — 2026-09-15

The graph representation/provider planner now has a test-only fault-injected
executable that raises `std::bad_alloc` before consuming graph or capability
evidence. Its public structured boundary returns
`FLOWPARALLEL_GRAPH_PLANNER_RESOURCE_EXHAUSTED` with
`disposition: no_artifact` and leaves stdout empty. The focused
`flowparallel_graph_planner_allocation_fault` test is part of the canonical
CTest suite.

This covers exhaustion at the graph-planner process boundary only. It does not
claim graph execution, CUDA availability, or effectful parallelism; those
remain separately bounded or refused by policy.

The safety state remains `CONTINUE`.

## Flowparallel CUDA-provider allocation-fault boundary — 2026-09-15

The optional CUDA provider-probe boundary now has a test-only fault-injected
executable that raises `std::bad_alloc` after CLI parsing and before input or
driver access. Its public structured boundary returns
`FLOWPARALLEL_CUDA_RESOURCE_EXHAUSTED` with `disposition: no_artifact` and
leaves stdout empty. The focused `flowparallel_cuda_allocation_fault` test is
part of the canonical CTest suite and is independent of CUDA hardware.

This covers exhaustion at the CUDA provider-probe process boundary only. It
does not claim CUDA availability, kernel execution, or effectful parallelism;
those remain provider- and Gate 4-dependent.

The safety state remains `CONTINUE`.

## Flowparallel CPU-provider allocation-fault boundary — 2026-09-15

The CPU provider-selection boundary now has a test-only fault-injected
executable that raises `std::bad_alloc` before consuming the execution plan.
Its public structured boundary returns `FLOWPARALLEL_CPU_RESOURCE_EXHAUSTED`
with `disposition: no_artifact` and leaves stdout empty. The focused
`flowparallel_cpu_allocation_fault` test is part of the canonical CTest suite.

This covers exhaustion at the CPU provider-selection process boundary only.
It does not claim that effectful parallelism, asynchronous execution, or
cancellation is admitted; those remain open Gate 4 work.

The safety state remains `CONTINUE`.

## Flowparallel runtime-planner allocation-fault boundary — 2026-09-15

The runtime provider-planning boundary now has a test-only fault-injected
executable that raises `std::bad_alloc` after CLI parsing and before reading
plan or capability evidence. Its public structured boundary returns
`FLOWPARALLEL_RUNTIME_PLANNER_RESOURCE_EXHAUSTED` with
`disposition: no_artifact` and leaves stdout empty. The focused
`flowparallel_runtime_planner_allocation_fault` test is part of the canonical
CTest suite.

This covers exhaustion at the planner process boundary only. It does not claim
that cancellation, backpressure, or effectful parallelism is admitted, nor
complete allocation-fault coverage for every provider and scheduler path;
those remain open Gate 4 and Gate 8 work.

The safety state remains `CONTINUE`.

## Flowparallel allocation-fault boundary — 2026-09-15

The parallel execution-planning boundary now has a test-only fault-injected
executable that raises `std::bad_alloc` before consuming the semantic report.
Its public structured boundary returns `FLOWPARALLEL_RESOURCE_EXHAUSTED` with
`disposition: no_artifact` and leaves stdout empty. The focused
`flowparallel_allocation_fault` test is part of the canonical CTest suite.

This covers exhaustion at the planning process boundary only. It does not
claim that effectful parallelism is admitted, nor complete allocation-fault
coverage for every provider, scheduler, or runtime path; those remain open
Gate 4 and Gate 8 work.

The safety state remains `CONTINUE`.

## Flowprepare allocation-fault boundary — 2026-09-15

The backend-artifact preparation boundary now has a test-only fault-injected
executable that raises `std::bad_alloc` before consuming its input artifact.
Its public structured boundary returns `FLOWPREPARE_RESOURCE_EXHAUSTED` with
`disposition: no_artifact` and leaves stdout empty. The focused
`flowprepare_allocation_fault` test is part of the canonical CTest suite.

This covers exhaustion at the preparation process boundary only. It does not
claim complete allocation-fault coverage for every artifact parser, policy,
or backend provider path; those remain open Gate 8 work.

The safety state remains `CONTINUE`.

## Flowtarget allocation-fault boundary — 2026-09-15

The target-policy resolver now has a test-only fault-injected executable that
raises `std::bad_alloc` after CLI parsing and before policy access. Its public
structured boundary returns `FLOWTARGET_RESOURCE_EXHAUSTED` with
`disposition: no_artifact` and leaves stdout empty. The focused
`flowtarget_allocation_fault` test is part of the canonical CTest suite.

This covers exhaustion at the target-policy process boundary only. It does not
claim complete allocation-fault coverage for every policy parser, validator,
or backend provider path; those remain open Gate 8 work.

The safety state remains `CONTINUE`.

## Flowlower allocation-fault boundary — 2026-09-15

The backend lowering boundary now has a test-only fault-injected executable
that raises `std::bad_alloc` before consuming the input artifact. Its public
structured boundary returns `FLOWLOWER_RESOURCE_EXHAUSTED` with
`disposition: no_artifact` and leaves stdout empty. The focused
`flowlower_allocation_fault` test is now part of the canonical CTest suite.
The normal, Clang 18.1.3 ASan/UBSan, and Valgrind 3.22.0 runs all preserve
the expected refusal; Valgrind reports no definite or indirect leaks.

This covers exhaustion at the lowerer process boundary only. It does not claim
complete allocation-fault coverage for every lowering-plan parser, provider,
or backend emission allocation site; those remain open Gate 8 work.

The safety state remains `CONTINUE`.

## Flowvalidate allocation-fault boundary — 2026-09-15

The canonical artifact-validation command now has a test-only fault-injected
variant that raises `std::bad_alloc` after option parsing and before input
consumption. Its public boundary emits valid structured JSON with
`FLOWVALIDATE_RESOURCE_EXHAUSTED`, `stage: runtime`, and
`disposition: no_artifact`; stdout remains empty. The focused
`flowvalidate_allocation_fault` test passed in both the normal GCC tree and
the Clang 18.1.3 ASan/UBSan tree.

This covers exhaustion at the validator process boundary. It does not claim
that every canonical parser allocation site has independent injection
coverage; that remains open Gate 8 work.

The safety state remains `CONTINUE`.

## Frankencore language and requirements allocation faults — 2026-09-15

Test-only fault-injected library variants now exercise allocation exhaustion
inside the shared language resolver and version-requirement APIs. Language
resolution returns an unresolved diagnostic; version validation and range
evaluation return invalid structured results. The focused
`frankencore_language_allocation_fault` and
`frankencore_requirements_allocation_fault` tests passed in both the normal
GCC tree and the Clang 18.1.3 ASan/UBSan tree.

This extends exhaustion evidence across the current parser-result boundaries.
It does not claim complete allocation-fault coverage for every Frankencore
provider or contract API; those remain open Gate 8 work.

The safety state remains `CONTINUE`.

## Frankencore package projection allocation fault — 2026-09-15

The read-only package inventory now has a test-only library variant that
injects `std::bad_alloc` inside `frankencore::packages::to_json_checked()`.
The public `noexcept` projection returns `valid: false`, an empty JSON string,
and the explicit `package inventory serialization exhausted memory` error.
`frankencore_packages_allocation_fault` passed in both the normal GCC tree
and the Clang 18.1.3 ASan/UBSan tree.

This closes allocation-exhaustion evidence for the package inventory’s
serialization boundary only. Provider discovery, parser allocation, and
other Frankencore APIs still require independent fault-injection evidence.

The safety state remains `CONTINUE`.

## Frankencore package reader boundaries — 2026-09-15

The package provider now exposes checked reader results for dpkg status,
APT-list metadata, APT-source configuration, and apt-indextargets discovery.
Provider allocation, standard, and unknown failures produce an invalid
`InventoryResult` and admit no partial inventory. The test-only package fault
library injects provider allocation exhaustion; the focused normal and
ASan/UBSan package tests pass. The existing readers remain compatibility APIs,
and their broader parser/provider fault matrix remains a future expansion.

The fault-injected evidence was tightened so the allocation failure is raised
inside the dpkg reader implementation itself, then translated by the checked
reader boundary. Normal, ASan/UBSan, and Valgrind focused runs remain green.

The safety state remains `CONTINUE`.

## Frankencore mutation validation boundary — 2026-09-15

Frankencore provenance now exposes `validate_checked(const MutationRecord&)`
as an explicit `noexcept` boundary. It preserves the existing validation
rules while translating allocation, standard, and unknown failures into an
invalid `ValidationResult`; the throwing-compatible `validate()` API remains
available for legacy callers. The provenance API test asserts the non-throwing
signature and covers both valid and invalid records.

The focused revision/provenance tests passed in the normal tree. This is a
direct validation boundary improvement, not a claim that every legacy C++
throwing helper has been removed or that allocation-fault coverage is
complete. The safety state remains `CONTINUE`.

## Flowoptimize allocation-fault boundary — 2026-09-15

Flowoptimize now has a test-only fault-injected executable that raises
`std::bad_alloc` before typed optimization consumes the input artifact. The
public process boundary returns `FLOWOPTIMIZE_RESOURCE_EXHAUSTED` with
`disposition: no_artifact` and leaves stdout empty. The focused
`flowoptimize_allocation_fault` test passed in both the normal GCC tree and
the Clang 18.1.3 ASan/UBSan tree.

This extends allocation-exhaustion evidence to the optimization boundary. It
does not claim complete allocation-fault coverage for all parser, provider,
or Frankencore paths; those remain open Gate 8 work.

The safety state remains `CONTINUE`.

## Flowanalyst allocation-fault boundary — 2026-09-15

Flowanalyst now has a test-only fault-injected executable that raises
`std::bad_alloc` before semantic projection. The public process boundary
returns `FLOWANALYST_RESOURCE_EXHAUSTED` with `disposition: no_artifact` and
does not publish stdout. The focused `flowanalyst_allocation_fault` test
passed in both the normal GCC tree and the Clang 18.1.3 ASan/UBSan tree.

This extends allocation-exhaustion evidence to the semantic-analysis
boundary. It does not claim complete allocation-fault coverage for every
parser, provider, or Frankencore API; those remain open Gate 8 work.

The safety state remains `CONTINUE`.

## Flowprepare and Flowtarget input classification — 2026-09-15

The two lowering companion tools now distinguish untrusted CLI and filesystem
input failures from parsed artifact-contract failures in structured mode.
`flowprepare` reports missing optimization artifacts and incomplete options as
`FLOWPREPARE_INPUT_INVALID` at stage `input`; `flowtarget` reports unavailable
policies and invalid policy names as `FLOWTARGET_INPUT_INVALID` at the same
stage. Both dispositions remain `no_artifact`, stdout remains empty, and JSON
contract failures retain the existing `*_CONTRACT_FAILURE` classification.

The diagnostics-mode pre-scan uses exact option/value comparisons before
normal option parsing, so an incomplete option that follows
`--diagnostics json` still receives the structured input result. The focused
GCC checks passed 2/2, and the same checks passed 2/2 in a fresh Clang 18.1.3
ASan/UBSan tree after all declared helper executables were built. The complete
canonical GCC suite passed 149/149 in 52.41 seconds; the complete fresh Clang
18.1.3 ASan/UBSan suite passed 149/149 in 102.55 seconds with
`detect_leaks=0` for the documented ptrace exclusion.

The initial focused sanitizer wrapper invocation found that only
`flowprepare` and `flowtarget` had been built while its declared `flowvalidate`
and `flowlower` helpers were absent. Building those declared dependencies made
the same focused tests pass; this was build-tree preparation, not a compiler
defect.

This closes stable input classification only for the tested Flowprepare and
Flowtarget process cases. Deeper parser/provider fault injection, complete
native API containment, isolation, and trust work remain open. The safety state
remains `CONTINUE`.

## Flowparallel planner failure classification — 2026-09-15

The runtime-provider and graph-provider planners now expose separate structured
contract and input results. Parsed plan, capability, calibration, and graph
violations produce `*_CONTRACT_FAILURE` at stage `contract`; missing or
oversized files and malformed policy or CLI options produce `*_INPUT_INVALID`
at stage `input`. Allocation exhaustion remains `*_RESOURCE_EXHAUSTED` at
stage `runtime`, and non-standard failures remain separately named. Every
tested failure leaves stdout empty and reports `no_artifact`.

Both planners now recognize the exact `--diagnostics json` pair inside their
top-level protected boundary with allocation-free comparisons. This removes
the prior diagnostic-mode `std::string` construction before `try`, where an
allocation failure could escape the process classification contract.

The focused exception-containment, runtime-planner, runtime-planner allocation,
graph-planner, and graph-planner allocation tests passed 5/5 under both GCC and
Clang 18.1.3 ASan/UBSan. The complete canonical GCC suite passed 149/149 in
54.78 seconds, and the complete Clang sanitizer suite passed 149/149 in 102.51
seconds with the documented leak setting.

This closes condition-specific process classification for these two planner
ingress boundaries only. The safety inventory remains provisional because
other provider and Frankencore APIs, broader fault injection, isolation, and
trust controls remain open. The safety state remains `CONTINUE`.

## Flowparallel CPU/CUDA provider failure classification — 2026-09-15

The CPU and CUDA selection providers now distinguish parsed execution-plan
violations from CLI and input failures. Malformed, duplicate-authority,
wrong-version, and wrong-type plan cases produce `FLOWPARALLEL_CPU_CONTRACT_FAILURE`
or `FLOWPARALLEL_CUDA_CONTRACT_FAILURE` at stage `contract`. Invalid numeric
options, missing plan files, and oversized input produce the corresponding
`*_INPUT_INVALID` result at stage `input`. Allocation exhaustion and unknown
non-standard failures remain distinct runtime outcomes; all tested failures
leave stdout empty with `no_artifact`.

Both providers now detect exact structured-diagnostics mode inside their
top-level protected boundary using allocation-free comparisons. This removes
the previous `std::string` allocation before `try` without changing CPU
selection, CUDA discovery, refusal, fallback, or execution policy.

The focused exception-containment, CPU provider/allocation, and CUDA
provider/allocation tests passed 5/5 under both GCC and Clang 18.1.3
ASan/UBSan. Complete suites passed 149/149 under GCC in 52.11 seconds and
149/149 under Clang sanitizers in 99.89 seconds with the documented leak
setting.

This closes condition-specific process classification for these two provider
ingress boundaries only. Broader native provider/API containment, fault
injection, isolation, and trust controls remain open. The safety state remains
`CONTINUE`.

## Flowparallel graph-provider validation order — 2026-09-15

The CPU reference and CUDA graph providers now classify malformed semantic
artifacts as `*_CONTRACT_FAILURE` at stage `contract`, bounded/CLI input
failures as `*_INPUT_INVALID` at stage `input`, and allocation exhaustion as a
runtime failure. The CUDA path separately reports discovery, execution, and
cleanup exceptions as `FLOWPARALLEL_GRAPH_CUDA_PROVIDER_FAILURE` at stage
`provider`. Structured failure stdout remains empty with `no_artifact`.

The CUDA graph provider now parses and validates the semantic report and graph
dimensions before opening CUDA Runtime or cuBLAS. This prevents missing or
failing host libraries from masking an invalid artifact. A valid compiler-chain
graph on this host reached provider discovery and returned the explicit
provider failure `cudaGetDeviceCount failed: 100`, with zero stdout bytes and a
`no_artifact` diagnostic; a malformed report is independently proven to fail
at the contract stage before discovery.

The focused exception-containment, graph-reference, graph-reference allocation,
and graph-CUDA allocation tests passed 4/4 under both GCC and Clang 18.1.3
ASan/UBSan. Complete suites passed 149/149 under GCC in 54.61 seconds and
149/149 under Clang sanitizers in 99.64 seconds with the documented leak
setting. The real-device four-shape CUDA firetest was not run in this restricted
host pass and is not claimed here.

This closes validation order and process classification for the tested graph
provider ingress only. Real-device execution assurance, broader provider/API
fault injection, isolation, and trust controls remain open. The safety state
remains `CONTINUE`.

## Flowparallel plan-ingress failure classification — 2026-09-15

The central plan-producing Flowparallel CLI now reports parsed semantic-report
violations as `FLOWPARALLEL_CONTRACT_FAILURE` at stage `contract`, bounded
input and invalid invocation as `FLOWPARALLEL_INPUT_INVALID` at stage `input`,
allocation exhaustion as `FLOWPARALLEL_RESOURCE_EXHAUSTED` at stage `runtime`,
and other runtime or non-standard failures separately. All structured failures
leave stdout empty with `no_artifact`.

The CLI now recognizes exact structured mode inside its protected boundary and
wraps bounded reader failures as input outcomes. The focused pipeline,
exception-containment, and allocation-fault tests passed 3/3 under both GCC and
Clang 18.1.3 ASan/UBSan. Complete suites passed 149/149 under GCC in 53.38
seconds and under Clang sanitizers in 103.14 seconds with the documented leak
setting.

This closes condition-specific process classification for the tested central
Flowparallel ingress only. Other native execution/provider APIs, broader fault
injection, isolation, and trust controls remain open. The safety state remains
`CONTINUE`.

## Flowparallel CUDA execution and calibration failure classification — 2026-09-15

The CUDA execution tool and matrix calibration benchmark now report invalid
options as `*_INPUT_INVALID` at stage `input`, CUDA discovery/execution/cleanup
failures as `*_PROVIDER_FAILURE` at stage `provider`, allocation exhaustion at
stage `runtime`, and unknown non-standard failures separately. Structured
failures leave stdout empty with `no_artifact`.

Both tools now recognize exact structured-diagnostics mode inside their
top-level protected boundary using allocation-free comparisons. The focused
exception-containment, matrix-diagnostic, matrix-allocation, and CUDA-execution
allocation tests passed 4/4 under both GCC and Clang 18.1.3 ASan/UBSan. Complete
suites passed 149/149 under GCC in 53.22 seconds and 149/149 under Clang
sanitizers in 102.21 seconds with the documented leak setting.

Valid host invocations reached CUDA provider discovery: CUDA execution returned
`FLOWPARALLEL_CUDA_EXECUTE_PROVIDER_FAILURE` for `cudaGetDeviceCount failed with
CUDA error 100`, and calibration returned
`FLOWPARALLEL_MATRIX_BENCHMARK_PROVIDER_FAILURE` for `cudaMalloc(A) failed:
100`; both had zero stdout bytes and `no_artifact`. This host evidence proves
failure containment, not successful real-device execution or calibration.

This closes condition-specific process classification for these two public
CUDA tools only. Real-device assurance, broader native provider/API fault
injection, isolation, and trust controls remain open. The safety state remains
`CONTINUE`.

## Flowmini startup allocation boundary — 2026-09-15

The canonical Flowmini frontend now recognizes an exact `--diagnostics json`
request with allocation-free comparisons inside its top-level protected
boundary before initializing policy storage. Its allocation-fault build now
injects exhaustion at that startup point, before policy setup or ordinary
argument-string construction, and proves an empty stdout plus
`FLOW_RESOURCE_EXHAUSTED`/`runtime`/`no_artifact` result.

The focused support, structured-diagnostic, UTF-8/source, and allocation-fault
tests passed 4/4 under both GCC and Clang 18.1.3 ASan/UBSan. Complete suites
passed 149/149 under GCC in 52.02 seconds and 149/149 under Clang sanitizers in
101.73 seconds with the documented leak setting.

This closes the identified Flowmini startup-allocation escape only. Wider
Stage 0 exception/API containment, fault injection, isolation, and trust work
remain open. The safety state remains `CONTINUE`.

## TinyVM lowerer process-failure classification — 2026-09-15

The portable TinyVM backend lowerer now accepts `--diagnostics json` and
separates malformed lowering contracts, invalid or unavailable input, output
publication failures, allocation exhaustion, runtime exceptions, and unknown
non-standard failures into stable `contract`, `input`, `output`, and `runtime`
stages. Structured failures leave stdout empty with `no_artifact`; successful
and explicitly unsupported lowering results remain on stdout.

The existing backend boundary now proves malformed JSON, missing and oversized
input, and output-open failure classification without creating a new output
artifact. The allocation-fault build proves a structured resource-exhaustion
result before input consumption. Both focused tests passed 2/2 under GCC and
Clang 18.1.3 ASan/UBSan. Complete suites passed 149/149 under GCC in 52.67
seconds and 149/149 under Clang sanitizers in 104.35 seconds with the documented
leak setting.

The underlying v2 writer still opens the destination directly. A write or
close failure can therefore leave a partial or truncated destination visible;
atomic artifact publication and injected write/close faults remain open. The
safety state remains `CONTINUE`.

## TinyVM v2 atomic artifact publication — 2026-09-15

The TinyVM v2 writer now completes validation and encoding before creating a
private sibling file. It writes, flushes, synchronizes, and closes that file,
then atomically renames it over the destination. Ordinary failure removes the
temporary file and does not replace an existing destination.

The new `tinyvm_artifact_v2_publication_fault` gate injects partial write,
`fsync`, close, and rename failures independently. Every case preserves the
prior destination byte-for-byte and leaves no sibling temporary file; the
success case publishes an independently readable artifact. The focused v2
artifact/lowering set passed 6/6 under GCC and Clang 18.1.3 ASan/UBSan.
Valgrind 3.22.0 reported zero errors, zero live blocks, and 4,969 allocations
matched by 4,969 frees. Complete suites passed 150/150 under GCC in 57.82
seconds and 150/150 under Clang sanitizers in 105.34 seconds with the documented
leak setting.

This proves ordinary Linux process-level atomic visibility, not persistence of
the rename across a host crash. Parent-directory synchronization, abrupt-death
orphan cleanup, v1 atomic publication, and cross-platform equivalents remain
open. The safety state remains `CONTINUE`.

## TinyVM v1 compatibility atomic artifact publication — 2026-09-15

The retained recovered-VM v1 writer now uses the same encode-first, private
sibling, write/flush/`fsync`/close, and atomic-rename publication sequence as
v2. The new v1 gate injects partial write, synchronization, close, and rename
failure independently; every case preserves the prior destination byte-for-byte
and removes the temporary file, while the success case publishes an
independently readable artifact.

The focused v1 allocation/publication tests passed 2/2 under GCC and Clang
18.1.3 ASan/UBSan. Valgrind 3.22.0 reported zero errors, zero live blocks, and
4,968 allocations matched by 4,968 frees. Complete suites passed 151/151 under
GCC in 58.71 seconds and 151/151 under Clang sanitizers in 106.64 seconds with
the documented leak setting.

This closes ordinary Linux atomic visibility for both retained binary artifact
formats. Parent-directory crash durability, abrupt-death orphan cleanup, and
cross-platform equivalents remain open. The safety state remains `CONTINUE`.

## Flowlower atomic LLVM publication — 2026-09-15

Flowlower now renders LLVM IR before creating a private sibling file, then
writes, flushes, synchronizes, and closes that file before atomically renaming
it over the requested destination. It recognizes exact structured mode before
ordinary option allocation and classifies publication failure as
`FLOWLOWER_OUTPUT_FAILURE` at stage `output` rather than as invalid input.

The new publication gate injects partial write, `fsync`, close, and rename
failure independently. Every case leaves stdout empty, reports `no_artifact`,
preserves the prior destination byte-for-byte, and removes the temporary file;
the normal case publishes executable LLVM IR. The focused backend, pipeline,
and publication tests passed 3/3 under GCC and Clang 18.1.3 ASan/UBSan. A
Valgrind 3.22.0 success-path run reported zero errors, zero live blocks, and 207
allocations matched by 207 frees. Complete suites passed 152/152 under GCC in
59.81 seconds and 152/152 under Clang sanitizers in 108.33 seconds with the
documented leak setting.

This proves ordinary Linux process-level LLVM artifact visibility, not
parent-directory crash durability or abrupt-death orphan cleanup. Flowmini
auxiliary file outputs and cross-platform equivalents remain separate. The
safety state remains `CONTINUE`.

## Flowmini atomic auxiliary artifact publication — 2026-09-15

Flowmini now renders FlowIR, AST-symbol, token-tree, and FlowIR-symbol file
outputs before creating a private sibling, then writes, flushes, synchronizes,
and closes the sibling before atomically renaming it over the destination.
Stdout dump modes remain streaming and unchanged. Structured publication
failure is `FLOW_OUTPUT_FAILURE` at stage `output`.

The new publication gate injects partial write, `fsync`, close, and rename
failure through the FlowIR path. Every case leaves stdout empty, reports
`no_artifact`, preserves the prior destination byte-for-byte, and removes the
temporary file; all named file-output modes use the same publisher. The focused
support, structured-diagnostic, UTF-8/source, and publication tests passed 4/4
under GCC and Clang 18.1.3 ASan/UBSan. Complete suites passed 153/153 under GCC
in 56.97 seconds and 153/153 under Clang sanitizers in 107.27 seconds with the
documented leak setting.

This proves ordinary Linux process-level publication behavior for the covered
frontend file artifacts. Parent-directory crash durability, abrupt-death orphan
cleanup, streamed stdout partial delivery, other native file producers, and
cross-platform equivalents remain open. The safety state remains `CONTINUE`.

## Trust and isolation execution admission — 2026-09-15

The public Frankencore contracts API now separates valid evidence from
execution authority. Direct execution is admitted only for evidence whose
validated policy outcome is `allowed`, which already requires trusted key
state, matched integrity, and supplier authentication or owner attestation.
Confirmation-required, quarantined, rejected, and unresolved outcomes refuse
execution.

An `allowed_with_isolation` outcome now fails closed when its claim is absent
or below isolated assurance. It also remains refused when a structurally valid
isolated or hardened claim says `independently_verified`, because no admitted
enforcement provider is wired to the decision point. Claim text therefore
cannot promote itself into execution authority.

The focused `frankencore_contracts_probe` passed 1/1 under GCC and Clang 18.1.3
ASan/UBSan. Valgrind 3.22.0 reported zero errors, zero live blocks, and 53
allocations matched by 53 frees. Complete suites passed 153/153 under GCC in
60.74 seconds and 153/153 under Clang sanitizers in 107.28 seconds with the
documented leak setting.

This is a contract-level refusal boundary, not an isolation implementation.
An independently verified Linux provider, provider-to-claim identity binding,
and adoption by execution consumers remain open. The safety state remains
`CONTINUE`.

## TinyVM v2 parent-directory durability result — 2026-09-15

The TinyVM v2 publisher now opens the destination's parent directory before
creating its private sibling, retains that descriptor through rename, and
`fsync`s the directory before reporting durable publication. Its new result API
separates pre-rename failure, durable publication, and publication whose
post-rename directory durability is uncertain. The compatibility boolean API
returns success only for the durable result.

An injected file-write, file-sync, file-close, or rename failure still
preserves the previous destination and removes the sibling. An injected
directory-sync failure instead proves that the replacement is visible and
independently validates while the API reports
`TINYVM_ARTIFACT_WRITE_DURABILITY_UNCERTAIN`. `flowtinylower --diagnostics
json` maps that state to `FLOWTINYLOWER_OUTPUT_DURABILITY_UNCERTAIN` with
`artifact_published_durability_uncertain`, never `no_artifact`.

The focused artifact, lowerer, and process-projection gates passed 3/3 under
GCC and Clang 18.1.3 ASan/UBSan. Valgrind 3.22.0 reported zero errors, zero live
blocks, and 4,993 allocations matched by 4,993 frees. Complete suites passed
154/154 under GCC in 57.51 seconds and 154/154 under Clang sanitizers in 110.24
seconds with the documented leak setting.

This closes the ordinary Linux parent-directory barrier and uncertain-result
classification for TinyVM v2 only. The retained v1 writer, Flowmini,
Flowlower, adversarial directory replacement, abrupt-death orphan cleanup, and
cross-platform durability remain open. The safety state remains `CONTINUE`.

## TinyVM v1 parent-directory durability result — 2026-09-15

The retained recovered-VM v1 compatibility writer now uses the shared
three-state artifact-write vocabulary and the same parent-directory lifecycle
as v2: open before sibling creation, retain through rename, synchronize after
rename, and report success through the compatibility boolean API only after
the directory barrier completes.

Its publication fault gate now distinguishes file synchronization from parent
directory synchronization. The injected directory barrier failure returns
`TINYVM_ARTIFACT_WRITE_DURABILITY_UNCERTAIN`; the replacement is already
visible, independently readable, and valid, while no private sibling remains.
Pre-rename write, file-sync, close, and rename faults retain the previous
destination.

The focused v1/v2 publication gates passed 2/2 under GCC and Clang 18.1.3
ASan/UBSan. Valgrind 3.22.0 reported zero errors, zero live blocks, and 4,991
allocations matched by 4,991 frees for the v1 gate. Complete suites passed
154/154 under GCC in 54.26 seconds and 154/154 under Clang sanitizers in 111.39
seconds with the documented leak setting.

This gives both TinyVM artifact formats the same ordinary Linux durability
result. Flowmini, Flowlower, adversarial directory replacement, abrupt-death
orphan cleanup, and cross-platform durability remain open. The safety state
remains `CONTINUE`.

## Flowlower parent-directory durability result — 2026-09-15

Flowlower now opens the LLVM destination's parent directory before creating its
private sibling, retains the descriptor through atomic rename, and
synchronizes the directory before reporting successful emission. Pre-rename
write, file-sync, close, and rename faults retain the previous destination and
continue to report `FLOWLOWER_OUTPUT_FAILURE` with `no_artifact`.

The publication fault gate now injects failure at the post-rename directory
barrier. The replacement LLVM IR is visible and contains the expected `main`
definition, stdout remains empty, and structured stderr reports
`FLOWLOWER_OUTPUT_DURABILITY_UNCERTAIN` with disposition
`artifact_published_durability_uncertain`. No private sibling remains.

The focused backend, pipeline, and publication gates passed 3/3 under GCC and
Clang 18.1.3 ASan/UBSan. A Valgrind 3.22.0 normal-publication run reported zero
errors, zero live blocks, and 208 allocations matched by 208 frees. Complete
suites passed 154/154 under GCC in 61.50 seconds and 154/154 under Clang
sanitizers in 113.26 seconds with the documented leak setting.

This closes the ordinary Linux parent-directory barrier and uncertainty
projection for LLVM artifacts. Flowmini, adversarial directory replacement,
abrupt-death orphan cleanup, and cross-platform durability remain open. The
safety state remains `CONTINUE`.

## Flowmini parent-directory durability result — 2026-09-15

Flowmini now opens the requested artifact's parent directory before creating
its private sibling, retains the descriptor through atomic rename, and
synchronizes the directory before reporting successful publication. This
applies uniformly to FlowIR, AST-symbol, token-tree, and FlowIR-symbol file
outputs. Pre-rename write, file-sync, file-close, and rename faults retain the
previous destination, remove the private sibling, and report
`FLOW_OUTPUT_FAILURE` with `no_artifact`.

The publication fault gate now independently injects post-rename directory
synchronization failure. The replacement FlowIR remains visible and starts
with its expected module declaration, stdout remains empty, no private sibling
remains, and structured stderr reports
`FLOW_OUTPUT_DURABILITY_UNCERTAIN` at stage `output` with disposition
`artifact_published_durability_uncertain`.

The focused frontend, support-boundary, UTF-8, and publication gates passed
4/4 under GCC and Clang 18.1.3 ASan/UBSan. A Valgrind 3.22.0 normal-publication
run reported zero errors, zero live blocks, and 684 allocations matched by 684
frees. Complete suites passed 154/154 under GCC in 59.76 seconds and 154/154
under Clang sanitizers in 114.03 seconds with the documented leak setting.

This gives every currently covered canonical compiler file-artifact publisher
the ordinary Linux parent-directory barrier and explicit uncertainty result.
Abrupt-death orphan cleanup, adversarial directory replacement, streamed
stdout partial delivery, other native producers, and cross-platform durability
remain open. The safety state remains `CONTINUE`.

## Compiler parent-directory close-failure evidence — 2026-09-15

The publication fault gates now exercise the post-rename parent-directory
descriptor-close branch for Flowmini, Flowlower, TinyVM v1, TinyVM v2, and the
TinyVM lowerer process projection. Each injected close failure occurs after a
successful directory synchronization, refuses success, leaves a visible valid
replacement with no private sibling, and reports the same explicit
published-but-durability-uncertain result as directory-sync failure. The
pre-rename no-artifact cases remain unchanged.

The five focused gates passed 5/5 under GCC and Clang 18.1.3 ASan/UBSan.
Valgrind 3.22.0 reported zero errors and zero live blocks for the expanded
TinyVM v1 artifact gate (5,009 allocations/frees), TinyVM v2 artifact gate
(5,012 allocations/frees), and process-level TinyVM lowerer close-failure path
(5,063 allocations/frees). Complete suites passed 154/154 under GCC in 62.39
seconds and 154/154 under Clang sanitizers in 114.47 seconds with the documented
leak setting.

This closes ordinary close-failure evidence for the covered Linux compiler
publishers; it does not close abrupt-death orphan cleanup, adversarial directory
replacement, streamed stdout partial delivery, other native producers, or
cross-platform durability. The safety state remains `CONTINUE`.

## Compiler abrupt-death publication recovery — 2026-09-15

Flowmini, Flowlower, and both TinyVM artifact writers now use the same bounded
Linux publication protocol. A writer opens and exclusively locks the parent
directory, removes only the exact versioned sibling owned by this protocol for
that destination, creates the sibling with `openat`, and publishes it with
`renameat` relative to the retained directory descriptor. The lock is released
by the kernel on process death. A subsequent cooperating publication can then
remove the one protocol-owned orphan without scanning, pattern deletion, PID
guessing, or touching another cooperating live writer.

Each publication gate now kills a writer from its file-write boundary with
exit status 86. Flowmini, Flowlower, TinyVM v1, TinyVM v2, and the TinyVM
lowerer process projection all preserve the previous destination, expose the
expected versioned private sibling after death, and remove it during the next
successful publication. The recovered artifacts independently validate and no
private sibling remains.

The five focused gates passed 5/5 under GCC and Clang 18.1.3 ASan/UBSan.
Complete suites passed 154/154 under GCC in 63.08 seconds and 154/154 under
Clang sanitizers in 115.43 seconds with the documented leak setting. Valgrind
3.22.0 reported zero errors and zero live blocks on the normal TinyVM v1 parent
(5,013 allocations/frees), TinyVM v2 parent (5,016 allocations/frees), TinyVM
lowerer (5,072 allocations/frees), Flowlower (208 allocations/frees), and
Flowmini (684 allocations/frees). The intentionally killed children terminate
before language/library teardown; Valgrind reports zero definite, indirect, or
possible loss there, while the kernel reclaims their still-reachable process
state.

This closes next-run cleanup only for private siblings created by the new
cooperating Linux protocol. Legacy randomized siblings, non-cooperating
writers, adversarial parent-path replacement, streamed stdout partial delivery,
other native producers, and cross-platform equivalents remain open. The safety
state remains `CONTINUE`.
