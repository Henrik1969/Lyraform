# Source activation runtime foundation — PASS at the prerequisite boundary

Mission `lyraform.source-activation-runtime`, revision 1. Branch
`experiment/configresolve-lyraform`, HEAD b50534c380879a5a70a77ed5c1dd442bfc4142bd
plus preserved pre-existing and experiment changes. No commit or merge.

Scout: native/TinyVM callable loaders currently erase envelope roles to carrier
spellings. Existing C++ failure-flow reference execution uses callbacks and
exception transport; it cannot implement ADR 0067's source runtime boundary.
The accepted law requires explicit activation results and immutable evidence.

Implemented `Flowtools/reference/disposition`: a private C activation-frame
library with explicit transitions and const evidence access. It accepts already
resolved authority, preserves module/revision, activation, producer operation,
disposition/obligation, selected response/parameter/route, route and host policies,
source/response provenance, attempt/correlation, type and commit evidence.
Success publishes once. Failure retains its immutable envelope until selected
recovery closes the obligation and publishes once. Quarantine returns a fault
envelope and containment identity with no normal value. Repeated or illegal
transitions refuse without changing the frame. No callback invocation, name
resolution, allocation, TLS, exception, unwind or process exit occurs in C code.
The linked library's only strong undefined runtime symbol is `strcmp`.

The ABI uses caller-owned immutable storage retained through host consumption;
it neither allocates nor truncates evidence. This private implementation choice
does not establish ordinary-record or general source lifetime semantics.

Evidence is source-derived: the test driver validates real graph-version-2
reports for fixed-single and explicitly selected two-route source fixtures, then
projects those resolved identities into runtime authority. It supplies test
payloads and does not execute Lyraform response bodies. Assertions cover wrong
function/parameter/route/type/provenance/commit, reinitialization, double closure,
publication after failure/quarantine, independent frame state and 64 KiB evidence
retention. Existing C++ reference models remain independent contract oracles.

Validation: Igor doctor/build/test passed; 192/192 CTests. Two source cases passed
Clang 18 ASan/UBSan. LeakSanitizer failed under the environment's ptrace setup;
ASan was rerun with only `detect_leaks=0`. Both cases separately passed Valgrind
with zero errors and zero live allocations. The ConfigResolve 96-case scalar
probe retained native/TinyVM parity and its renaming/hostile-oracle controls.
Evidence: `experiments/configresolve/evidence/activation-runtime/`.

What became usable: a linkable, explicit-state C boundary for the generated
executor. What remains unsupported: actual compiler-generated envelope calls,
source recovery execution, ownership/disposal along that path, host serialization,
TinyVM parity, response-attempt faults and transformed successor chains. Source
lowering plans remain blocked; this does not complete Gate 4, Mission 07, the
ConfigResolve port or the language goal. Runtime transition checks do not by
themselves prove generated control flow suppresses continuation after a fault.

Next bounded mission: connect the preserved source graph to native and TinyVM
execution, using this exact frame API and established TextOutcome authority;
prove actual source responses, exclusive output, complete evidence and no hidden
termination. Reuse this library rather than creating another runtime model.
