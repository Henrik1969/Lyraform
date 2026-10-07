# Current-truth consolidation

Date: 2026-09-26.

## Objective

Reconcile every current-facing project document with the latest accepted
Lyraform authority and add an executable drift check. Keep dated evidence and
historical terminology unchanged where they remain historically accurate.

## Baseline

- branch: `main`;
- HEAD and local `origin/main`: `ecdac7184236dab24423ca7495e882f771cdf287`;
- latest integrated semantic checkpoint:
  [uniform owned-obligation transfer](../checkpoints/2026-09-26-uniform-owned-transfer.md);
- worktree clean except for the unrelated untracked `meta-discusions.md`;
- no staging, commit, push, branch, `master`, FlowLFS, or repository-host work.

## Current-authority surfaces

- root and component current-status documents;
- active architecture and development indexes;
- onboarding and alpha-testing entry points;
- presentation current-status metadata;
- current semantic contract descriptions.

## Historical surfaces

Dated checkpoints, migration reports, reconnaissance, archived implementations,
and old branch/version records are evidence of their own time. Do not rewrite
their test counts, names, decisions, or limitations to resemble the present.

## Required work

1. Establish one current-status landing page.
2. Remove floating test totals from evergreen documents; exact totals belong
   in revisioned checkpoints and status snapshots.
3. Reconcile owned-value wording with ADRs 0052, 0059, and 0060.
4. Refresh presentation metadata as a dated snapshot of the latest verified
   checkpoint.
5. Repair current navigation and decision-index ordering.
6. Add a deterministic documentation drift check and register it in CTest.

## Non-goals

- language, compiler, runtime, ownership, or failure-policy expansion;
- forwarding-chain implementation;
- rewriting historical evidence;
- broad naming replacement;
- safety, production-readiness, or portability claims.

## Gate

PASS requires coherent current-facing authority, preserved historical evidence,
an executable drift check, valid JSON and links for the touched surfaces,
shell/diff checks, and the canonical Igor build/test gate. Record exact results
in a new checkpoint.
