# Canonical disposition lanes and wire closure: stage checkpoints

Date: 2026-10-09.
Selected mission: [canonical disposition lanes and wire closure](../tasks/canonical-disposition-lanes-and-wire-closure.md).
Base SHA: `86a5c69d58bacb09f393cef2a3d4e798ac9fe3ad`.
Branch: `mission/canonical-disposition-lanes-wire-closure`.
Final mission result: **BLOCKED**, not DONE.

## Entry and protected scope

Local `main`, `origin/main` and the remote-advertised main SHA agreed at entry.
The only existing untracked paths were the selected mission,
`meta-discusions.md` and `output/`. The selected mission was read, not rewritten;
unrelated files remain untouched and unstaged. The mission explicitly authorizes
the dedicated branch and coherent commits/pushes. No main/master writes, PR,
merge, history rewrite, workflow change, FlowLFS or Flowselection changes occur.

Repository AGENTS.md and both standing protocols were read. ADRs 0054–0065 and
the relevant dated source-authority/reference-flow checkpoints were consulted as
accepted authority and factual evidence. Historical mission links did not grant
additional scope. Build graph: root CMake superbuild, shared header contracts,
Flowanalyst, binding, scheduling, optimization, preparation and both backends;
Igor uses `/tmp/lyraform-build`.

## Stage 0 checkpoint — PASS

Question: which source facts can support this route today?
The [machine inventory](../architecture/disposition-seam-inventory-v1.json)
records earliest identity producers, all relevant path families and error
classifications. Fresh TextOutcome analysis confirms source location 7:5,
operation/fact 0, owner 38, `TextFailure`, `atomic_tagged_result` and
`operation:0:outcome`. It has tagged local accounting, not an envelope function,
selected consumer or recovery transition. Runtime-dependent guards are refused.
Reference plans supply their own numeric IDs and C++ callbacks; source graph
wires are separate strings. Native `flow_graph_fail` is terminal implementation
behavior, not an admitted activation-containment route.

Changed file: inventory. Gate evidence: inspected producer emission and shared
validators, fresh frontend/semantic artifact, existing reference/backends and
runtime termination code. No hand-authored fixture is called source-derived.
Next step: independent declarative bridge; source association remains missing.

## Stage 1 checkpoint — PASS for the bounded declarative bridge

Question: can graph and plan projections agree without a second semantic rule?
`disposition_route_projection.hpp` derives route semantics from independently
parsed/validated existing plans and compares independently supplied graph node
bindings, source endpoints and the bridge. A standalone versioned bundle is
consumed by `flowvalidate` and the shared API. Both reject executable claims.
A non-executable source graph v1 retains existing string wire/endpoint shape;
plan numeric wires remain unchanged. Unresolved provider-atom graph nodes in
fixtures are declarative placeholders, not compiled ordinary functions.

The explicit map binds graph identity/provenance, producer function/operation/
disposition, lane/type, both wire representations, exact destination and scope,
consumer/transition or containment authority, policy/revision, authorized route
set and correlation law. Source endpoint provenance must equal producer origin.
Bridge route IDs use distinct namespaced plan/route components. No numeric
coincidence equates operation, attempt, signal, delivery or obligation identity.

Changed files: bridge header, `validate.hpp`, CMake registration, projection
unit/CLI tests. Positive evidence: recovery plus activation fault round-trip.
Hostile evidence: missing/extra/duplicate/reordered/dangling/renamed/foreign
routes, lane/type/port/scope/operation/disposition/function/policy/commit/origin
mutations, wrong containment, future versions and ready claims. Normal optional
outputs are not made mandatory. Validation proves consistency with supplied
semantic authority, not authenticity of a source producer. No scheduler or
backend consumes this as an executable graph.

## Stage 2 checkpoint — BLOCKED; independent diagnostic work complete

Question: does existing source establish the complete association?
No admitted source form supplies the required producer-to-consumer association,
read-only envelope parameter and transition/commit projection. The exact
alternatives and recommendation are in the [decision brief](2026-10-09-disposition-lanes-decision-brief.md).
No new parser spelling or inferred source semantics were implemented.

Changed files: `disposition_diagnostic.hpp`, bridge diagnostic adapter,
`flowvalidate.cpp`, shared tests. Six stable codes carry phase/classification,
source, operation/disposition/type, route/missing endpoint, boundary, cause,
legal repair classes, redaction and explicit no-attempt state. Human rendering
uses the same machine fact. Five bridge refusal classes are exercised through
the real CLI; unsupported-source is a shared diagnostic projection only.
Artifact-origin locations are explicitly unverified. Missing identities remain
-1/unknown rather than being invented. This does not pass the mission's original
source/import diagnostic integration requirement, or runtime error-line gate.

## Stage 3 checkpoint — BLOCKED, not attempted

No source-derived typed response route can enter the compiler chain without the
missing Stage 2 facts. Existing TextOutcome backend tests remain regression
evidence only. There is no LLVM/TinyVM execution or dispatch/receipt parity
claim for this bridge. All source, binding, scheduling, optimization and backend
implementations retain their existing meaning.

## Stage 4 checkpoint — BLOCKED at observable activation state

