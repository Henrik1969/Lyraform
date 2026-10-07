# Legacy oracle deprecation boundary

Date: 2026-09-26.

## Decision

The Flowmini parser, `ModuleSpec`, and direct runtime are deprecated legacy.
They are excluded from Lyraform's canonical source, semantic, artifact,
planning, and execution authority.

Legacy behavior may be used to:

- recover historical intent;
- identify questions requiring an explicit language decision;
- preserve compatibility obligations;
- provide differential and regression evidence while canonical coverage grows.

It may not admit source, settle an unresolved semantic question, repair a
canonical refusal through fallback, or become a backend's alternate language
model. A behavior observed in legacy becomes canonical only through an explicit
decision, canonical representation, validation contract, and tests.

This boundary is accepted in
[ADR-0061](../architecture/decisions/0061-legacy-is-deprecated-oracle-only.md).

## Operational isolation

```text
igor run
    -> bounded canonical staged chain
    -> fail closed on incomplete coverage

igor run-canonical
    -> explicit alias of the same canonical path

igor run-legacy
    -> deprecated behavior oracle
    -> unconditional human-readable deprecation warning
```

Canonical refusal never triggers legacy fallback. The old executable name and
the `flowmini.*`/`flowcore.*` serialized namespaces remain only where scripts or
compatibility contracts require them; names do not confer authority.

The machine-readable authority map classifies legacy source as
`deprecated_legacy`, `oracle_only_non_authoritative`, and
`excluded_from_canonical`. The `lyraform_legacy_deprecation` test guards that
classification, Igor routing, warning, ADR status, and current documentation.

## Verification

On `main` baseline `ecdac7184236dab24423ca7495e882f771cdf287` plus the retained
uncommitted convergence stages:

- authority-map and current-documentation guards: PASS;
- legacy-deprecation guard: PASS;
- four focused convergence gates: **4/4 PASS**, 0.31 seconds;
- `igor run` canonical positive: PASS, native exit status 42;
- canonical unsupported-source refusal: PASS, exit status 2 with
  `FLOWANALYST_SOURCE_OPERATION_GAP`;
- explicit `igor run-legacy` oracle probe: PASS, exit status 0 and mandatory
  deprecation warning;
- accumulated-tree canonical suite: **170/170 PASS**, 79.92 seconds;
- `master` touched: NO;
- FlowLFS touched or merged: NO;
- force used: NO.

No C++ semantic implementation changed at this boundary. The sanitizer,
Valgrind, determinism, and fresh-build evidence recorded at Gate 1 remains the
applicable implementation evidence.

## Result

**PASS.** Legacy is retained as evidence, not as a second compiler authority.
Lyraform can consult its history without inheriting accidental semantics from
it.
