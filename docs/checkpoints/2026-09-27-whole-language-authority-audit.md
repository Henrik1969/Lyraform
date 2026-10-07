# Whole-language authority audit — Gate 5

Date: 2026-09-27.

## Objective

Prove that every source form represented by the current typed frontend, every
captured graph declaration, and the remaining accepted legacy entry points has
one explicit authority disposition. This is an audit under the language freeze,
not a syntax or semantics expansion.

## Baseline

- branch: `main`;
- HEAD and `origin/main`:
  `ecdac7184236dab24423ca7495e882f771cdf287`;
- Stage 4B complete authority envelope: PASS;
- `master` and `flowlfs-v0.1-alive`: protected and untouched;
- unrelated `meta-discusions.md`: untouched.

## Result

The machine-readable
[source-form audit](../architecture/canonical-source-form-audit-v1.json)
contains 67 bounded form dispositions:

| Classification | Rows |
|---|---:|
| canonical and connected end to end | 36 |
| deprecated legacy, oracle-only, and isolated | 2 |
| recognized but explicitly unsupported | 19 |
| refused before semantic admission | 9 |
| historical and non-active | 1 |

The rows deliberately partition broad structural kinds. For example, an
initialized admitted `let` is canonical-connected, while an uninitialized or
unresolved-type `let` is explicitly unsupported. A statically decidable guard
is connected, while a runtime-dependent guard is refused until general failure
flow exists. A bounded Text output statement with one authorized provider is
connected, while invalid output type/provider combinations are refused.

This prevents the structural AST's broad recognition surface from being
mistaken for broad semantic or execution admission.

## Executable drift protection

`tools/check-canonical-source-form-audit.sh` extracts the currently emitted
source-unit, top-level, statement, statement-source-form, expression, and type
reference kinds from the typed AST implementation and requires every kind to
appear in the audit. It separately guards all six captured graph declaration
families and the explicit `parseModule`/`igor run-legacy` isolation boundary.

Any future typed AST kind therefore fails the gate until it receives one of the
five permitted dispositions. The guard does not infer semantic authority from
mere parser recognition.

## Important retained boundaries

- ordinary record construction/layout, general lists/arrays/generics,
  member/index placement execution, runtime-dependent guards, and general
  failure flow remain unsupported or refused;
- deferred parallel candidates remain non-executable evidence;
- graph execution is canonical only for the validated native graph-plan-v2
  subset;
- `module` source and `igor run-legacy` remain deprecated oracle-only routes;
- historical Flowmini version records remain historical, not active language
  authority.

## Verification

- source-form manifest schema, uniqueness, category completeness, typed-kind
  coverage, graph coverage, and legacy isolation guard: PASS;
- configured canonical suite: **173/173 PASS**, 62.87 seconds;
- fresh GCC and Clang trees reconfigured with the audit gate; current-document,
  authority-freeze, source-form-audit, and legacy-isolation tests: **4/4 PASS**
  in each tree;
- Stage 4B fresh GCC/Clang, sanitizer, Valgrind, backend parity, and
  determinism evidence remains applicable because Gate 5 changes no compiler
  implementation or artifact contract;
- final configured CTest and documentation/drift results are recorded below.

## Gate 5

**PASS.** Every current typed source kind, captured graph declaration family,
and accepted legacy entry route has an explicit disposition. No admitted
construct is knowingly assigned meaning by both the canonical and legacy
models, and no recognized unsupported form is represented as executable merely
because it parses.

This completes the canonical-language-authority convergence campaign under its
language freeze. Reopening the language surface or selecting the first deferred
semantic family requires an explicit post-campaign decision; it is not implied
by this audit.