The declarative fault bridge validates an exact activation scope and
`halt_and_quarantine`, never ordinary recovery. Existing reference Gate 5/6
contracts remain unchanged and tested by the canonical suite. They retain the
original expected failure as `unresolved_contained` on response fault.
Native `flow_graph_fail` exits 70; it has no receipt-bearing activation state.
No return/recovery replacement, process quarantine or top-level policy was
invented. The decision brief records the missing protocol. Source/backend
fault parity and no-continuation execution remain unproved by this mission.

## Stage 5 checkpoint — reference evidence PASS; executable gate BLOCKED

Question: when may one recovery's evidence be released?
`disposition_evidence_epoch.hpp` owns a noncopyable reference evidence container:
one original obligation, one transition, depth one, 64 KiB deterministic
serialized live evidence and a 16 KiB closure reserve. Correlation separately
names graph, signal, delivery, activation and attempt; its canonical encoding
must equal the envelope correlation. Closure independently validates the
existing receipt, original identity/commit/provenance, selected policy and
fresh response attempt. A required observer prevents release after closure.

Changed files: epoch header and shared test. Evidence covers positive closure,
open-obligation/observer retention, duplicate closure, forged and truncated
receipt/provenance/policy/correlation, over-budget admission and oversized
closure. Limit rejection preserves live evidence and never creates success.
The container is a reference API within its owner's lifetime, not a native
runtime lifetime manager. No runtime expected budget-failure/fault route or
arbitrary recursive compaction was invented. Executable Stage 5 remains blocked
with Stage 3; reference success is not the mission's final gate.

## Verification

- `./igor doctor`: PASS.
- `cmake -S . -B /tmp/lyraform-build -G Ninja`: PASS.
- `./igor build`: PASS.
- `./igor test`: **184/184 PASS**, 58.68 seconds.
- Focused `ctest --test-dir /tmp/lyraform-build -R
  '^flowcontracts_disposition_route_' --output-on-failure`: **2/2 PASS**, 0.48 seconds.
- Shared regression: **47 hostile bridge mutations**, each rejected through
  direct bundle validation and the general artifact validator; **6 diagnostic
  classes**, **12 evidence refusals**, two declarative lane round-trips.
- CLI regression: **5/5 refusal cases**, exact human/machine fact agreement,
  original supplied line 4/column 5 retained with `verified: false`.
- Clang 18 focused ASan/UBSan: **1/1 PASS**, using
  `-fsanitize=address,undefined -fno-omit-frame-pointer -g`,
  `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and
  `UBSAN_OPTIONS=halt_on_error=1`. LeakSanitizer is disabled for the managed
  ptraced environment; Valgrind independently checks leaks.
- Valgrind 3.22.0 focused probe: **0 errors**, **0 bytes in use at exit**,
  **476,916 allocations / 476,916 frees**.
- Fresh TextOutcome frontend and semantic v1/v2 analysis: PASS;
  `flowvalidate` accepts the emitted v2 lowering-plan semantic report. Function
  symbol 37 owns operation 0 in v2; the operation/obligation/span trace agrees.
- `git diff --check`: PASS. New shell test `sh -n`: PASS.
- Documentation and canonical authority guards: PASS in the canonical suite.

Logs and generated artifacts reside under `/tmp/disposition-*`; no generated
runtime output is curated into the repository. Exact publication SHAs are
recorded in the publication follow-up below and the runner's local publication
record. The selected mission is included unchanged as task-owned authority.

## Trace and remaining boundary

Actual source trace stops honestly at:
`text_outcome.flow:7:5 -> operation 0 -> disposition fact 0 -> tagged owner 38
-> operation:0:outcome -> complementary branches / recovery operation 3`.
It has no graph failure endpoint, response-envelope function, policy selection,
signal/delivery correlation or recovery closure receipt for this mission.

Independent fixture trace (not source execution):
`imported.flow:4:5 -> operation 20 / disposition 10 -> producer-0.failure
-> wire-0 <-> numeric wire 40 -> consumer 50 / route 60 / function 80
-> transition 90 / policy laboratory v1 -> obligation 200
-> graph-A / signal-7 / delivery-9 / activation-11 / attempt-13
-> response attempt-14 -> validated Success<Reading> receipt -> epoch 1 closure`.
Fault fixture: operation 21 / disposition 11 / wire 41 -> authority 51,
activation scope 71, safety policy v1, halt/quarantine. This fixture contains no
source emission or backend execution and is not a response-fault execution link.
CLI refusal diagnostics retain supplied `imported.flow:4:5` as unverified origin.

The source-association and containment decisions are the next steps. No
continuation beyond this selected mission is authorized or performed.

## Publication and runner handoff

Implementation and gate-evidence commit
`e077f6745e1974ec4d50c94af70297c166d85bf5` was pushed to
`origin/mission/canonical-disposition-lanes-wire-closure` and verified with
`git ls-remote`. Published `main` remained
`86a5c69d58bacb09f393cef2a3d4e798ac9fe3ad`. This follow-up records publication
only; it changes no tested implementation. No pull request was created.

The local runner state is `BLOCKED`, for the semantic decisions in the linked
brief, not a tool outage. Mission-owned tracked work is committed; the unrelated
untracked `meta-discusions.md` and `output/` remain preserved. The exact final
branch tip is recorded in `/tmp/disposition-publication.json` and the task's
final response, avoiding a self-referential commit hash inside this file.
