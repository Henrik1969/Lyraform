# Lyraform agent authority and workspace hygiene mission

Date: 2026-10-10

Status: explicitly selected by Henrik on 2026-10-10 and installed at
`docs/tasks/agent-authority-and-workspace-hygiene.md`. This initial commit
preserves the mission and standing priorities for the next stage; execution of
the cleanup work packages has not yet begun.

## Objective

Install Henrik's approved Lyraform Mission Convergence Rule as a primary
standing priority, then make agent direction, mission selection, local run
state, temporary output, and durable evidence unambiguous without rewriting
project history or touching the completed disposition Gate 2 review branch.

The bounded question is:

> Can every agent determine exactly which documents authorize its work and put
> every working artifact in one predictable class without treating historical
> prose, stale run state, generated output, or chat context as ambient authority?

The hygiene machinery must serve these five standing priorities, in order:

1. prefer a bounded vertical slice;
2. check every change against canonical law;
3. identify what remains to be canonized;
4. simplify, reuse, and generalize deliberately;
5. consolidate before expanding.

File placement and runner convenience never outrank those priorities.

## Verified starting facts

- `origin/main` is `86a5c69d58bacb09f393cef2a3d4e798ac9fe3ad`.
- The current checkout is the completed review branch
  `codex/disposition-foundation-source-gate2` at
  `418e1e2172a8605436de34391b6e8d1729f803b5`; this mission must not modify or
  integrate it.
- The current checkout contains protected untracked user material:
  `meta-discusions.md` and `output/`. The latter contains the locally generated
  failure-flow PDF. Both remain untouched.
- `build/` is ignored generated material (approximately 280 MiB). It remains
  untouched unless Henrik separately requests deletion.
- `.agents/` and `.codex/` are empty local directories and are not tracked.
- The repository contains 25 task documents and 85 checkpoint documents, but
  task lifecycle/status metadata is inconsistent.
- The tracked root `.codex-run-state` is a historical shared state file. The
  current runner instead supports Git-local state, but its default
  `.git/codex/run-state` is still shared between missions.
- `tools/run-autonomous-codex` accepts any repository file as `--task`, despite
  documenting explicit mission selection.
- The runner already tells agents to read the `AGENTS.md` chain and only the
  selected mission plus explicitly supplied context. Preserve and enforce that
  good boundary.
- Henrik supplied and approved
  `/home/henrik/Hentet/LYRAFORM_STANDING_MISSION_CONVERGENCE_RULE_2026-10-10(1).md`
  for installation at `docs/development/mission-convergence-rule.md`. Its five
  requirements are vertical slice, canonical law, canonization,
  simplification and reuse, and consolidation. Preserve its meaning and make
  it part of the normal `AGENTS.md` instruction chain.

Re-verify all starting facts before editing. If `origin/main` changed, inspect
the change and stop only if it alters this mission's authority or overlaps its
files.

## Authority and branch

Once explicitly selected, this mission authorizes an isolated clean worktree
and a dedicated branch named `codex/agent-authority-workspace-hygiene` from the
verified current `origin/main`.

It authorizes bounded edits, tests, documentation, coherent commits, and an
ordinary push of that branch. It does not authorize merging a pull request,
editing `main`, force-pushing, deleting branches or user files, or changing
language/compiler semantics.

Read and follow:

1. the applicable `AGENTS.md` chain;
2. the Stage Execution Protocol, Mission Convergence Rule, and Autonomous
   Next-Stage Selection named by that chain;
3. this selected mission;
4. only the architecture, protocol, and evidence documents explicitly named by
   this mission or by an applicable `AGENTS.md` rule.

Historical tasks and checkpoints remain evidence, not executable instruction.

## Canonical direction chain to establish

Document and enforce this order:

```text
applicable AGENTS.md chain
    durable repository law
        ↓
standing development rules named by AGENTS.md
    stage execution, mission convergence, autonomous continuation
        ↓
explicitly selected docs/tasks/<mission>.md
    scope, authority, state, permissions, definition of done
        ↓
mission-named ADRs / protocols / context / prior checkpoints
    binding meaning or factual evidence for that mission only
        ↓
local run manifest and state under Git-local metadata
    administrative progress only; never project semantics
        ↓
new dated checkpoint
    durable result evidence; never ambient authority for later work
```

Explicitly state that chat history, old task documents, old checkpoints,
`.agents/`, `.codex/`, generated `output/`, and stale run-state files do not
select or authorize work.

The selected mission may narrow a standing convergence requirement only when
it names the prerequisite, dependent executable slice, exact narrower maturity
claim, and stop boundary. Silence is not an exception.

