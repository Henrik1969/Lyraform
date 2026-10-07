# Canonical parallel-candidate connectivity — Stage 3B

Date: 2026-09-27.

## Objective

Audit the existing proven-pure `parallel_candidates` projection against the
canonical callable-plan-v2 operation model. Connect only current evidence; do
not admit scheduling, async execution, effectful parallelism, retries,
cancellation, backpressure, or runtime concurrency.

## Scout result

The existing candidate producer already computed a bounded relation between
source calls when both callees were proven pure, the calls occupied distinct
statements in one scope, and their argument reads and result destinations were
disjoint. The artifact identified candidates and peers only by source
expression IDs. Flowparallel counted the proof string without validating the
relation against lowering operations.

The `region_dependency` matrix is a different fact family. Its nodes are
analysis scopes and symbols, and its edges record analysis prerequisites. It
is not an executable-operation dependency graph and has not been reinterpreted
as one.

## Bounded authority

For callable lowering-plan v2:

- each deferred candidate names its canonical source-call `operation_id`;
- each peer names both its canonical operation ID and retained expression ID;
- the operation must be a marked source-call projection whose callee has a
  proven-pure function effect fact;
- candidate expression, statement, and callee identity must equal the
  operation;
- shared validation derives argument reads from canonical operation operands
  and the write identity from the operation result;
- shared validation recomputes the complete bounded independence relation and
  requires the emitted candidate graph to match exactly;
- missing, duplicate, asymmetric, invented, impure, self-referential, or
  drifting candidates are refused.

Historical lowering-plan v1 remains readable compatibility evidence and makes
no Stage 3B connectivity claim.

## Semantic boundary

`status: deferred` is mandatory. The evidence states only that the current
bounded pairwise conflict test found no dependency. It does not select a
scheduler, create execution waves, promise concurrent execution, or establish
general concurrency semantics.

## Evidence

- fresh configure/build: PASS;
- focused candidate, projection, identity-preservation, analyst, Flowparallel,
  and graph-planner tests: **7/7 PASS**;
- hostile wrong operation/expression/callee/peer, self-peer, missing,
  duplicate, fabricated proof, false-ready, shared-read, same-result, and
  cross-scope mutations: refused;
- full canonical suite: **171/171 PASS**, 83.79 seconds;
- ASan/UBSan hostile candidate gate: PASS (`detect_leaks=0` because the host
  ptrace wrapper prevents LeakSanitizer operation);
- Valgrind Flowanalyst producer: **0 errors**, 0 bytes in 0 blocks at exit;
- Valgrind Flowparallel consumer: **0 errors**, 0 bytes in 0 blocks at exit;
- repeated callable-plan-v2 semantic emission: byte-identical;
- shared `flowvalidate` validation: PASS;
- current-documentation, canonical-authority, and legacy-deprecation drift
  guards: PASS;
- repository diff whitespace check: PASS.

## Gate 3B

**PASS.** Existing bounded pure-call independence evidence is now exactly tied
to canonical source-call operation identities and independently recomputed by
the shared contract validator. No execution, scheduling, concurrency, async,
cancellation, retry, backpressure, or effectful-parallelism meaning was added.

The next stage is not to execute these candidates. Stage 4 should first audit
which downstream artifacts preserve, summarize, or discard the evidence and
whether any compatibility path can bypass callable-plan-v2 validation. Any
proposal to turn deferred evidence into scheduling authority requires a
separate explicit concurrency-policy decision.
