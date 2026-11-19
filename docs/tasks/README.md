# Lyraform mission lifecycle and selection

This directory contains selectable mission documents. A mission becomes active
only when Henrik or an authorized continuation explicitly selects its tracked
file. Presence here does not make a task current, and checkpoints elsewhere do
not grant task authority.

## Newly prepared stabilization mission — 2026-11-01

- [Contract enforcement and vertical stabilization](contract-enforcement-stabilization.md)
  — owner-requested five-gate mission, revision 1; planned, implementation not started.
  Priorities: acceptance holes, accepted numeric/predicate rules, executable
  documentation, existing source recovery across both backends, and bounded real
  application composition. No new language or contract semantics are authorized.
  Earlier mission statuses below are historical records to revalidate, not inherited state.

## Current mission

- [Complete ordinary record construction](ordinary-record-construction.md)
  — selected next under accepted ADR 0068; implementation and verification pending.
- [Source recovery execution](source-recovery-execution.md)
  — incomplete; callable ABI work does not establish graph execution or parity.
- [Source activation runtime foundation](source-activation-runtime-foundation.md)
  — prerequisite PASS; source execution remains unsupported.
- [Source disposition Graph IR preservation — Gate 4](source-disposition-graph-preservation-gate4.md)
  — recovery identity preservation verified; transformation remains incomplete.
- ConfigResolve inventory, scalar probe and record admission correction have
  checkpoints dated 2026-10-17. Complete record construction is now accepted in
  [ADR 0068](../architecture/decisions/0068-complete-ordinary-record-construction.md);
  execution remains to be implemented and verified.

## Prior review mission

- [Source disposition policy selection — Gate 3](source-disposition-policy-selection-gate3.md)
  — PASS on the review branch and waiting for human review; versioned route
  and host-policy selection are declarative and execution remains blocked.
  Gate 4 has not started.

## Prepared missions

- [Language maturation](lyraform-language-maturation.md) — revision 3 selected for the branch experiment; evidence-backed stages toward a ratified practical profile and
  ConfigResolve port and consumer (revision 3). First implementation endpoint: disposition Graph IR Gate 4.
  See the [assessment](../development/language-maturation-assessment.md).
- [ConfigResolve tagged scalar Value boundary](configresolve-tagged-value-boundary.md) — frozen at the generic declaration boundary; runtime construction and projection are deferred to the main-branch follow-up. See `docs/reports/2026-10-17-configresolve-lyraform-experiment-end-report.md`.

## Completed missions with explicit task-local result

- [Bounded unique-transfer forwarding chain](bounded-unique-transfer-forwarding-chain.md)
  — task records PASS on 2026-09-26 and links its checkpoint.
- [Agent authority and workspace hygiene](agent-authority-and-workspace-hygiene.md)
  — PASS on 2026-10-10; operational tooling only, with no language change.
- [Disposition foundation and source authority through Gate 2](disposition-foundation-and-source-authority-gate2.md)
  — PASS on 2026-10-10; source-derived topology remains declarative and
  execution-unsupported.
- [Gate 2 and hygiene convergence integration](gate2-hygiene-convergence-integration.md)
  — PASS and merged into `main` on 2026-10-10; combines both complete source
  ancestries without beginning Gate 3.

## Superseded missions

None are classified here. A newer document does not supersede an older task
without durable evidence saying so.

## Historically blocked missions

None are classified here. A document that merely discusses blocking conditions
is not evidence that its mission ended blocked.

## Unclassified missions

Every other task currently in this directory is unclassified for lifecycle
purposes. Read its durable checkpoint evidence before classifying it; do not
infer status from filenames, chat history, stale `.codex-run-state`, or a
passing present-day test.

Before selecting an unclassified historical mission again, add a bounded
convergence addendum or a new mission that names the current baseline and
satisfies the contract below.

## New-mission convergence contract

Every new selectable mission records all eight items:

1. user-visible behavior or bounded prerequisite and exact endpoint;
2. accepted law and one owner for each new semantic fact;
3. unresolved decisions and precise stop conditions;
4. existing components to reuse and transitional components to retire;
5. one positive example, one negative example, and one hostile mutation;
6. clean-checkout command or reproducible artifact and claimed maturity;
7. evidence capable of falsifying the claim; and
8. consolidation required before the next adjacent slice.

The gate report then states what became usable, what remains declarative or
unsupported, what became simpler or was removed, and which accepted law and
falsifying evidence were exercised.
