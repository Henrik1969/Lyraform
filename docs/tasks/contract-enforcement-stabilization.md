# Lyraform contract enforcement and vertical stabilization

ID: `lyraform.contract-stabilization`; revision: `1`; owner: Henrik Sørensen.
Prepared: 2026-11-01. Owning context: LyraForm. Status: planned; definition
prepared at the owner's request, implementation not started by this record.
Canonical definition: this Markdown document. The companion `.mission.json`
binds its SHA-256 and provides portable measurable criteria and dependencies.
State: `.mission-state/lyraform.contract-stabilization/1/state.json`.

## Intent, outcome and authorization

Stabilize the existing intended language by closing unsafe acceptance paths and
making existing contracts hold from actual source through observable execution.
Do not invent a new language, new contract language, new syntax or new semantics.

Henrik's 2026-11-01 instruction requests a mission from the five assessment
priorities, in order, closing holes vertically through the whole stack and
conforming to the language's design and intent. This definition scopes that
work; it is not evidence that implementation has begun or any gate has passed.
Preparation does not resume the old ConfigResolve experiment or inherit its
state. Future execution selects this exact revision and observes actual tool
permissions. Local changes necessary for the selected stabilization work are
within the mission's intended scope; commits, staging, pushes, merges, releases,
policy overrides and unrelated repository edits are not authorized here.

The measurable outcome is five passed gates below: known acceptance holes
closed; existing numeric/predicate laws consistently enforced; executable
documentation accurate; one accepted failure/recovery path executable on native
LLVM and TinyVM; and a bounded real application composition proving those same
rules together. Required executable gates cannot pass by refusing all programs.
The mission does not claim that the entire intended language is complete.

## Binding procedure and boundaries

Follow [AGENTS.md](../../AGENTS.md), [local policies](../../Policies/README.md),
[EXECUTION-001](../../Policies/EXECUTION-001.md),
[stage protocol](../development/stage-execution-protocol.md),
[convergence rule](../development/mission-convergence-rule.md),
[verification gates](../development/verification-gates.md), and
[bounded continuation](../development/autonomous-next-stage-selection.md).
Shared WORK-001, STATE-001 and AGENT-001 v1.0, GOV-001 v1.1, the Development
handbook and relevant component policies remain applicable. No exception is
introduced. Each implementation stage gets a bounded convergence addendum or
child mission directly in `docs/tasks/` before work; child evidence is assessed
here rather than importing child status.

Only existing accepted source forms and meanings may acquire implementation.
Prefer existing shared semantic facts, contracts, validators and runtime APIs.
An internal representation change is permitted only when necessary to preserve
accepted meaning, with exact version/admission compatibility and hostile tests;
it must not establish new user-visible behavior, policy power, disposition or
contract vocabulary. Reuse existing formats when they can express the fact.
If existing authority cannot determine the choice, retain explicit unsupported
admission, write a decision brief, and stop dependent work. Do not implement a
recommended semantic answer merely because it seems sensible.

No broad language profile, arbitrary FFI, generalized concurrency, new ownership
model, new collection/storage semantics, new retry/cancellation semantics,
self-hosting, full ConfigResolve port or application-specific compiler dispatch
belongs to this mission. Existing native/ABI compatibility surfaces must not
become canonical language authority by accident.

## Baseline and provenance — T0

The assessment observed `experiment/configresolve-lyraform`, HEAD
`b50534c380879a5a70a77ed5c1dd442bfc4142bd`, with extensive pre-existing dirty
work. A fresh GCC 13.3 build passed with warnings, all 193 registered tests
passed, and six finite ConfigResolve cases agreed on native and TinyVM.
These are dated observations, not inherited mission measurements.

Assessment and evidence at preparation:
`/home/henrik/Dokumenter/ChatGPT/FrankenCore/reviews/lyraform-2026-11-01/assessment.md`.
The assessment is defect evidence, not authority to define language semantics.
If unavailable to an executor, reproduce the self-contained cases below.

