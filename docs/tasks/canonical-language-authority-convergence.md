# Canonical language authority convergence

Date: 2026-09-26.

## Mission

Connect Lyraform's existing implemented semantic islands through one canonical
compiler authority without adding language syntax or meaning.

```text
one structural source authority
    -> one identity/type/scope authority
    -> one canonical semantic operation model
    -> versioned validated projections
    -> LLVM / TinyVM
```

The legacy parser, `ModuleSpec`, direct runtime, and historical Flowmini
artifacts are deprecated. They may serve as behavior oracles and historical
evidence only. They are excluded from canonical admission and execution and may
not become competing language authorities.

## Baseline

- branch and local upstream: `main` at
  `ecdac7184236dab24423ca7495e882f771cdf287`;
- retain the verified current-truth and bounded-forwarding working-tree stages;
- preserve unrelated `meta-discusions.md`;
- no staging, commit, push, `master`, or FlowLFS work without separate authority.

## Language freeze

Until this mission reaches its final authority gate, do not add or broaden:

- syntax or source forms;
- scalar, aggregate, indexing, generic, reference, alias, or mutation meaning;
- guard meaning or runtime guard behavior;
- failure, fault, propagation, recovery, or policy-sink meaning;
- ownership, borrowing, parameter-transfer, sharing, or lifetime meaning;
- cancellation, async, backpressure, parallel-effect, retry, or ordering meaning;
- ABI/FFI surface or provider authority;
- self-hosting implementation.

Existing semantics may be represented, validated, connected, preserved,
lowered, or refused more consistently. Any step that requires choosing new
meaning stops with a decision brief.

## Stage 0 — freeze and authority inventory

Question: can every currently claimed canonical island, compatibility path,
and duplicate authority be classified and guarded without changing meaning?

Deliverables:

- a machine-readable authority map;
- an executable drift gate;
- a durable reconnaissance checkpoint;
- an ordered bridge dependency graph.

Gate 0 passes when every current island has an evidence-ladder position, one
named semantic owner or an explicit unresolved/compatibility classification,
and all known execution paths are visible.

## Stage 1 — source-ingress convergence

Make structural parsing and source-completeness admission the only canonical
source authority for already admitted forms. Migrate by bounded existing source
family. Preserve explicit compatibility artifacts and refuse incomplete source;
do not reinterpret it.

First bounded bridge: `lyraform.source_operation_coverage/v1` accounts for
every structural statement and blocks callable-plan-v2 execution on any gap.
`igor run` and its explicit `run-canonical` alias consume this proof and do not
fall back to deprecated legacy execution. See the Gate 1 checkpoint.

## Stage 2 — semantic-producer convergence

Make Flowanalyst/shared semantic components the single producer of identities,
types, legality, operations, guards, dispositions, targets, and ownership facts.
Direct execution adapters must consume validated canonical facts rather than
reconstruct or independently resolve them.

First bounded bridge: lowering operations and identifier-target projection for
already-admitted initialized `int`/`Bool` declarations and placements consume
the destination identity and type from the one Flowanalyst scalar-fact
production pass. Forms outside that scalar slice retain their existing
authorities and are not reinterpreted. See the Stage 2A checkpoint.

The Stage 2B target-producer audit found no duplicate target authority:
Flowanalyst resolves one `lyraform.target_fact`, and shared validation binds it
to operation, scalar, member-chain, type, and provenance identities. No code
was changed merely to manufacture convergence work.

Stage 2C connects bounded static guard evaluation to that same scalar
destination identity. Guard proofs and disposition facts remain Flowanalyst
products, and shared validation requires exact operation, dependency, route,
type, proof-set, lifecycle, owner, and provenance agreement.

Stage 2D confirms one Flowanalyst producer for bounded TextOutcome accounting,
unique return transfer, one-hop forwarding, and final disposition. The
producer-derived must-account identity and owned carrier type are established
once and consumed by every transfer and the final obligation projection.

Stage 2E makes provider/effect requirements an exact canonical set. Flowanalyst
remains the sole producer; shared validation proves that every declared
requirement is used by an external operation or native-graph provider and that
every such use has one declaration. Flowbind applies policy and host-provider
authorization only after that proof. Plan-less inspection reports remain an
explicit compatibility input and cannot enter the canonical staged chain.

