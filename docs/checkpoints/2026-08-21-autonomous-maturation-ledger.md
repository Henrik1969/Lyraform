# Flowcore autonomous maturation ledger

## Mission completion reconciliation — 2026-09-07

Implementation and Firetest checkpoint `8c2b19d` is pushed and was verified clean.
Every definition-of-done item is mapped to its evidence in
[the final result](2026-09-07-reusable-flow-chain-result.md). The current presentation
index now points to that verified code checkpoint, records 81/81 and preserves
explicit experimental/platform limitations. This update is part of the owner's
authorized mission reconciliation, not an independent presentation-only refresh.

Final canonical rerun: `cmake --build /tmp/flowcore-reusable-current -j3` and
`ctest --test-dir /tmp/flowcore-reusable-current --output-on-failure -j3` passed
81/81 in 8.46 seconds. Reinstallation followed by
`sh /tmp/flowcore-reusable-acceptance/run-installed.sh` again exited 42 and
confirmed unchanged installed tool hashes and the recorded ELF digest.

All required implementation, tests, native proofs and documentation gates are
complete. State remains CONTINUE only through publication of this reconciliation;
a final state-only DONE checkpoint follows verification that it is pushed and
clean. No force push, merge, branch deletion, settings change or external PR
mutation was performed. The next action after DONE is outside this mission.

## Final Firetest hardening — 2026-09-07

Continued from pushed `bb3b3c7`. Fresh GCC and Clang builds exposed a real clean-
configure defect: the example project's older CMake policy discarded inherited
superbuild target expressions when creating cache defaults. `FlowcoreProject`
now initializes a standalone tool default only if no value is already defined.
Both fresh builds then passed after clearing the affected cache entries, proving
first-initialization behavior rather than relying on a warmed cache.

The historical categorized driver ran migrated native `return`, argv and ABI
programs in the compatibility interpreter (50 expected-native positives failed,
with zero Valgrind errors). This was the wrong execution boundary, not evidence
of native compiler failure. Added an explicit `--native-pass-boundary` mode:
91 positives use the existing canonical chain, while the 37 negative fixtures
and 16 support fragments retain their refusal expectations. The corrected
support-inclusive Valgrind run passed 53/53 with zero errors. A dedicated support
boundary is now in root CTest, raising the current count to 81.

Exact additional pressure commands (in addition to the preceding checkpoint):

```sh
cmake -S . -B /tmp/flowcore-final-gcc -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
cmake -S . -B /tmp/flowcore-final-clang -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
# After diagnosing and fixing first-cache initialization:
cmake -S . -B /tmp/flowcore-final-gcc -U 'FLOWCORE_FLOW*'
cmake -S . -B /tmp/flowcore-final-clang -U 'FLOWCORE_FLOW*'
cmake --build /tmp/flowcore-final-gcc -j3
ctest --test-dir /tmp/flowcore-final-gcc --output-on-failure -j3
cmake --build /tmp/flowcore-final-clang -j3
ctest --test-dir /tmp/flowcore-final-clang --output-on-failure -j3
cmake --build /tmp/flowcore-reusable-current-sanitize -j3
ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=detect_leaks=0 ctest --test-dir /tmp/flowcore-reusable-current-sanitize --output-on-failure -j3
FLOWCORE_ROOT="$PWD" FLOWMINI_BIN=/tmp/flowcore-reusable-current/flowmini/flowmini FLOWANALYST_BIN=/tmp/flowcore-reusable-current/flowanalyst/flowanalyst FLOWPARALLEL_BIN=/tmp/flowcore-reusable-current/flowtools/flowparallel/flowparallel FLOWOPTIMIZE_BIN=/tmp/flowcore-reusable-current/flowoptimize/flowoptimize FLOWBIND_BIN=/tmp/flowcore-reusable-current/flowbind/flowbind FLOWLOWER_BIN=/tmp/flowcore-reusable-current/flowlower/flowlower bash tools/run-flowmini-test-suite.sh --root Lyraform/flowmini_v25_symboltable_projection --build-dir /tmp/flowcore-reusable-current/flowmini --no-build --native-pass-boundary --run-support --valgrind --timeout 60 --report /tmp/flowcore-final-categorized
```

GCC 13.3.0: 81/81 (8.42 seconds). Clang 18.1.3: 81/81 (7.78 seconds).
GCC ASan/UBSan: 81/81 (26.09 seconds), retaining the documented environment's
`detect_leaks=0` exclusion; independent Valgrind 3.22.0 checks cover leaks.
Valgrind `--leak-check=full --errors-for-leak-kinds=definite,indirect,possible
--error-exitcode=99` ran the installed pager plus each of Flowmini, Flowanalyst,
Flowbind, Flowparallel, Flowoptimize and Flowlower on the retained pager artifacts:
all seven reports have zero errors and all heap blocks freed. Logs and inputs are
`/tmp/flow-final-*-valgrind.*` and `/tmp/flowcore-installed-pager`.

A Python ThreadPoolExecutor with eight workers ran 64 independent installed
Flowmini `--dump-frontend-bundle /tmp/flowcore-installed-pager/flow_less.flow`
processes. Every valid JSON capture had SHA-256
`73a8508f698e5a97c6a15b45712c89289fe5a8dc0bfe9a2a50695bf6bfb7a095`.
No new build warnings were reported. Markdown has no configured repository lint
tool here; changed local links and `git diff --check` are checked directly.
No in-repository PR title/body metadata file exists to reconcile; no PR was merged
or externally modified. Historical ledgers retain their checkpoint-specific counts.

State remains CONTINUE until this hardening checkpoint and the reconciled final
status are committed and pushed. No implementation gate or semantic blocker remains.

## Flow-owned native pager — 2026-09-07

Continued from pushed `51a4d0a`. The pager now owns key/command interpretation,
page transitions, bounds, page extraction, output text and error-code selection
in ordinary Flow functions. Its native source graph connects a selected input
startup capability to navigation and rendering receivers. Independently linked
application libraries provide immutable raw input batches, bounded ncurses input
and generic output transport. Compiler tools link none of these providers.
Removed all C++ pager node implementations and registrations after equivalent
native positives, negatives and source-variation tests passed.

Inline arrow placement had consumed an enclosing closing brace and silently
lost later function declarations. The AST builder now leaves that brace to its
block; the renamed/changed pager regression compiles an inline failure placement
followed by navigation, rendering and main, and checks changed source behavior.
Default graph-v1 refusal now uses its own historical fixture rather than relying
on the migrated application's old shape.

The pager driver generates deterministic, evidence-bearing contracts and exact
policies and verifies all six compiler tools and selected provider/runtime hashes
remain unchanged. Installed scripts, source, generator and libraries also support
compilation with only `FLOWCORE_PREFIX`; no repository or build-tree compiler is
needed. Both fake and terminal inputs use the same Flow source.

Verification commands:

```sh
cmake --build /tmp/flowcore-reusable-current -j4
ctest --test-dir /tmp/flowcore-reusable-current --output-on-failure -R 'flow_less|graph_lowering_refusal'
ctest --test-dir /tmp/flowcore-reusable-current --output-on-failure -j4
cmake --build /tmp/flowcore-reusable-current-sanitize -j4
ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=detect_leaks=0 ctest --test-dir /tmp/flowcore-reusable-current-sanitize --output-on-failure -j4
cmake --install /tmp/flowcore-reusable-current --prefix /tmp/flowcore-final-install
FLOWCORE_PREFIX=/tmp/flowcore-final-install /tmp/flowcore-final-install/share/flowcore/examples/flow_less/build-flow-less.sh /tmp/flowcore-installed-pager
/tmp/flowcore-installed-pager/flow_less
sh /tmp/flowcore-reusable-acceptance/run-installed.sh
git diff --check
```

Focused 3/3; final normal 80/80 (7.25 seconds), ASan/UBSan 80/80 (23.28
seconds). Pager checks cover navigation order/bounds, quit, empty input, invalid
page size, unknown command, changed source meaning with identical capabilities,
injected failing output, real pseudo-terminal input, missing files, and injected
terminal EOF with verified cleanup. Installed pager exits 0 with page 3/3 and
`epsilon`; ELF SHA-256 is
`6ae423e0a2f26a48bed8cc13a63ebf11275d5b99cfe6e85c8f029b79ff8e60e4`.
Installed acceptance `september_unregistered_consumer` uses newly generated
`gettid`, exits 42, preserves all six installed tool hashes, and retains ELF hash
`86ac3acba71f522aa13b5d58e733486737c1b4b9ffc19ed5224ab1c75470f400`.
Artifacts/logs are under `/tmp/flowcore-installed-pager`,
`/tmp/flowcore-reusable-acceptance`, and `/tmp/flow-pager-*`, outside Git.

Scope: Linux x86-64, scalar graph payloads, immutable bounded input batches, one
startup activation, final rendering after the terminal batch (default one key).
No streaming/redraw, persistent receiver state or aggregate graph contract is
claimed. Provider evidence checks binding-time bytes; it does not pin later loads
or recursively authorize dynamic dependencies. Existing writable-pointer and
single-import short-name compatibility remain explicitly documented, not new
language semantics. No pager-specific compiler dispatch remains.

State stays CONTINUE through this checkpoint. Next: reconcile the presentation
status and final mission gate inventory, verify the pushed clean checkpoint, then
record completion only after every definition-of-done requirement is satisfied.

## Native scalar source-graph activation — 2026-09-07

Continued from pushed `e8584a8`. Explicit `--graph-plan-version 2` with callable
plan v2 admits completely resolved scalar producer/receiver graphs. External
startup calls match captured callable provider identities and exact Flowbind
grants, including generated/live-provider evidence. Default graph v1 remains
non-executable; unknown providers, cycles, unsupported policies and mismatched
ports/types/identities are refused.

Flowparallel now publishes a separate deterministic FIFO graph schedule with
activation, wire, input/output signal and delivery identity. Flowoptimize and
backend preparation preserve and validate it against the source graph. LLVM
emits one fresh function invocation per delivery; fan-out reuses its result.
The installed shared `flowgraph_runtime` emits attributed output/drop traces and
structured arithmetic failures with no successful output. Its explicitly bindable
`flow_graph_raise(c_int)` capability also supports source-selected failure codes
with active operation and wire provenance. It requires an ordinary exact grant.
TinyVM explicitly refuses native graphs rather than silently projecting scalars.

The new native test generates a provider after the tools are built, checks all
six compiler hashes remain unchanged, and proves repeated receiver invocation,
fresh initialized locals, FIFO order, shared fan-out signals, distinct deliveries,
full endpoint/source identity, unconnected-output diagnostics, input selection,
unused-selection independence, renamed source behavior, arithmetic and explicit
source failure, and refusal of missing grants, changed provider identities,
forged schedules and cycles. A prior graph-refusal test exposed preparation
checking binding presence before version-1 graph refusal; that ordering is fixed.

Exact verification:

```sh
cmake --build /tmp/flowcore-reusable-current -j4
ctest --test-dir /tmp/flowcore-reusable-current -R 'native_source_graph|provider_call_identity|source_graph_artifact' --output-on-failure
ctest --test-dir /tmp/flowcore-reusable-current --output-on-failure -j4
cmake --build /tmp/flowcore-reusable-current-sanitize -j4
ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=detect_leaks=0 ctest --test-dir /tmp/flowcore-reusable-current-sanitize --output-on-failure -j4
sh /tmp/flowcore-reusable-acceptance/run.sh
git diff --check
```

Focused 3/3, complete normal 80/80 (5.84 seconds), ASan/UBSan 80/80
(17.99 seconds). Generated scalar acceptance remains exit 42 with unchanged
compiler hashes and the same ELF digest. A retained native graph ELF and captured
artifacts are `/tmp/native-graph*`; test/build logs are `/tmp/flow-native-graph-*`.
All generated files remain outside Git. Native graph scope is Linux x86-64,
scalar payloads, one startup output, no cycles or provider policies, and at most
65,536 statically scheduled activations. No streaming or aggregate receiver
contract is implied.

State remains CONTINUE. Next: move pager command interpretation, page bounds and
rendering into Flow functions using this native graph path and injectable I/O
providers; remove the C++ navigation implementation after equivalent gates pass.
The wider mission is not complete and no total blocker is recorded.

## Explicit startup-provider selection evidence — 2026-09-07

Continued from pushed `f2983af`. Added the non-authorizing
`flowcore.graph_provider_map` v1 selection artifact and Flowanalyst
`--graph-providers` input. Arbitrary graph implementation names resolve to one
qualified external source callable; no application/factory-name compiler table
selects the adapter. The bounded contract is a producer invoked once at startup,
zero arguments, one returned output on `out`. It follows the existing initial
producer activation and does not change the approved receiver contract.

