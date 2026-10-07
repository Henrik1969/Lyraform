# General Effectful Parallel Scheduling — Mission 01

Date: 2026-10-07.

**Execution status:** Gate 1 PASS in the working tree based on synchronized
`main` at `16a1cfff6e560ed343286af0f9f604edbb3d02e5`. The durable evidence is
recorded in
[`2026-10-07-general-effectful-parallel-scheduling.md`](../checkpoints/2026-10-07-general-effectful-parallel-scheduling.md).

## Objective

Establish the first general, backend-independent authority for deriving and
executing a bounded effectful parallel schedule from ordinary Lyraform graph
semantics, without adding parallel, thread, worker, or `async` source syntax.

The implementation must be general in architecture rather than universal in
effect coverage: the first executable slice may admit only one evidence-backed
effect/resource family, but it must use shared effect, conflict, ordering,
policy, and disposition contracts rather than application names or backend
special cases.

## Baseline

- canonical branch: `main`;
- begin from the then-current synchronized `main` and record the exact HEAD;
- architectural authority:
  [ADR 0064](../architecture/decisions/0064-graph-derived-parallelism-and-policy-scheduling.md);
- existing bounded evidence: Flowanalyst pure-call independence,
  `parallel_independent_v1` dependency waves, native worker execution, TinyVM
  parity, and Flowparallel capability/calibration planning;
- preserve all unrelated failure-flow work already present in the worktree;
- do not touch `master` or `flowlfs-v0.1-alive`.

## Bounded question

What exact canonical proof allows two already-admitted effectful operations to
execute concurrently while preserving the same graph-visible values, effect
order, failure dispositions, commits, provenance, and deterministic reference
result as an admitted serial execution?

## Reconnaissance baseline

The existing implementation already provides the following reusable pieces:

- Flowanalyst assigns canonical operation identities and exact external
  provider tuples and emits declared effect contracts;
- callable-plan v2 preserves call operands, results, owner function, source
  identity, ABI access/ownership facts, and provider evidence;
- shared Flowcontracts validation recomputes pure-call independence and rejects
  forged candidate relationships;
- source-graph schedule v4 derives deterministic dependency waves from graph
  topology;
- the native graph runtime executes fresh receiver activations concurrently,
  joins a complete wave, buffers results, and prevents a failed wave from
  advancing dependent work;
- TinyVM validates the same schedule and supplies the deterministic serial
  projection;
- governed scalar `readonly` external calls already have exact LLVM/TinyVM
  typed-call parity;
- Flowparallel already consumes runtime capabilities, calibration, and
  minimum-benefit policy for provider selection.

The missing authority is equally specific:

- function effects do not yet summarize external calls transitively;
- `effect: readonly` names an effect class but does not prove provider
  reentrancy or concurrent-call safety;
- no canonical resource-domain identity connects an effectful operation to the
  resource it observes;
- schedule v4 derives topology waves without effect-conflict evidence;
- native parallel lowering currently admits only return-only pure receiver
  bodies;
- current artifacts cannot prove that policy selected an effectful schedule
  from a compiler-derived legal strategy set.

Therefore arbitrary `io`, filesystem, terminal, pointer, owned-resource,
persistent-state, and irreversible operations are not valid first slices.

## Selected first executable slice

Mission 01 begins with **bounded concurrent scalar observation**:

```text
one existing scalar startup root
    -> fan-out to at least two fresh receiver activations
    -> each receiver performs one exact scalar provider observation
    -> results remain activation-local and publish only after the wave joins
```

Each admitted observation must satisfy all of these conditions:

- exact provider capability identity and evidence are already authorized;
- declared effect is `readonly`;
- parameters and result use admitted by-value scalar carriers only;
- no pointer, buffer, descriptor, owned, borrowed, opaque, aggregate, or output
  parameter participates;
- the provider effect profile binds an exact resource-domain identity and the
  `concurrent_observation_v1` guarantee to that capability identity;
- the receiver is fresh, non-persistent, acyclic, and contains only the
  bounded provider call and return projection needed for the observation;
- data dependencies, result destinations, and graph wires do not conflict;
- the provider call has no admitted failure result. Unexpected worker/provider
  failure retains the existing no-partial-wave-publication fault behavior.

The focused fixture must use a deterministic injected provider with two
immutable scalar observations. Host calls such as `getpid` and `getuid` may be
used for existing parity regression, but environmental observations are not
the differential oracle for this mission.

This slice is effectful because it observes an external provider/resource. It
is intentionally not yet mutable or fallible effect execution.

## Governing law

```text
source declares operations, wires, effects, resources, and constraints
    -> semantic authority resolves exact identities
    -> graph authority derives dependency and conflict relations
    -> policy selects only a proved legal schedule class
    -> runtime capability and calibration select a provider
    -> backend executes without changing semantic meaning
```

Provider availability, worker count, accelerator presence, or measured speedup
is never evidence of semantic independence.

## Scope

- confirm the inventoried effect facts, missing resource identities, graph
  operations, schedule artifacts, policy inputs, and runtime planner evidence
  against the recorded reconnaissance baseline;
