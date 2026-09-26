# Canonical Disposition Authority — Mission 01

## Mission type

Compiler convergence / failure-flow foundation.

## Authority

ADR 0054 through ADR 0058 and the standing Stage Execution Protocol.

## Baseline

Begin on synchronized `main` at
`6ba9db303db453af952327069814c3a607b26d03` with the in-progress bounded static
guard stage preserved in the same isolated working tree.

No staging, commit, push, branch mutation, or repository-host mutation is
authorized by this mission.

## Objective

Establish the smallest versioned canonical disposition component that can be
produced by semantic analysis, independently validated, preserved exactly
through current stage artifacts, and rejected when a consumer or hostile
mutation changes its meaning.

This mission proves the authority and projection boundary. It does not enable
new runtime failure behavior.

## Bounded semantic slice

The first producer is the already-admitted, statically proven guarded scalar
transition. Its disposition fact records that the operation's admitted
projection can complete successfully and that the potential
`Failure<GuardViolation>` branch was eliminated by semantic proof before
execution.

The fact must retain linkage to:

```text
lowering operation identity
source statement and expression identity
owning function/scope identity where available
success payload/destination type identity
guard fact identity or identities establishing admission
exclusive completion law
commit law
source provenance
```

This is not a claim that all operations are infallible or that runtime faults
cannot occur. It is a bounded proof about the admitted guarded transition.

## Required work

### Phase 0 — reconnaissance

- Confirm branch, HEAD, origin/main, and dirty paths.
- Inventory existing `TextOutcome`, graph failure, provider result, guard fact,
  scalar fact, and artifact-preservation contracts.
- Record which fields are canonical facts and which are current projections.

### Phase 1 — canonical contract

- Add a compiler-stage-independent `lyraform.disposition_fact` version 1
  contract in Flowcontracts.
- Represent possible dispositions as an explicit closed set for this bounded
  fact; do not infer them from missing fields.
- Require exactly-one runtime completion cardinality.
- Record commit behavior explicitly.
- Validate operation, source, guard-proof, type, and provenance linkage.
- Preserve compatibility with artifacts created before disposition authority by
  treating absence as no claim, not implicit success.

### Phase 2 — semantic producer

- Make Flowanalyst emit the fact only for the bounded statically proven guarded
  scalar transition.
- Derive it from canonical scalar, guard, operation, symbol, and origin facts.
- Do not dispatch on fixture names or application policy.
- Do not emit an executable disposition fact for blocked plans.

### Phase 3 — projection preservation

- Prove exact fact preservation through Flowparallel, Flowoptimize, Flowprepare,
  LLVM preparation, and TinyVM preparation where those stages currently carry
  the lowering plan.
- Consumers validate the canonical fact before accepting it.
- No stage may reinterpret success, failure, fault, commit, type, guard proof,
  or provenance.

### Phase 4 — hostile validation

At minimum reject mutations to:

```text
format or version
fact identity
operation/statement/expression linkage
completion cardinality
possible-disposition set
success type
commit law
guard-proof linkage
provenance
duplicate fact identity
missing or invented operation
blocked-plan promotion
```

LLVM and TinyVM preparation must reject the same forged authority.

### Phase 5 — developer evidence

- Add deterministic human and structured diagnostics where the bounded
  validator exposes a disposition mismatch.
- Follow ADR 0057: report the failed fact and relation; do not guess source
  intent.
- Do not call a diagnostic a recovery route.

### Phase 6 — verification

Run:

```text
focused disposition contract tests
focused guard lifecycle tests
artifact identity/preservation tests
LLVM and TinyVM hostile-consumer tests
Igor doctor
Igor build
Igor test
git diff --check
available focused sanitizer and Valgrind probes where meaningful
```

Record exact counts and limitations.

## Explicit non-goals

Do not implement:

- runtime-dependent guard overwatch;
- guard recovery or propagation syntax;
- top-level failure-policy syntax;
- fault-containment regions;
- generalized function failure effects;
- universal tagged `Disposition<T,E,F>` source values;
- graph failure-port syntax;
- provider-response migration;
- retry, timeout, cancellation, backpressure, or parallel failure composition;
- exceptions, hidden unwinding, implicit termination, or diagnostic-and-drop;
- changes to `master` or `flowlfs-v0.1-alive`.

## Stop conditions

Stop rather than guess if:

- the fact requires selecting public source syntax;
- operation identity cannot be preserved across current artifacts;
- a current stage strips or rewrites the fact;
- the implementation would classify an unknown runtime guard as executable;
- a failure/fault route would be silently synthesized;
- schema compatibility would require treating absence as implicit success;
- canonical or hostile tests fail without a bounded correction;
- unrelated user work would be overwritten.

## Gate 1

PASS only when:

```text
canonical disposition fact v1 exists
bounded guarded transition emits it deterministically
independent validation passes
all current carrying stages preserve it exactly
hostile mutations are rejected by validator, LLVM, and TinyVM consumers
runtime-dependent guards remain refused
canonical tests pass
no exception or implicit failure route is introduced
```

At Gate 1, produce a checkpoint and apply Autonomous Next-Stage Selection.