Source graph evidence now retains selection, function identity, provider tuple,
carrier/effect/evidence facts and provenance. The standalone validator rejects
invalid selection versions, duplicate names, wrong activation/ports, invalid
producer identity, arguments, carrier mismatch and invented evidence. Semantic
checks diagnose producer-to-receiver port/type mismatches. Selecting a provider
still cannot authorize or execute a graph; all downstream refusals remain.

Verification commands:

```sh
cmake --build /tmp/flowcore-reusable-current -j4
ctest --test-dir /tmp/flowcore-reusable-current -R source_graph_artifact --output-on-failure
ctest --test-dir /tmp/flowcore-reusable-current --output-on-failure -j4
cmake --build /tmp/flowcore-reusable-current-sanitize -j4
ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=detect_leaks=0 ctest --test-dir /tmp/flowcore-reusable-current-sanitize --output-on-failure -j4
sh /tmp/flowcore-reusable-acceptance/run.sh
git diff --check
```

Focused 1/1, complete normal 79/79 and complete ASan/UBSan 79/79 pass; native
acceptance still exits 42 with unchanged compiler binaries. Logs remain under
`/tmp/flow-graph-provider-*`. The design note distinguishes selection evidence
from executable authority. State remains CONTINUE. Next: authorize the selected
startup calls, publish bounded serial graph activation scheduling, and lower
native receiver invocations without erasing ports, wires or fan-out signals.
No total blocker exists; Flow-owned paging remains unfinished.

## Typed callable results for receiver preparation — 2026-09-07

Continued from pushed `6b4ed54`. Native callable definitions previously hardcoded
all results to i32. LLVM and TinyVM now retain declared boolean, wide integer,
size and text result carriers. Return literals use the callable result contract.
Both consumers reject missing return paths and result-carrier mismatches; two
returning branch arms satisfy the result requirement. LLVM refuses escaping
writable pointer results. This is callable preparation, not graph admission.

The expanded captured-artifact test executes a wide value beyond 32 bits, a
wide literal fallback, both-arm boolean returns, text returned to an authorized
strlen operation, and repeated initialized local computation. Native execution
and both TinyVM engines return 42. It rejects missing results and mutated
boolean result contracts. The test exposed TinyVM's missing mixed integer
conversion and opaque comparison admission: integer operands now match existing
LLVM conversion behavior, while unsupported opaque binary operations fail at
compilation rather than at execution.

Exact verification:

```sh
cmake --build /tmp/flowcore-reusable-current -j4
ctest --test-dir /tmp/flowcore-reusable-current -R callable_lowering_boundary --output-on-failure
ctest --test-dir /tmp/flowcore-reusable-current --output-on-failure -j4
cmake --build /tmp/flowcore-reusable-current-sanitize -j4
ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=detect_leaks=0 ctest --test-dir /tmp/flowcore-reusable-current-sanitize --output-on-failure -j4
sh /tmp/flowcore-reusable-acceptance/run.sh
git diff --check
```

Focused 1/1, normal 79/79 (5.75 seconds), ASan/UBSan 79/79 (17.90 seconds).
Generated acceptance remains exit 42 with all six compiler hashes unchanged
through compilation. Logs are `/tmp/flow-callable-*`. State stays CONTINUE;
next is governed producer contracts and native graph activation. No new owner
decision or total blocker is recorded. The broader pager mission remains open.

## Loaded-provider evidence verification — 2026-09-07

Continued from pushed `fb43581`. Flowbind now hashes the actual loaded library
and the file owning each resolved symbol, comparing SHA-256 to the generated
evidence already authorized by the exact policy. A matching semantic/policy
identity with invented provider bytes is refused. A generated custom library
passes, then fails after replacement with a different implementation exporting
exactly the same symbol. Ready binding reports retain loaded path/hash evidence.
The Linux loader (`dlinfo`/`dladdr`) and OpenSSL Crypto supply this provider check.
The JSON serializer also escapes all capability and failure text correctly.

Verification:

```sh
cmake --build /tmp/flowcore-reusable-current -j4
ctest --test-dir /tmp/flowcore-reusable-current -R 'native_binding_generation|flowbind_provider' --output-on-failure
ctest --test-dir /tmp/flowcore-reusable-current --output-on-failure -j4
cmake --build /tmp/flowcore-reusable-current-sanitize -j4
ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=detect_leaks=0 ctest --test-dir /tmp/flowcore-reusable-current-sanitize --output-on-failure -j4
sh /tmp/flowcore-reusable-acceptance/run.sh
git diff --check
```

Focused 2/2, normal 79/79 (5.57 seconds), ASan/UBSan 79/79 (17.45 seconds).
Native acceptance exits 42 with all six unchanged compiler hashes and the same
recorded ELF digest. Logs are `/tmp/flow-provider-*`; no transient files enter Git.
Provider byte checks occur at binding time, not through a pinned native runtime
loader, and do not prove C prototypes independently of the explicit specification.
These limits remain documented rather than hidden behind a readiness claim.

State remains CONTINUE. Next: implement native graph provider/receiver contracts
and ordered activation through durable artifacts, then Flow-owned pager behavior.
The owner receiver decision remains approved; no total blocker is recorded.

## Generated evidence authorization identity — 2026-09-07

Recovered clean `ba61221`; focused recovery passed 3/3. Generated ABI blocks
now carry a quoted versioned evidence identity containing exact specification
and provider SHA-256 hashes. AST and symbol projection preserve it separately
from source import aliases. Semantic requirements, external operations, exact
policy grants and binding capabilities retain the identity. LLVM and backend
artifact preparation compare it; malformed versions/digests, removed evidence,
changed semantic identity and changed grants are refused. Historical handwritten
bindings retain explicitly empty evidence compatibility; their grants cannot
satisfy an evidence-bearing generated declaration. No prototype verification
claim is inferred from symbol discovery.

The generator no longer emits wall-clock timestamps, and all three generated
outputs compare byte-for-byte across repeated generation. The generated native
acceptance continues to use previously built binaries without source changes.

Exact commands:

```sh
cmake --build /tmp/flowcore-reusable-current -j4
ctest --test-dir /tmp/flowcore-reusable-current -R 'native_binding_generation|flowbind_provider|flowlower_pipeline' --output-on-failure
ctest --test-dir /tmp/flowcore-reusable-current --output-on-failure -j4
cmake --build /tmp/flowcore-reusable-current-sanitize -j4
ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=detect_leaks=0 ctest --test-dir /tmp/flowcore-reusable-current-sanitize --output-on-failure -j4
sh /tmp/flowcore-reusable-acceptance/run.sh
git diff --check
```

Focused 3/3, complete normal 79/79 and complete ASan/UBSan 79/79 pass.
Native acceptance exits 42, all six compiler hashes remain unchanged during
compilation, and ELF SHA-256 remains
`86ac3acba71f522aa13b5d58e733486737c1b4b9ffc19ed5224ab1c75470f400`.
Logs and generated artifacts remain under `/tmp/flow-evidence-*` and the existing
acceptance directory. No generated output enters Git.

State remains CONTINUE. Next: compare the actually loaded provider bytes against
this evidence at Flowbind, then continue native graph delivery and Flow-owned
paging. This checkpoint establishes identity propagation, not live-provider drift
verification, native graphs, or mission completion. No total blocker exists.

## Report-only lowering validation — 2026-09-07

Continued from pushed `05a08fc`. Flowlower now runs the shared optimization and
lowering-authority validators before reporting readiness, even without an LLVM
output request. Missing plans, blocked plan states, incompatible versions,
malformed operand arrays, duplicate operation IDs and invalid transformation
field types are refused. Contract failures publish structured JSON diagnostics
with the failing artifact path; other refusals also publish JSON. The common
JSON serializer now escapes report strings, including hostile control characters.

The focused test exposed two historical target fixtures containing incomplete
synthetic reports. They now derive from a complete captured optimization artifact.
No unsupported native execution is claimed by report-only boundary validation.

Exact verification:

```sh
cmake --build /tmp/flowcore-reusable-current -j4
ctest --test-dir /tmp/flowcore-reusable-current -R 'flowlower_pipeline|source_graph_artifact|provider_call_identity|flowvalidate_artifacts' --output-on-failure
ctest --test-dir /tmp/flowcore-reusable-current --output-on-failure -j4
cmake --build /tmp/flowcore-reusable-current-sanitize -j4
ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=detect_leaks=0 ctest --test-dir /tmp/flowcore-reusable-current-sanitize --output-on-failure -j4
sh /tmp/flowcore-reusable-acceptance/run.sh
git diff --check
```

Focused **4/4**, normal **79/79** (5.58 seconds), ASan/UBSan **79/79**
(16.80 seconds). Builds and whitespace checks passed. Generated native acceptance
still exits 42 with all six compiler hashes unchanged during compilation.
Logs: `/tmp/flowcore-lower-validation-{ctest,sanitize,acceptance}.log`.

State is CONTINUE. The exact next implementation is generated contract/evidence
identity propagation and enforcement (Gate 1), followed by executable durable
graph lowering and Flow-owned paging (Gate 6). Generated manifest hashes currently
remain outside the authorization tuple; do not repeat historical completion
claims. No total blocker or new owner decision is recorded.

## Resolved provider calls and native symbol separation — 2026-09-07

Continued from pushed `55b7c0b`. Inspection confirmed an actual Gate 1 correctness
gap: Flowanalyst selected external-call metadata by native leaf name rather than
resolved function identity. Renamed external functions were emitted as ordinary
calls; colliding provider names could select the first requirement; ordinary
source names could discover unused external capabilities. Provider facts are now
catalogued by resolved function symbol and binding requirements derive only from
actual calls to those symbols. The one-through-four namespace gate now asserts
exact external provider contracts and distinct carrier tuples.

LLVM ordinary function definitions/calls now use symbol-derived `flow.function.ID`
names, separate from provider native names. Identical native declarations for one
library/ABI are deduplicated after every exact contract is authorized. Conflicting
library/ABI identities for one native symbol fail explicitly before an LLVM file
is written. The native entry name is reserved. No application/profile dispatch
was introduced.

The `provider_call_identity` gate compiles two separately authorized renamed libc
bindings plus a source function named `abs`; native exit is **81**. It proves that
omitting the second effect grant fails, unused external declarations do not become
requirements from a source-name collision, and incompatible native providers do
not collapse by symbol. All artifacts are generated after building the tools.

Exact verification:

```sh
cmake --build /tmp/flowcore-reusable-current -j4
ctest --test-dir /tmp/flowcore-reusable-current -R 'provider_call_identity|callable_lowering_boundary|namespace_ambiguity' --output-on-failure
ctest --test-dir /tmp/flowcore-reusable-current --output-on-failure -j4
cmake --build /tmp/flowcore-reusable-current-sanitize -j4
ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=detect_leaks=0 ctest --test-dir /tmp/flowcore-reusable-current-sanitize --output-on-failure -j4
sh /tmp/flowcore-reusable-acceptance/run.sh
git diff --check
```

Focused **3/3**, normal **79/79** (5.59 seconds), ASan/UBSan **79/79**
(16.62 seconds). Both builds passed. Existing generated acceptance exits **42**
and all six compiler hashes remain unchanged during compilation. Logs are
`/tmp/flowcore-provider-identity-{ctest,sanitize,acceptance}.log`.

The audit also found that generated manifests record hashes but Flowbind does not
yet bind those versioned evidence identities into plan authorization; its report
still states provider-signature evidence is not provided. Historical claims of
complete Gate 1 evidence binding are therefore too broad. Next: carry generated
contract/evidence identities through semantic requirements, exact policy grants,
binding capabilities and lowering; then continue native graph delivery and
Flow-owned paging. State remains CONTINUE, with ordinary work and no total blocker.

## Durable graph evidence and downstream refusal — 2026-09-07

Continued from pushed `946b5c1`. Frontend capture now retains provider policy
literals (kind, text, node/key identity and provenance). Flowanalyst preserves
explicit graph nodes, wires, endpoint provenance, policies and receiver resolutions
inside `lowering_plan.source_graph`, using the new non-executable
`flowcore.source_graph` v1 evidence contract. A typed Flowcontracts reader validates
identities, ports, receiver shape, policy values and provenance; standalone captured
files round-trip canonically after the producer exits.

Adversarial status-laundering tests exposed Flowlower accepting graph evidence when
no LLVM output was requested. Both its driver and emitter now refuse the graph.
Flowbind, Flowparallel, Flowoptimize, Flowprepare and the backend-artifact validator
also refuse independently. No native graph admission or authorization is inferred
from syntax or from a provider name. Invalid graphs remain evidence, never an
executable scalar projection.

Exact verification:

