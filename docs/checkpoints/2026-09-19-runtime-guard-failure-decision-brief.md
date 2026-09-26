# Decision brief — runtime guard failure and pre-commit overwatch

Date: 2026-09-19.

Status: decided — Candidate D accepted on 2026-09-19. See ADR 0054 and
ADR 0055.

## Problem

When an active guard cannot be proved statically, what observable failure does
runtime pre-commit overwatch produce, and where is that failure routed?

```lyraform
x : int(1)
guard positive : x > 0
sensor() -> x
```

The runtime can compute a candidate and test `candidate > 0` before commit.
What it must not invent is the meaning of the false branch.

## Why the decision is required

The choice establishes public failure and effect semantics. It determines:

- whether guarded placement itself produces a value;
- whether callers must acknowledge a possible guard refusal;
- whether failure terminates an activation, a function, a graph node or a
  larger execution region;
- how recovery is expressed without exceptions;
- how guard identity, candidate state, dependencies and provenance survive;
- how LLVM, TinyVM, parallel execution and durable error history agree;
- whether an unhandled guard failure is an ordinary typed failure or a policy
  termination.

The repository has a bounded tagged `TextOutcome` and durable error-state
history, but neither is canonical general language-level guard failure
authority. Lyraform explicitly does not use exception-style unwinding.

## Invariants common to every viable model

```text
compute candidate
evaluate every affected active guard against candidate/new state
on success: commit once
on failure: do not commit, do not weaken the guard
```

Failure evidence must include the guard identity, predicate identity,
candidate/new state or a safe representation of it, dependencies, source
operation and provenance. Machine-readable paths cannot contain personality
output.

## Candidate A — explicit outcome-producing placement

**Meaning**

A placement that may fail guard overwatch has an explicit typed result such as
`Outcome<Commit, GuardFailure>`. Source syntax must bind, route or explicitly
discard that result.

**Pros**

- failure is visible in types and call contracts;
- no hidden control transfer or exception unwinding;
- local recovery can be deterministic and explicit;
- backend-neutral tagged representation follows existing bounded outcome work.

**Cons**

- current `value -> destination` syntax has no result position;
- requires a general Outcome/error algebra and recovery syntax first;
- changes effect and function-signature reasoning;
- ordinary placement becomes syntactically heavier when proof is unavailable.

**Consequences**

The compiler must distinguish infallible proved placement from fallible guarded
placement and require explicit result handling. Existing unhandled placement
syntax must refuse when runtime failure is possible.

**Migration impact**

No currently admitted runtime guard source exists. New syntax and general
failure contracts are required.

## Candidate B — explicit enclosing failure effect

**Meaning**

A guarded placement may raise a declared, typed failure effect to its enclosing
function, activation or graph node. This is explicit effect propagation, not
stack-unwinding exceptions. A handler or declared propagation boundary owns
recovery.

**Pros**

- keeps placement syntax concise;
- composes naturally with function and graph execution boundaries;
- can carry structured guard provenance;
- admits centralized policy without committing invalid state.

**Cons**

- Lyraform's general effect/failure propagation syntax is not frozen;
- an implicit effect would hide a real control edge;
- handler order and composition with multiple guards require authority;
- parallel cancellation and backpressure consequences are unfinished.

**Consequences**

Functions and graph activations need explicit failure-effect contracts. Every
backend must lower the same pre-commit branch and propagation edge.

**Migration impact**

Requires a general typed effect and handler model before runtime guards can be
admitted.

## Candidate C — execution-policy termination with durable error event

**Meaning**

Guard failure emits a structured durable error event and terminates a defined
execution region according to declared policy. It produces no recoverable
language value at the placement site.

**Pros**

- operationally simple and fail-closed;
- aligns with durable diagnostic/error-state infrastructure;
- suitable for profiles that forbid local recovery;
- avoids exception semantics.

**Cons**

- turns ordinary invalid external input into region termination;
- recovery depends on external policy rather than ordinary language flow;
- termination region and restart semantics are not yet canonical;
- durable logging is not itself a language failure model.

**Consequences**

Profiles must define termination, publication, restart and persistence rules.
This may be useful as one policy, but making it the only source meaning would
over-specialize guards toward assertions.

**Migration impact**

Requires execution-profile and error-history integration, plus exact region
semantics.

## Candidate D — keep unknown transitions compile-time refused

**Meaning**

Only statically proved guard preservation is admitted. Runtime-dependent
transitions remain compile errors until the general failure/effect model is
chosen independently.

**Pros**

- preserves current fail-closed behavior;
- introduces no hidden control transfer or guessed recovery;
- keeps backend behavior identical;
- is compatible with every later runtime model.

**Cons**

- guards cannot yet protect ordinary runtime input;
- useful programs remain unexpressible;
- does not complete the semantic brief's runtime direction.

**Consequences**

The bounded static stage remains the only admitted guard execution. Runtime
overwatch is delayed, not rejected as a design goal.

**Migration impact**

None. This is the current implementation state.

## Rejected model — assertion/trap or exception unwinding

Treating every guard failure as a programmer assertion, process trap, or
exception-style stack unwind is not a viable default. External data can
legitimately violate a guard, and Lyraform's safety direction requires explicit
failure contracts rather than invisible exceptional control flow.

## Recommendation

Retain Candidate D now and define the general typed failure/effect boundary as
the prerequisite. Candidate A is the clearest local model because it makes a
fallible state transition explicit and avoids hidden control flow. Candidate B
may be the better compositional projection for functions and graph
activations. They may ultimately be two projections of one canonical failure
fact, but declaring that equivalence now would itself be a semantic decision.

Candidate C should remain an execution-policy projection for environments that
choose termination, not the universal language meaning.

## Historical choices presented

```text
A. Explicit outcome-producing placement
B. Explicit enclosing failure effect
C. Policy termination with durable error event
D. Keep runtime-dependent transitions refused until a general model exists
E. Define another prerequisite first
```

No runtime guard lowering should proceed until this decision or its prerequisite
is established.

## Decision

Candidate D was accepted for the current guard stage.

Lyraform already has an architectural law that failures are explicit flow,
not exception-style hidden control transfer. What remains unresolved is the
canonical language-level carrier and routing contract for that flow.

ADR 0055 now establishes the common disposition algebra: a runtime guard
violation is an expected `Failure<GuardViolation>`, while contradictory
trusted enforcement state is a `Fault<F>`. This does not yet authorize runtime
guard lowering because the disposition-routing and carrier contracts remain
unresolved.

Therefore:

```text
statically safe guard transition
    admitted

statically violating guard transition
    compile-time semantic refusal

runtime-dependent guard transition
    refused until general failure-flow semantics are canonical
```

Runtime guard maturation must reuse the general failure-flow model. It must not
invent a guard-specific exception, trap, result type, termination policy, or
recovery channel. The durable architectural law is recorded in
[ADR 0054](../architecture/decisions/0054-failure-is-explicit-flow.md).
The result-disposition law is recorded in
[ADR 0055](../architecture/decisions/0055-refusal-failure-fault-dispositions.md).

The prerequisite scout and next bounded decision are recorded in
[the failure-flow scout](2026-09-19-general-failure-flow-scout.md) and
[the operation-result decision brief](2026-09-19-operation-result-flow-decision-brief.md).
The next unresolved safety boundary is documented in
[the unconsumed-disposition decision brief](2026-09-19-unconsumed-failure-fault-decision-brief.md),
which was subsequently resolved by ADR 0056: an incomplete guard-failure route
is a dangling wire error.
