# Changelog

This repository is a language-design and implementation lab. Numbered Flowmini
stages are intentionally preserved as historical implementation checkpoints.

## Current checkpoint

### 2026-10-07 — canonical convergence integration

- Completed the frozen-surface compiler convergence campaign through
  source-operation accounting, scalar/target/guard/disposition authority,
  provider/effect authorization, exact operation connectivity, and preservation
  across planning, optimization, LLVM preparation, and TinyVM preparation.
- Added executable authority and source-form audits that refuse semantic gaps
  rather than delegating them to legacy behavior.
- Isolated the Flowmini parser, `ModuleSpec`, and direct runtime behind
  `igor run-legacy`; `igor run` now uses the fail-closed canonical stage chain.
- Extended unique owned-obligation transfer through one bounded forwarding
  owner while retaining complete final-owner accounting.
- Added declarative typed failure-consumer and policy-selection contracts over
  exact ordinary function identities. The function input is a canonical
  failure-envelope projection; general response execution remains refused.
- Accepted bounded disposition-evidence epochs: evidence may compact only at a
  proven closure boundary and may never be silently discarded to satisfy a
  policy budget.
- Refreshed current-status, onboarding, alpha-testing, presentation, and
  component documentation. Exact verification totals remain in dated
  checkpoints rather than evergreen documents.

- Added the first executable read-only kernel profiles: `getpid` and
  `clock_gettime`, followed by the one-byte `getrandom` profile; each is
  policy-gated and tested through native LLVM-to-ELF execution. The remaining
  kernel declarations remain binding-ready but lowering-deferred.

- First real I/O capability slice: `flowcat_file_main` reads argv-supplied
  files through policy-authorized `open`/`read`/`write`/`close` bindings.

- Active milestone: Lyraform v0.29 reusable native language chain
- Implementation base: `Lyraform/compiler`
- Active branch: `main`
- Current integration baseline: the canonical authority envelope and current
  source-form audit are green; use `igor check` and `igor test` for the checked
  out revision and dated checkpoints for exact totals
- Current bundle contract: `flowmini.frontend_bundle` version 2

The detailed implementation history is maintained in
[`Lyraform/CHANGELOG.md`](Lyraform/CHANGELOG.md), and the active/historical stage
map is maintained in [`Lyraform/VERSION_INDEX.md`](Lyraform/VERSION_INDEX.md).

## Historical Flowcore v0.26 language-chain vertical slice

- Activated from the tagged `flowmini-v0.24-frontend-border` checkpoint.
- Preserved v0.24 as a closed implementation line.
- Added typed AST-to-symbol and AST-to-scope origin provenance.
- Hardened the versioned frontend bundle and independent consumer.
- Added semantic analysis, capability binding, optimization preservation,
  explicit LLVM lowering profiles, and the first native ELF application.

## Flowmini v25_symboltable_projection — historical projection milestone

- Established the typed AST-to-symbol and AST-to-scope origin boundary.
- Hardened the versioned frontend bundle and independent consumer.

## Flowmini v24_explicit_ast

- Completed the typed raw AST and factual structural SymbolTable projection.
- Established the versioned frontend export and independent-consumer boundary.
- Closed the raw frontend/export border with recorded Tier 3 Firetest evidence.

## Flowmini v22_unit_kinds — historical

- Added explicit source unit roles:
  - `program` = executable/root source unit
  - `unit` = defining/importable source unit
- Enforced import law:
  - units may be imported
  - programs may not be imported as ordinary units
  - imported files must not define `main`
  - root execution requires a program with `main`
- Categorized examples.
- Added expected stdout and diagnostic substring suite support.
- Suite passes `75 / 75`.

## Flowmini v21_structural_bridge — historical

- Integrated TokenTree as a structural token-tree inspection path.
- Integrated SymbolTable as a structural symbol projection path.
- Preserved existing execution behavior.

## Earlier Flowmini snapshots

Earlier snapshots are preserved under `_archive/flowmini/previous-stages/` and
indexed in `Lyraform/VERSION_INDEX.md`.