```sh
cmake --build /tmp/flowcore-reusable-current -j4
ctest --test-dir /tmp/flowcore-reusable-current -R 'source_graph_artifact|graph_lowering_refusal|source_receiver_frames' --output-on-failure
ctest --test-dir /tmp/flowcore-reusable-current --output-on-failure -j4
cmake --build /tmp/flowcore-reusable-current-sanitize -j4
ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=detect_leaks=0 ctest --test-dir /tmp/flowcore-reusable-current-sanitize --output-on-failure -j4
sh /tmp/flowcore-reusable-acceptance/run.sh
git diff --check
```

Both builds passed. Focused **3/3**; normal **78/78**, 5.45 seconds;
ASan/UBSan **78/78**, 18.12 seconds. Native acceptance exited 42 and all six
compiler hashes remained unchanged during compilation. ELF SHA-256 remains
`86ac3acba71f522aa13b5d58e733486737c1b4b9ffc19ed5224ab1c75470f400`.
Logs: `/tmp/flowcore-graph-artifact-{ctest,sanitize}.log`.

State remains CONTINUE. During the next native graph investigation, inspection
found Flowanalyst selecting external-call metadata by leaf/native symbol instead
of its already resolved function identity. Investigate and repair that exact
provider-identity boundary first, then continue native graph execution and
Flow-owned paging. No semantic blocker is claimed.

## Fresh compatibility receiver frames — 2026-09-07

Recovered clean `4281338` on `v29-language-maturation`. Gate 6 remains the first
unfinished gate. Added distinct source-function references to the compatibility
runtime schema and isolated function-body projections. Every delivered scalar
input constructs a fresh local record and execution graph; one captured result
creates one outer output signal. Fan-out preserves that signal across distinct
deliveries. Internal runtime identities are scoped to their incoming delivery.
No function/application name selects a provider factory or compiler emitter.

The new `source_receiver_frames` gate covers repeated input, local mutation,
missing conditional results after a successful activation, two-way fan-out,
int/Bool/text carriers, failure with no output, forward function definitions,
wrong ports/types/roles, missing functions, cycles and refusal of lossy legacy
FlowIR export. Existing graph refusal remains in place for native compilation.
This interpreter slice retains its existing placement-based function semantics;
modern native `return`/`guard`/`when` and aggregate graph receivers remain work.
No Flow-owned paging or native graph completion is claimed.

Exact verification:

```sh
cmake --build /tmp/flowcore-reusable-current -j4
ctest --test-dir /tmp/flowcore-reusable-current -R 'source_receiver_frames|graph_lowering_refusal|flowcore_graph_routing' --output-on-failure
ctest --test-dir /tmp/flowcore-reusable-current --output-on-failure -j4
cmake --build /tmp/flowcore-reusable-current-sanitize -j4
ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=detect_leaks=0 ctest --test-dir /tmp/flowcore-reusable-current-sanitize --output-on-failure -j4
ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=detect_leaks=0 ctest --test-dir /tmp/flowcore-reusable-current-sanitize -R source_receiver_frames --output-on-failure
sh /tmp/flowcore-reusable-acceptance/run.sh
git diff --check
```

Both builds passed. Focused **3/3**, normal **77/77** in 5.30 seconds,
ASan/UBSan **77/77** in 17.07 seconds; expanded carrier/diagnostic assertions
also passed the focused normal and sanitizer test. Native acceptance exited 42,
all six unchanged compiler hashes passed, and ELF SHA-256 remains
`86ac3acba71f522aa13b5d58e733486737c1b4b9ffc19ed5224ab1c75470f400`.
Logs are `/tmp/flowcore-receiver-frames-{ctest,sanitize}.log`.

State stays CONTINUE. Next: preserve the complete graph and receiver identities
through independently validated stage artifacts, implement native activation,
and then move pager navigation to Flow. No new owner decision is required.

## Receiver connection and delivery identity checkpoint — 2026-09-07

Continued immediately after pushing `2327913`. Source receivers now require
explicit input/output direction, connected input and matching declared types
between receiver functions. Specific diagnostic assertions cover wrong ports,
missing input, invalid role, unresolved function, duplicate node/wire identity,
unknown endpoint, incompatible types and unsupported syntax version. A compatible
two-function graph has only the expected temporary execution-refusal diagnostics.

The compatibility runtime assigns each routed envelope a distinct delivery ID,
while fan-out retains the originating signal ID and distinct wire identities.
Runtime failure diagnostics include delivery identity. The graph gate verifies
one producer evaluation and one evaluation of each destination, plus no normal
output from a failing activation. This is runtime identity evidence, not yet
source-function execution or activation-local storage proof.

Exact verification:

```sh
cmake --build /tmp/flowcore-reusable-current -j4
ctest --test-dir /tmp/flowcore-reusable-current -R 'graph_lowering_refusal|flowcore_graph_routing' --output-on-failure
ctest --test-dir /tmp/flowcore-reusable-current --output-on-failure -j4
cmake --build /tmp/flowcore-reusable-current-sanitize -j4
ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=detect_leaks=0 \
  ctest --test-dir /tmp/flowcore-reusable-current-sanitize --output-on-failure -j4
sh /tmp/flowcore-reusable-acceptance/run.sh
git diff --check
```

Both builds passed. Focused **2/2**; complete normal **76/76** (5.23 seconds);
complete ASan/UBSan **76/76** (17.19 seconds). Native generated-binding acceptance
exited **42**, checked all six unchanged compiler hashes during compilation, and
retained Linux x86-64 ELF SHA-256
`86ac3acba71f522aa13b5d58e733486737c1b4b9ffc19ed5224ab1c75470f400`.
Logs are `/tmp/flowcore-receiver-contract-{ctest,sanitize}.log`; no generated build
or log output is committed. Source activation/native graph execution and Flow-owned
paging remain ordinary unfinished work. State remains **CONTINUE**, not BLOCKED
or DONE. Next: executable fresh receiver frames and durable graph lowering.

## Approved receiver recovery and syntax boundary — 2026-09-07

Recovered `8f77f84` on `v29-language-maturation`, synchronized with origin.
Incoming user changes were the owner activation decision in the mission and
`.codex-run-state = CONTINUE`; both are preserved in this checkpoint. The owner
has approved fresh state per delivered input, one function invocation, one
logical successful output, and runtime fan-out preserving signal identity.
The historical receiver blocker below is resolved and must not be asked again.

Gate 6 remains the first unfinished gate. Recovery CTest passed **76/76**.
This slice captures `flowmini.graph_syntax` v1 node references and full wire
endpoints with original source provenance. Explicit `node receiver : fn name`
references remain distinct from provider factories and function declarations.
Flowanalyst resolves receiver function/parameter identities, enforces the bounded
one-parameter/one-result definition shape, and publishes non-executable analysis.
It independently rejects graph execution even if frontend diagnostics are removed
from a captured bundle. Unknown graph versions, malformed arrays, duplicate
node/wire identities and unknown endpoints are diagnosed. Unsupported native
execution is retained; this is not a completed graph backend or Flow-owned pager.

Focused `graph_lowering_refusal` passes with renamed source, preserved endpoints,
receiver identity, malformed syntax and hostile captured-file coverage. Exact
complete verification commands (same Debug/sanitizer configurations as below):

```sh
cmake --build /tmp/flowcore-reusable-current -j4
ctest --test-dir /tmp/flowcore-reusable-current --output-on-failure -j4
cmake --build /tmp/flowcore-reusable-current-sanitize -j4
ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=detect_leaks=0 \
  ctest --test-dir /tmp/flowcore-reusable-current-sanitize --output-on-failure -j4
git diff --check
```

Both builds passed. Normal CTest: **76/76**, 5.44 seconds. ASan/UBSan:
**76/76**, 18.30 seconds. `git diff --check` passed. Logs remain outside Git in
`/tmp/flowcore-graph-syntax-{ctest,sanitize}.log`.

Exact next action: validate receiver port/type connections, then implement
fresh invocation frames and graph delivery through durable stage artifacts and
native lowering. Move pager navigation into ordinary Flow functions after that
path executes. State remains **CONTINUE**, with no semantic blocker and no
mission completion claim. Existing scalar and platform limitations are unchanged.

## Resumed recovery and negative-result repair — 2026-09-06

Recovered `71381d7` on `v29-language-maturation`, synchronized with origin.
The only incoming worktree change was `.codex-run-state = CONTINUE`.
Re-read the mission, backend/product plans and receiver decision, then verified
the actual source and builds rather than relying on the prior green report.

The first fresh normal and ASan/UBSan runs each passed 74/76. Both failures
were exposed by this session's inherited nice value of -3: the native
`getpriority` executable exits 253, while the tests expected either -3 or
attempted shell arithmetic on TinyVM's unsigned 18446744073709551613 result.
The native assertion now masks the expected status to eight bits. Provider
parity parses the JSON integer exactly with Python before masking; neither
shell overflow nor JSON floating-point rounding can alter the comparison.

A deterministic generated `labs` program returning the negated c_long result
then exposed a real backend defect: TinyVM negation constructed an i32 zero
from the contextual return type and subtracted an i64 operand, causing
`arithmetic carrier mismatch`. Negation now constructs zero with the operand
carrier, matching existing LLVM behavior. The retained regression checks
authorized native/TinyVM execution, exit 214 for -42, matching stdout and
missing/wrong-policy refusal. No compiler source/profile selector was added.

Focused verification passed 4/4: `flowlower_pipeline`,
`tinyvm_governed_provider_parity`, `tinyvm_scalar_backend_parity`, and
`tinyvm_backend_lowering_boundary`. Complete verification uses the same Debug
and ASan/UBSan build configurations documented below:

```sh
cmake --build /tmp/flowcore-reusable-current -j4
ctest --test-dir /tmp/flowcore-reusable-current --output-on-failure
cmake --build /tmp/flowcore-reusable-current-sanitize -j4
ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=detect_leaks=0 \
  ctest --test-dir /tmp/flowcore-reusable-current-sanitize --output-on-failure
sh /tmp/flowcore-reusable-acceptance/run.sh
git diff --check
```

The complete normal suite passed **76/76** in 20.11 seconds; the complete
ASan/UBSan suite passed **76/76** in 62.21 seconds. Both builds and
`git diff --check` passed. Native acceptance
again exited **42**, all six compiler hashes remained unchanged, and the
Linux x86-64 ELF SHA-256 remains
`86ac3acba71f522aa13b5d58e733486737c1b4b9ffc19ed5224ab1c75470f400`.
The acceptance sources/artifacts and test logs remain under `/tmp`.

Gate 6 remains the first unfinished mission gate. Source inspection reconfirms
that `NodeDecl` contains role/id/kind only, `buildCheckedGraph` requires an
AtomRegistry factory, and `PagerNavigateNode::run` owns navigation in C++.
The [receiver decision](../architecture/source-graph-activation-decision.md)
is still a proposal; this continuation requested the specific decision again.
The alternatives and public-semantic dependency recorded below still apply.
No native graph implementation or Flow-owned navigation is claimed.

All identified independent repairs are now verified. The receiver question
has no answer in this resumed session, so the mission state returns to
**BLOCKED**, not DONE, under the task's material-public-language-choice rule.
The smallest required input remains acceptance of the proposed fresh-state,
one-input/one-function/one-output activation contract, or a replacement
node/plug contract. This checkpoint preserves the repairs and evidence on the
authorized development branch; the previously incoming CONTINUE state was
honored during recovery and safe work. No unrelated changes were present.

## Recovery audit — 2026-09-06

This section supersedes the historical completion claim below. Recovered HEAD
was `7182fbf` on `v29-language-maturation`; the worktree already changed AGENTS.md
to this mission and run state to CONTINUE. Those user changes were preserved.
No compiler application/profile selectors were found in the five stage source
directories. The first substantive unfinished objective is Gate 6, with a
related silent-projection correctness gap in Gate 1.

Confirmed fixes in this checkpoint:

- Clean out-of-tree testing initially passed 70/75. Five tests selected absent
  sibling-build binaries. CMake now supplies current target paths to generated
  binding acceptance, conformance, parallel smoke/reference and the flowcat
  example pipeline. All five focused tests passed after repair.
- ASan exposed the smoke test's 1 GiB virtual-memory limit preventing its shadow
  map from being reserved. Address-sanitized builds disable that address-space
  cap while retaining bounded input and the 30-second timeout. Normal builds
  retain the cap. No sanitizer error is suppressed.
- The compiler-chain acceptance test records and checks hashes of all six tools
  across generated binding and native execution. Source/provider selection does
  not replace compiler binaries.
- `flow_less` actually selects C++ `PagerNavigateNode`; page commands, state and
  extraction are not implemented in Flow. Before the repair, frontend export
  dropped its graph and Flowanalyst admitted only `marker : int(1)` as a ready
  native plan. Export now diagnoses graph keywords/connection tokens through
  lexer facts with original source provenance. Flowanalyst propagates the
  refusal and Flowparallel cannot schedule the blocked plan. Strings and
  comments containing graph words do not trigger it. Interpreter graph tests
  remain green. This is explicit unsupported behavior, not graph lowering.
