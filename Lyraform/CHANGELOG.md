# Lyraform compiler changelog

## v29_reusable_native_chain — active

- Promoted the reusable native language chain to the active v0.29 milestone.
- Made `main` and `Lyraform/compiler` the current authority.
- Added fail-closed source-operation coverage and the canonical Igor execution
  path; legacy parser/runtime behavior is now explicit oracle-only behavior.
- Connected scalar, target, guard, disposition, ownership, provider/effect,
  call, and deferred parallel-candidate identities through one preserved and
  independently revalidated callable-plan-v2 authority envelope.
- Added whole-language source-form and authority audits under a frozen language
  surface.
- Extended unique must-account outcome transfer through one bounded forwarding
  owner with LLVM/TinyVM parity and hostile artifact validation.
- Added the first declarative typed failure-consumer and policy-selection
  contract. It accepts canonical failure-envelope inputs but does not yet admit
  response-transition execution, propagation, retry, or runtime guard routing.
- Added complete declarative response-transition authority for the bounded
  `recover` and `transform` classes. Recovery closes the original obligation
  through typed success; transformation creates a linked successor failure.
  Source projection, graph routing, and execution remain deliberately absent.
- Added a LaTeX/TikZ semantic--syntax--graph projection map that illustrates
  each bounded failure-flow operation while visibly separating canonical law,
  provisional notation, and currently refused execution behavior.
- Added the first bounded executable failure-flow reference projection. One
  established typed failure envelope now crosses one explicit serial wire to
  one exact policy-selected ordinary function identity and emits a validated
  recovery or linked-transformation receipt. Source syntax, compiler/backend
  integration, retry, sinks, faults, and parallel failure routing remain
  outside the admitted slice.
- Accepted bounded disposition-evidence epochs and policy-visible retention
  limits as future implementation law.
- Recorded the canonical scheduling law: parallel opportunity is derived from
  graph and effect evidence, while deployment policy and measured runtime
  feasibility select execution mechanisms. No `async` or parallel source
  syntax is introduced.
- Prepared the first bounded mission for general effectful parallel scheduling
  through canonical resource/conflict facts, explicit dispositions, policy,
  calibrated provider selection, and serial-equivalent backend evidence.
- Implemented that mission's first executable family: exact profiled read-only
  scalar observations now carry provider, resource, access, conflict, serial-
  reference, join, and publication authority across Flowanalyst, Flowparallel,
  Flowoptimize, Flowlower, LLVM, and TinyVM. Native execution proves worker
  overlap while TinyVM retains deterministic serial projection. Unprofiled,
  forged, mutable, pointer-bearing, aggregate, fallible, and broader effect
  families remain refused.

## v27_namespaced_provider_chain — historical

- Promoted the language-chain slice to the active v0.27 milestone.
- Added explicit namespace aliases for imports and qualified ABI calls such as
  `curses.initscr()`, `libc.puts()`, and `linux.read()`.
- Preserved provider ownership in AST, semantic reports, and binding
  requirements, with ambiguity diagnostics for unqualified collisions.
- Added the ncurses-backed `sel` application proof and native-provider
  inventory/selection maintenance artifacts.

## v26_language_chain — historical

- Established the verified vertical language chain as the v0.26 milestone.
- Added semantic analysis, capability/ABI verification, optimization boundary,
  explicit LLVM lowering profiles, and a native ELF application proof.
- Added named-target semantic entrypoint checks while leaving target selection
  and separate artifact emission as the next lowering boundary.
- Packaged the reproducible `flowcat` application example with policy, reports,
  LLVM, expected output, and preserved inspection builds.

The v25 projection directory and branch names are historical; the active
implementation now lives at `Lyraform/compiler` on `main`.

## v25_symboltable_projection — historical

- Activated from the tagged `flowmini-v0.24-frontend-border` checkpoint on
  branch `v25-symboltable-projection`.
- Preserved v0.24 as a closed implementation line and created
  `flowmini_v25_symboltable_projection` without build or test artifacts.
- Established SymbolTable projection maturation, factual cross-links,
  provenance, projection coverage, and frontend-bundle hardening as the v0.25
  scope.
- Kept semantic resolution, contract checking, Graph IR, and runtime lowering
  outside v0.25.
- Advanced `flowmini.frontend_bundle` to version 2 with typed symbol/scope
  origins, canonical arena IDs where available, exact structural roles, and
  source-location provenance.
- Hardened the independent consumer with kind, role, ID, ownership, uniqueness,
  source-map, and reverse-origin validation; expanded the gate to seven goldens,
  one isolated run, and nineteen required failures.
- Preserved imports in structural inspection modes so import symbols and
  multi-file origins are observable without changing execution policy.
- Expanded SymbolTable projection coverage to 12/12 with `import_demo`.

## v24_explicit_ast — closed frontend-export border

- Added observable typed AST payloads for expressions, type references, and
  statements.
- Added recursive precedence-aware expression ownership and validation.
- Fixed raw-AST unary `not` ownership so its predicate operand is represented
  instead of silently disappearing; added focused unary/break/continue coverage.
- Added arena-owned statements and blocks with stable IDs and structural-parent
  validation.
- Added arena-owned top-level declarations with stable `DeclarationId` values,
  complete source-unit ownership, and validated JSON compatibility projections.
- Completed the C5 typed-statement migration, including explicit conditional,
  loop, placement, assignable-target, and return source-form ownership.
- Added structural SymbolTable projection from the AST; current focused golden
  coverage is 11/11 and the v0.24 factual export boundary is complete.
- Added canonical refined-type declarations with invariant expressions and
  ordered ABI blocks containing library/convention clauses, ABI type contracts,
  ABI structs, and extern functions. Their SymbolTable projection remains
  factual and unresolved.
- Established the accepted-language coverage matrix and canonical type policy.
- Current gates: AST goldens 26/26, SymbolTable projections 11/11, suite 78/78,
  and CTest 2/2.
- Added the versioned `flowmini.frontend_bundle` export, generic SymbolTable JSON
  snapshot, direct structural origin links, and an independent consumer gate
  with five goldens, an isolated consumer run, and eleven negative attacks,
  including real multi-file import provenance.
- Formally closed the v0.24 raw frontend/export border after the complete Tier 3
  Firetest passed on 2026-08-16.
- Semantic analysis, remaining accepted-language coverage, and Graph IR remain
  future work.

## v22_unit_kinds

- Added explicit source roles:
  - `program`
  - `unit`
- Enforced import law.
- Categorized examples into `pass`, `fail`, `support`, and `docs`.
- Added expected stdout and diagnostic checks.
- Suite baseline: `75 / 75`.

## v21_structural_bridge

- Integrated TokenTree as a structural token-tree inspection path.
- Integrated SymbolTable as a structural symbol projection path.
- Preserved the existing execution path.

## v20_bool

- Added internal `Bool`.
- Added `true` and `false`.
- Comparisons produce `Bool`.
- `if` and `while` require `Bool`.
- No implicit integer truthiness.

## Earlier versions

See `VERSION_INDEX.md`.