- implement the selected bounded concurrent-scalar-observation family without
  broadening it to adjacent effect classes;
- define versioned, backend-neutral access and conflict facts for that slice;
- derive ordering edges, independent sets, joins, and deterministic waves from
  those facts;
- establish explicit pre-execution, in-flight, commit, abort, and failure
  dispositions;
- preserve scheduling authority through Flowparallel, Flowoptimize, Flowlower,
  LLVM, and TinyVM or explicitly refuse a backend that cannot implement it;
- retain a deterministic serial reference projection;
- allow policy and measured feasibility to choose serial or parallel execution
  only after semantic legality is proven;
- emit inspectable scheduling and execution provenance.

## Required scout classifications

Classify current behavior as:

```text
canonical effect fact
canonical resource identity
derived graph dependency
policy choice
runtime capability/calibration evidence
backend mechanism
legacy or accidental behavior
undefined
```

The scout must confirm the exact integration points and that no existing
resource identity is being mistaken for one. If the reviewed provider profile
cannot establish resource identity and concurrent-observation authority
without inventing source scheduling meaning, stop with a decision brief before
implementation.

## Minimum canonical contract

The selected design must represent facts equivalent to:

```text
EffectAccessFact
    operation identity
    owner function and activation identity
    exact provider capability identity
    effect identity
    resource identity
    access class
    concurrency guarantee
    origin/provenance

EffectConflictFact
    first operation identity
    second operation identity
    conflict or independence proof
    required ordering, if any
    proof provenance

EffectScheduleFact
    graph identity
    deterministic waves/joins
    policy-authorized strategy set
    serial reference projection
    failure and commit contract
```

Names and serialization may differ if the existing code supports a cleaner
equivalent. The facts must have one canonical producer and independent
validation at every consumer.

The first serialized profile should be equivalent to:

```text
flowcore.provider_effect_profile/v1
    exact capability identity
    exact provider evidence identity
    effect: readonly
    resource_domain: <stable nonempty identity>
    access: observe
    concurrency: concurrent_observation_v1
    failure: infallible_scalar_v1
```

This profile is reviewed provider-contract evidence. It is not deployment
policy and cannot authorize execution. Binding policy must still authorize the
exact provider tuple independently. Deployment scheduling policy can use the
profile only after semantic analysis has incorporated and validated it.

## Implementation sequence

### Stage A — shared effect-scheduling contracts

Add one shared Flowcontracts component for provider effect profiles, effect
access facts, pairwise conflict proofs, and schedule facts. Establish bounds,
unique identities, exact capability/evidence linkage, and deterministic JSON
round trips.

Likely owned paths:

- `Flowcontracts/include/flowcontracts/effect_scheduling.hpp`;
- `Flowcontracts/tests/effect_scheduling_tests.cpp`;
- native-binding effect-profile schema and example under
  `docs/architecture/schemas/`;
- focused Flowcontracts/CMake registration.

Gate A: malformed, duplicate, unbound, unsupported, or contradictory profile
and access facts fail independently of Flowanalyst.

### Stage B — canonical producer and transitive receiver summary

Teach Flowanalyst to consume the reviewed provider-effect profile as an
explicit input, bind it to the exact provider capability/evidence tuple, and
emit one canonical access fact for each admitted external operation. Derive a
receiver-function effect summary from its canonical operations rather than
from function names or body text.

For Mission 01, the only effectful summary admitted to parallel consideration
is one bounded scalar observation. Pure receiver behavior remains unchanged;
unknown, mixed, nested, mutable, fallible, or unprofiled effects remain
non-parallelizable.

Gate B: every effectful receiver activation either names an exact validated
access summary or is explicitly excluded before scheduling.

### Stage C — effect-aware graph conflict derivation

Extend source-graph scheduling so schedule v4 cannot label a topology wave
`independent` from topology alone. For each pair in a candidate wave, derive
and record:

```text
data dependency relation
effect/resource relation
conflict or independence proof
required ordering, if any
```

Two Mission-01 observations may share a wave only when both have exact
`concurrent_observation_v1` evidence. Missing or forged evidence refuses the
requested parallel schedule. Serial scheduling remains legal when selected by
policy; an explicitly required parallel schedule does not silently downgrade.

Gate C: schedule recomputation from canonical graph/access facts is
deterministic, and every consumer rejects a changed wave or conflict proof.

### Stage D — preservation and policy boundary

Carry the exact effect profile, access facts, conflict proofs, and legal
strategy set through Flowparallel, Flowoptimize, and backend preparation.
Introduce no semantic decision in those stages. Flowparallel may combine the
legal strategy set with deployment policy, runtime capabilities, and verified
calibration, but it may not broaden the set.

Gate D: hostile mutations at every carrying stage fail closed, and serial
selection preserves the same canonical facts as parallel selection.

### Stage E — native effectful worker execution

Extend the existing LLVM graph worker lowering to admit the bounded receiver
summary instead of requiring every worker body to contain only `return_value`.
Reuse the ordinary typed external-call lowering inside the worker function;
do not add a second ABI dispatcher or scheduler.

The runtime must:

