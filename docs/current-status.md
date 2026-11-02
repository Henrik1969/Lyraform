# Lyraform current status

## Stabilization work in progress — 2026-11-01

[G1 passed on its preserved snapshot](checkpoints/2026-11-01-contract-stabilization-g1.md); G2 is active. The safety checks
now admit only the bounded R1/R2 scalar refinement lifecycle: direct
construction and assignment whose predicates are statically proved; other
value-bearing refinements with unenforced predicates remain refused. Integer
quotients (`div`) whose safety
cannot be established and has no admitted failure route remain refused. The
bounded numeric slice now also admits `+`, `-`, `*`, and
true division (`/`) with explicit integer-to-`float64` promotion; static zero
divisors remain refused. The accepted cases execute on LLVM and TinyVM.

The historical pass corpus now reports these contract refusals separately.
The two `flow_less` integration tests currently verify admission refusal, not
successful paging or terminal interaction: provider-dependent division in the
pager lacks an admitted numeric failure route. Their original execution checks
remain in place for when that source is legitimately admitted. A green test
entry is not a claim that these applications currently execute.

Owner clarification: integer overflow must follow an explicitly authorized
conversion route to a wider integer type or fail. Silent wrap, truncation, or
implicit widening is not permitted. Missing failure routing cannot be repaired
by a backend-selected trap or default handler. [ADR 0071](architecture/decisions/0071-explicit-integer-width.md) now requires
explicit-width concrete integer types and rejects bare `int`. Active examples
and tests using bare int require migration; the earlier G1 test counts do not
claim that the current migration passes the full suite. The [entry-result model](architecture/decisions/0073-explicit-entry-result-host-mapping.md)
is now approved: an explicitly typed language result and a separately selected
host mapping. Its concrete declaration and implementation remain under review.


Explicit integer conversion checks now enforce
[ADR 0072](architecture/decisions/0072-explicit-value-preserving-integer-conversion.md).
Typed initialization preserves conversion intent; assignment, call, return and
mixed arithmetic cannot silently change integer identities. Signed int32/int64
widening and proved narrowing have native/TinyVM regression coverage. Unknown
narrowing remains refused without an admitted failure route. The broad G2
migration and its integration tests remain open.