- The new `graph_lowering_refusal` gate tests captured bundles independently,
  renamed source/program identity, downstream refusal and literal/comment
  independence. README and architecture notes now distinguish native scalar
  lowering from compatibility-interpreter graphs.

Exact verification commands (logs and build outputs remain outside Git):

```sh
cmake -S . -B /tmp/flowcore-reusable-current -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/flowcore-reusable-current -j4
ctest --test-dir /tmp/flowcore-reusable-current --output-on-failure
cmake -S . -B /tmp/flowcore-reusable-current-sanitize -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  '-DCMAKE_C_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer' \
  '-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer'
cmake --build /tmp/flowcore-reusable-current-sanitize -j4
ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=detect_leaks=0 \
  ctest --test-dir /tmp/flowcore-reusable-current-sanitize --output-on-failure
git diff --check
```

Both complete builds succeeded. Normal CTest: **76/76**, 22.32 seconds.
ASan/UBSan CTest: **76/76**, 61.27 seconds. Leak detection retains the earlier
external-provider environment exclusion. Coverage includes malformed binding
input, native execution, namespace collisions, terminal read/error/EOF paths,
resource cleanup and graph interpretation. The pass corpus now contains 91
programs; the pipeline matrix has seven accepted, two semantic-only and one
blocked fixture.

Additional retained native proof: `sh /tmp/flowcore-reusable-acceptance/run.sh`
generates a new `september_capability` binding for libc `gettid`, compiles
`september_unregistered_consumer` with already-built tools, and checks its
positive-result branch exits **42**. All six compiler SHA-256 checks pass. The
result `/tmp/flowcore-reusable-acceptance/september` is a Linux x86-64 ELF PIE,
SHA-256 `86ac3acba71f522aa13b5d58e733486737c1b4b9ffc19ed5224ab1c75470f400`.
Source, generated provider evidence, policy, captured stage artifacts, LLVM,
binary and reproduction script are retained there, not committed. The permanent
`native_binding_generation` gate covers the same generated-binding route.

Gate 6 cannot be claimed complete: the current NodeDecl/AtomRegistry boundary
has no source function identity or payload/parameter/result mapping, and callable
v2 specifies sequential function calls only. The concrete proposed next step
and alternatives are in
[the source receiver decision](../architecture/source-graph-activation-decision.md).
The user was asked whether to admit one stateless Flow function activation per
delivered input, or require the broader node/plug design. No answer has yet
been received. At this checkpoint state remains CONTINUE while the verified
changes are committed and pushed.

## Blocked continuation — 2026-09-06

Verified checkpoint `3d15f72` is committed and pushed to
`origin/v29-language-maturation`. The worktree was clean after that push; this
state-only checkpoint records the remaining semantic dependency. Run state is
**BLOCKED**, not DONE. The historical mission-complete claim is withdrawn.

Exact blocker: source-defined graph receivers have no admitted activation
contract. The current node schema names built-in factories only; ordinary
callable v2 does not define when a delivered port activates a Flow function,
how its result becomes an output signal, or whether receiver state persists.
Selecting those rules materially changes public language/architecture. The
mission's immediate steering override requires stopping for such a choice.

Attempted alternatives and evidence are in
[the decision note](../architecture/source-graph-activation-decision.md):
renaming or relocating C++ navigation fails Flow ownership; sequential main
calls erase graph delivery; existing callable v2 lacks port activation; a full
stateful/join design introduces additional unapproved semantics. The safe
independent work is complete: clean-build repair, sanitizer repair, unsupported
projection refusal, adversarial tests, preserved native acceptance, and honest
documentation. No failing test, missing credential, workload estimate or context
limit is being used as a blocker.

Smallest required decision from Henrik: admit the proposed stateless receiver
contract (one delivered input invokes one explicitly identified Flow function;
one return emits on an explicit output; fresh local state; attributed failure;
scheduling remains separate), or require a broader node/plug contract first.
The pending question has not received an answer. After that decision, restore
CONTINUE and implement Gate 6 through the durable compiler boundaries before
removing graph refusal or claiming Flow-owned paging. The final mission gates
and native compiled graph demonstration remain open.

Preserved verification: 76/76 normal and 76/76 ASan/UBSan, native acceptance ELF
exit 42 with unchanged compiler hashes, and no build/log artifacts committed.
Platform scope remains Linux x86-64/libc/LLVM with ncurses and a pseudo-terminal;
positive c_pointer storage and single-provider import aliases remain documented
compatibility behavior. No PR merge, force push or published-history rewrite
was performed.

## Historical implementation record

- Baseline audited: current v25 imports already preserve explicit aliases such
  as `curses`, `libc`, and `linux` in AST/SymbolTable projections; the existing
  compatibility path for unaliased legacy imports remains to be tightened.
- Generated binding policies now carry exact declared parameter and return
  carriers. Flowbind accepts the shorter four-field policy as an explicit
  legacy wildcard and binds generated grants exactly.
- Added a hostile generated-signature-policy test; it is rejected.
- Flowbind now parses semantic reports and ABI manifests as structured JSON,
  rejects duplicate JSON keys, and compares aggregate size, alignment, field
  order, names, types, and offsets against semantic facts.
- Runtime graph connections now preserve destination ports and stable per-build
  wire identities in envelope routing metadata.
- Added deterministic graph-routing coverage for destination-port delivery and
  wire identity traces.
- Added the first `flow_less` application slice. `pager.fake` is a deterministic
  provider with policy-controlled page size and navigation; `pager.render` is a
  provider-neutral plain projection. Invalid page sizes fail closed.
- Added `pager.ncurses` as a separate dynamically loaded provider using the same
  page-record contract. Its pseudo-terminal test verifies `q` handling and the
  final plain projection without linking Flowmini directly to ncurses.
- Flowanalyst now exports `abi_type_contracts`, preserving provider-declared ABI
  carrier representation, ownership, access, lifetime, nullability, and
  opacity facts. The ncurses pipeline asserts the typed opaque-window carrier.
- `flow_less` now reads a real temporary text file through `pager.file.ncurses`,
  renders its first page in a pseudo-terminal, and rejects a missing path.
- Flowanalyst now emits an additive `flowcore.lowering_plan` v1 containing
  report-local call operations and exact provider/signature facts.
- Flowbind validates every external operation in that plan against semantic
  requirements before authorizing the binding.
- Flowparallel and Flowoptimize preserve the structured lowering plan.
- Added the first profile-free native proof: `profile_free_getpid` uses an
  arbitrary program name and generic zero-argument `c_int` lowering to emit and
  execute an ELF binary without a matching source-name profile.
- Lowering-plan operations now export typed operand descriptors for integer
  literals and identifiers. A second arbitrary profile-free proof generates a
  `getpgid(c_int)` binding, preserves its operand through Flowparallel and
  Flowoptimize, and executes the resulting ELF without a compiler change.
- Flowparallel and Flowoptimize no longer enumerate known profile names to
  preserve a report's lowering profile; they mechanically carry the declared
  value and structured lowering plan forward.
- Migrated the existing `getpid` native example to the generic lowering path:
  its source name is now arbitrary, the analyst emits no special profile, and
  the dedicated getpid lowerer dispatch was removed. Existing corpus and native
  execution checks still pass.
- Added a namespace adversarial gate with three independent providers exposing
  the same function name. The unqualified call is rejected with provenance-aware
  ambiguity diagnostics, while three explicit aliases compile to three distinct
  qualified operations.
- Added generic profile-free `return_value` lowering for typed integer literals.
  The plan carries the value through both intermediate stages and Flowlower
  emits an executable ELF whose exit status is 42.
- Extended generic return lowering to nested integer binary expressions. A
  profile-free `40 + 2` program now produces an LLVM `add` and executes with the
  expected result.
- Added generic local value flow: `let` initializers become
  `value_definition` operations with symbol identity, and return expressions
  can consume those values through the plan. The profile-free local-value ELF
  test executes successfully.
- Added generic boolean branch operations with explicit then/else block IDs.
  A profile-free conditional-return program now lowers to LLVM branches and
  executes the selected return path.
- Extended branch conditions to source-derived integer comparisons. Local SSA
  values can now feed an `icmp` predicate in a profile-free conditional ELF.
- External `c_int` call results now retain their result-symbol identity and can
  feed later generic return operations. LLVM emission requires a ready binding
  report containing the authorized symbol; missing and mismatched reports are
  rejected.
- Migrated the getuid native example to this result-placement path. Its program
  name is arbitrary, its lowering profile is `none`, and the handwritten
  getuid profile branches were removed from analysis, binding, and lowering.
- Generic external results can now feed subsequent integer expressions. The
  arbitrary getppid acceptance program emits a call, adds one to its result,
  and returns the derived SSA value.
- Migrated the existing getppid native example to the generic result path and
  removed its source-name dispatch from all three compiler stages.
- External result symbols can now drive generic comparison branches. An
  arbitrary getppid program emits the authorized call and `icmp`, takes the
  source-defined true branch, and exits 42.
- Migrated the remaining zero-argument `c_int` identity examples (`getgid`,
  `geteuid`, `getegid`, and `getpgrp`) to the generic plan and removed their
  analyst, binder, and LLVM profile branches. Their previously implicit result
  behavior is now expressed by explicit Flow return statements.
- Migrated the one-argument `getpgid` and `getsid` examples to the same generic
  path. Generic external operands can now consume initialized local symbols,
  with initializer, call, and result-dependent expression instructions emitted
  in dependency order.
- Generic `c_int` result calls now accept an arbitrary non-empty sequence of
  `c_int` literal or initialized-local operands, derive the LLVM declaration
  and call argument lists from the exact plan signature, and reject mismatched
  carrier lists. `getpriority(c_int,c_int)` is migrated to this path with its
  native return behavior expressed in Flow source.
- The generated `gettid` acceptance fixture now retains `lowering_profile:
  none`; its binding is generated at test time, exactly authorized, lowered by
  the generic zero-argument `c_int` machinery, linked, and executed without a
  compiler profile branch.
- Added generic profile-free `c_long` external-call lowering with source-derived
  `c_int` arguments, exact binding authorization, positive-result validation,
  and structured refusal outside the supported carrier shape. The generated
  `sysconf` fixture now uses that path and has no source-name profile branches.
- Added profile-free `c_ulong` argument/result lowering with carrier-derived
  64-bit local initialization and unsigned result validation. The generated
  `getauxval` fixture now compiles and executes without source-name dispatch.
- Replaced the generated system-information profile with generic ordered scalar
  capability-sequence lowering. An arbitrarily named program now carries five
  zero-argument `c_int`/`c_long` calls through the plan, requires authorization
  for every native symbol, and emits and executes without a handwritten LLVM
  block or source-name selection.
- Added generic unary integer operand propagation and migrated the libc `abs`
  example off its source-name profile. Its `-42` initializer, result placement,
  and explicit return now drive the emitted LLVM and native exit status 42.
- Added generic ASCII string-literal storage, `c_string` argument flow, and
  `c_size_t` result-to-return lowering. The libc `strlen` example now returns 8
  from explicit Flow source with no source-name selection or handwritten LLVM.
- Added generic nullable `c_string` result flow and branch-local string-call
  lowering. The generated getlogin/puts proof now has an arbitrary program
  name, expresses its null fallback as a Flow `if`, retains
  `lowering_profile: none`, and has no analyst, binder, or LLVM profile branch.
- Generalized ordered capability sequences across `c_string`, `c_int`,
  `c_long`, `c_ulong`, `c_size_t`, and pointer LLVM carriers. The libc
  integration fixture now emits its source-defined `strlen`, `abs`, and `puts`
  sequence generically and no longer has compiler profile dispatch.
- Admitted single mixed-carrier calls on the same generic path and migrated
  `rmdir(c_string)`. Its emitted call now uses the Flow source literal instead
  of the legacy handwritten emitter's null pointer.
- Migrated the remaining scalar-only kernel fixtures (`fork`, `socket`,
  `listen`, and `unshare`) to generic single-operation lowering. Their exact
  `c_int` operands now come from Flow initializers and their source-name profile
  branches are removed.
- Migrated `sethostname` and all remaining explicit-null `c_pointer` kernel
  fixtures to generic mixed-carrier lowering. `c_pointer(0)` is represented as
  LLVM `null`; nonzero integer-to-pointer conversion remains unsupported rather
  than guessed. The corresponding source-name profile table and handwritten
  emitters are removed.
- Migrated `openat`, `lseek`, and `unlinkat` to generic source-derived calls and
  removed their analyst, binder, and LLVM profiles. A trial migration of all
  remaining kernel profiles correctly exposed that `clock_gettime` cannot write
  through the source-declared null pointer; the five buffer-writing profiles
  were retained pending explicit storage semantics rather than hiding an
  application-specific allocation in generic lowering.
