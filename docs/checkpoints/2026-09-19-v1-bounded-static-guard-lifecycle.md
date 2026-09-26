# Lyraform v1 bounded static guard lifecycle checkpoint

Date: 2026-09-19.

Gate: **PASS** for the bounded straight-line scalar stage.

## Baseline

- branch: `main`;
- `HEAD`: `6ba9db303db453af952327069814c3a607b26d03`;
- `origin/main`: `6ba9db303db453af952327069814c3a607b26d03`;
- protected scope: `master` and `flowlfs-v0.1-alive` untouched;
- prior gate: guard reconnaissance stopped for a public syntax decision;
- accepted syntax: ADR 0053, keyword-led named lifecycle statements.

The pre-stage canonical suite was **164/164 PASS**. The worktree contained only
the guard scout and syntax-decision documents created for this stage before
implementation began.

## Bounded question

What does a named active guard mean for initialized local `int` values and
straight-line identifier placements, and where is preservation proved before
backend execution?

## Canonical answer

```lyraform
x : int(1)
guard positive : x > 0
4 -> x
unguard positive
```

`positive` is a scoped guard identity. Activation proves its predicate in the
current known state. Every later placement affecting a dependency is evaluated
against the candidate state before the compiler admits the transition.

The canonical classifications are:

```text
proven_safe       admit; no runtime check is emitted
proven_violation  refuse during semantic analysis
not_provable      refuse closed until runtime overwatch has a valid failure model
```

A violating transition never commits and never weakens the guard. Multiple
active guards compose. `unguard name` removes only that identity. Guard state
is maintained per lexical block and is not exported past block analysis.

## Authority accounting

- The structural frontend owns complete guard/unguard syntax and source
  provenance through dedicated AST statements.
- Symbol projection owns guard identity as a `Contract` symbol with
  `contract_role=guard`; guard contracts are excluded from type identity.
- Flowanalyst owns dependency resolution, bounded constant-state proof,
  lifecycle state, admission and diagnostic classification.
- `lyraform.guard_fact` version 1 owns serialized proof evidence.
- Flowcontracts validates lifecycle, identity, dependencies, provenance,
  classification/execution agreement and affected-operation linkage.
- Flowparallel, Flowoptimize and Flowprepare preserve the facts exactly.
- LLVM and TinyVM execute only transitions already admitted by semantic
  authority; neither backend decides guard truth.

## Implemented bounded slice

Supported:

- `guard <name> : <predicate>` and `unguard <name>`;
- initialized local `int` declarations;
- identifier dependencies resolved to scalar symbols;
- integer comparisons `==`, `!=`, `<`, `<=`, `>`, `>=`;
- integer literals, known local integer values and signed literal candidates;
- straight-line identifier placement;
- composed guards and selective deactivation;
- deterministic facts for activation, affected transitions and explicit
  deactivation;
- fail-closed unknown runtime values.

Explicitly outside this slice:

- runtime overwatch and guard-failure routing;
- `if`, loops and cross-control-flow proofs;
- conjunction/disjunction predicates;
- aggregate/member, index, alias and reference dependencies;
- parameters, cross-call preservation and effect summaries;
- concurrency and optimizer guard elimination;
- lexical guard-block syntax.

## Serialized contract

Each fact carries:

```text
format/version
event and classification
guard symbol identity and name
statement, scope and predicate identities
sorted dependency symbol identities
affected operation identity where available
execution disposition
source/AST/line/column provenance
```

A ready plan cannot contain `proven_violation` or `not_provable`. Transition
facts must refer to an assignment or call operation that writes one of the
declared dependencies. Pre-stage artifacts without `guard_facts` remain valid
as compatibility artifacts and make no guard-proof claim.

## Evidence

Positive evidence proves:

- activation and a safe placement;
- explicit deactivation followed by an otherwise violating placement;
- two composed guards on one dependency;
- selective removal of one guard while the other remains active;
- deterministic byte-identical analysis;
- exact preservation through semantic, execution, optimization and backend
  preparation artifacts;
- accepted lowering by LLVM and TinyVM.

Negative evidence proves refusal of:

- a false guard at activation;
- a statically violating transition;
- a call result whose preservation is not provable;
- `unguard` of an inactive identity;
- three incomplete or excessive source forms.

The established typed-binding spelling `guard : int(1)` remains structurally
valid. Statement-start expression use of `guard` or `unguard` is reserved by
ADR 0053.

Ten hostile fact mutations cover invalid guard identity, missing/wrong
dependencies, invalid predicate identity, invalid provenance, contradictory
classification, contradictory execution, wrong operation linkage, unknown
event and unsupported version. Flowvalidate, LLVM lowering and TinyVM lowering
all refuse the forged artifacts.

## Verification

- `./igor doctor`: PASS;
- `./igor build`: PASS;
- focused guard lifecycle: **1/1 PASS**, covering two positive fixtures, four
  semantic refusals, three malformed forms, one contextual-name collision,
  four preserved stages, ten hostile facts and two backends;
- related scalar/analyzer collision set: **4/4 PASS**;
- canonical `./igor test`: **165/165 PASS**, 75.77 seconds;
- Clang 18.1.3 Debug ASan/UBSan focused guard suite: **1/1 PASS**, 1.17
  seconds, with LeakSanitizer disabled because the host ptrace wrapper is
  incompatible;
- Valgrind 3.22.0 admitted and violating analyzer probes: **0 errors**, **0
  bytes in 0 blocks at exit** for both;
- `git diff --check`: PASS;
- changed shell script passes `sh -n`.

Verification is local Linux evidence, not safety certification, fresh-clone or
cross-platform assurance.

## Untouched scope

No exceptions, assertion semantics, runtime failure handler, aggregate guard,
hidden alias mutation, new source extension, backend semantic inference,
FlowLFS content, `master` history, commit, staging or push was introduced.

## Remaining gap and next gate

The static lifecycle question is answered. The first not-provable transition
requires pre-commit runtime overwatch, but Lyraform does not yet have a
canonical general guard-failure value and routing model. Choosing one changes
language failure/effect semantics and cannot be selected by implementation
convenience.

See
[the runtime guard-failure decision brief](2026-09-19-runtime-guard-failure-decision-brief.md).

GATE: PASS — BOUNDED STATIC GUARD LIFECYCLE

The next gate was resolved on 2026-09-19: Candidate D was accepted. Runtime
dependent transitions remain refused until Lyraform has one general explicit
failure-flow model. See ADR 0054 and the general failure-flow scout.

NEXT GATE: PASS — FAIL-CLOSED RUNTIME-GUARD DISPOSITION RECORDED