## Canonical placement model to establish

```text
AGENTS.md
    durable repository rules

docs/tasks/
    selectable mission documents and a human-readable lifecycle index

docs/architecture/decisions/
    accepted semantic and architectural decisions

docs/development/mission-convergence-rule.md
    standing convergence priorities and required mission/gate contract

docs/checkpoints/
    dated immutable result evidence

docs/current-status.md
    integrated current truth and links; not a mission selector

.git/codex/runs/<mission-id>/
    local state and launch manifest; never tracked

/tmp/lyraform-codex-<uid>/<mission-id>/
    transcripts and ephemeral working artifacts

.local/codex/<mission-id>/
    optional repository-local persistent scratch; ignored

build directories
    ignored or external; never checkpoint evidence by themselves

output/
    local delivery/export area; ignored unless a specific artifact is
    deliberately promoted to an appropriate tracked destination with provenance
```

Curated artifacts belong in the semantically appropriate tracked directory
(for example tests, `docs/architecture/`, or `docs/checkpoints/`), not in a
generic output dump.

## Convergence contract for this mission

1. **Usable behavior and endpoint:** launching the autonomous runner with one
   tracked mission produces an inspectable authority manifest, isolated state,
   and mission-specific transcript location; invalid authority sources are
   refused before execution. This is developer-tooling maturity, not compiler
   or language maturity.
2. **Accepted law and owners:** `AGENTS.md` owns durable repository law; the
   Mission Convergence Rule owns the five standing priorities; one explicitly
   selected task owns scope and permissions; accepted ADRs own language and
   architecture meaning; the runner owns only administrative launch/state
   validation; Git history and dated checkpoints own recorded evidence.
3. **Unresolved decisions and stops:** uncertain task lifecycle is labelled
   `unclassified`; no status is inferred. Stop on any dependency on the tracked
   root state, overlapping `origin/main` change, semantic-policy choice, or
   need to move/delete protected user material.
4. **Reuse and retirement:** extend `AGENTS.md`, the existing project-hygiene
   and stage protocols, and the existing runner/test harness. Retire the shared
   tracked root run-state and shared default runner state. Add no competing
   agent framework or duplicate rulebook.
5. **Examples and attack:** positive—one tracked `docs/tasks/*.md` mission
   launches with isolated state; negative—a checkpoint passed as `--task` is
   refused; hostile—a stale or cross-mission manifest/state pair is refused.
6. **Reproducibility and maturity:** a clean checkout can run the focused fake-
   Codex runner suite, hygiene checker, and canonical Igor gates. The claim is
   an operationally enforced developer workflow.
7. **Falsifying evidence:** any arbitrary/untracked task accepted, state reused
   across mission identities, generated output dirtied in a clean checkout, or
   undocumented authority source disproves the claim.
8. **Consolidation before the next slice:** update navigation and task
   lifecycle index, record a dated checkpoint, and leave no second run-state or
   output convention active.

## Work package A — standing priorities, rules, and navigation

Update only the minimum existing authority surfaces:

- Install the approved downloaded rule at
  `docs/development/mission-convergence-rule.md`, preserving its five
  priorities, required convergence contract, gate questions, and limitation
  that it grants no Git or semantic authority by itself.
- `AGENTS.md`: add the exact required Mission Convergence Rule link beside the
  Stage Execution Protocol link, then add the concise direction chain and
  placement rule.
- `docs/development/project-hygiene.md`: make the detailed placement model
  current, including agent state, transcripts, scratch, curated promotion, and
  the rule that workspace mechanics serve convergence rather than replace it.
- `docs/development/stage-execution-protocol.md`: define mission selection and
  local-state isolation consistently and incorporate the required convergence
  contract and four gate-report questions by reference rather than duplication.
- `docs/development/autonomous-next-stage-selection.md`: clarify that an
  autonomously framed continuation receives a fresh mission identity/state and
  may not inherit old `DONE` or `BLOCKED` state; it must satisfy the five
  convergence priorities before adjacent expansion.
- `docs/index.md`: link the Mission Convergence Rule, task lifecycle/index, and
  relevant development policy.
- Add `docs/tasks/README.md` as a human navigation and lifecycle index. It must
  distinguish available/current work, completed missions, superseded missions,
  and blocked historical missions without rewriting their historical contents.
- Add a compact new-mission template or checklist under `docs/tasks/README.md`
  containing the eight required convergence-contract entries. Existing
  historical missions are not retroactively rewritten. Any still-selectable
  mission lacking the contract is marked as needing a convergence addendum
  before its next execution.

Do not create a second competing repository rulebook.

