# Lyraform mission: Gate 2 and hygiene convergence integration

Date: 2026-10-10

## Mission type

Short-loop convergence integration / branch consolidation.

## Objective

Integrate exactly two completed review branches into one short-lived branch
based on synchronized `main`:

```text
codex/disposition-foundation-source-gate2
codex/agent-authority-workspace-hygiene
```

The combined branch must preserve the complete Gate 2 source-disposition
authority and the complete agent/workspace hygiene authority, reconcile their
current-truth surfaces, pass clean verification, and stop ready for one merge
into `main`.

This mission exists to close branches, not grow another long-running line.
Do not begin Mission 07 Gate 3 or any adjacent language feature here.

## Branch budget and loop closure

Use only:

```text
codex/gate2-hygiene-convergence
```

Do not create child branches, per-gate branches, parallel implementation
branches, or another mission branch. Preserve both source branches through
ordinary merge ancestry; do not squash, rebase, cherry-pick, or rewrite their
published commits.

The intended loop is:

```text
verify baseline
    -> merge hygiene authority
    -> merge Gate 2 authority
    -> reconcile exact overlaps
    -> clean build and hostile verification
    -> push one review branch
    -> WAITING FOR HUMAN REVIEW
    -> merge to main before Gate 3 starts
```

No later maturation mission begins from this branch while it is unmerged.
After Henrik approves and merges it, branch/worktree cleanup is a separate
explicit repository-maintenance action; do not infer branch-deletion authority.

## Verified drafting baseline

At drafting time:

```text
origin/main
    86a5c69d58bacb09f393cef2a3d4e798ac9fe3ad

origin/codex/disposition-foundation-source-gate2
    418e1e2172a8605436de34391b6e8d1729f803b5
    9 commits ahead of origin/main

origin/codex/agent-authority-workspace-hygiene
    6566cd8dbff0d29fd4ff549a52b90120bf536cb7
    3 commits ahead of origin/main

merge base of both review branches
    86a5c69d58bacb09f393cef2a3d4e798ac9fe3ad
```

The dedicated worktree was created clean from `origin/main`. Re-fetch and
verify every SHA, branch relationship, worktree, and remote before merging.
If `origin/main` or either branch changed, inspect the exact change. Stop if it
alters mission meaning, overlaps unresolved work, or invalidates the recorded
evidence.

## Authority

Read and follow:

1. the applicable `AGENTS.md` chain;
2. the Stage Execution Protocol;
3. the Mission Convergence Rule from the hygiene branch once integrated;
4. this mission;
5. the Gate 2 and hygiene mission/checkpoint documents carried by their exact
   branch tips;
6. accepted ADRs 0054 through 0067 for semantic consistency only.

Historical tasks and checkpoints are evidence, not ambient instruction.
Neither merge may weaken accepted language law or turn declarative Gate 2 facts
into executable claims.

## Convergence contract

1. **Usable endpoint:** one pushed review branch contains both complete branch
   ancestries, one coherent authority chain, the source-derived closed typed
   disposition topology through Gate 2, and clean reproducible verification.
   It ends at merge readiness, not policy or execution maturity.
2. **Accepted law and fact owners:** ADRs 0054–0067 retain disposition meaning;
   semantic analysis remains the one legality owner; the hygiene rule owns
   mission selection and artifact placement; Git merge ancestry owns branch
   integration provenance. No new semantic fact is introduced.
3. **Unresolved decisions and stops:** stop on semantic conflict, changed
   baseline, non-administrative merge conflict, failed canonical gate, missing
   source branch ancestry, or any need to modify FlowLFS, `master`, or unrelated
   user work. Routine textual union is an engineering decision.
4. **Reuse and retirement:** reuse the two completed branches, their tests,
   checkpoints, runner, documentation guards, and canonical build. Retire the
   tracked root `.codex-run-state` in the combined tree. Add no second runner,
   status model, disposition schema, or parallel integration branch.
5. **Evidence:** positive—the combined Gate 2 source specimen and hygiene
   runner gates pass; negative—unsupported disposition execution remains
   refused; hostile—ancestry omission, forged topology, invalid mission source,
   stale manifest, and false executable/current-status claims are rejected.
6. **Reproducibility and maturity:** verify in a fresh external build directory
   and then from a fresh clone of the pushed review branch. Claim integrated
   review-branch maturity only; do not claim `main` integration before merge.
7. **Falsifying evidence:** either tip absent from ancestry, root runner state
   still tracked, canonical tests failing, Gate 2 becoming executable, hygiene
   attacks being accepted, or documentation claiming unmerged work is on
   `main` disproves PASS.
8. **Consolidation before expansion:** update task lifecycle navigation and a
   dated integration checkpoint, leave the branch clean and pushed, and do not
   select Gate 3 until this branch is merged into synchronized `main`.

## Protected scope

Do not:

- implement Gate 3 policy artifacts or selection;
- add language syntax, semantics, Graph IR, lowering, backend, runtime, guard,
  retry, cancellation, parallel-failure, or quarantine execution;
- write directly to `main` or `master`;
- merge the review branch into `main`;
- create or merge a pull request without separate explicit authorization;
- touch FlowLFS or Flowselection;
- change repository settings, workflows, releases, issues, or branch
  protection;
- delete branches or worktrees;
- force-push or rewrite history;
- move, stage, or modify the protected `meta-discusions.md` or local `output/`
  material in the primary checkout.

## Stage A — baseline and branch proof

Record:

- branch, HEAD, remote tips, merge bases, and ahead/behind counts;
- all worktrees and dirty/untracked paths;
- exact commit sequences on both source branches;
- exact source-branch diffs against `origin/main`;
- current test registration and relevant task/checkpoint files;
- absence of FlowLFS and `master` ancestry from the integration range.

