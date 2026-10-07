# ADR 0064: Parallel opportunity is derived from the graph

## Status

Accepted architectural law on 2026-10-07.

## Decision

Lyraform source does not acquire `async`, thread, worker, or parallel-placement
syntax. A developer describes ordinary operations, ports, wires, effects,
resource identities, ordering requirements, and dispositions. The compiler
derives legal independence, dependencies, joins, and conflicts from that
canonical graph evidence.

Execution strategy is a separate projection:

```text
canonical graph and effect facts
    -> compiler-derived dependency/conflict proof
    -> versioned scheduling-policy decision
    -> runtime capability and calibration evidence
    -> provider-specific execution schedule
```

Policy may permit, require, limit, or refuse an execution strategy only within
the freedom proved by the graph. Runtime capability discovery and measurement
may select among policy-authorized providers. Neither policy nor a provider may
declare dependent or conflicting operations independent, remove an ordering
edge, invent a join, or change failure and commit meaning.

Serial, worker-parallel, accelerator, asynchronous-dispatch, and distributed
execution are therefore execution mechanisms. They are not distinct source
meanings. Every admitted strategy must preserve the same observable graph
semantics or be refused explicitly.

## Authority boundaries

- Source and semantic analysis establish operation, data, effect, resource,
  provenance, failure, and disposition facts.
- Graph analysis derives dependency, conflict, independence, and join facts.
- Versioned policy selects only among schedules authorized by those facts.
- Runtime planning consumes deployment capabilities and verified calibration.
- Backends execute the selected schedule without redefining legality.

The source developer may state semantic constraints that necessarily affect a
schedule, such as an effect on a particular resource or a required ordering.
That is not a request for a thread or asynchronous execution.

## Current evidence

- Flowanalyst derives bounded pure-call `parallel_candidates` from canonical
  operation identity, proven purity, disjoint inputs, and non-conflicting
  destinations.
- Source-graph scheduling derives deterministic dependency waves for the
  bounded `parallel_independent_v1` policy.
- Native and TinyVM tests preserve graph results and schedule evidence without
  adding parallel source syntax.
- Flowparallel provider planning already separates graph evidence from runtime
  capability, calibration, and minimum-benefit policy.

This evidence proves the architecture for bounded pure work. It does not yet
admit general effectful parallel execution.

## Effectful consequence

General effectful parallel scheduling requires stronger canonical evidence:

- exact effect and resource identities;
- read, write, consume, produce, commit, and irreversible access classes;
- conflict and ordering relations derived before scheduling;
- explicit failure, cancellation, partial-work, and commit/abort disposition;
- deterministic serial reference behavior;
- backend-independent validation and provenance.

The compiler may expose an effectful parallel opportunity only after those
facts prove that concurrent execution preserves the graph's meaning. Unknown,
aliased, dynamically unresolved, irreversible, or contradictory effects fail
closed rather than being speculatively parallelized.

## Consequences

- No language-surface expansion is needed to gain new execution strategies.
- `async` is not a canonical Lyraform language feature.
- A source program remains semantically the same when policy or deployment
  selects serial execution instead of a parallel provider.
- Performance measurements influence provider selection, never source
  validity or semantic independence.
- Scheduling artifacts and policy choices remain inspectable and replayable.
- General cancellation, queue/backpressure, and effectful commit protocols are
  still required before their corresponding execution strategies are admitted.

## Invariant

The developer declares the graph and its semantic constraints. The compiler
proves scheduling freedom. Policy chooses within that freedom. The runtime
measures feasibility and executes the choice. No later layer may invent more
freedom than the canonical graph proves.