At execution entry, record actual branch/HEAD/available remote refs, exact dirty
paths and source hashes, tool versions, build configuration, active feature
inventory and related checkpoints. Preserve unrelated work; do not reset,
clean, stage or assume main integration. Reuse the suitable checkout unless
isolation is actually needed. Record a reproducible exact source snapshot that
includes the needed uncommitted changes; a clean checkout of HEAD alone is not
the reviewed baseline. Use independent build directories.

Create the accepted-law register before changing behavior. Preserve four
labels: accepted meaning, implemented coverage, proposed meaning, and
undefined/unsupported. Cite exact authority and enforcing component for each
numeric, refinement, guard, ownership, failure and execution claim.

## Priority and dependencies

`T0 -> T1/G1 -> T2/G2 -> T3/G3 -> T4/G4 -> T5/G5`.

This order is mandatory. Baseline reconnaissance may inspect later dependencies,
and regression evidence and truthful documentation accompany every fix. Later
implementation must not bypass a blocked earlier gate. Independent work inside
the current gate may continue. A genuinely missing semantic decision remains
visible; passing unrelated tests does not close it.

## G1 — Close the reproduced acceptance holes

**Question:** can an invalid refinement or unsafe division reach a ready plan,
executable artifact or successful publication through any applicable path?

Reproduce these exact source cases through the canonical public toolchain and
separately invocable stages:

```text
program invalid_refinement
type Positive refines int {
    invariant value > 0
}
main {
    x : Positive(-1)
    x -> return
}
```

```text
program invalid_division
main {
    a : int(1)
    b : int(0)
    a / b -> return
}
```

The assessment's first case executed -1 without enforcement. The second emitted
unchecked native `sdiv`, terminated with SIGFPE at O0, produced a non-contractual
exit value at O2, and faulted on TinyVM. An optimization-dependent numeric exit
is not an expected result to put in a golden file.

Use accepted refusal/enforcement law (ADRs 0054/0055), current refined-type
support bounds, scalar authority and source-operation coverage. Refuse refined
value use until its predicate obligations are actually implemented; recognizing
a declaration is not proof of inhabitation. Prove safe division or refuse it;
runtime-dependent division remains unsupported until its accepted enforcement
and routing are executable. Do not add a private trap/exception/default handler.

Trace construction, placement, function arguments/results, nested records,
callable lowering and graph lowering wherever currently supported. Unsupported
positions must refuse rather than skip checks. Preserve ordinary valid scalar
programs and already supported static guard behavior; demonstrate safe division
such as 6/2. Test zero, signed minimum/-1, dynamic operands and invalid refined
values at each supported boundary. Test O0/O2 native results and TinyVM with
complete values/dispositions, not only truncated process exit codes.

At every applicable artifact consumer, mutate/remove required proof, identity,
type and readiness fields and attempt compatibility-version downgrade. Required
evidence cannot be made optional by deleting it or changing an outer status.
Do not claim authentication of an entirely rewritten program artifact.

**G1 pass:** both defects reproduced then closed, zero invalid admissions in
the registered boundary matrix, positive controls preserved, required hostile
cases rejected before publication, and pipeline verification passed. A properly
explained refusal can close a safety hole; it cannot claim refinement/runtime
division feature completion. Any invalid value or unchecked division still
executable falsifies the gate.

## G2 — Consolidate and enforce existing numeric/predicate meaning

**Question:** do all admitted scalar operations and predicates have one accepted
meaning, independently preserved by every applicable consumer and backend?

Consolidate existing authority for integer widths/ranges, literals, conversions,
overflow, division/remainder, operator typing, Bool distinctions, initialization,
predicate dependencies/purity, evaluation failure and guard lifecycle. This is
an evidence-backed account of accepted law, not permission to choose missing
overflow, floating-point, termination or conversion policy. Historical runtime
behavior and LLVM instruction behavior cannot fill a semantic gap.

For each item, cite accepted authority or explicitly classify it unresolved.
Prove accepted behavior with small boundary cases and composed expressions;
reject unresolved executable paths under the existing fail-closed rule. Leave
future floating-point/variant cases unsupported where their rules are missing.
Do not equate effect-free expressions with total, safe or terminating ones.