## Work package B — runner plumbing

Harden `tools/run-autonomous-codex` and its focused tests:

1. `--task` must resolve to a tracked Markdown file directly under
   `docs/tasks/`. Reject arbitrary repository files, checkpoints, generated
   files, symlink escapes, and untracked mission files.
2. Derive a stable mission ID from the repository-relative task path plus a
   bounded content digest.
3. Default local state to `.git/codex/runs/<mission-id>/state` rather than one
   shared `.git/codex/run-state`.
4. Store a local launch manifest beside state containing at least task path,
   task digest, starting branch, and starting HEAD. Refuse mismatched reuse
   rather than silently inheriting another mission's state.
5. Keep any explicit `--state-file` compatibility path inside the repository's
   Git-local `codex/` namespace. Reject tracked/worktree state and arbitrary
   external state paths.
6. Put default transcripts under a mission-specific directory in
   `/tmp/lyraform-codex-$UID/` and print task, mission ID, state, manifest, log
   directory, branch, and HEAD before execution.
7. Preserve the rule that the runner grants no commit, push, merge, semantic,
   or continuation authority.
8. Add hostile focused tests for arbitrary task files, untracked tasks,
   checkpoint-as-task, symlink escape, state-path escape, stale manifests, and
   cross-mission state reuse.
9. Have the launch prompt name the Mission Convergence Rule through the
   `AGENTS.md` chain and require the selected mission's convergence contract.
   The runner validates presence and provenance; it does not invent missing
   semantic content.

Do not execute Codex recursively as part of verification; use the existing fake
Codex test boundary.

## Work package C — retire ambiguous root state

Remove the tracked root `.codex-run-state` from the current project tree after
the runner and documentation no longer depend on it. Keep `.codex-run-state`
ignored so old local tooling cannot accidentally reintroduce it. Record that
historical tasks and commits retain their original state evidence.

Do not alter historical task/checkpoint wording merely because it names the old
file.

## Work package D — local clutter boundary

- Add `output/`, `.agents/`, and `.codex/` to the local-only ignore policy,
  with documentation that none is an authority source.
- Preserve the existing local `output/` PDF and empty hidden directories; do
  not move or delete them during this mission.
- Preserve `meta-discusions.md` unchanged and do not add an ignore rule for it
  without a separate disposition decision from Henrik.
- Do not delete or relocate `build/`.
- Do not bulk-move tasks, checkpoints, ADRs, source, tests, or historical
  records. Index them in place.

## Verification

At minimum run and record:

```text
bash -n tools/run-autonomous-codex
sh tools/test-run-autonomous-codex.sh
the new agent/workspace hygiene checker
documentation-link/current-documentation checks affected by the change
git diff --check
./igor check
./igor test
```

Use a fresh external build directory if the canonical graph requires a build.
Report exact counts and environmental limitations.

## Stop conditions

Stop rather than guess if:

- the cleanup would require changing language or architecture meaning;
- a current tool outside this bounded runner depends on tracked
  `.codex-run-state`;
- `origin/main` changes in overlapping files during implementation;
- classifying a task as completed, blocked, superseded, or available lacks
  durable evidence;
- protected untracked user material would need to be moved or deleted;
- the work would need to modify or integrate the Gate 2 branch, `master`, or
  `flowlfs-v0.1-alive`.

Unknown task lifecycle remains `unclassified` with an explanatory link; never
invent completion status.

## Definition of done

- One documented and tested authority chain exists.
- The approved Mission Convergence Rule is installed, linked from `AGENTS.md`,
  indexed, and treated as a primary standing requirement.
- Future selectable missions expose the required eight-part convergence
  contract; historical missions remain historically accurate.
- The runner can select only tracked `docs/tasks/*.md` missions.
- Each mission version receives isolated Git-local state and a launch manifest.
- Transcript and scratch placement is predictable and local-only.
- The shared tracked root run-state file is retired without rewriting history.
- `docs/tasks/README.md` lets a returning maintainer find current, completed,
  blocked, superseded, and unclassified missions.
- Generated `output/` no longer dirties status, while the existing PDF is
  preserved.
- No historical checkpoint is promoted into ambient authority.
- No compiler/language behavior changes.
- Focused and canonical verification passes.
- A dated checkpoint records the migration and remaining risks.
- Only the dedicated cleanup branch is committed and pushed normally.
- Gate 2 branch touched: NO.
- `main` modified directly: NO.
- `master` touched: NO.
- FlowLFS touched: NO.
- User notes or output deleted/moved: NO.
- Force used: NO.

Expected result: `WAITING FOR HUMAN REVIEW`.
