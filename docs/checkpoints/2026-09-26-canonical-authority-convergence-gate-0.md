# Canonical language authority convergence — Gate 0

Date: 2026-09-26.

## Result

The language surface is frozen for compiler convergence. Eleven current
semantic/execution surfaces and five known bridge gaps are classified in the
machine-readable canonical authority map. No syntax or semantic meaning was
added.

## Baseline

- branch and local upstream: `main` at
  `ecdac7184236dab24423ca7495e882f771cdf287`;
- retained the verified current-truth and bounded-forwarding working-tree
  stages;
- no staged, committed, pushed, `master`, or FlowLFS change;
- unrelated `meta-discusions.md` preserved.

## Reconnaissance

The compiler is not split at the scalar type algebra: both direct bounded
scalar execution and Flowanalyst use `flowcontracts/scalar_analysis.hpp` and
`scalar_semantics.hpp`. The remaining split is source and operation authority:

1. direct bounded scalar execution resolves its structural model and adapts it
   to `ModuleSpec` without consuming the staged semantic-report projection;
2. Flowanalyst produces the canonical staged facts used by the compiler chain;
3. one explicit `flowmini::parseModule(tokens)` fallback still owns accepted
   non-migrated compatibility source;
4. canonical member reconstruction is semantically admitted but has no shared
   executable reconstruction operation;
5. general failure routing remains a language decision and is not eligible for
   autonomous bridge implementation.

The compatibility parser remains an oracle and compatibility implementation,
not canonical authority. The direct scalar adapter is transitional but already
uses shared scalar rules. LLVM and TinyVM remain consumers, never source
semantic owners.

## Deliverables

- mission: `docs/tasks/canonical-language-authority-convergence.md`;
- authority map:
  `docs/architecture/canonical-language-authority-map-v1.json`;
- executable drift guard: `tools/check-canonical-language-authority.sh`;
- canonical CTest: `lyraform_canonical_authority_freeze`.

The guard validates unique surface identities, required classifications,
language-freeze prohibitions, the one explicit legacy fallback, shared scalar
authority, direct-adapter visibility, and the documented member execution gap.

## Bridge dependency order

```text
source routing and completeness visibility
    -> one staged semantic producer for already admitted facts
    -> explicit operations connecting admitted semantic islands
    -> exact preservation and dual-backend consumption
    -> compatibility fallback retirement by proven source family
    -> whole-language authority audit
```

An already decided but non-executable semantic operation may be connected
without a new language decision. Owned parameter modes, general failure
routing, indexing/bounds, references, aliases, and other unresolved meanings
remain decision gates and are excluded.

## Focused verification

- authority-map JSON parse: PASS;
- freeze script syntax: PASS;
- standalone authority freeze: PASS;
- focused authority/convergence CTests: **10/10 PASS**, 14.52 seconds;
- `git diff --check`: PASS.

- final `./igor test`: **168/168 PASS**, 83.32 seconds.

## Gate

Gate 0: PASS for freeze and authority inventory.

The first implementation bridge must be selected from already accepted
meaning. General failure routing and owned parameter transfer are excluded
because they require new semantic decisions.

Post-gate reconnaissance rejected aggregate reconstruction as the first bridge.
Although member write authority is admitted by ADR 0052 and Gate 9, ordinary
record construction is still an unsupported operand, ordinary aggregate ABI
layouts are absent, and construction completeness was deliberately deferred.
Lowering reconstruction now would therefore invent prerequisite semantics
during the freeze. The gap remains an honest refusal and is classified as
`blocked_prerequisite` in the authority map.

The next bounded candidate is source-to-operation coverage for the existing
provider-free staged execution path. It may proceed only if admission can prove
that every execution-relevant source statement has a canonical operation; an
`ok` report that silently omits a statement is not executable authority.