Reuse the shared scalar/guard/disposition rules and existing typed artifacts.
Remove duplicate decisions or unchecked fallbacks only after proving replacement
coverage. Include parameter/return, record-field, branch and loop contexts where
admitted, deterministic analysis, malformed literals, stale proof and O0/O2/VM
comparison. Preserve valid cases with expected values derived from accepted law,
not from either backend as the sole oracle.

**G2 pass:** every inventoried executable numeric/predicate behavior has accepted
authority and enforcement evidence; every unresolved entry is non-executable;
zero unexplained backend differences or invalid admissions in that inventory.
If accomplishing this requires a new semantic decision, G2 stays open. Correctly
refused unresolved families must remain explicit exclusions, not disappeared
rows or implicit completion of the whole numeric language.

## G3 — Make support claims and teaching examples executable

**Question:** can a developer determine what this exact revision accepts and
executes without consulting contradictory historical records?

Reconcile the current-status landing page, canonical source-form inventory,
programmer-facing entry documentation, relevant component READMEs and mission
index. Keep separate columns for recognition, structural validity, semantic
admission, preservation, LLVM execution and TinyVM execution, including exact
limits and branch provenance. Update existing inventory mechanisms instead of
creating a competing source of truth. Historical documents remain dated.

Run every example presented as currently runnable through its advertised public
command in a clean build of the recorded source snapshot. The manual's initial
integer `print` example currently fails with `FLOWANALYST_SOURCE_OPERATION_GAP`;
correct or label it without adding output semantics to satisfy documentation.
Include at least one nearby rejected example per supported family. State backend
limitations rather than silently falling back to legacy execution.

**G3 pass:** zero advertised-current example failures, zero unsupported features
claimed executable, and no contradictory current claims in the inventoried
documents. Record exact denominator and commands; documentation checks must test
behavior and provenance, not merely the presence of expected phrases.

## G4 — Complete one already accepted failure/recovery vertical slice

**Question:** can real source produce an expected failure, invoke exactly the
authorized recovery function and rejoin with valid typed success on both backends?

Use the existing TextOutcome producer and accepted typed read-only failure
envelope, consumer, selected route, exclusive success junction, rejoin, ownership
and evidence laws. Reconcile the existing
[source-recovery mission](source-recovery-execution.md) and
[Mission 07](canonical-source-disposition-and-quarantine-execution-mission-07.md)
against the present source; reuse their accepted scope, not their completion or
authorization. Authorities include ADRs 0055–0060 and 0062–0067.

Start from source, derive facts in Flowanalyst, validate through Flowcontracts,
bind required providers, preserve graph/route identities through Flowparallel,
Flowoptimize and preparation, lower to LLVM and TinyVM, execute and inspect
result, publication, cleanup and obligation evidence. Each non-applicable stage
must have a recorded reason; it must not be silently skipped.

Required cases: normal success bypasses recovery; forced declared provider
failure invokes only the selected ordinary response; two already-authorized
policy choices select the correct identities; unselected response never runs;
recovery produces the exact promised type and rejoins once; final-owner cleanup
occurs once; original commit/provenance is preserved; wrong/missing route,
cross-attempt wire, lost obligation and illegal fault-to-recovery routing refuse.
Renaming and source-body changes must behave normally, without fixture dispatch.

Account for every declared fault using existing accepted containment. No
reachable exit/unwind or diagnostic-and-drop may substitute for the promised
activation result. If the chosen path needs still-unimplemented containment,
complete that accepted prerequisite in a bounded child stage or keep G4 open.
Do not invent a fault-free claim, recovery of integrity faults, propagation,
retry, cancellation or new carrier syntax to make the specimen run.

**G4 pass:** actual source-generated native and TinyVM executables show matching
typed results and canonical disposition/ownership observations for all cases.
A reference executor, hand-edited ready flag, C callback harness or blanket
refusal does not satisfy this gate. General failure handling remains unclaimed.

## G5 — Demonstrate stable composition in a real bounded application

**Question:** do supported records, calls, values, ownership and recovery remain
correct when combined in an application rather than isolated fixtures?

