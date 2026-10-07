# Lyraform current status

**Current:** this page is the landing point for present project truth.
Exact verification totals and timings belong to the linked dated checkpoint,
not to evergreen orientation documents.

```text
project:       Lyraform
authority:     main
toolchain:     Igor
lineage:       v0.29 reusable native language chain
maturity:      experimental / unstable / not production-ready
certification: none claimed
```

## Latest verification checkpoint

The latest current-truth verification is
[Canonical convergence integration — 2026-10-07](checkpoints/2026-10-07-convergence-integration.md).
It verifies the accumulated canonical compiler campaign, declarative failure
consumer, accepted envelope/evidence laws, current documentation, and
executable drift guards as one coherent integration. The earlier
[current-truth consolidation](checkpoints/2026-09-26-current-truth-consolidation.md)
remains the detailed authority for the original documentation landing page.

## Latest semantic checkpoint

The latest integrated compiler semantic checkpoint is
[Canonical authority envelope — 2026-09-27](checkpoints/2026-09-27-canonical-authority-envelope.md).
It records the Stage 4B proof that every fact in the current callable-plan-v2
semantic envelope is preserved across active carrying stages, consumed only by
a separately validated authorization branch, or remains explicitly refused.
The earlier stage checkpoints below remain the detailed authority for each
bounded semantic slice. The
[bounded unique-transfer forwarding checkpoint](checkpoints/2026-09-26-bounded-unique-transfer-forwarding-chain.md)
remains the focused execution evidence for that ownership slice.

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
records the exact evidence and boundary. Source projection, compiler-plan integration, general
graph behavior, LLVM/TinyVM execution, response-attempt failure, propagation,
retry, sinks, and runtime guard failure routing remain refused.
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
  exact-identity expected-failure recovery or transformation;
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
