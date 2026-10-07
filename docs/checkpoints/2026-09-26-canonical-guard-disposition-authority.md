# Canonical guard/disposition authority — Stage 2C

Date: 2026-09-26.

## Objective

Determine whether bounded static guard facts, disposition facts, and their
lowering operations share one identity/type authority or independently
reconstruct semantic decisions.

The language freeze remains active. Runtime guard overwatch, general failure
routing, new guard syntax, and new disposition meaning are excluded.

## Scout result

Flowanalyst is already the sole producer of `lyraform.guard_fact` and
`lyraform.disposition_fact`. Shared Flowcontracts validation enforces:

- guard lifecycle and unique fact identity;
- exact affected operation and statement linkage;
- affected destination membership in the guard dependency set;
- ready plans contain only statically proven guard transitions;
- disposition operation, statement, expression, scope, and function-owner
  identities;
- canonical scalar payload type and destination route;
- exact, nonempty guard-proof sets with no omitted or extra proof;
- exactly-one completion and atomic destination commit;
- provenance and downgrade resistance;
- every proven guarded operation has disposition authority.

One duplicate remained before this contract boundary: static guard state
tracking looked up initialized declaration and placement destinations by name
even though bounded scalar facts had already established those identities.

## Implementation

- guard initialization state consumes scalar declaration destination identity;
- statically checked scalar transitions consume scalar placement destination
  identity;
- call-producing transitions outside the scalar-fact slice consume the already
  constructed operation destination before fail-closed runtime-guard refusal;
- no guard or disposition schema changed;
- no runtime-dependent transition became executable.

The focused positive check now asserts one exact chain:

```text
scalar destination identity
    == operation result identity
    == guard dependency affected by the operation
    == disposition success-route identity

scalar destination type
    == disposition success payload type
```

## Gate evidence

- focused scalar/target/guard/authority gates: **6/6 PASS**, 5.30 seconds;
- guard lifecycle corpus: activation, composition, selective unguard,
  disposition authority, five semantic refusals, three malformed forms, one
  contextual collision, four preserved stages, eleven hostile guard facts,
  twenty hostile disposition facts, and LLVM/TinyVM consumers: PASS;
- Igor doctor: PASS;
- accumulated-tree canonical suite: **170/170 PASS**, 80.76 seconds;
- Clang 18.1.3 ASan/UBSan scalar and guard gates: **2/2 PASS**, 7.50
  seconds, with leak detection disabled under host ptrace supervision;
- Valgrind 3.22.0 proven-safe and proven-violation probes: **2/2 clean**,
  zero errors and zero bytes live at exit;
- repeated guard semantic reports: byte-identical PASS;
- documentation, authority, and legacy-deprecation drift guards: PASS;
- `git diff --check`: PASS.

## Gate

Stage 2C: **PASS**. Bounded static guard evaluation, guard evidence,
disposition evidence, scalar identity/type authority, and operation identity
now form one validated chain. Runtime-dependent guards remain refused because
the general failure-flow carrier is not canonical.

Recommended next stage: audit TextOutcome disposition and ownership-transfer
production. That work may only remove duplicated authority or strengthen an
already accepted contract; it must not introduce general propagation or policy
sink semantics.
