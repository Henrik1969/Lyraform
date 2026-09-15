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

## Runtime capability discovery boundary — 2026-09-15

Runtime capability discovery now exposes `discover_checked()` as an explicit
`noexcept` API. Allocation, standard, and unknown discovery failures become an
invalid result with no capability snapshot admitted; the reference probe then
serializes only a valid snapshot through its checked JSON boundary. Normal,
fault-injected, sanitizer, and memory-focused runtime tests pass. Broader
platform assurance remains future work. The safety state remains `CONTINUE`.

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

The safety state remains `CONTINUE`.

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