- Migrated the safe explicit-null probes `getrandom`, invalid-descriptor `read`,
  and invalid-descriptor `write` to generic lowering. Only `clock_gettime` and
  `uname` remain among the kernel compatibility profiles because successful
  execution requires sized writable storage absent from their current Flow
  declarations.
- Added source-derived writable-storage descriptors for positive `c_pointer(N)`
  initializers. The lowering plan records exact byte count, read/write access,
  and call lifetime; Flowbind rejects malformed or zero-sized descriptors.
- Migrated `clock_gettime` and `uname` to generic mixed-carrier lowering using
  explicit 16-byte and 390-byte source allocations. Their analyst, binder, and
  handwritten LLVM profile branches are removed, and both native ELFs execute.
- Replaced the imported short-name toggle with a permanent ambiguity set. One
  provider retains the diagnosed compatibility alias; two, three, and four
  colliding providers remain ambiguous, while four qualified calls remain
  stable and distinct.
- Repaired the transitional `sel` native emitter's input handling. Flow source
  declares 4096 bytes of writable storage, the read reserves one byte for the
  terminator, and LLVM distinguishes positive reads, EOF, and errors before
  using the result as an offset. The error path restores ncurses and exits 2.
- Added ABI type cleanup capability facts and exact provider contract identity
  to lowering-plan operations. Ncurses pointer results now retain external
  ownership, opaque access, external lifetime, nullability, and `endwin`
  cleanup identity through the middle stages.
- Flowbind rejects mutated resource facts and resource acquisition without the
  declared cleanup operation. The standalone ncurses example now lowers as a
  generic ordered mixed-carrier sequence; its source-name profile and
  handwritten LLVM emitter are removed.
- Removed the empty-program lowering profile. A valid version-1 generic plan
  with zero operations now emits a minimal native `main`, including independent
  explicitly selected target artifacts.
- Added stable runtime signal identities distinct from wire identities. Each
  output activation creates one signal, fan-out deliveries retain it across
  distinct wires, and traces preserve full source/destination port identity.
- Expanded graph coverage for deterministic fan-out order, multiple destination
  ports, unconnected-output diagnostics, and invalid-port contract rejection.
- Removed the `sel` source-unit/profile selector from Flowanalyst and Flowlower.
  Its current terminal slice is now selected from a profile-free structured
  plan containing the required external operations, and every operation is
  checked against the ready binding report. An arbitrarily renamed copy follows
  the same path, while a binding with a mutated `wgetch` identity is rejected.
- Added a generic `length(list<string>)` lowering operand for parameterized
  entry points. A profile-free arbitrary program now branches on its actual
  native argument count and produces distinct tested exit statuses with and
  without an application argument.
- Added generic checked `list<string>` indexing for parameterized entry points.
  The plan retains the source parameter and index expression; native lowering
  guards argc before loading argv. An arbitrary profile-free program prints its
  source-selected argument, while the missing-argument path exits 64 without
  dereferencing argv out of bounds.
- Made the transitional profile-free `sel` backend contingent on source-derived
  entry-argument and key-selection branches. The plan must contain an argv
  length definition, checked argv indexing, a branch consuming the length
  symbol, and an equality branch for the quit key; the capability set alone is
  no longer sufficient to select the terminal emitter.
- Removed the final flowcat application/source-name profiles. Flow source now
  declares argv traversal, bounded writable storage, open/read/write/close
  calls, error branches, index mutation, and returns. Flowanalyst publishes
  generic `loop` and `assignment` operations and resolves placement results
  through parent scopes; Flowlower selects the transitional file-copy emitter
  from those structured facts and exact authorized capabilities.
- Added explicit ABI carrier conversion operations for typed initializers.
  Flowcat now converts the `c_long` read result to its declared `c_size_t`
  write count in the plan, and Flowbind verifies every external operand count
  and carrier type against the exact provider signature.
- Added reusable profile-free integer loop and mutation lowering. A previously
  unknown program initializes local `c_int` values, evaluates its structured
  loop comparison on every iteration, applies a source assignment through
  mutable storage, and returns the final value without any source-name or
  capability selector.
- Removed the last source-unit exception from Flowanalyst capability discovery
  and the obsolete `sel_main`/`abi_kernel_getpid_main` branches from Flowbind.
  Requirements are now derived only from actual calls, binding reports describe
  the versioned plan as generic, and Flowlower public wording no longer calls
  accepted plans profiles.
- Removed the transitional terminal capability-set recognizer and fixed LLVM
  emitter. Flowlower now parses the optimization plan and binding report as
  typed JSON, authorizes every external operation by its exact provider and ABI
  tuple, and emits values, calls, nested branches, conversions, cleanup, and
  returns in source statement/block order.
- Made `sel` read behavior source-derived. Flow source explicitly initializes
  its compatibility buffer through authorized `memset`, branches on negative,
  zero, and positive read results, performs `endwin` cleanup before returning 2
  on failure, writes only a positive byte count, and returns the selected or
  cancelled status from its key branch.
- Added adversarial terminal proofs for different behavior under the same
  capability set, changed source operation order, an unused policy grant, and
  renamed source/program identity. No compiler path is selected by those facts.
- Documented the temporary `c_pointer(N)` writable-allocation interpretation
  and the explicit public-language choice among bounded buffer, storage
  declaration, and allocation-operation designs.
- Extended typed structured lowering with nested loops, assignments, dynamic
  argv indices, integer promotion/conversion, loop-carried external results,
  cleanup branches, early returns, and reachability validation. Removing a
  controlling loop now rejects the plan instead of silently dropping child
  blocks.
- Removed the transitional file-copy capability recognizer and handwritten LLVM
  emitter. `flowcat` now uses only generic typed-plan machinery and an explicit
  Flow `sendfile` loop; short transfers advance the kernel-managed input offset
  and continue without invented pointer arithmetic.
- Added `sendfile` to the provider contract and exact policy boundary without
  adding any compiler dispatch. The arbitrary renamed copy and a two-megabyte
  multi-iteration native transfer pass with the already-built toolchain.
- Added typed call-site effect contracts to every external lowering operation.
  They retain the declared external effect, declared certainty, determinism,
  and one exact argument-resource record per ABI parameter with memory effect,
  ownership, access, lifetime, nullability, and opacity.
- Flowbind now verifies those effect and argument-resource facts against the
  provider declaration and exported ABI type contract. Adversarial mutations
  of `sendfile` determinism and pointer memory access are rejected.
- Replaced ncurses' generic `c_pointer` handle with a distinct
  `ncurses_window` ABI carrier. Its external ownership, opaque access, external
  lifetime, nullability, and `endwin` cleanup identity now remain distinct from
  ordinary pointer authority at acquisition and every window-consuming call.
- Flowparallel and Flowoptimize now preserve ABI type contracts alongside the
  lowering plan. Flowbind and typed Flowlower accept provider-declared pointer
  carriers from their exact `repr` contract instead of adding the ncurses type
  name to compiler dispatch. Hostile lifetime and cleanup mutations are
  rejected, and both the standalone ncurses ELF and `sel` remain executable.
- Flowbind now validates acquired-resource cleanup path-sensitively across
  structured branches. Every reachable exit after `initscr` must execute
  exactly one contract-matched `endwin`; cleanup before acquisition, repeated
  acquisition, double cleanup, and resource actions in loops without an
  explicit lifetime proof are rejected. Adversarial `sel` plans cover an early
  error exit with missing cleanup and an ordinary path with duplicate cleanup.
- Removed Flowlower's 750-line substring-based compatibility parser and all
  fallback LLVM emitters. A typed JSON driver now validates optimization report
  identity, version, status and target selection, while the structured-plan
  emitter handles empty plans, scalar calls, nullable pointers, values,
  branches, checked argv indexing, loops, assignments, cleanup and returns.
  Missing loop bodies and controlling blocks are rejected instead of silently
  emitting altered behavior.
- Removed the transitional `lowering_profile` field from Flowanalyst, Flowbind,
  Flowparallel and Flowoptimize. Tests now assert the versioned lowering-plan
  contract and source-derived operations directly; no required compiler stage
  contains or consumes profile vocabulary.
- Atom contracts now distinguish required inputs, optional activation inputs
  and terminal nodes. Graph validation rejects an unconnected required sink
  input, accepts either optional port of the routing probe, and rejects a
  terminal contract that exposes outputs. Runtime node failures retain the
  receiving node/input plus wire and signal identity in diagnostics before the
  original failure propagates.
- `flow_less` no longer uses the bundled fake pager implementation. Its Flow
  source explicitly connects `pager.input.fake => pager.navigate =>
  pager.render => halt.record`; the input provider emits raw lines, commands
  and page size, while the provider-neutral navigation node owns page-state
  transitions. Source-selected command order changes observable output and an
  unknown command fails with navigation-node wire/signal provenance.
- The ncurses slice now uses the same Flow-owned graph boundary. The injectable
  `pager.input.file.ncurses` provider reads the file and normalizes terminal
  keys, `pager.navigate` applies page-state transitions, and `pager.render`
  projects output. The pseudo-terminal gate observes both explicit `=>` routes
  and the expected first-page result.

## Evidence

- Focused binding checkpoint: `flowbind_provider`, `flowcore_stdlib_boundary`,
  and `native_binding_generation` passed.
- Focused graph checkpoint: `flowcore_graph_routing` passed.
- Current complete checkpoint: **54/54 CTest tests passed** after the namespace
  ambiguity gate; the profile-free comparison-branch lowering gate is also
  green and its native ELF returned the expected status 42.
- Focused identity migration checkpoint: `profile_free_generic_lowering` and
  `flowlower_pipeline` passed, including native observable-result checks for all
  six newly migrated identity examples.
- Focused multi-operand checkpoint: `profile_free_generic_lowering`,
  `flowbind_provider`, and `flowlower_pipeline` passed, including the native
  `getpriority` result assertion.
- Focused generated-binding checkpoint: `native_binding_generation`,
  `flowbind_provider`, and `flowlower_pipeline` passed.
- Focused `c_long` checkpoint: `native_binding_generation` and
  `flowlower_pipeline` passed, including compilation and execution of the
  generated `sysconf` ELF.
- Focused `c_ulong` checkpoint: `native_binding_generation`,
  `profile_free_generic_lowering`, and `flowlower_pipeline` passed.
- Focused ncurses checkpoint: `ncurses_flow_pipeline`, `flow_less_pager`, and
  `flow_less_ncurses_pager` passed.
- Focused ordered-sequence checkpoint: `native_binding_generation`,
  `profile_free_generic_lowering`, `flowbind_provider`, and
  `flowlower_pipeline` passed. The complete canonical suite then passed
  **54/54** tests.
- Focused unary migration checkpoint: `flowlower_pipeline`,
  `flowanalyst_pipeline`, `flowbind_provider`, both pass-corpus gates, and
  `profile_free_generic_lowering` passed. The complete suite again passed
  **54/54** tests.
- Focused string/size migration checkpoint: the same six focused gates passed,
  followed by the complete canonical suite at **54/54**.
- Focused nullable-string checkpoint: `native_binding_generation`,
  `profile_free_generic_lowering`, `flowbind_provider`, and
  `flowlower_pipeline` passed, including hostile signature-policy rejection and
  execution of the generated ELF. The complete canonical build and suite then
  passed **54/54**.
- Focused mixed-carrier checkpoint: `flowlower_pipeline`,
  `flowanalyst_pipeline`, `flowbind_provider`, `native_binding_generation`,
  `profile_free_generic_lowering`, and both pass-corpus gates passed. Native
  stdout remained exactly `Flowcore libc bindings`; the complete canonical
  build and suite then passed **54/54**.
- Focused rmdir checkpoint: `flowlower_pipeline`, `flowanalyst_pipeline`,
  `flowbind_provider`, `profile_free_generic_lowering`, and both pass-corpus
  gates passed. The native ELF executed, the LLVM call referenced the
  source-derived string global, and an adversarial assertion rejected the old
  null-pointer shape. The canonical build and suite passed **54/54**.
- Focused scalar-kernel checkpoint: `flowlower_pipeline`,
  `flowanalyst_pipeline`, `flowbind_provider`, `profile_free_generic_lowering`,
  and both pass-corpus gates passed, including all four native ELFs. The
  canonical build and suite passed **54/54**.
- Focused pointer-carrier checkpoint: `flowlower_pipeline`,
  `flowanalyst_pipeline`, `flowbind_provider`, `profile_free_generic_lowering`,
  and both pass-corpus gates passed, including native execution of all migrated
  fixtures. The canonical build and suite passed **54/54**.
- Focused final scalar/string kernel checkpoint: `flowlower_pipeline`,
  `flowanalyst_pipeline`, `flowbind_provider`, `profile_free_generic_lowering`,
  and both pass-corpus gates passed. Native `openat`, `lseek`, and `unlinkat`
  ELFs executed. The canonical build and suite passed **54/54**. The attempted
  generic `clock_gettime(c_int,c_pointer(0))` execution produced a segmentation
  fault, confirming that typed writable storage must be represented before its
  compatibility profile can be removed.
