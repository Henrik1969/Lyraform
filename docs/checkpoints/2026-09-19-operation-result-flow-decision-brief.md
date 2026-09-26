# Decision brief — refusal, failure and fault semantics

Date: 2026-09-19.

Status: **DECIDED — Candidate A accepted on 2026-09-19.**

Durable authority:
[ADR 0055](../architecture/decisions/0055-refusal-failure-fault-dispositions.md).

## Problem

What are the canonical dispositions of an attempted operation, and which of
them belong to the executable program's graph?

The design discussion identifies materially different cases:

```text
malformed source
incompatible types
provable guard contradiction
runtime division by zero
runtime guard violation
provider/environment failure
resource exhaustion
assertion failure
corrupt artifact or impossible runtime state
```

Calling all of these “errors” would collapse compiler refusal, legitimate
runtime outcomes and execution-integrity failures into one unsafe bucket.

## Current evidence

### Canonical language and architecture facts

- Failure is explicit flow, not exception-style hidden control transfer.
- Invalid source or statically illegal semantics are refused before execution.
- Failed state transitions publish no partial committed normal state.
- Rejected attempts, operational error states and committed mutations are
  distinct semantic families.
- Backends execute established semantics and may not invent failure routing.

### Canonical implementation behavior

- The parser and semantic analyzer emit structured refusals and no executable
  artifact for invalid source.
- `Outcome<Text,TextFailure>` proves one bounded tagged runtime outcome across
  LLVM and TinyVM.
- Public process/provider boundaries translate internal C++ exceptions to
  structured diagnostics or results.
- Durable error-state history records operational lifecycle separately from
  ordinary mutation.

### Design proposal under review

`meta-discusions.md` proposes three broad layers:

```text
program construction/admission refusal
expected runtime failure
broken-program or system fault
```

It also proposes one general precondition rule:

```text
precondition provably true
    admit without runtime check

precondition provably false
    refuse statically

precondition unknown
    lower runtime enforcement plus explicit failure flow
```

This is coherent with guards, division, bounds, representability, provider
requirements and other contracted operations, but it is not yet a durable
language decision.

### Undefined

- whether runtime failure and execution fault share one result algebra;
- whether faults are ordinary recoverable graph values;
- exact success/failure/fault cardinality for one attempt;
- unconsumed-failure and unconsumed-fault policy;
- typed category hierarchy and source syntax;
- partial/degraded results and diagnostic co-emission;
- recovery, retry, timeout, cancellation and backpressure semantics.

## Terminology proposed for this decision

```text
refusal
    no executable operation is admitted

failure
    the operation is valid in general, but this concrete value or environment
    prevents successful completion according to its declared contract

fault
    the execution model, trusted artifact, runtime invariant or implementation
    boundary has been violated or cannot be relied upon
```

Examples:

| Case | Proposed disposition |
|---|---|
| malformed declaration | refusal |
| incompatible `Bool -> int` | refusal |
| known `-1` under active `x > 0` guard | refusal |
| unknown divisor evaluates to zero | failure |
| sensor value violates active guard | failure |
| file missing/provider unavailable | failure |
| declared bounded allocation exhausted | failure |
| failed programmer assertion | fault unless its contract defines otherwise |
| contradictory validated artifact | fault |
| impossible backend representation | fault |

Resource exhaustion is contract-sensitive: a declared bounded provider may
expose exhaustion as an expected failure, while inability to preserve runtime
integrity may be a fault.

## Candidate A — three distinct dispositions

**Meaning**

Construction/admission refusal remains outside execution. An admitted runtime
attempt completes with exactly one semantic disposition:

```text
Success<T>
Failure<E>
Fault<F>
```

`Failure<E>` is ordinary typed failure flow and may be recovered, transformed,
retried or propagated according to contracts. `Fault<F>` is explicit
integrity-failure evidence routed to a separately governed containment policy;
it is not silently recoverable as ordinary business/data failure.

Tagged values, graph ports, provider responses and execution records may be
different projections of the same discriminated disposition.

Diagnostics, metrics and audit events may accompany any disposition but are
not additional semantic completions.

**Pros**

- keeps compile-time refusal outside runtime semantics;
- prevents ordinary recovery code from swallowing integrity corruption;
- gives expected failures normal typed graph behavior;
- preserves exactly-one completion and no partial commit;
- scales from guards and division to providers and bounded resources;
- allows stronger trust/containment policy for faults.

**Cons**

- requires a principled boundary between failure and fault;
- generic code may need separate failure and fault propagation contracts;
- assertions and resource failures need explicit classification rules;
- graph and function projections become richer than a binary Outcome.