Use the existing finite ConfigResolve policy-resolution specimen as the pinned
application context. Reproduce its six existing differential cases first; then
compose its already-admitted record/Text/function behavior with G4's accepted
resource/failure/recovery boundary in a meaningful bounded consumer. Pin the
source, fixtures and expected observations before extending the specimen.
Keep application logic in Lyraform source. Native providers expose generic
capabilities only; no dispatch by module, function or fixture name in the
compiler/runtime, and no wrapper around the C++ resolver masquerading as a port.

Exercise successful and failed inputs, selected recovery, record copies and
function boundaries, ownership transfer/cleanup, repeatability and both
backends. Use only established ownership paths; general owned parameters and
additional forwarding remain excluded unless already accepted semantics and
representation make their implementation unambiguous and necessary to this
bounded endpoint. Do not expand the mission to a general ownership campaign.

Do not force general variants into this gate: ADR 0069's declaration law does
not by itself authorize inventing constructor or projection syntax. Record
unimplemented variant execution and full ConfigResolve parity as open limits.
If the selected bounded application requires new language meaning, record that
dependency and leave G5 open rather than shrinking the acceptance target after
failure. Any material target change requires a revision preserving old criteria.

**G5 pass:** one reproducible useful Lyraform application/consumer passes the
pinned original and composed cases on both backends, including failure/cleanup
observations. Measure compile time, artifact size, runtime and evidence storage
for stated finite workloads; report raw observations and limits without inventing
performance targets or claiming unmeasured scalability. No full library-port or
production-readiness claim follows.

## Single ownership, reuse and simplification

| Fact | Owner and downstream duty |
|---|---|
| Syntax and completeness | Existing structural frontend; no semantic invention |
| Types, value admission, guards, effects, obligations and legal routes | Flowanalyst with the shared accepted semantic rules |
| Artifact consistency and required evidence | Flowcontracts; validate meaning already established, fail on gaps |
| Provider authorization | Flowbind under existing exact capability/ABI/effect contracts |
| Scheduling freedom | Canonical graph/effects; policy cannot enlarge that freedom |
| Preservation and transformations | Flowparallel, Flowoptimize, preparation; record exact preservation or proved transformation |
| Execution | Native LLVM and TinyVM implement the same admitted meaning |
| Failure containment and publication | Existing activation authority/host-policy boundaries, never invented backend behavior |

At each gate identify reused mechanisms, newly necessary code and removable
duplicate checks, stale claims or transitional paths. Retire only what is owned
by this mission and proven replaceable; preserve historical evidence. Every
closure report must answer what became usable, what remains unsupported, what
became simpler, and which evidence could have disproved the claim.

## Verification and completion

Run focused positive/negative/hostile tests while iterating. At gates run the
canonical `./igor doctor`, `./igor build`, `./igor test` with a correctly owned
build context, plus relevant AST/SymbolTable/category gates and diff checks.
Use the existing native-pass/support boundary runner where required. At greater
architectural border closure, run Tier 3 Firetest: fresh GCC and Clang builds,
ASan/UBSan, meaningful Valgrind probes, support-inclusive tests, repeated
determinism checks and supported documentation checks. Explain unavailable tools
and use established substitutes; unexplained skips cannot pass a required gate.
The initial 193-test observation is not a fixed future test count.

Retain exact source snapshot/revision, commands, tools, counts, failures,
exclusions, source-to-fact-to-consumer trace and observable outputs. For native
versus VM comparison, compare full values, disposition and cleanup evidence;
process exit status alone is insufficient. Use adversarial evidence capable of
falsifying shared assumptions, not only validator self-consistency.

Required completion measurements C1–C5 correspond exactly to G1–G5. All five
must pass with intact evidence. Missing prerequisites, unsupported mandatory
execution, unavailable required verification or unresolved language decisions
remain open; unknown is not passed. A refusal may establish safe non-admission,
not satisfy G4/G5 execution or erase supported positive controls.

Fresh state begins planned, all tasks pending, all criteria unmeasured. Retain
revision history and exact mission digest under STATE-001; do not import old
mission state or mark these goals passed from the assessment. Durable gate
checkpoints belong in `docs/checkpoints/`; mission evidence remains portable.
Consolidate each gate before proceeding in priority order. Finish with a scoped
stabilization report and remaining exclusions, not a claim of a new language.