Current identity: **Lyraform**. Canonical development directory: **LyraForm/**.
See [placement and historical naming policy](../Policies/PLACEMENT-001.md). Earlier names and
version labels are historical evidence, not competing current authority.

**Current:** this page is the landing point for present project truth.
Exact verification totals and timings belong to the linked dated checkpoint,
not to evergreen orientation documents.

```text
project:       Lyraform
authority:     main
toolchain:     Igor
historical lineage: v0.29 reusable native language chain
maturity:      experimental / unstable / not production-ready
certification: none claimed
```

## Latest verification checkpoint

The latest current-truth verification is the
[2026-11-02 refinement assignment vertical slice](checkpoints/2026-11-02-refinement-assignment-slice.md),
which preserves the R1 construction proof when a refined scalar local is
assigned a new statically valid value. Invalid and unknown assignments remain
refused. The preceding
[2026-11-02 refinement construction vertical slice](checkpoints/2026-11-02-refinement-construction-slice.md),
which admits one statically proved scalar refinement construction through LLVM
and TinyVM, remains the construction authority. The
[unreachable-invalid-branch vertical slice](checkpoints/2026-11-02-unreachable-invalid-branch.md),
which locks the whole-program semantic admission boundary, remains the
preceding control-flow checkpoint. The
[numeric promotion vertical slice](checkpoints/2026-11-02-numeric-promotion-slice.md)
records the focused cross-backend arithmetic evidence and the current
whole-suite result of 160/198 tests; the repository remains non-green during
the explicit-width and integration migrations. The earlier
[Lyraform language-maturation publication](reports/2026-11-02-lyraform-language-maturation-publication.md)
records the earlier 166/196 measurement. Neither is a claim of language-
maturation completion.

The preceding
[Gate 2 and hygiene convergence integration — 2026-10-10](checkpoints/2026-10-10-gate2-hygiene-convergence-integration.md)
remains the authority for the integrated `main` snapshot. It verifies the
complete mission/workspace hygiene authority together with the source-derived
disposition topology through Gate 2. The reviewed branch was merged into
`main` with ordinary ancestry after clean and fresh-clone proof.
The earlier
[canonical convergence integration](checkpoints/2026-10-07-convergence-integration.md)
remains the detailed verification of the preceding compiler campaign, and the
[current-truth consolidation](checkpoints/2026-09-26-current-truth-consolidation.md)
remains the detailed authority for the original documentation landing page.

## Latest semantic checkpoint

The latest integrated compiler semantic checkpoint is
[Source-declared disposition authority — Gate 2](checkpoints/2026-10-10-source-disposition-authority-gate-2.md).
It records one source-derived, closed, typed and deterministic disposition
topology for the bounded `TextOutcome` specimen. The topology remains
explicitly declarative: policy selection, executable Graph IR, backend routing
and runtime containment are not admitted.
The earlier stage checkpoints below remain the detailed authority for each
bounded semantic slice. The
[bounded unique-transfer forwarding checkpoint](checkpoints/2026-09-26-bounded-unique-transfer-forwarding-chain.md)
remains the focused execution evidence for that ownership slice.

### Active bounded review work

The selected [source-disposition policy-selection Gate 3 mission](tasks/source-disposition-policy-selection-gate3.md)
is confined to the review branch `codex/disposition-policy-selection-gate3`.
It binds deterministic one-route and explicitly supplied multi-route policy to
the exact Gate 2 source topology and admits only ADR 0067's initial
`report_and_fail` host action. The artifacts remain declarative, lowering stays
blocked, and this branch does not claim Gate 3 is integrated into `main`. Its
[review checkpoint](checkpoints/2026-10-13-source-disposition-policy-gate-3.md)
records the exact clean and hostile evidence.

## Canonical convergence campaign

The compiler-island convergence work is governed by
[Canonical language authority convergence](tasks/canonical-language-authority-convergence.md).
[Gate 0](checkpoints/2026-09-26-canonical-authority-convergence-gate-0.md)
records the machine-readable authority inventory and executable freeze guard.
[Gate 1](checkpoints/2026-09-26-canonical-source-operation-coverage.md)
adds fail-closed statement-to-operation accounting and the first bounded Igor
path through the canonical staged compiler chain.
[Stage 2A](checkpoints/2026-09-26-canonical-scalar-operation-authority.md)
connects bounded scalar semantic facts to lowering-operation and target
projection identities and types.
[Stage 2B](checkpoints/2026-09-26-canonical-target-producer-audit.md)
confirms that target facts already have one producer and enforced operation,
scalar, member-chain, type, and provenance relationships.
[Stage 2C](checkpoints/2026-09-26-canonical-guard-disposition-authority.md)
connects bounded static guard state and transition evaluation to canonical
scalar destination identity and audits exact disposition linkage.
[Stage 2D](checkpoints/2026-09-27-canonical-text-outcome-ownership-authority.md)
unifies the producer-derived TextOutcome obligation identity and owned carrier
type across disposition, direct return, and one-hop forwarding evidence.
[Stage 2E](checkpoints/2026-09-27-canonical-provider-effect-authority.md)
requires the declared provider/effect requirements to exactly equal the
provider identities used by canonical operations and graph providers before
Flowbind may apply authorization policy.
[Stage 3A](checkpoints/2026-09-27-canonical-operation-connectivity.md)
audits the existing operation links and makes callable-plan-v2 source-call
projections exact, operation-identified, and fail-closed against drift.
[Stage 3B](checkpoints/2026-09-27-canonical-parallel-candidate-connectivity.md)
connects existing deferred pure-call independence evidence to exact canonical
operation identities without admitting scheduling or runtime parallelism.
[Stage 4A](checkpoints/2026-09-27-canonical-authority-preservation.md)
preserves and revalidates that callable-plan-v2 authority through Flowparallel,
Flowoptimize, prepared backend artifacts, and Flowlower.
[Stage 4B](checkpoints/2026-09-27-canonical-authority-envelope.md)
extends that proof to the complete current callable-plan-v2 authority envelope,
including source, target, ABI, scalar, guard, disposition, and ownership facts.
[Gate 5](checkpoints/2026-09-27-whole-language-authority-audit.md)
classifies every current typed source kind and its bounded variants as
canonical-connected, deprecated oracle-only, explicitly unsupported, refused,
or historical, with an executable drift guard.
[Legacy oracle deprecation](checkpoints/2026-09-26-legacy-oracle-deprecation.md)
records and tests the authority boundary between canonical Lyraform and the
retained historical implementation.
Gate 5 completes this campaign. The language surface remains frozen until an
explicit post-campaign decision selects the next semantic family; completion
does not itself authorize new syntax or meaning.

The accepted post-campaign direction for expected runtime failure is
[typed failure consumers with policy-selected routing](architecture/decisions/0062-typed-failure-consumers-and-policy-routing.md).
Developer-defined ordinary functions provide the available recovery,
transformation, retry-request, and propagation behavior. Policy may select only
among routes already authorized by canonical semantic and consumer contracts.
The first
[declarative failure-consumer authority checkpoint](checkpoints/2026-09-27-canonical-failure-consumer-authority.md)
establishes a versioned, closed-set consumer and policy-selection contract over
exact ordinary function identities. Each selected function receives the typed
failure-envelope projection carrying the producer's obligation, attempt,
commit, and provenance evidence rather than a naked payload. The accepted
[response-transition decision](checkpoints/2026-09-27-failure-response-disposition-decision-brief.md)
keeps the developer function ordinary and places transition meaning in a
separate canonical contract. The bounded
[response-transition Gate 2](checkpoints/2026-10-07-canonical-failure-response-transition.md)
implements that shared declarative authority for exact `recover` and
`transform` transitions over every authorized consumer route. It closes an
original obligation only through typed success or replaces it with a
parent-linked typed failure while preserving origin commit and provenance
laws. [ADR 0063](architecture/decisions/0063-bounded-disposition-evidence-epochs.md)
requires evidence to remain live while an obligation depends on it, permits
compaction only at proven closure boundaries, and forbids silent truncation at
policy limits. The first
[bounded serial executable failure-flow mission](tasks/canonical-executable-failure-flow-mission-01.md)
now composes those still-declarative authorities into one separately versioned
shared execution plan. One established `Failure<E>` envelope crosses one
explicit typed wire to one exact policy-selected function identity; successful
response completion emits an independently validated recovery or
transformation receipt. Its [Gate 1 checkpoint](checkpoints/2026-10-07-canonical-executable-failure-flow.md)
records the exact evidence and boundary. Mission 02 extends the same reference
execution law to a finite producer failure set: producer and consumer closed
sets must match, every type receives one explicit wire and one exact
policy-selected route, and a concrete envelope can dispatch only through that
prevalidated identity. Its
[Gate 2 checkpoint](checkpoints/2026-10-07-canonical-failure-flow-closed-set.md)
records complete coverage and hostile evidence. Source projection,
compiler-plan integration, general
graph behavior, LLVM/TinyVM execution, response-attempt failure, propagation,
retry, sinks, and runtime guard failure routing remain refused.
Mission 03 additionally admits one fixed serial `transform -> recover`
reference chain. Its explicit handoff binds the transforming transition to the
successor plan's disposition, operation, type, obligation, payload,
correlation, commit evidence, and provenance; arbitrary-depth propagation and
retention-policy formats remain absent. The
[Gate 3 checkpoint](checkpoints/2026-10-07-canonical-failure-flow-chain.md)
records its execution and hostile evidence.
Mission 04 closes one bounded response-attempt failure path. A recovering
response may itself produce one declared expected failure with a distinct
obligation while the original remains open. One exact recovery plan may rejoin
only with the original promised result type; faults, undeclared host escapes,
recursive response failure, and inferred rejoin remain refused. Its
[Gate 4 checkpoint](checkpoints/2026-10-07-canonical-response-attempt-failure.md)
records the executable and hostile evidence.
ADR 0065 establishes that a fault-containment authority halts and quarantines
its exact declared scope, suppresses normal publication, and permits no local
continuation. Mission 05 implements the first activation-scope reference
projection with one exact fault wire, authority dispatch, policy selection,
and independently validated containment receipt. The receipt accounts for the
fault but never reclassifies it as success or ordinary failure. Its
[Gate 5 checkpoint](checkpoints/2026-10-08-canonical-fault-containment.md)
records the bounded execution and hostile evidence. Broader scopes,
escalation, trusted restart, source/compiler/backend integration, and
top-level termination policy remain refused.
Mission 06 connects one selected response attempt's declared fault to that
authority. The response fault receives a distinct obligation and exact
containment route; the original expected-failure obligation remains
`unresolved_contained`, and the composite receipt proves that the quarantined
activation produced no normal publication, continuation, or rejoin. Its
[Gate 6 checkpoint](checkpoints/2026-10-08-canonical-response-fault-containment.md)
records the link and hostile evidence.
The provisional
[semantic--syntax--graph projection map](architecture/failure-flow-semantic-syntax-graph-map.tex)
places those unresolved source and graph choices beside the canonical facts
without admitting a spelling or executable route.

The Flowmini parser, `ModuleSpec`, and direct runtime are deprecated legacy.
They remain available through `igor run-legacy` only as behavior and regression
oracles. They cannot authorize canonical source, artifacts, semantics, or
execution. `igor run` is the fail-closed canonical path.

For any checkout, discover and run that revision's actual test graph:

```sh
./igor check
./igor test
```

Do not infer a checkout's registered test total from an older document.

## Implemented semantic boundary

- shared structural frontend and canonical semantic facts;
- complete source-statement accounting before callable-plan-v2 execution;
- bounded scalar and member-target authority;
- named guard activation/deactivation with bounded static proofs;
- explicit refusal, failure, and fault categories;
- no dangling admitted failure or fault disposition;
- canonical disposition facts preserved and validated across compiler stages;
- one shared serial reference execution projection for an explicitly wired,
  exact-identity expected-failure recovery or transformation, including
  complete finite closed-set routing and one fixed transform-to-recovery
  successor chain;
- bounded `Outcome<Text,TextFailure>` accounting with success-only payload use
  and exactly-once cleanup;
- one uniform unique-ownership transfer law;
- one executable direct-return projection, extended through at most one
  nullary forwarding owner, followed by complete local accounting at the entry
  caller;
- compiler-derived pure-operation independence and deterministic graph waves,
  with bounded native worker execution and TinyVM parity selected through an
  external schedule policy rather than source syntax;
- runtime provider planning that keeps capability discovery, calibration, and
  minimum-benefit policy separate from semantic legality;
- governed LLVM and TinyVM execution for the admitted slices.

Parallelism is not a Lyraform source feature. Developers declare ordinary
operations, wires, effects, resources, and ordering constraints. The compiler
derives legal scheduling freedom from those facts; versioned policy chooses
within that freedom; runtime capability and calibration determine feasibility.
`async` likewise names a possible execution mechanism, not canonical language
syntax. See [ADR 0064](architecture/decisions/0064-graph-derived-parallelism-and-policy-scheduling.md).
The first bounded effectful scheduling stage is implemented by
[General Effectful Parallel Scheduling — Mission 01](tasks/general-effectful-parallel-scheduling-mission-01.md).
It admits only exact, generated-evidence-backed, read-only, infallible by-value
scalar provider observations. Flowanalyst produces provider/access authority;
shared contracts derive and validate resource-conflict waves; LLVM executes a
joined native worker wave with deterministic publication; TinyVM executes the
same schedule as its deterministic serial reference projection. This is not
general effectful parallelism and introduces no source scheduling syntax.

## Explicitly unsupported

- copied or shared owned values and parallel fan-out;
- more than one forwarding owner and owned parameters;
- containers, recursion, and general path-sensitive outcome accounting;
- runtime-dependent guard execution;
- general propagation syntax and policy sinks;
- compiler/source/backend integration of the bounded serial failure-flow
  reference projection;
- operational cancellation and queue/backpressure contracts;
- effectful parallel scheduling beyond the bounded concurrent scalar-
  observation family, including mutation, fallible effects, aliasing,
  irreversible commits, and partial-failure recovery;
- asynchronous provider dispatch where it requires cancellation, suspension,
  delivery, or queue behavior not yet represented by admitted contracts;
- arbitrary native ABI/FFI;
- safety certification or production use.

Unsupported forms must fail closed. A backend may lack implementation support
for admitted semantics; it may not redefine source validity.

## Authority map

- Binding architecture: [architecture decisions](architecture/README.md).
- Current semantic-stage detail: [Flowanalyst current status](../Flowanalyst/CURRENT.md).
- Whole-language source-form disposition:
  [canonical source-form audit](architecture/canonical-source-form-audit-v1.json).
- Verification policy: [verification gates](development/verification-gates.md).
- External evaluation: [tester and critic onboarding](onboarding/README.md).
- Historical project rename: [Flowcore to Lyraform](history/FLOWCORE_TO_LYRAFORM.md).

Historical Flowcore, Flowmini, version, and test-count references remain valid
when they describe an actual historical state or compatibility contract.

## Disposition lane bridge investigation (2026-10-09)

The dedicated disposition-lanes mission adds a declarative graph/plan bridge,
bounded explanatory refusal projections and a reference evidence epoch. These
are consistency contracts, not new source admission or LLVM/TinyVM failure-lane
execution. TextOutcome remains the existing executable tagged projection.
The mission is blocked on the source producer-to-consumer association and
read-only envelope signature/commit projection; native activation quarantine
also lacks an observable receipt-bearing result boundary. See the
[seam inventory](architecture/disposition-seam-inventory-v1.json),
[decision brief](checkpoints/2026-10-09-disposition-lanes-decision-brief.md) and
[stage evidence](checkpoints/2026-10-09-disposition-lanes-and-wire-closure.md).

### Decision resolution (2026-10-10)

The semantic blocker is resolved. Source will declare closed producer
disposition sets, typed read-only failure-envelope response functions, closed
consumer membership, and explicit graph association through a typed consumer
junction. Versioned policy may select only among those source-authorized routes
and uses the same artifact whether supplied externally or embedded. Faults
return an explicit quarantined activation result and containment receipt;
command exit belongs only to an explicit host policy. See
[ADR 0066](architecture/decisions/0066-source-declared-disposition-consumers-and-policy-selection.md),
[ADR 0067](architecture/decisions/0067-explicit-quarantined-activation-results.md),
and the bounded [Mission 07](tasks/canonical-source-disposition-and-quarantine-execution-mission-07.md).

This resolves the design decision, not the implementation campaign. The
canonical parser and semantic analyzer now admit the bounded source forms and
produce the Gate 2 source topology. The selected Gate 3 review branch adds
declarative route and host-policy selection, but no production executable
policy path exists. Graph IR, LLVM, and TinyVM do not yet execute the consumer
junction; native `flow_graph_fail` still terminates compatibility paths with
`exit(70)`; and TinyVM has no executable quarantined-activation result.
General retry, propagation, cancellation, backpressure, concurrent failure
composition, and broader containment scopes remain outside the admitted
language. Lyraform remains experimental, unstable, not safety-certified, and
not production-ready. Mission 07 must prove each later gate before any of
these decisions may be described as executable support.

## Branch experiment observation — 2026-10-17

On `experiment/configresolve-lyraform`, an explicitly requested disposition graph
projection now preserves the selected recovery route, exclusive success junction,
resolved rejoin destination and activation containment through binding and plan
stages. Request it with the existing `--graph-plan-version 2` alongside the v2
lowering plan and required policy artifacts. The plan remains blocked for runtime
execution. This is partial Gate 4 work, not main integration or Gate 5/6 support.
Transformation projection still requires preserved successor authority.
See [projection contract](architecture/disposition-graph-projection.md).

The same experiment corrected ordinary-record v2 admission (unsupported literals
are refused early) and TinyVM admission (blocked plans cannot emit a misleading
entry-only executable). Ordinary record execution remains unsupported pending
its explicit construction decision. The ConfigResolve port currently has only a
bounded scalar differential probe; full library/application parity is unfinished.

2026-10-17 experiment-only envelope admission checkpoint: ordinary calls now
reject bare failure payloads where immutable failure envelopes are required;
direct same-type envelope forwarding retains semantic admission. Full suite
191/191; integer ConfigResolve probe still passes. Envelope runtime representation
and full Gate 4 remain incomplete. See
[checkpoint](checkpoints/2026-10-17-failure-envelope-admission.md).

2026-10-17 experiment-only envelope identity checkpoint: disposition graph
version 2 preserves the response parameter identity and immutable envelope
projection; shared validation checks its callable signature through preparation
and binding. Full suite 191/191; 136 hostile graph-stage mutations rejected;
96-case integer probe retained native/TinyVM parity. Runtime envelopes and
recovery execution remain unsupported. See
[checkpoint](checkpoints/2026-10-17-failure-envelope-identity.md).

2026-10-17 experiment-only activation-frame prerequisite passed: a private C
runtime retains immutable evidence, enforces exclusive publication/recovery and
returns quarantine without process exit. 192/192 CTests, two ASan/UBSan cases
(leak check replaced by Valgrind due ptrace), two clean Valgrind cases, scalar
probe retained. Source response execution is still unsupported. See
[checkpoint](checkpoints/2026-10-17-source-activation-runtime.md) and selected
[source recovery execution mission](tasks/source-recovery-execution.md).

2026-10-17 experiment recovery mission: graph version 3 preserves the complete
tagged producer return and exact owner. Native prepared-artifact lowering now
explicitly refuses blocked plans; earlier refusal had depended on an unsupported
signature. 192/192 tests and 200 hostile graph-stage mutations pass; scalar probe
retains parity. Source recovery execution is still unsupported. See
[checkpoint](checkpoints/2026-10-17-tagged-producer-return.md).

## 2026-10-17 — complete record construction accepted

Henrik adopted the bounded complete-construction proposal; ADR 0068 is now
canonical authority. The selected implementation mission is
[ordinary record construction](tasks/ordinary-record-construction.md).
Implementation is pending. Earlier record-decision-pending statements describe
their historical checkpoint, not current authority. The recovery mission remains
incomplete; its [native callable ABI checkpoint](checkpoints/2026-10-17-native-envelope-callable-abi.md)
records 193/193 regression tests without claiming whole-graph execution.

### Record construction: first implementation evidence

[Shape validation checkpoint](checkpoints/2026-10-17-record-construction-shape.md):
missing, duplicate and unknown fields are now diagnosed in local/nested
construction; consecutive record declarations are preserved. All 193 regression
tests pass. Record execution and the broader port remain incomplete; the active
record mission continues with typed values and backend support.

### Record scalar field typing

[Bounded field typing checkpoint](checkpoints/2026-10-17-record-scalar-field-types.md):
int/Bool record fields now use established scalar compatibility and operator
rules. The 193-test suite passes; general record values and execution remain
unfinished under the active construction mission.

### Record construction bounds

[Record literal bounds checkpoint](checkpoints/2026-10-17-record-construction-bounds.md):
recursive by-value record dependencies and unproved initializer calls now cause
explicit refusal for record literals. Provider-result bindings remain unaffected.
All 193 regression tests pass; typed record values and execution remain unfinished.

### Ordinary record type projection

[Type catalog checkpoint](checkpoints/2026-10-17-ordinary-record-type-catalog.md):
finite int/Bool/nested record identities and fields now survive the compilation
pipeline in a validated private catalog. Forty hostile contracts are rejected;
193/193 tests pass. The catalog is non-executable; typed value operands and
backend support remain unfinished under the active record construction mission.

### Typed record construction operands

[Construction operand checkpoint](checkpoints/2026-10-17-ordinary-record-construction-operands.md):
local nested literal records now carry typed named fields referencing the private
catalog. Shared validation rejects eleven hostile operand changes and premature
execution status. Refused coverage no longer claims executable operation IDs.
All 193 tests pass. Projection, copies and native/TinyVM execution remain unfinished.

### Typed record reads and field projection

[Projection operand checkpoint](checkpoints/2026-10-17-ordinary-record-projection-operands.md):
typed local copies and nested field reads now preserve variable/type/field identity.
Shared validation rejects ten hostile identity and type changes. All 193 tests
pass. Runtime independence, control-flow admission and backend execution remain
unfinished; the record mission and complete port stay active.

### Native record value units

[Native value checkpoint](checkpoints/2026-10-17-native-record-values.md):
private aggregate construction, copies and projection execute correctly in six
native emitter unit binaries at O0/O2. Instrumented ASan and Valgrind checks pass;
193/193 regression tests pass. The source plan remains blocked, public admission
is unchanged, and TinyVM parity/function record ABIs remain unfinished.

### TinyVM record value parity

[TinyVM value checkpoint](checkpoints/2026-10-17-tinyvm-record-values.md):
private scalar-slot records now match native unit results in both VM engines.
Copied field storage independence is checked after execution; all nine Valgrind
runs report zero errors. The 193-test suite passes. Public admission and function
record ABIs remain unfinished; the overall ConfigResolve port is not complete.

### Record construction control flow

[Control-flow checkpoint](checkpoints/2026-10-17-record-control-flow.md):
shared validation now proves local record definitions precede reads through
structured branches and loops. Eight hostile mutations are rejected, including
branch-local escapes and reads after a possibly skipped loop. All 193 tests and
the 96-case scalar probe pass. Public execution remains blocked pending remaining
admission checks; the record mission and complete ConfigResolve port stay active.

### Boolean record initializers

[Boolean initializer checkpoint](checkpoints/2026-10-17-record-boolean-not.md):
both backends now implement the established Bool `not` rule. Six record source
variants agree across native O0/O2 and both VM engines, with malformed operands
refused. All 193 tests, instrumented native ASan runs and 18 Valgrind runs pass.
Public record execution and the full ConfigResolve port remain unfinished.

### Record facts before admission

[Fact ordering checkpoint](checkpoints/2026-10-17-record-fact-ordering.md):
record catalog, local types and operands now resolve before statement admission;
serialization consumes immutable facts. Twelve pre/post reports are byte-identical
and all 193 tests pass. This removes a dependency-order barrier to executable
record admission; public execution restrictions remain in force.

### Public finite record execution

The finite scalar/nested ordinary-record bound now has an explicit execution
contract. A target-wrapped Lyraform program constructs, copies and projects
private records through the public LLVM path and produces a real binary with
exit result 12. The same optimized artifact reaches TinyVM and returns 12 with
no trap. Hostile contract mutations are rejected. Broader initializers, record
function ABIs and complete arithmetic admission remain unfinished.
### Public finite record function boundaries (2026-10-17)

Pure finite scalar/nested records now cross ordinary source function
parameters and returns in both native LLVM and TinyVM lowering. The accepted
slice includes nested `read(make())` calls with private layout and independent
value copies. Member writes, resource lifetimes, recursive records, and other
unsupported callable forms remain outside the bound; see
`docs/checkpoints/2026-10-17-record-function-boundaries.md`.

### Finite ConfigResolve fact resolution (2026-10-17)

Three scalar `Fact` records now cross a Lyraform function boundary and execute
all four bounded conflict policies, with missing and rejected-tie outcomes.
Six generated cases match native LLVM and TinyVM; the accepted scope is recorded in
`docs/checkpoints/2026-10-17-finite-fact-resolution.md`.

Record-valued returns from both sides of a conditional now use one stable
private TinyVM result tree, so branch selection preserves initialized values;
the bounded evidence is in `docs/checkpoints/2026-10-17-record-result-branch.md`.

The finite resolver also validates integer facts against a source `Policy`
record before selection; native and TinyVM agree on valid, out-of-range, and
missing cases. See `docs/checkpoints/2026-10-17-finite-policy-validation.md`.

The resolver now exposes its checked outcome through a typed `Resolution` record
(`status`, `value`, `chosen`). The source constructs each branch result once,
reads it through a function boundary, and produces exit result 33 in both native
LLVM and TinyVM (`trap:0`). This remains the finite ConfigResolve acceptance
slice; diagnostics, collections, mutation, and resource lifetimes are still
separate missions.

### Finite ConfigResolve Text records (2026-10-17)

The generic finite record bound now admits immutable `Text` fields alongside
`int` and `Bool`. A `Message{Text, Bool}` source record crosses a function
boundary and returns 7 in native LLVM and TinyVM (`trap:0`); see
`docs/checkpoints/2026-10-17-configresolve-text-value-record.md`.

### Tagged ConfigResolve `Value` boundary (2026-10-17)

The experiment is frozen after defining a generic closed tagged scalar carrier for
ConfigResolve's `Null | Bool | Int | Double | Text` value model. Ordinary
records are deliberately not being used as a fake union: they cannot enforce
one active alternative. The first proposed executable slice is `Null`, `Bool`,
`Int`, and `Text`; floating-point admission remains pending explicit policy
decisions.

The source boundary is now concrete: `type Value variant { case ... }` is
parsed as a closed declaration with five unique, typed cases. A declaration
probe accepts the generic shape and lowers an independent integer main path;
construction, checked projection, and backend execution remain deliberately
refused until their carrier semantics are implemented. The end report is
`docs/reports/2026-10-17-configresolve-lyraform-experiment-end-report.md`.
