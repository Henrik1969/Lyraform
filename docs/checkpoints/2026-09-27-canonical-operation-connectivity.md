# Canonical operation connectivity — Stage 3A

Date: 2026-09-27.

## Objective

Inventory the already-admitted semantic fact families against the canonical
lowering operation model. Connect only the smallest factual gap without adding
language, execution, graph, scheduling, failure, ownership, or ABI meaning.

## Connectivity inventory

| Fact family | Existing operation connection | Result |
|---|---|---|
| source-operation coverage | exact statement-to-operation identity set | connected |
| scalar facts | embedded in declaration/assignment operation; exact source, destination, type and provenance checks | connected |
| target facts | embedded and carrying their operation identity; exact statement, scalar and member-chain checks | connected |
| static guard facts | `affected_operation_id` with exact destination dependency and lifecycle validation | connected |
| disposition facts | exact producer/guarded operation identity plus route and lifecycle operation sets | connected |
| ownership transfer | exact producer, return, call, owner and function operation identities | connected |
| provider/effect requirements | exact provider identity set over external and native-graph operations | connected in Stage 2E |
| source-call projection | expression/statement/callee fields only; no operation identity and no exact shared validation | **gap** |
| member reconstruction | exact target/operation fact, but backend execution deliberately unsupported | connected fact; execution excluded |

Function `effect_facts` are function-level authority rather than operation
facts. Their proven-pure result feeds source-call purity; external declared
effects remain provider-operation authority. These are distinct and are not
collapsed.

## Confirmed defect

A callable-plan-v2 semantic report could mutate the top-level call projection's
callee while leaving its canonical lowering operation unchanged. Both
Flowvalidate and Flowparallel accepted and preserved the disagreement.

The projection is inspectable compatibility data consumed by later stages, so
allowing it to contradict operation authority created a second mutable account
of the same source call.

## Implementation

Every callable-plan-v2 operation now carries a Boolean
`source_call_projection` marker. Operations produced from a structural source
call set it; synthesized operations do not. Every top-level call projection
carries the corresponding `operation_id`.

Shared validation requires:

- every marked operation is a call, external call, or TextOutcome call;
- every marked operation has exactly one projection;
- no unmarked operation may be claimed by a projection;
- operation, expression, statement, scope, callee, callee symbol, arguments,
  and optional result identities agree exactly;
- projected purity agrees with the existing function effect fact;
- missing, duplicate, invented, or drifting projections are refused.

Historical lowering-plan v1 remains readable and makes no Stage 3A
connectivity claim.

## Initial focused evidence

- exact provider/source identity and hostile projection mutations: PASS;
- TextOutcome operation/disposition regression: PASS;
- end-to-end identity preservation: PASS;
- Flowanalyst pipeline: PASS;
- Flowbind provider boundary: PASS;
- graph-planner compatibility fixture: PASS;
- combined focused result: **6/6 PASS**, 12.69 seconds.

## Final verification

- canonical Igor suite: **170/170 PASS**, 85.43 seconds;
- ASan/UBSan focused gate: **6/6 PASS**, 23.70 seconds;
- expanded purity-mutation gate rechecked under ASan/UBSan: **PASS**;
- Valgrind Flowanalyst local-call producer: **0 errors**, 0 bytes in 0 blocks
  at exit;
- Valgrind Flowparallel canonical external-call consumer: **0 errors**, 0
  bytes in 0 blocks at exit;
- shared validation of local and external semantic reports: **PASS**;
- repeated callable-plan-v2 semantic emission: byte-identical;
- wrong operation, callee, purity, effect evidence, missing projection,
  duplicate projection, missing marker, and false marker attacks: refused;
- current-documentation drift guard: **PASS**;
- canonical-authority drift guard: **PASS**;
- legacy-deprecation drift guard: **PASS**;
- repository diff whitespace check: **PASS**.

One external fixture selected for the first memory probe is intentionally
blocked by callable-plan-v2 source completeness. It was not counted. The probe
was repeated with the canonical-complete Flowcat fixture and passed.

## Gate 3A

**PASS.** Every currently admitted semantic fact family in the Stage 3
inventory either already has an exact canonical operation connection or is
explicitly classified as function-level authority. The one mutable duplicate,
the source-call projection, now consumes exact lowering-operation and effect
authority in callable plan v2. No language or execution meaning changed.

Recommended next stage: audit the existing proven-pure
`parallel_candidates`/dependency projection against canonical call operation
identities. Connect only current evidence; do not admit scheduling, async,
effectful parallelism, retries, cancellation, or runtime concurrency behavior.