**Consequences**

Runtime guard violation becomes `Failure<GuardViolation>` and commits nothing.
A forged or contradictory guard artifact becomes a fault at the consuming
boundary, not `GuardViolation`. Fault containment remains explicit and
versioned even if policy terminates the activation or process.

**Migration impact**

The bounded `TextOutcome` remains a success/failure projection. Existing
structured fatal diagnostics become evidence toward fault projection, not yet
a complete language contract.

## Candidate B — binary success/failure with fault categories

**Meaning**

Runtime attempts complete with `Success<T>` or `Failure<E>`. Integrity faults
are severe failure categories in the same algebra.

**Pros**

- smaller universal result model;
- reuses familiar tagged Outcome representation;
- one propagation mechanism for all unsuccessful completion.

**Cons**

- ordinary recovery may accidentally consume compiler/runtime corruption;
- policy must rediscover severity from categories at every boundary;
- weakens the architectural distinction between legitimate operation failure
  and an untrustworthy execution state;
- encourages catch-all handling under another name.

**Consequences**

Every recovery boundary must prove which failures it is authorized to handle.
Unknown categories must fail closed.

**Migration impact**

Most compatible with `TextOutcome`, but requires expanding it beyond its
current bounded authority.

## Candidate C — success/failure flow plus terminal out-of-band fault

**Meaning**

Ordinary operations use `Success<T> | Failure<E>`. A fault never becomes a
language value or graph port; the runtime reports structured evidence and
terminates the governed execution region.

**Pros**

- keeps ordinary graph failure handling simple;
- prevents continuation after an integrity failure;
- aligns with fail-stop safety profiles.

**Cons**

- fault routing becomes a second control mechanism outside the graph;
- containment region and termination policy become implicit unless carefully
  declared;
- distributed runtimes still need a transportable fault observation;
- may prevent safe quarantine/escalation strategies.

**Consequences**

Fault evidence remains explicit at runtime boundaries, but source programs
cannot transform or route it. Policy owns termination.

**Migration impact**

Fits current fatal CLI projections but does not generalize them into graph
semantics.

## Candidate D — contract-specific disposition only

**Meaning**

Each operation contract independently declares its unsuccessful outcomes;
there is no universal failure/fault distinction beyond shared diagnostics.

**Pros**

- maximum domain specificity;
- avoids prematurely freezing a universal type hierarchy.

**Cons**

- guard, division, bounds, providers and resources may invent incompatible
  mechanisms;
- generic propagation and recovery become impossible or adapter-heavy;
- backends and schedulers lack one completion law;
- undermines the goal of one semantic authority.

**Consequences**

The compiler can share the prove/refuse/enforce pattern, but runtime routing
remains fragmented.

**Migration impact**

Minimal immediately, expensive as more fallible operations mature.

## Recommendation

Recommend **Candidate A — three distinct dispositions**.

It preserves the most important boundaries revealed by the discussion:

```text
refusal is not runtime
failure is legitimate runtime flow
fault means execution integrity cannot be treated as ordinary failure
```

For one admitted attempt, success, failure and fault should be mutually
exclusive semantic completions. Diagnostics and events may co-emit because
they are observations, not competing completion states. Partial or degraded
work must be represented explicitly inside the selected disposition, including
whether anything was committed.

This recommendation also establishes the general compiler rule for an
operation precondition `P`:

```text
prove P       → admit without check
disprove P    → refuse statically
P unknown     → runtime enforcement producing declared Failure<E>
```

If enforcement itself discovers corruption or cannot preserve its contract,
the result is `Fault<F>`, not the operation's expected failure.

This decision does not choose source spelling, concrete carrier layout,
failure-category hierarchy, recovery syntax, or unconsumed-result policy. The
last of those was subsequently resolved by ADR 0056.

## Decision

Candidate A was accepted on 2026-09-19:

> Refusal remains outside execution. Every admitted attempt completes with
> exactly one of `Success<T>`, `Failure<E>`, or `Fault<F>`.

The accepted law is recorded in ADR 0055. ADR 0056 subsequently establishes
that unconsumed failure or fault paths are dangling wire errors. Carrier
syntax, concrete graph wiring, and recovery forms remain separate decisions.

## Historical choices presented

```text
A. Refusal outside execution; exclusive Success<T> | Failure<E> | Fault<F>
B. Refusal outside execution; exclusive Success<T> | Failure<E>, faults are failures
C. Success/failure flow; faults are structured terminal runtime events only
D. Every contract defines its own dispositions; no universal runtime algebra
E. Defer and define another prerequisite first
```