- Focused null-probe checkpoint: the same six focused gates passed, including
  native execution of `getrandom`, `read`, and `write`; the canonical build and
  suite passed **54/54**.
- Focused writable-storage checkpoint: `flowcore_pass_corpus`,
  `profile_free_generic_lowering`, `flowanalyst_pipeline`, `flowbind_provider`,
  and `flowlower_pipeline` passed, including native execution of
  `clock_gettime` and `uname` and rejection of a zero-byte descriptor. The
  canonical build and suite passed **54/54**.
- Focused namespace checkpoint: `namespace_ambiguity`, `flowanalyst_pipeline`,
  and both `flowcore_pass_corpus` registrations passed with one-through-four
  provider coverage.
- Focused `sel` read-safety checkpoint: `sel_tui_pipeline`,
  `flowlower_pipeline`, and both `flowcore_pass_corpus` registrations passed.
  The TUI gate covers positive input, EOF, and closed-stdin error behavior.
- Focused resource checkpoint: `ncurses_flow_pipeline`, `sel_tui_pipeline`,
  `flowbind_provider`, `flowlower_pipeline`, and both pass-corpus registrations
  passed. Adversarial cases reject invented cleanup identity and missing
  cleanup, and the profile-free ncurses ELF runs in a pseudo-terminal.
- Focused empty-plan checkpoint: `flowanalyst_pipeline`,
  `profile_free_generic_lowering`, `flowlower_pipeline`, and both pass-corpus
  registrations passed, including two separately attributed native targets.
- Focused graph-law checkpoint: `flowcore_graph_routing` and both pass-corpus
  registrations passed with wire/signal provenance and fan-out assertions.
- Focused profile-free terminal checkpoint: `sel_tui_pipeline`,
  `flowanalyst_pipeline`, `flowbind_provider`, `flowlower_pipeline`, and both
  pass-corpus registrations passed. The gate includes renamed-source and
  hostile-binding cases plus positive input, EOF, and read-error execution.
- Focused entry-argument checkpoint: `profile_free_generic_lowering` and
  `flowlower_pipeline` passed, including native execution of both argc branch
  outcomes.
- Focused checked-argv checkpoint: `profile_free_generic_lowering` passed with
  native selected-argument output and missing-argument refusal.
- Focused structured-terminal checkpoint: `sel_tui_pipeline` passed with an
  arbitrarily renamed source, explicit argument and quit-key branches,
  branch-removal refusal, hostile-binding refusal, native positive input, EOF,
  and closed-stdin error behavior. The canonical build and suite then passed
  **54/54** tests.
- Focused profile-free file-copy checkpoint: `flowanalyst_pipeline`,
  `flowbind_provider`, `flowcore_stdlib_boundary`, `flowcat_flowcore_pipeline`,
  and `flowlower_pipeline` passed. An arbitrarily renamed source emitted the
  same native path, removing loop operations was rejected, two files produced
  the expected output, and a missing file returned 1. The canonical build and
  suite passed **54/54** tests.
- Focused operand-contract checkpoint: `flowanalyst_pipeline`,
  `flowbind_provider`, and `flowlower_pipeline` passed. The semantic gate
  asserts the explicit `c_long` to `c_size_t` conversion and an adversarial
  write operation with a mutated operand carrier is rejected.
- Focused generic-loop checkpoint: `profile_free_generic_lowering`,
  `flowlower_pipeline`, `flowanalyst_pipeline`, and both pass-corpus
  registrations passed. The native arbitrary counter loop exited 4, its LLVM
  contains the reusable loop back-edge, and deleting the mutation operation
  caused structured refusal.
- Independent sanitizer checkpoint: configured `/tmp/flowcore-reusable-chain-sanitize`
  with `-fsanitize=address,undefined -fno-omit-frame-pointer`, built the complete
  tree, and ran `ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=detect_leaks=0 ctest
  --test-dir /tmp/flowcore-reusable-chain-sanitize --output-on-failure`.
  **54/54** tests passed, including malformed-input fuzzing, native linking,
  profile-free execution, terminal/resource, graph, and kernel gates.
- Focused generic-terminal checkpoint: `sel_tui_pipeline`,
  `flowlower_pipeline`, `native_binding_generation`, and
  `profile_free_generic_lowering` passed. The native pseudo-terminal gate
  covered argument input, positive stdin input, EOF, and closed-stdin failure;
  adversarial variants proved literal and operation-order sensitivity and
  unused-capability independence.
- Canonical post-terminal checkpoint: the complete build succeeded and
  **54/54** CTest tests passed in 19.35 seconds.
- Focused reusable file-loop checkpoint: `flowanalyst_pipeline`,
  `flowbind_provider`, `flowcore_stdlib_boundary`,
  `flowcat_flowcore_pipeline`, `flowlower_pipeline`,
  `profile_free_generic_lowering`, and `sel_tui_pipeline` passed. Native tests
  copied two small files and a two-megabyte file exactly, rejected a missing
  file, retained cleanup/error returns, and rejected a plan with removed loops.
- Canonical post-file-emitter checkpoint: the complete build succeeded and
  **54/54** CTest tests passed in 17.96 seconds.
- Focused typed-effect checkpoint: `flowanalyst_pipeline`,
  `flowbind_provider`, `sel_tui_pipeline`, and `flowlower_pipeline` passed,
  including hostile effect and argument-memory mutations.
- Canonical typed-effect checkpoint: the complete build succeeded and
  **54/54** CTest tests passed in 19.43 seconds.
- Focused typed-window checkpoint: `ncurses_flow_pipeline`, `sel_tui_pipeline`,
  `flowbind_provider`, `flowlower_pipeline`, `flowanalyst_pipeline`, and both
  pass-corpus registrations passed. The canonical build and suite then passed
  **54/54** tests in 19.58 seconds.
- Focused path-sensitive resource checkpoint: `flowbind_provider`,
  `ncurses_flow_pipeline`, and `sel_tui_pipeline` passed, including missing
  branch-cleanup and double-cleanup rejection. The complete canonical build and
  suite then passed **54/54** tests in 17.75 seconds.
- Focused typed-only lowering checkpoint: `flowlower_pipeline`,
  `profile_free_generic_lowering`, `native_binding_generation`,
  `ncurses_flow_pipeline`, `sel_tui_pipeline`, `flowcat_flowcore_pipeline`, and
  both pass-corpus registrations passed. The complete canonical suite passed
  **54/54** tests in 18.93 seconds after all legacy Flowlower emitters were
  removed.
- Profile-field removal checkpoint: the complete canonical build succeeded and
  **54/54** CTest tests passed in 18.46 seconds with no `lowering_profile`
  identifier remaining outside historical mission/ledger documentation.
- Focused connectivity/failure checkpoint: `flowcore_graph_routing`, both
  pass-corpus registrations, `flow_less_pager`, and
  `flow_less_ncurses_pager` passed. The complete canonical build and suite then
  passed **54/54** tests in 17.95 seconds.
- Focused Flow-owned navigation checkpoint: `flow_less_pager`,
  `flow_less_ncurses_pager`, `flowcore_graph_routing`, and both pass-corpus
  registrations passed. The complete canonical build and suite then passed
  **54/54** tests in 17.48 seconds.
- Focused injectable ncurses checkpoint: `flow_less_pager`,
  `flow_less_ncurses_pager`, and both pass-corpus registrations passed. The
  complete canonical build and suite then passed **54/54** tests in 19.33
  seconds.
- Final canonical verification: `cmake --build build -j4` succeeded and
  `ctest --test-dir build --output-on-failure` passed **54/54** tests in 19.33
  seconds. This includes generated-binding acceptance, native LLVM linking and
  execution, malformed Flowbind input, `sel`, `flowcat`, ncurses ownership,
  graph laws and both `flow_less` providers.
- Final sanitizer verification rebuilt
  `/tmp/flowcore-reusable-chain-sanitize` with
  `-fsanitize=address,undefined -fno-omit-frame-pointer`; with leak detection
  disabled for the external-provider test environment, the complete suite
  passed **54/54** tests in 37.13 seconds.
- Final dispatch inventory found no `lowering_profile`, application/source-name
  selectors, terminal/file-copy capability-set recognizers, or handwritten
  application emitters in Flowanalyst, Flowbind, Flowparallel, Flowoptimize or
  Flowlower. The mission commit range is `a1b51d2^..HEAD` on
  `v25-symboltable-projection`.

## Remaining work

- The unqualified single-provider import alias remains explicitly transitional;
  selective-opening syntax is not yet part of the language.
- Platform limitation: native demonstrations target the tested Linux
  x86-64/libc/LLVM environment; ncurses acceptance requires
  `libncursesw.so.6` and a pseudo-terminal. The current bounded-storage
  compatibility representation still interprets positive `c_pointer(N)` as
  call-lifetime writable storage pending the documented public-language choice.

## Exact next action

Gate 6 remains unfinished. The receiver activation decision is approved. Implement
fresh source-function activation frames, preserve the complete graph through the
middle stages and native lowering, then move pager navigation into Flow. Do not
ask for the approved decision again. State remains CONTINUE; no total blocker is
recorded. Historical DONE claims do not describe this checkout.

## 2026-09-13 Text boundary checkpoint

- Implemented the first generic `Text` slice across Flowmini AST metadata,
  Flowanalyst, Flowbind, Flowparallel/Flowoptimize preservation, and Flowlower.
- `Text` literals are UTF-8 validated; initializer-known concatenation folds
  left-to-right; empty Text remains a non-null materialized value; and an
  initializer-known Text can cross an ordinary Text-returning function.
- `print` creates an externally authorized operation only for the declared
  `puts_text(Text): c_int` capability. Existing numeric print compatibility and
  borrowed `c_string` ABI calls remain green. Text/c_string confusion, dynamic
  concatenation, and invalid UTF-8 have explicit diagnostics.
- Evidence: `tools/test-text-value.sh` passed semantic, authorization, native
  output, callable-return, and refusal checks; the pass corpus passed **92**
  programs; a fresh 115-target build passed **82/82** CTest tests in 35.54s.
- Checkpoint commit `a2380da` is pushed to `origin/main`.

## Exact next action

Implement runtime Text concatenation with an explicit bounded-storage policy,
then add failure-path and TinyVM parity evidence. Keep the compile-time-only
slice clearly distinguished until that gate passes.

## 2026-09-13 TinyVM Text parity checkpoint

- Extended TinyVM's existing opaque string-handle carrier to admit the declared
  `Text` carrier. The runtime provider accepts `puts(Text)` only through the
  exact `libc.so.6` / `puts` / `c` / `io` / `Text` / `c_int` policy tuple.
- Fixed a generic TinyVM lowering gap exposed by this fixture: an external call
  that intentionally discards its result now receives a typed temporary slot;
  no result symbol is fabricated in the source artifact.
- Evidence: `tools/test-tinyvm-text-parity.sh` passed independently captured
  LLVM and TinyVM execution with identical Text output; the fresh complete CTest
  graph passed **83/83** in 36.68s.
- Compatibility boundary: this proves parity only for literal and
  initializer-known Text values. It does not admit runtime-created Text or
  allocation ownership.

## Exact next action

Define the bounded runtime Text storage contract and implement one owned
concatenation path with explicit exhaustion behavior. Then add the corresponding
LLVM/TinyVM differential and failure evidence. State remains CONTINUE.

## 2026-09-13 bounded runtime Text checkpoint

- Added the declared `text_runtime` ABI and provider-owned `Text + Text` path.
  `flow_text_concat` copies both NUL-terminated inputs into fresh heap storage,
  caps the combined UTF-8 byte length at **4096**, and returns failure on null,
  overflow, or allocation exhaustion.
- Flowanalyst admits dynamic Text concatenation only when exactly one declared
  `Text,Text -> Text` memory capability exists. Otherwise the source-linked
  `FLOWANALYST_TEXT_DYNAMIC_CONCAT` refusal remains in force.
- Flowlower emits the authorized provider call and traps through `llvm.trap`
  when a non-null Text result contract is violated. A returned provider-owned
  value survives an ordinary Text function return and prints natively.
- Evidence: `tools/test-text-runtime.sh` passed successful concat and a 4097-byte
  exhaustion case; `tools/test-tinyvm-runtime-text-parity.sh` passed the
  LLVM/TinyVM differential and failure case; the fresh complete suite passed
  **85/85** in 36.26s.
- TinyVM now registers provider-owned result bytes as provider-local opaque
  handles for the activation. Those handles are never serialized as host
  pointers; the artifact still carries only the governed import contract.
