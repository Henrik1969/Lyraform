# Lyraform mission lifecycle and selection

This directory contains selectable mission documents. A mission becomes active
only when Henrik or an authorized continuation explicitly selects its tracked
file. Presence here does not make a task current, and checkpoints elsewhere do
not grant task authority.

## Current mission

None. Selecting a mission requires an explicit current request or an authorized
continuation with a fresh mission identity and convergence contract.

## Completed missions with explicit task-local result

- [Bounded unique-transfer forwarding chain](bounded-unique-transfer-forwarding-chain.md)
  — task records PASS on 2026-09-26 and links its checkpoint.
- [Agent authority and workspace hygiene](agent-authority-and-workspace-hygiene.md)
  — PASS on 2026-10-10; operational tooling only, with no language change.

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