## Stage 3 — operation-model connectivity

Connect existing scalar, member reconstruction, static guard, disposition,
TextOutcome, provider/effect, and ownership-transfer facts through one validated
operation model. This stage may lower semantics already accepted by ADRs; it may
not invent the missing general Graph IR or new language behavior implicitly.

Stage 3A inventories those links and closes the remaining call-site projection
gap for callable plan v2. Every source-call projection now names one marked
lowering operation and must exactly preserve its expression, statement, scope,
callee, arguments, result, and function-effect-derived purity. Historical plan
v1 remains readable compatibility evidence and makes no Stage 3A connectivity
claim.

Stage 3B connects the existing deferred `parallel_candidates` evidence to
canonical callable-plan-v2 operation identities. Shared validation recomputes
the bounded pure-call independence relation from canonical operation operands,
results, scopes, statements, and proven effect facts, and requires the emitted
candidate graph to match exactly. The separate `region_dependency` matrix
continues to describe scope/symbol analysis ordering; it is not reinterpreted
as an operation schedule. Candidate status remains `deferred` and supplies no
runtime parallelism authority.

## Stage 4 — projection and compatibility isolation

Require every carrying stage and backend to validate the same authority.
Remove each deprecated legacy dependency from normal operation only after the
corresponding canonical positive, negative, hostile, preservation, and
LLVM/TinyVM evidence exists. Until removal, the route stays explicitly
oracle-only and isolated; it cannot authorize canonical execution.

Stage 4A closes the callable-plan-v2 carrying path for source-call and deferred
independence authority. Flowparallel execution plans, Flowoptimize reports, and
prepared backend artifacts preserve the exact `external_operations`,
`effect_facts`, `parallel_candidates`, and lowering plan. Each consumer
revalidates their relationships; Flowparallel summary counts must equal the
preserved evidence. Direct semantic input to Flowoptimize follows the same
contract. Plan v1 remains historical compatibility without a Stage 4A claim.

Stage 4B closes the remaining callable-plan-v2 authority envelope. The source
identity, target catalog, ABI contracts, aggregate-layout set, and complete
lowering plan are preserved exactly through Flowparallel, Flowoptimize, and
backend preparation. Consumers validate top-level structure and the lowering
plan's source coverage, scalar, target, guard, disposition, and ownership
relationships. Empty authority sets remain explicit. Flowbind stays a
separate, validated policy-authorization branch rather than a second semantic
producer. Unsupported member execution and deferred scheduling remain refused;
this stage adds no syntax, ABI, aggregate, failure, or scheduling meaning.

## Stage 5 — whole-language authority audit

For every source form the toolchain accepts, prove exactly one of:

```text
canonical and connected end to end
deprecated legacy, oracle-only, and isolated
recognized but explicitly unsupported
refused before semantic admission
historical and non-active
```

The mission passes only when no admitted canonical construct receives meaning
from two models and no compatibility behavior can silently enter a canonical
artifact or executable plan.

Stage 5 records that proof in
`docs/architecture/canonical-source-form-audit-v1.json`. It partitions current
source variants across the five required outcomes, covers every typed AST kind
and captured graph declaration, and keeps `module`/`run-legacy` explicitly
oracle-only. Bounded canonical variants name their authority; wider variants of
the same structural kind are separately classified as unsupported or refused.
The executable audit guard fails if an AST kind is added without a disposition.

## Required evidence at every bridge

- positive existing behavior;
- earliest-authority refusal;
- hostile artifact mutation at every consumer;
- exact identity, type, provenance, route, ownership, and commit preservation;
- deterministic LLVM/TinyVM parity where execution already exists;
- direct-path and compatibility-path isolation;
- complete canonical suite plus applicable sanitizer and Valgrind evidence.

## Stop conditions

Stop before implementation when a bridge requires new language, ownership,
failure, scheduling, ABI, policy, or lifecycle meaning. Do not weaken a refusal,
infer semantics from legacy behavior, or use backend behavior as source
authority to make a gate pass.