- A provider failure is an explicit TinyVM import trap (`TV1_TRAP_UNRESOLVED_IMPORT`)
  and an LLVM `llvm.trap`, with the same non-success disposition in the
  differential fixture.
- The runtime fixture now chains one owned result into another concat and prints
  the final value twice, covering repeated provider-owned storage use on both
  backends.

## Exact next action

Define the backend-neutral `Outcome<Text, TextFailure>` representation and map
the current provider failures into it before adding a second provider shape.
Then cover fan-out ownership/lifetime cases. Keep artifact-visible host
pointers prohibited. State remains CONTINUE.

## 2026-09-13 Text outcome contract checkpoint

- Defined the proposed backend-neutral `Outcome<Text, TextFailure>` in
  `docs/architecture/text-outcome-v0.1.md`.
- The initial stable failure vocabulary is `invalid_input`, `exhausted`, and
  `provider_unavailable`; host pointers and backend trap numbers are explicitly
  excluded from the semantic value.
- The current `flow_text_concat` null result plus LLVM/TinyVM trap mappings are
  retained as transitional compatibility until a typed `text_outcome`
  lowering operation is implemented.

## Exact next action

Implement the typed `text_outcome` operation at the backend-neutral artifact
boundary, then map the existing provider success/failure paths through it and
add fan-out ownership evidence. State remains CONTINUE.

## 2026-09-13 Text outcome metadata checkpoint

- Added validated `result_outcome` metadata to runtime Text external operations.
  It carries `Text` success, `TextFailure` failure, and the stable
  `invalid_input`, `exhausted`, and `provider_unavailable` codes through the
  generic artifact chain without serializing a host pointer.
- The metadata is a compatibility seed, not yet the tagged `Outcome` value
  required by the proposal. Existing LLVM/TinyVM trap mappings remain explicit
  transitional backend behavior.
- Evidence: focused Text runtime, TinyVM parity, contract-identity, and pass
  corpus gates passed; the complete fresh suite passed **85/85** in 39.42s.

## Exact next action

Replace the metadata-only compatibility seed with a typed `text_outcome`
operation and backend-neutral success/failure value, then add fan-out
ownership/lifetime evidence. State remains CONTINUE.

## 2026-09-13 typed Text outcome boundary checkpoint

- Promoted dynamic owned-Text construction from a metadata-bearing
  `external_call` to the distinct `text_outcome` lowering operation.
- The artifact contract now requires a serializable tagged
  `Outcome<Text,TextFailure>` declaration for that operation and rejects a
  missing or mismatched outcome shape. Flowbind, Flowprepare, LLVM Flowlower,
  and TinyVM Flowlower all preserve and admit the operation explicitly.
- Added a negative validator assertion for a missing typed outcome and kept
  ordinary `puts(Text)` calls as `external_call` operations.
- Extended the runtime fixture to fan out one immutable owned `Text` input into
  two owned concatenation results and reuse one result; LLVM and TinyVM remain
  output-equivalent. The complete fresh CTest graph passed **85/85** in 38.30s.

## Exact next action

Replace the provider's null-as-failure adapter with an explicit tagged
success/failure transport carrying `TextFailure` codes on both backends, while
retaining the current `text_outcome` artifact contract and adding cleanup-once
evidence. State remains CONTINUE.

## 2026-09-13 provider tagged Text transport checkpoint

- Added the reference provider's explicit `FlowTextOutcome` transport with
  stable `success`, `invalid_input`, `exhausted`, and
  `provider_unavailable` codes, an owned success pointer, and an idempotent
  dispose operation that clears the pointer after cleanup.
- Kept `flow_text_concat` as a compatibility adapter over the tagged API, so
  existing LLVM/TinyVM output and the governed import tuple remain unchanged
  until their ABI carriers are ready for the new result shape.
- Added a direct provider API test covering successful ownership, invalid input,
  bounded exhaustion, and cleanup; the complete fresh CTest graph passed
  **86/86** in 38.78s.

## Exact next action

Define the cross-backend carrier for the tagged provider result and route the
existing `text_outcome` operation through it, with explicit failure-code
observation on LLVM and TinyVM. Preserve the artifact rule that host pointers
never appear in serialized plans. State remains CONTINUE.

## 2026-09-13 cross-backend tagged Text carrier checkpoint

- Routed LLVM `text_outcome` lowering through the provider's tagged
  `{code,value}` carrier and explicitly checked both the failure code and
  success pointer before continuing.
- Routed TinyVM's governed `flow_text_concat` thunk through
  `flow_text_concat_outcome`; it preserves `invalid_input`, `exhausted`, and
  `provider_unavailable` in its fault reason before the existing import-fault
  mapping. No host pointer enters the TinyVM artifact or serialized plan.
- The full fresh CTest graph passed **86/86** in 39.10s, including LLVM/TinyVM
  runtime parity, tagged API ownership, exhaustion, and validator refusal
  coverage.

## Exact next action

Expose the observed `TextFailure` code as a recoverable language-level outcome
instead of an unconditional backend trap, then add an explicit last-owner
cleanup test for both backends. State remains CONTINUE.

## 2026-09-13 structured Text failure result checkpoint

- `flowtinyrun` now carries a recognized tagged Text failure into its
  machine-readable execution record as `Outcome<Text,TextFailure>` with the
  stable failure code. The existing TinyVM fault and trap fields remain for
  compatibility, and LLVM's native process disposition remains unchanged.
- Exhaustion coverage now asserts the structured `exhausted` code in addition
  to the deterministic TinyVM trap. The complete fresh CTest graph passed
  **86/86** in 37.43s.

## Exact next action

Add a Flow-level `Outcome<Text,TextFailure>` carrier and explicit recovery
branching for the bounded concat example, then prove that successful values are
disposed exactly once after the final consumer on LLVM and TinyVM. State
remains CONTINUE.

## 2026-09-13 bounded memory ABI checkpoint

- Promoted `std/abi/memory.flow` from binding-only evidence to a bounded LLVM
  execution slice. The generic typed plan now carries two `c_pointer(8)` local
  storage operands and a `c_size_t(8)` length through `memset`, `memcpy`, and
  `memcmp` without source-name-specific lowering.
- The standard-library boundary now runs the full Flowmini → Flowanalyst →
  Flowparallel → Flowoptimize → Flowbind → Flowlower → clang path and requires
  the native result to be zero after copying initialized bytes. Exact
  `libc.so.6` capability authorization remains mandatory.
- The capability matrix records the pointer-plus-length contract precisely:
  bounded local storage is executable on LLVM; arbitrary native pointers and
  TinyVM storage-handle execution remain outside the claim.
- Focused evidence: `flowcore_stdlib_boundary` passed after the new native
  execution assertion.

## 2026-09-13 TinyVM bounded memory parity checkpoint

- Extended TinyVM's governed runtime provider with activation-local storage
  handles backed by the artifact's declared byte lengths. `memset`, `memcpy`,
  and `memcmp` now resolve the exact `memory` ABI tuples and never receive a
  host pointer from the serialized artifact.
- Added cross-backend parity evidence for the generic memory fixture: LLVM and
  TinyVM both return the `memcmp` equality result, while a forged count beyond
  the eight-byte declaration is rejected as a deterministic governed import
  fault.
- Added `tinyvm_memory_parity` to the canonical CTest graph. The test uses the
  `flowprepare` backend artifact boundary and validates both normal execution
  and the hostile bounds mutation.
- The complete canonical CTest graph passed **87/87** in 38.79s.

## 2026-09-13 Flow Text status recovery checkpoint

- Added the governed `flow_text_concat_status(Text,Text):c_int` provider
  facade. It returns the stable `TextFailure` code and disposes the temporary
  provider-owned success allocation internally, so Flow can branch before
  requesting an owned Text result.
- Added a generic Flow fixture with explicit success/failure branching. LLVM
  and TinyVM both print the successful value for the success case and the
  fallback for a deliberately exhausted 4097-byte request; the same exact
  provider tuples authorize both paths.
- The atomic `Outcome<Text,TextFailure>` value is still distinguished from this
  two-operation recovery facade; no serialized host pointer or allocator
  address is introduced.
- Focused evidence: `text_recovery_boundary` passed with LLVM/TinyVM output
  parity and exhaustion recovery.
- The complete canonical CTest graph passed **88/88** in 39.04s.
- The tagged provider API gate additionally covers status-probe success and
  exhaustion without transferring an owned allocation to the caller.

## Exact next action

Replace the two-operation status recovery facade with an atomic backend-neutral
`Outcome<Text,TextFailure>` value and prove that successful values are disposed
exactly once after the final consumer on LLVM and TinyVM. State remains
CONTINUE.

## 2026-09-13 atomic TextOutcome carrier checkpoint

- Added `concat_outcome(Text,Text):TextOutcome` as the Flow-level atomic result
  carrier. The provider returns the tagged `{code,value}` value by value, while
  serialized plans contain only the declared carrier and exact provider tuple.
- Added Flow field projection for `.code` and `.value`, explicit recovery
  branching, and the `dispose(Text)` final-owner capability. LLVM lowers the
  carrier as a local `{i32,ptr}` value; TinyVM uses a checked outcome handle and
  governed projections, with runtime cleanup clearing the value before teardown.
- Added success and bounded-exhaustion parity coverage. Both backends print the
  same recovered result, the LLVM artifact contains exactly one dispose call,
  and the complete atomic gate passed.
- Focused evidence: `ctest --test-dir /tmp/flowcore-canonical-build
  --output-on-failure -R text_outcome_boundary` passed **1/1**; the dependent
  recovery gate passed **2/2**.

## Exact next action

Select and mature the next non-Text standard capability as a complete
policy-gated library slice, beginning with its ABI contract, provider/runtime
boundary, LLVM/TinyVM evidence where applicable, and onboarding/documentation
coverage. State remains CONTINUE.

## 2026-09-13 libm c_double library checkpoint

- Added `std/abi/math.flow` with explicit `libm.so.6` contracts for
  `sqrt(c_double):c_double` and `floor(c_double):c_double`.
- Extended the generic LLVM carrier path for typed double literals, native math
  calls, and source-derived floating-point comparisons. The test does not infer
  authority from symbol presence: both exact grants are required before linking.
- Added `abi_math_main` and onboarding references. The focused native gate
  passed and prints `libm ok` plus `libm floor ok`.
- TinyVM deliberately remains closed for `c_double`; its unsupported inventory
  now records the missing float carrier/instruction semantics rather than
  silently mapping the calls to an integer or host-specific fallback.

## Exact next action

Select the next non-Text library boundary after `libm`, prioritizing a bounded
resource or system interface whose ownership and failure semantics can be
verified on both LLVM and TinyVM. State remains CONTINUE.

## 2026-09-13 bounded libc strnlen checkpoint

- Added the exact `strnlen(c_string,c_size_t):c_size_t` contract to
  `std/abi/libc.flow`. The length limit remains an explicit Flow value and the
  call is authorized by the complete provider tuple.
- Added native LLVM and governed TinyVM execution coverage using the same
  `Lyraform` input and an eight-byte limit. Both backends print `strnlen ok`;
  TinyVM admits the tuple through a dedicated typed thunk.
- Focused evidence: `strnlen_library_boundary` passed with LLVM/TinyVM output
  parity. The standard-library matrix and TinyVM parity inventory record the
  bounded slice.

## Exact next action

Select the next library-oriented capability slice, with preference for a
bounded operation that broadens the existing pointer-length or resource
contracts while retaining exact LLVM/TinyVM parity and explicit refusal for
unsupported carriers. State remains CONTINUE.

## 2026-09-13 bounded memmove parity checkpoint

- Extended `std/abi/memory.flow` with exact `memmove(c_pointer,c_pointer,c_size_t)`
  authorization and added it to the generic bounded memory program.
- LLVM and TinyVM both execute the new provider tuple using the existing
  eight-byte storage contract. TinyVM resolves it through its own checked
  storage thunk; no native pointer enters the artifact. Partial-overlap proof
  remains pending because pointer slicing is not yet an admitted carrier.
- The standard-library boundary, memory parity gate, capability matrix, and
  parity inventory now record four memory operations rather than three.

## Exact next action

Select another library-oriented capability with a distinct failure or resource
contract, while keeping the complete canonical suite and onboarding evidence
reconciled. State remains CONTINUE.

## 2026-09-13 ctype scalar library checkpoint

- Added `std/abi/ctype.flow` with exact `tolower(c_int):c_int` and
  `toupper(c_int):c_int` contracts over `libc.so.6`.
- Added the newly named `abi_ctype_main` fixture and a dedicated policy,
  binding, LLVM-link, and TinyVM execution gate. Both backends print the same
  conversion evidence and require the complete provider tuples.