- launch only activations already grouped by the validated schedule;
- join the complete wave;
- publish buffered results in deterministic activation order;
- publish no dependent-wave result after any worker fault;
- record operation, activation, provider, resource-domain, policy, worker, and
  result/fault provenance;
- retain existing worker and activation bounds.

Gate E: actual overlapping execution is observed with a synchronization probe,
while serial and parallel runs produce identical graph-visible values and
ordered publication evidence.

### Stage F — TinyVM reference projection

Preserve and validate the same effect-aware schedule in TinyVM. Mission 01 does
not require TinyVM to create host threads: its deterministic serial execution
is the reference projection, provided it executes each activation exactly once
and preserves the same values, dispositions, and provenance identities.

Gate F: LLVM parallel execution and TinyVM serial projection pass exact
normalized parity; a forged effect profile, conflict proof, schedule, provider
tuple, or resource identity is rejected before execution.

### Stage G — assurance and closure

Add the focused end-to-end fixture and hostile matrix, update the authority and
safety inventories, and run the canonical and sanitizer/Valgrind gates.

Expected principal test paths:

- `tools/test-effectful-parallel-graph.sh`;
- shared-contract unit tests;
- Flowanalyst positive/refusal cases;
- canonical authority-preservation mutations;
- native LLVM/TinyVM differential parity;
- worker overlap, launch-failure, allocation-failure, and repeated-determinism
  probes;
- scheduling-admission drift checks.

Gate G is Mission 01 Gate 1.

## Safety laws

- Unknown resource identity or aliasing is a conflict, not permission to run.
- Writes to the same resource do not run concurrently without a separately
  proven commutative or transactional contract.
- Irreversible effects remain ordered or refused until their partial-work and
  recovery law is complete.
- Failure cannot publish successful dependent work or silently discard a
  committed effect.
- A requested stronger schedule cannot silently downgrade when policy requires
  that exact strategy; an optional policy may select serial explicitly.
- Serial and parallel executions must agree on all graph-observable results and
  dispositions admitted by the selected contract.
- Diagnostic observation is not a synchronization or recovery mechanism.
- A provider's `readonly` effect declaration alone is insufficient concurrency
  authority; exact resource and concurrent-observation evidence is mandatory.
- Provider-contract evidence and deployment policy remain separate. Neither
  can substitute for the other.

## Required evidence

Positive evidence:

- two profiled scalar observations are derived as independent without source
  scheduling annotations;
- a conflicting, unknown, or unprofiled peer cannot enter a requested parallel
  wave and produces an exact refusal rather than an invented ordering;
- policy can select serial or parallel from the same canonical graph;
- runtime calibration can reject an unprofitable provider without changing
  semantics;
- the admitted backend executes the parallel schedule and matches the serial
  reference result and provenance.
- a controlled probe proves that two admitted observations overlap in native
  execution rather than merely appearing in the same artifact wave.

Negative and hostile evidence:

- unknown, missing, forged, duplicated, or aliased resource identity;
- forged independence or removed ordering edge;
- read/write and write/write conflict;
- failure before effect, during effect, before commit, and after any admitted
  commit boundary;
- worker launch/provider loss and partial initialization;
- unsupported irreversible, nested, reentrant, cancellation, retry, or
  backpressure requests;
- contradictory policy, capability, calibration, and schedule artifacts;
- `readonly` calls lacking exact concurrent-observation or resource-domain
  evidence;
- pointer-bearing, owned, borrowed, opaque, mutable-output, aggregate, and
  fallible provider calls presented as Mission-01 observations;
- backend attempts to execute a schedule stronger than canonical authority.

Verification must include deterministic artifact replay, serial/parallel
differential tests, repeated stress execution, applicable sanitizer and
Valgrind gates, and the canonical `igor doctor`, `igor build`, and `igor test`
gates with exact results.

## Explicit non-goals

- no `async`, `parallel`, thread, worker, lock, or scheduler source syntax;
- no manual source placement onto CPUs, cores, GPUs, queues, or event loops;
- no inference of semantic safety from benchmarks or hardware capability;
- no automatic retry, hidden cancellation, exception unwinding, or implicit
  failure propagation;
- no general shared mutation, arbitrary native ABI/FFI, distributed execution,
  or irreversible-effect parallelism;
- no parallel file, descriptor, terminal, network, clock, randomness, mutable
  output, allocation, cleanup, or failure-producing provider operations;
- no weakening of existing pure-graph, ownership, failure-flow, or provider
  admission laws;
- no application-specific compiler dispatch;
- no self-hosting expansion.

## Gate 1

PASS only when one bounded effectful family crosses the complete canonical
chain from resolved effect/resource identity through independently validated
conflict proof, policy-authorized schedule, measured provider selection, and
serial-equivalent execution with explicit failure/commit evidence.

BLOCKED is the correct result if the selected family lacks canonical resource
identity, aliasing law, commit boundary, or failure disposition. Record the
smallest missing semantic decision rather than inventing it in the scheduler.

## Deliverable

A durable checkpoint containing the baseline, classifications, selected first
effect family, authority model, implementation, positive/negative/hostile
evidence, backend results, untouched scope, residual risks, and Gate 1 result.
