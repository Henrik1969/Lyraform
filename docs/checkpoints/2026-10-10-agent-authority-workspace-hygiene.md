# Agent authority and workspace hygiene checkpoint

Date: 2026-10-10

## Result

PASS on branch `codex/agent-authority-workspace-hygiene`, beginning at
`2ea3fe743b35ee0ada68897b942dc4dab4e6f9b6`.

## What became usable

- `AGENTS.md` now defines one direction chain from durable law through one
  selected tracked mission, named authority/evidence, isolated local state,
  and a dated checkpoint.
- `tools/run-autonomous-codex` accepts only tracked, non-symlink Markdown
  missions directly beneath `docs/tasks/` with an explicit convergence
  contract.
- Each mission revision receives a digest-bound ID, Git-local state, immutable
  launch manifest, and mission-specific transcript directory.
- `docs/tasks/README.md` provides cautious lifecycle navigation and leaves
  unsupported classifications explicitly unclassified.
- `tools/check-agent-workspace-hygiene.sh` enforces the durable wiring and is
  registered in the canonical CTest graph.
- `output/`, `.agents/`, `.codex/`, and `.local/` are local-only surfaces; the
  shared tracked `.codex-run-state` has been retired from the current tree.

## Negative and hostile evidence

The focused fake-Codex suite refuses:

- missing task selection;
- paths outside the repository;
- tracked arbitrary files and checkpoint documents;
- untracked task files;
- mission symlink escapes;
- missions without a convergence contract;
- state paths outside the Git-local `codex/` namespace;
- cross-mission state reuse;
- forged or stale launch manifests; and
- invalid state values.

Terminal `DONE` and `BLOCKED` states are handled without invoking Codex, and a
normal launch leaves the worktree clean.

## What remains local or unsupported

- This branch does not classify historical task lifecycle without durable
  evidence; such tasks remain `unclassified`.
- It does not integrate the completed disposition Gate 2 branch or change
  compiler, language, policy, graph, backend, or runtime meaning.
- Existing historical checkpoints and missions may retain references to the
  old root run-state file as historically accurate evidence.

## Simplification and retirement

One shared tracked root state and one shared default local state convention are
replaced by a single mission-ID-based convention. No second agent framework or
semantic authority was added.

## Canonical law and falsifying evidence

The Mission Convergence Rule, Stage Execution Protocol, and repository safety
law were checked. Acceptance of an untracked/checkpoint mission, state reuse
across mission identities, generated worktree dirt, or dependence on the
removed root state would have falsified the claim.

## Verification

Fresh external build directory:
`/tmp/lyraform-agent-hygiene-build-20261010`.

- `bash -n tools/run-autonomous-codex`: PASS.
- shell syntax for both focused hygiene scripts: PASS.
- `sh tools/test-run-autonomous-codex.sh`: PASS.
- `FLOWCORE_ROOT=$PWD sh tools/check-agent-workspace-hygiene.sh`: PASS.
- `FLOWCORE_ROOT=$PWD sh tools/check-current-documentation.sh`: PASS.
- `git diff --cached --check`: PASS.
- `./igor --build-dir /tmp/lyraform-agent-hygiene-build-20261010 doctor`:
  PASS.
- `./igor --build-dir /tmp/lyraform-agent-hygiene-build-20261010 build`:
  PASS, 322 Ninja steps.
- `./igor --build-dir /tmp/lyraform-agent-hygiene-build-20261010 test`:
  **183/183 PASS** in 58.27 seconds.

The build retained two pre-existing `-Wunused-function` warnings in
`Flowparallel/src/graph_cuda.cpp`. Sanitizer and Valgrind runs were not applied
because this bounded change affects shell tooling, documentation, ignore rules,
and CTest registration rather than compiled runtime behavior.

## Protected scope

- disposition Gate 2 branch touched: NO
- `main` modified directly: NO
- `master` touched: NO
- FlowLFS or Flowselection touched: NO
- protected user notes/output moved or deleted: NO
- force used: NO

## Gate

PASS. The branch is ready for human review and later convergence integration.