- Updated the standard-library matrix, TinyVM parity inventory, and onboarding
  focused-gate command. Focused evidence: `ctype_library_boundary` passed
  **1/1**; the complete canonical build and CTest suite passed **92/92** in
  38.14 seconds. The pass corpus independently reports **95/95** programs
  through semantic and lowering boundaries.

## Exact next action

Continue broadening the library surface with a bounded interface that adds a
distinct resource or failure contract, while retaining exact LLVM/TinyVM
parity and explicit refusal for unsupported carriers. State remains CONTINUE.

## 2026-09-13 file descriptor resource checkpoint

- Extended the existing `std/abi/file_io.flow` boundary with executable
  `open(c_string,c_int):c_int` and `close(c_int):c_int` parity. The new
  `abi_file_resource_main` fixture opens `/dev/null`, checks the failure-capable
  descriptor result, and closes only an acquired descriptor.
- TinyVM now tracks descriptors returned by its admitted `open` thunk, rejects
  closing descriptors it does not own, and closes any still-owned descriptors
  during provider teardown. The wider `read`/`write`/`sendfile` surface remains
  explicitly unsupported until its buffer, offset, partial-transfer, and
  resource semantics are implemented.
- Focused evidence: `file_resource_boundary` passed **1/1** with identical
  LLVM/TinyVM output. The complete canonical build and CTest suite passed
  **93/93** in 38.71 seconds; the pass corpus now reports **96/96** programs.

## Exact next action

Continue the resource boundary with an explicitly bounded read/write slice,
or document and test its refusal if the required initialized-byte and
partial-transfer semantics cannot yet be represented. State remains CONTINUE.

## 2026-09-13 bounded file I/O checkpoint

- Extended the `file_io` provider slice with bounded
  `read(c_int,c_pointer,c_size_t):c_long` and
  `write(c_int,c_pointer,c_size_t):c_long` calls. The new
  `abi_file_io_main` fixture reads eight bytes from `/dev/zero` and writes
  eight bytes to `/dev/null` through tracked descriptors and an eight-byte
  storage handle.
- TinyVM validates descriptor ownership and storage bounds before dispatch;
  its runtime preserves descriptors across calls and cleans any remaining
  descriptors at teardown. `sendfile` remains refused because offset and
  partial-transfer semantics are not yet represented.
- Focused evidence: `file_io_boundary` passed **1/1** with identical
  LLVM/TinyVM output and the authorized `sendfile` refusal was confirmed. The
  complete canonical build and CTest suite passed **94/94** in 38.97 seconds;
  the pass corpus now reports **97/97** programs.

## Exact next action

Continue with the first unfinished mission gate: mature resource failure and
partial-transfer semantics around the admitted file boundary, or add a
structured refusal gate that proves unsupported `sendfile` input cannot reach
either backend. State remains CONTINUE.

## 2026-09-13 file I/O failure checkpoint

- Extended `abi_file_io_main` with a `/dev/full` write and explicit negative
  result branch. LLVM and TinyVM both report `write failure ok` while still
  closing the acquired descriptor.
- Focused evidence: `file_io_boundary` passed **1/1** with bounded success,
  EOF, error-result, cleanup, and structured `sendfile` refusal coverage. The
  complete canonical CTest suite passed **94/94** in 38.63 seconds.

## Exact next action

Continue the first unfinished mission gate with partial-transfer and stronger
initialized-byte semantics, keeping `sendfile` refused until its offset and
transfer contract can be represented without raw host pointers. State remains
CONTINUE.

## 2026-09-13 initialized-byte file I/O checkpoint

- TinyVM storage now carries initialized-byte state. Bounded `read` marks only
  positively transferred bytes initialized, and bounded `write` refuses an
  uninitialized range before calling the host provider.
- The focused `file_io_boundary` gate passed **1/1**, including a successful
  read-to-write path and an explicit trap-7 uninitialized-write refusal. The
  complete canonical CTest suite passed **94/94** in 35.51 seconds.

## Exact next action

Continue the first unfinished mission gate with partial-transfer semantics or
the next approved receiver/graph contract; keep raw-pointer offset operations
such as `sendfile` explicitly unsupported until their storage representation is
complete. State remains CONTINUE.

## 2026-09-13 partial-transfer file I/O checkpoint

- Extended `abi_file_io_main` with a bounded `/etc/hostname` read using a
  4096-byte storage handle. The fixture requires a positive transfer smaller
  than the requested limit, proving that the actual transfer count—not the
  requested count—is the initialized range.
- Focused evidence: `file_io_boundary` passed **1/1** with partial read,
  bounded read/write, EOF, `/dev/full` error, uninitialized-write refusal,
  descriptor cleanup, and `sendfile` refusal. The complete canonical CTest
  suite passed **94/94** in 37.74 seconds.

## Exact next action

Continue with the next approved receiver/graph contract while retaining the
file boundary’s explicit refusal for raw-pointer offset operations and broader
platform-dependent transfer cases. State remains CONTINUE.

## 2026-09-13 independent-root graph checkpoint

- Extended `native_source_graph` with two independent startup providers feeding
  the same source receiver. Each root now has explicit evidence of its own FIFO
  activation sequence, fresh receiver delivery, distinct output-signal identity,
  and fan-out reuse without re-execution.
- Updated the native graph contract note and current onboarding/root-suite
  counts. The canonical suite remains **94/94**; no implementation-specific
  graph dispatch was added.

## Exact next action

Continue the first unfinished mission gate with the next receiver/graph law,
retaining explicit scheduling/activation separation and the refusal of raw
pointer graph payloads. State remains CONTINUE.

## 2026-09-13 native wide-scalar graph checkpoint

- Extended `native_source_graph` with a provider-to-receiver chain carrying an
  exact `c_long` value larger than 32 bits. The value survives native provider
  dispatch, a fresh source receiver frame, arithmetic, and fan-out without
  narrowing.
- The trace assertions retain source/output signal identity, shared fan-out
  signal identity, distinct delivery identities, and complete graph execution
  provenance for the wide carrier.
- Focused native graph evidence passed **1/1**; the implementation continues to
  use generic carrier/type lowering rather than source-name dispatch.

## Exact next action

Continue Gate 6 with the next receiver/graph boundary while retaining exact
carrier authorization, scheduling/activation separation, and explicit refusal
of unsupported aggregate or raw-pointer graph payloads. State remains CONTINUE.

## 2026-09-13 native Boolean graph checkpoint

- Extended `native_source_graph` with a typed `c_int -> Bool -> c_int`
  receiver chain. Native lowering now has executable evidence for a Boolean
  graph payload, a source-defined Boolean branch, and exact signal propagation
  across each typed wire.
- The test also retains distinct delivery identities and confirms that the
  graph remains generic: the receiver names and program name are not compiler
  dispatch selectors.
- Focused native graph evidence passed **1/1**; the complete canonical suite
  remains **94/94**.

## Exact next action

Continue Gate 6 with the next receiver/graph contract while keeping aggregate,
streaming, persistent-state, and raw-pointer payloads explicitly outside the
admitted surface. State remains CONTINUE.

## 2026-09-13 native raw-pointer graph refusal checkpoint

- Added a hostile native graph fixture whose scalar provider is wired to a
  `c_pointer` receiver. Flowanalyst rejects the connection at the graph type
  boundary with `FLOWANALYST_GRAPH_PROVIDER_TYPE`; no native object is emitted.
- This preserves the distinction between admitted scalar graph carriers and
  bounded pointer-plus-length ABI operations. Raw host pointers cannot become
  graph payloads merely because the backend has an LLVM pointer representation.
- Focused native graph evidence passed **1/1**; the complete canonical suite
  remains **94/94**.

## Exact next action

Continue Gate 6 with the next receiver/graph boundary while retaining explicit
refusal of raw-pointer, aggregate, and streaming payload semantics. State
remains CONTINUE.

## 2026-09-13 native aggregate graph refusal checkpoint

- Added a dedicated `FLOWANALYST_GRAPH_RECEIVER_CARRIER` admission rule for
  native source receivers. Only the currently admitted scalar carriers reach
  graph planning; record/aggregate and raw-pointer receiver signatures fail
  before native emission.
- Extended `native_source_graph` with hostile `c_pointer` and record-shaped
  receiver fixtures. Positive `c_int`, `c_long`, and `Bool` graph paths remain
  generic and executable.
- The complete canonical suite passed **94/94** after rebuilding Flowanalyst;
  state remains `CONTINUE`.

## Exact next action

Continue Gate 6 with the next receiver/graph law while preserving explicit
scalar-carrier admission and refusal of aggregate, streaming, persistent-state,
and raw-pointer graph semantics.

## 2026-09-13 native startup failure checkpoint

- Extended `native_source_graph` with an authorized startup provider that calls
  `flow_graph_raise(23)`. The root activation emits a structured
  `source_failure` with startup provenance and exit 70, publishes no normal
  output, and never enters its downstream receiver.
- This proves the same failure law at both receiver and root-provider activation
  boundaries without adding provider-name or application-specific dispatch.
- Focused native graph evidence passed **1/1**; the canonical suite remains
  **94/94**.

## Exact next action

Continue Gate 6 with the next receiver/graph contract while retaining the
no-output-on-failure law and explicit separation of provider authority,
scheduling, activation, and payload carriers. State remains CONTINUE.

## 2026-09-13 graph scheduling/activation separation checkpoint

- Added hostile captured-schedule mutations for the FIFO policy and
  `fresh_single_input_v1` activation contract. Flowoptimize and Flowlower reject
  both mutations before consuming the schedule.
- This keeps scheduling delivery order independent from receiver activation
  semantics and prevents a durable artifact from laundering persistent or
  parallel behavior into the bounded native graph surface.
- Focused native graph evidence passed **1/1**; the complete canonical suite
  remains **94/94**.

## Exact next action

Continue Gate 6 with the next receiver/graph boundary while preserving strict
schedule identity, fresh activation, and failure provenance. State remains
CONTINUE.

## 2026-09-13 native graph schedule-bound checkpoint

- Added a generated 65,537-activation graph fixture. Flowparallel rejects it at
  the documented 65,536 static-schedule limit, and no partial execution plan is
  produced for backend lowering.
- Indexed graph wires by source node during schedule expansion so the bound is
  checked without rescanning the complete wire set for every activation.
- The bounded positive graph paths and all existing identity/failure checks
  remain unchanged. Focused native graph evidence passed **1/1**.

## Exact next action

Continue Gate 6 with the next receiver/graph boundary while retaining the
bounded schedule, strict identity validation, fresh activation, and failure
provenance laws. State remains CONTINUE.

## 2026-09-13 native graph unsigned carrier checkpoint

- Extended the generic native graph gate with independent `c_ulong` and
  `c_size_t` provider roots. Values above the 32-bit range survive startup,
  receiver, and observer calls with their ABI-width identities intact.
- The existing c_int, c_long, Bool, fan-out, failure, schedule, and refusal
  coverage remains green; focused native graph evidence passed **1/1**.

## Exact next action

Continue Gate 6 with the next receiver/graph boundary while preserving exact
carrier width, root scheduling, activation provenance, and bounded execution.
State remains CONTINUE.

## 2026-09-13 native graph borrowed-string checkpoint

- Extended the executable graph gate with a borrowed `c_string` provider and
  receiver fan-out. The captured pointer reaches both observers through one
  receiver output activation without being confused with a raw `c_pointer`.
- The complete admitted scalar graph carrier set now has executable evidence;
  focused native graph evidence passed **1/1**.
- A fresh current-checkout ASan/UBSan build passed the complete **94/94** suite
  with `ASAN_OPTIONS=detect_leaks=0:verify_asan_link_order=0`; the link-order
  option is required here for dynamically loaded text-runtime tests and avoids
  preloading ASan into external helper tools.
- A fresh CMake install under `/tmp/flowcore-latest-install` built the installed
  `flow_less` example with only installed tools, runtime, source, generator,
  and providers; the linked binary rendered `-- page 3/3 --` and `epsilon`.
- The freshly installed pager also passed direct Valgrind with zero errors,
  zero bytes in use at exit, and all 8 allocations freed.

## Exact next action

Reconcile the final state file and push the closing checkpoint. State remains
CONTINUE until that clean pushed checkpoint is verified.

## 2026-09-13 finite stream phase opened

- Henrik reprioritized post-v0.29 maturation as streams first, then persistent
  state, aggregate payloads, reentrant/parallel scheduling, and TinyVM graph
  execution last.
- Opened the finite scalar stream contract in
  `source-graph-stream-activation-decision.md`: explicit count/item provider
  identities, ascending bounded indices, fresh receiver activations, preserved
  signal/wire/delivery identity, and stop-on-failure semantics.
- The stream phase starts with a maximum of 4,096 items and remains synchronous,
  scalar, and non-persistent. No stream implementation is claimed yet.

## Exact next action

Implement the finite stream provider-map and source-graph artifact contract,
then add independent schedule and native execution evidence. State remains
CONTINUE.
