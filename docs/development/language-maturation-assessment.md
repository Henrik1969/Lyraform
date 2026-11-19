# Lyraform maturation assessment — 2026-10-17

Scope: current LyraForm working tree, branch `codex/disposition-policy-selection-gate3`,
HEAD `b50534c380879a5a70a77ed5c1dd442bfc4142bd`, including uncommitted work.
This is an evidence-based planning assessment, not a fresh implementation test or
certification of all historical language proposals.

## Intended language and authority

The [core promise](../architecture/flowcore-core-promise.md) makes architecture
visible as contracted graph structure before lowering. `=>` connects graph
edges; `->` places values. Its earlier naming is historical provenance.
[ADR 0052](../architecture/decisions/0052-independent-aggregate-values.md)
requires independent aggregate values; it explicitly leaves write authority,
ownership and backend layout for further decisions.
[ADR 0064](../architecture/decisions/0064-graph-derived-parallelism-and-policy-scheduling.md)
derives parallel opportunity from graph/effect facts. Policy chooses legal
strategies; source does not acquire thread/async placement syntax.
[ADR 0066](../architecture/decisions/0066-source-declared-disposition-consumers-and-policy-selection.md)
and [ADR 0067](../architecture/decisions/0067-explicit-quarantined-activation-results.md)
require explicit typed failure routes and activation quarantine, without
exception unwinding or runtime invention of meaning.

Shared Development policy, local policy maps and FrankenCore authority govern
this work. Capabilities is an implementation collection, not another policy
umbrella. No policy override is proposed here. The historical programmer manual,
bootstrap inventory and experiments establish intentions or evidence, not blanket
permission to add their syntax or semantics.

## What is usable and what remains

The [source-form audit](../architecture/canonical-source-form-audit-v1.json)
currently classifies 72 entries: 36 connected end to end in bounded slices,
24 recognized but unsupported for execution, 9 refused, 2 isolated legacy,
and 1 historical. These are classifications, not a language-completion percentage.

| Area | Inspected evidence and boundary | Maturation consequence |
|---|---|---|
| Scalar programs, calls, branches, loops | Bounded typed operations, `if`, `while`, calls and returns connect through the canonical chain | Reuse the chain; do not replace it with a second interpreter |
| Modules and providers | Bounded contract imports, ABI calls and exact provider authorization exist | General library composition, separate compilation and package closure need explicit evidence |
| Failure and faults | Gate 3 validates source topology and route/host policy on the review branch; runtime dispatch remains unsupported | First finish Graph IR preservation, backend recovery and quarantine under existing accepted law |
| Records and placement | ABI aggregate slices exist; ordinary records and member execution remain unsupported | Implement complete pure scalar/finite nested construction under ADR 0068 and independent values under ADR 0052; member writes and resource lifetimes remain separate |
| Collections and text | Bounded argv/list<string>, Text and TextOutcome paths exist | General construction, arrays, indexing/bounds, ownership and generic domains need bounded decisions and execution proofs |
| Control and contracts | Bounded branches/loops and static guards work; break/continue and refined invariant execution are unsupported | Admit only required families with exact failure and lifecycle meaning; do not revive equals assignment from history |
| I/O and resources | Bounded provider-backed output/file-producer infrastructure exists | Build practical file/configuration workflows with explicit effects, cleanup and typed error routes |
| Parallel execution | Bounded pure-work graph evidence and providers exist | General effectful parallelism needs conflict/commit/disposition proofs; CUDA availability is not language completeness |
| Toolchain | Igor and staged compiler/build/test machinery exist | Supply a reproducible application workflow and honest supported-language guide |
| Self-hosting | Bootstrap inventory still records missing reader, recursive data, collections, artifact I/O, linking and resource closure | Later proof of breadth; not a prerequisite for writing the first useful application |

Source ownership is split deliberately: structural compiler frontend captures
syntax/spans; Flowanalyst establishes semantic identities; Flowcontracts validates
shared artifacts; Flowbind selects authorized providers; Flowoptimize preserves
meaning; preparation/lowering projects to backends; TinyVM/native execute;
Flowparallel plans legal provider execution; Igor drives the developer workflow.
Each affected context's local policy must be read before implementation.

## Evidence limits and inconsistencies to reconcile

[Current status](../current-status.md) and the
[Gate 3 checkpoint](../checkpoints/2026-10-13-source-disposition-policy-gate-3.md)
describe review evidence, not integration into main. The source audit's
`graph.disposition_wire` entry still says policy unsupported, while its consumer
entry and Gate 3 checkpoint report bounded selection. Reconcile this exact
wording against source/tests during the baseline gate; do not infer execution.
The presentation snapshot's earlier 174-test figure and the
[workspace health](workspace-health.md) observation of 189/189 describe different
snapshots. Neither proves the whole language. The latter build preceded the
separately recorded CUDA-helper cleanup. No tests were rerun for this assessment.

## Practical endpoint and longer horizon

Selected acceptance target (Henrik, 2026-10-17): port the existing
[ConfigResolve](../../../ConfigResolve/README.md) engine into Lyraform, with a
reusable library and consumer. This replaces the provisional file-processing CLI.
Its facts/policy/provenance model, schemas, loaders, store transactions and scoped
views exercise substantial real development. Existing contracts and tests supply
comparison evidence; known scalar defects and experimental behavior require
explicit classification. The mission defines parity and anti-wrapper criteria.

“Full language” has no closed, ratified feature inventory today. Establish a
versioned language profile from accepted law and explicit owner decisions,
then require every included family to have source-to-execution evidence. Keep
excluded/deferred families visible. The first practical profile is an intermediate
milestone, not permission to shrink the eventual language silently.

Follow the [maturation mission](../tasks/lyraform-language-maturation.md).