Run focused pre-merge checks from each already-built branch where available.
Do not fabricate a fresh baseline count from old checkpoints.

### Gate A

PASS only when both source tips are unchanged, descend from the same verified
`origin/main`, and the integration worktree is clean except for this selected
mission commit.

## Stage B — integrate hygiene authority

Merge `origin/codex/agent-authority-workspace-hygiene` with an ordinary
non-fast-forward merge preserving all three published commits.

Verify immediately:

- Mission Convergence Rule and direction chain are present;
- `docs/tasks/README.md` exists;
- runner state is mission-specific and Git-local;
- arbitrary, untracked, checkpoint, symlink, stale-manifest, and cross-mission
  task/state attacks remain refused;
- `.codex-run-state` is absent from the current tracked tree;
- archived downloads retain their provenance and checksums.

### Gate B

PASS only when hygiene focused tests and documentation checks pass before the
semantic branch is merged.

## Stage C — integrate Gate 2 authority

Merge `origin/codex/disposition-foundation-source-gate2` with an ordinary
non-fast-forward merge preserving all nine published commits.

Expected overlap is limited to shared current-rule, test-registration, and
documentation surfaces. Resolve such overlap by preserving the union:

- hygiene authority and placement rules;
- disposition evidence-liveness and explicit-failure rules;
- both CTest registrations;
- deletion of the administrative root `.codex-run-state` from the current
  tree while preserving historical references to it;
- both mission and checkpoint histories.

Do not resolve a semantic conflict by choosing whichever side compiles. If the
branches disagree about accepted meaning, stop with a decision brief.

### Gate C

PASS only when both branch tips are ancestors of `HEAD`, the Gate 2 source
specimen still produces its exact closed typed declarative topology, hostile
mutations still refuse, and all execution claims remain false.

## Stage D — current-truth reconciliation

Make only the smallest consolidation edits required by the combined tree:

- update `docs/tasks/README.md` with explicit PASS evidence for both completed
  missions and identify this integration mission as current until its gate;
- add a dated convergence-integration checkpoint;
- update current-facing status/navigation only where needed to describe the
  review branch without pretending it is already on `main`;
- ensure historical records remain historically accurate;
- remove no compatibility path or executable mechanism merely because it is
  old.

At the final gate, mark this mission PASS and the lifecycle index as awaiting
merge review. Do not select Gate 3 in the same commit.

## Stage E — clean verification and publication

Use a fresh external build directory and run at minimum:

```text
bash -n tools/run-autonomous-codex
sh tools/test-run-autonomous-codex.sh
the agent/workspace hygiene checker
the current-documentation checker
the focused Gate 2 source-disposition test
./igor doctor
./igor build
./igor test
git diff --check
```

Run the focused Clang ASan/UBSan and Valgrind probes required by the Gate 2
checkpoint unless the host lacks the recorded tools; report exact limitations.

Commit only reconciliation/checkpoint changes after the two merge commits.
Push `codex/gate2-hygiene-convergence` normally. Then clone that pushed branch
into a new temporary directory and reproduce configure, build, focused tests,
and the canonical suite from zero local state.

Do not predict the combined CTest count. Record the exact discovered count.

## Definition of done

- one short-lived integration branch contains both complete source ancestries;
- no child or parallel branch was created;
- hygiene and Gate 2 focused hostile tests pass;
- clean build and canonical tests pass;
- fresh-clone reconstruction of the pushed review branch passes;
- current documentation distinguishes review-branch evidence from integrated
  `main` truth;
- task lifecycle and checkpoint evidence are reconciled;
- root `.codex-run-state` is retired from the current tree;
- Gate 2 remains declarative and execution-unsupported;
- no Gate 3 or adjacent feature work was started;
- branch is clean, pushed, and ready for one human-reviewed merge;
- `main` written directly: NO;
- `master` touched: NO;
- FlowLFS touched: NO;
- force used: NO.

Final state:

```text
PASS
WAITING FOR HUMAN REVIEW AND MERGE
```

Do not start the next mission until this branch is merged into synchronized
`main`.

## Task-local result

**PASS — MERGED INTO `main`.**

Both source tips are present through ordinary merge ancestry. The combined
tree preserves the Gate 2 declarative boundary and the mission/workspace
hygiene authority, retires the tracked root run-state file, and passes its
clean local and fresh-clone verification. No Gate 3 work was started.

Henrik explicitly approved continuation after review. The exact reviewed tip
was merged with ordinary ancestry into `main` as
`9f3c40ea36047bae4eb788356b4c30e67ba686fc`; current-truth documentation was
then reconciled without adding semantic scope.

See the
[dated integration checkpoint](../checkpoints/2026-10-10-gate2-hygiene-convergence-integration.md)
for exact evidence and the two bounded test-hygiene repairs made during
integration.

## Final report

Return:

```text
GATE 2 + HYGIENE CONVERGENCE: PASS / BLOCKED

Starting main SHA:
Gate 2 source tip:
Hygiene source tip:
Final integration SHA:
Branch:

Both source tips in ancestry:
Tracked root run state retired:
Gate 2 declarative boundary preserved:
Hygiene authority boundary preserved:

Focused hygiene tests:
Focused Gate 2 tests:
ASan/UBSan:
Valgrind:
Canonical clean build/tests:
Fresh-clone build/tests:

Gate 3 work started: NO
Child branches created: NO
main written directly: NO
master touched: NO
FlowLFS touched: NO
force used: NO

Merge readiness:
Remaining risks:
```
