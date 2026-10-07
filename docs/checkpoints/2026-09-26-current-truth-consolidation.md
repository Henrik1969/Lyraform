# Current-truth consolidation — 2026-09-26

Gate: **PASS**.

## Baseline and scope

- branch: `main`;
- starting HEAD and local `origin/main`:
  `ecdac7184236dab24423ca7495e882f771cdf287`;
- mission: [Current-truth consolidation](../tasks/current-truth-consolidation.md);
- latest semantic checkpoint at entry:
  [Uniform owned-obligation transfer](2026-09-26-uniform-owned-transfer.md);
- worktree clean at entry except for the unrelated untracked
  `meta-discusions.md`;
- documentation and its deterministic validation only; no compiler, runtime,
  language, ownership, or failure-policy expansion.

No staging, commit, push, branch mutation, repository-host change, `master`
change, or FlowLFS work occurred. `meta-discusions.md` was not modified.

## Consolidated current authority

`docs/current-status.md` is now the current-truth landing page. It identifies:

- Lyraform as the project and semantic identity;
- `main` as authority and Igor as the human-facing toolchain;
- the v0.29 experimental, unstable, non-production-ready status;
- the latest documentation and semantic checkpoints;
- implemented guard, disposition, outcome, and unique-transfer boundaries;
- explicit unsupported ownership, failure-routing, concurrency, ABI, and
  certification boundaries.

The root README, Lyraform current/compiler pages, documentation index,
onboarding guide, alpha-testing guide, Flowmini testing guide, architecture
index, verification guide, presentation authority, and presentation snapshot
now route current claims through that authority.

## Drift removed

Floating active-suite totals of 81, 94, 103, 107, and 156 were removed from
evergreen entry points. Exact totals remain in dated checkpoint or status
snapshot evidence. Readers are instructed to discover the checked-out
revision's graph with `igor check` and execute it with `igor test`.

The Text outcome contract now distinguishes multiple admitted read-only borrows
from ownership fan-out. Observations do not create owners; assignment and return
require ADR 0060's unique transfer relation. Implicit copy, sharing, and fan-out
remain refused. ADR 0059 and ADR 0060 are indexed in numerical order.

The presentation snapshot now identifies Lyraform, points at the integrated
`ecdac7184236dab24423ca7495e882f771cdf287` compiler checkpoint, records its
revisioned verification evidence, and retains its historical
`flowcore.presentation-status/v1` schema only as a compatibility identifier.

## Executable drift guard

`tools/check-current-documentation.sh`, registered as
`lyraform_current_documentation`, validates:

- project, branch, toolchain, maturity, and certification statements;
- required current-checkpoint links;
- absence of floating exact totals in evergreen entry points;
- unique-ownership wording and absence of the stale fan-out claim;
- ADR 0059/0060 ordering;
- presentation JSON shape, coherent test result, repository identity, snapshot
  commit existence, and authority-file existence;
- local Markdown links across the current-facing authority set.

Historical checkpoints, migration reports, reconnaissance, and archived
lineage documents retain their original names and counts.

## Verification

- `./igor doctor`: **PASS**;
- `./igor build`: **PASS**, no compilation work required after configure;
- focused `lyraform_current_documentation`: **1/1 PASS**, 0.05 seconds;
- canonical `./igor test`: **167/167 PASS**, 84.26 seconds;
- presentation JSON parse and semantic assertions: **PASS**;
- current-facing local-link validation: **PASS**;
- shell syntax: **PASS**;
- `git diff --check`: **PASS**.

Sanitizer and Valgrind suites were not repeated because this stage changes no
compiler or runtime executable behavior. The preceding uniform-transfer
checkpoint retains the latest executable sanitizer and leak evidence.

## Result and next boundary

```text
current authority surfaces coherent       PASS
historical evidence preserved             PASS
semantic wording agrees with ADRs         PASS
floating active test totals removed       PASS
automated drift detection                 PASS
canonical suite                           PASS
FlowLFS touched                           NO
master touched                            NO
meta-discusions.md touched                NO
```

The next compiler maturation boundary remains a bounded unique-transfer
forwarding chain under ADR 0060. This documentation stage does not authorize or
implement it.
