# Preserve current research baseline

- Mission ID: preserve-current-research-baseline
- Definition revision: 1
- Owner: Henrik Sørensen
- Context: Lyraform; existing `experiment/configresolve-lyraform` branch.
- Authorization: user's 2026-11-12 checkpoint/publication request in the KISS
  audit conversation. This mission does not adopt a previous mission's state.
- Intent: preserve the developed implementation and minimal reproducible audit
  evidence without integrating into main or changing language meaning.
- Policies: [local map](../../Policies/README.md),
  [EXECUTION-001](../../Policies/EXECUTION-001.md), shared WORK-001, STATE-001
  and AGENT-001 as linked there; [executor guidance](../../AGENTS.md).

## Goal and bounded tasks

1. Inspect Git state, fetch origin, verify audited baseline ancestry and identify
   any local work. Preserve all existing work and history.
2. Run `./igor doctor`, `./igor check`, `./igor build`, `./igor test` using an
   isolated temporary build. Require zero exit status from each; stop for review
   on failure and retain evidence. No semantic fixes are authorized.
3. Preserve minimal existing KISS sources, scripts, results and provenance.
   Exclude binaries, build trees, caches, personal paths and secrets. Explicitly
   identify missing reports and distinguish adapted scripts from original bytes.
4. Review the evidence-only diff and destination; commit it and normally push
   the existing development branch. Success requires the fetched remote branch
   to equal the local checkpoint commit and retain the audited ancestor.
5. Hand off the exact branch, commit, repository, verification results, evidence
   location, known limitations and main divergence. Stop before integration.

No force push, reset, rebase, branch deletion, main/master modification, new
language feature, compiler redesign or governance change is authorized.

## Evidence and status

Implementation baseline: `9459705a9de14a91d62f796aa6bcd7f727fac165`.
The initial fetched development branch equals this revision; it is 22 commits
ahead and zero behind origin/main (`b50534c380879a5a70a77ed5c1dd442bfc4142bd`).
The initial worktree is clean. The baseline is HEAD itself and hence an ancestor.

Fresh canonical verification passed all four commands, with 207/207 tests.
See [the curated checkpoint](../checkpoints/2026-11-12-kiss-baseline/README.md).
Publication is verified after the checkpoint commit through remote Git equality;
this pre-publication record does not itself establish push completion.
The final handoff supplies the resulting commit rather than a self-referential
commit identifier inside the archive.
