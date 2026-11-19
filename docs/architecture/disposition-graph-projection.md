# Disposition graph projection — experimental compiler contract

Owner: source semantic analysis, under ADR 0066/0067. The existing
`--graph-plan-version 2` request projects accepted source disposition topology
and selected policy into `lowering_plan.disposition_graph` version 3. This is a
non-executable typed subgraph, not another source language or policy authority.
Gate 3 without the graph request retains its prior declarative artifact contract.

The projection is computed once by the shared mechanical projection helper;
validators compare it with that exact projection of the preserved authority.
They do not resolve source names, choose routes or invent default behavior.
`disposition_graph_version` and graph must travel together. Missing either,
foreign identities, changed lanes/schedule/edges, extra fields and execution
claims are rejected. Historical artifacts without either make no graph claim.
Removing every indication of a newer contract is not cryptographically detectable
without trusted origin evidence; validators are not artifact signatures.

## Nodes and edges

One source consumer projects to a failure dispatcher and an exclusive success
junction. Keeping these roles separate avoids a fake producer/response execution
cycle. The producer success path bypasses response execution. The failure path
selects exactly the source-authorized ordinary response from the validated policy.
Recovery enters the typed success junction, then its resolved destination callable.
Fault goes only to activation containment. Source wire identities remain on the
projected success/failure/fault/rejoin edges; projection-local response edges use
route identities. Node/edge identity collisions are refused.

Module/provenance, source obligation, commit law, policy identity and host policy
remain attached. Schedule is serial and disposition-boundary transforms require
an equivalence proof. Runtime execution remains unsupported; no projection flag
makes LLVM/TinyVM executable. The lowerers must still refuse this blocked plan.

## Binding and preparation

Flowbind validates the source plan and retains the disposition authority and graph
in its `disposition_projection` evidence. This does not make Flowbind a semantic
owner. Preparation requires exact agreement between the binding evidence and
optimization plan, then preserves the plan. Parallel/optimization stages retain
and validate the same graph; no provider chooses a different semantic route.

## Current boundary

The first projection supports selected recovery, typed rejoin and fault
containment. Transformation is explicitly refused until its successor destination
and wire are preserved by semantic authority. This is still an incomplete Gate 4;
it does not complete the full language or ConfigResolve port. The later execution
gates must implement and verify exclusive publication, evidence liveness,
obligation closure and native/TinyVM receipts without exceptions or hidden exit.

2026-10-17 successor audit: the single-consumer semantic topology now refuses
transformation routes even when a wire is present. A nonexistent node, success
function or containment node cannot account for a transformed failure. Isolated
transform signature syntax remains available for inspection; no successor-chain
execution is implied. A separately bounded source-chain stage must establish
exact destination type, envelope/obligation linkage and closure before projection.

## Envelope identity (projection version 2)

Current source response witnesses carry `parameter_symbol_id`. Callable parameter
facts carry `input_projection: immutable_failure_envelope` as well as the payload
type spelling. Shared validation ties each witnessed response to its exact
ordinary definition, single parameter identity, envelope projection, incoming
payload type and outgoing result type. Carrier spelling alone is not authority
for passing a payload as an envelope.

Version 2 requires every closed-set response witness to contain its parameter
identity. Each selected ordinary-response node preserves that identity, immutable
projection and incoming failure type. The graph version and surrounding marker
must agree through exact projection validation. Version 1 is retained solely as
a historical non-executable projection; its old shape establishes no envelope
ABI or execution permission. Removing one witness from version 2 is invalid.

These are preservation facts, not runtime envelope storage. Native/TinyVM lowering
must still implement disposition/obligation, producer attempt, correlation,
origin commit and provenance evidence before execution is admitted.

Binding also runs shared disposition-fact validation before producing evidence.
This rejects a graph whose callable signature contradicts its source response
witness. A source topology cannot omit the operation-authority facts and thereby
skip those cross-checks. The version-2 regression exercises these corruptions at
semantic, parallel, optimized and prepared boundaries.

## Producer return carrier (projection version 3)

Version 3 additionally retains the producer's `result_projection` on its graph
node. The source producer and callable declare an `atomic_tagged_result` carrier
(`TextOutcome` for the currently admitted producer). Their declared source
success type remains `Text`; the backend return carrier preserves both tag and
payload. The return operation must transfer the exact producer owner directly,
not describe a conversion of `TextOutcome` into `Text`. Shared validation binds
the callable, projection, return operation, carrier and owner identities.
Projections attached to unrelated callables are refused.

Historical graph versions 1/2 retain their non-executable meanings. Version 3
requires the producer witness and still grants no execution permission. Native
structured lowering reads the preserved carrier when loading callable signatures;
envelope-call ABI, graph execution and ownership cleanup remain prerequisites.
Ordinary calls to a source-associated disposition producer are refused in the
current bound: their independent call-site failure routes have not been admitted.
The static graph association must not silently account for additional attempts.
