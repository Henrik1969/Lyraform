# Lyraform Copilot contract

Lyraform is an experimental language and system-architecture project. Treat
repository evidence as authoritative and never infer maturity from aspirations,
file names, issue titles, or prose that predates the latest tagged checkpoint.

For project-presentation work:

- Read `docs/presentation/current-status-v1.json` first.
- Read every file named by its `authority` and `evidence` fields before editing.
- Preserve the distinction between achieved work, experimental work, and future
  goals. Never present a plan as an implemented capability.
- Do not change compiler/runtime code, tests, schemas, architecture decisions,
  checkpoints, ledgers, task records, or generated artifacts.
- Limit presentation changes to files explicitly named by the assigned task.
- Every numeric test claim, maturity claim, active branch, version, milestone,
  backend claim, bootstrap claim, or production-readiness claim must agree with
  the status manifest and its evidence.
- If authoritative sources conflict, do not choose one silently. Leave the
  disputed claim unchanged and report the conflict in the pull-request body.
- Do not change repository settings, branches, tags, releases, issue state, or
  pull-request targets.
- Do not merge your own pull request.

Presentation should explain Lyraform accurately to a technically curious reader
without marketing inflation. Prefer concrete verified capabilities, explicit
non-claims, and links to durable repository evidence.

Before finishing:

1. Re-read the changed files against `docs/presentation/current-status-v1.json`.
2. Run `git diff --check`.
3. In the pull-request body, list changed presentation surfaces, authoritative
   evidence used, commands run, and any unresolved conflict or uncertainty.
