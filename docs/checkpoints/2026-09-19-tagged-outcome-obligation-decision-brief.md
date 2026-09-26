# Decision brief — tagged outcome route obligation

Date: 2026-09-19.

Status: **DECIDED — Candidate A accepted on 2026-09-19.**

Durable authority:
[ADR 0059](../architecture/decisions/0059-tagged-outcome-must-account.md).

## Problem

When an operation routes `Failure<E>` into a tagged `Outcome<T,E>` value, has
the failure wire reached a complete destination merely because the value was
stored, or does the value carry an outstanding obligation that must be
accounted for before its lifetime or enclosing boundary ends?

```lyraform
outcome : TextOutcome(concat_outcome(left, right))
```

If `outcome` is never inspected, recovered, or propagated, the bytes are stored
but the possible `TextFailure` has no final accountable disposition.

## Why the decision is required

The choice affects:

- dangling-wire validation;
- variable and function-exit analysis;
- copying, aliases, and ownership of outcome values;
- branch exhaustiveness;
- recovery and propagation;
- optimizer dead-value elimination;
- graph/value projection equivalence;
- debugger reporting;
- LLVM and TinyVM parity.

Promoting the existing tagged representation without this law could make a
canonical fact certify a route that is only storage.

## Current evidence

### Canonical facts

- Every failure and fault requires an explicit, statically inspectable route.
- Logging or observing a disposition does not consume it.
- One canonical disposition fact may project as a tagged value or graph wires.
- Failed Text construction has no normal `Text` result and transfers no partial
  owned bytes.

### Existing implementation

- `Outcome<Text,TextFailure>` is a bounded, backend-neutral tagged value.
- Its initial failure codes are `invalid_input`, `exhausted`, and
  `provider_unavailable`.
- LLVM uses a tagged `{code,value}` carrier and TinyVM uses a governed opaque
  outcome handle.
- The example branches on `.code` before consuming `.value`.
- The compiler validates the tagged representation but does not yet establish
  a general must-account lifetime law.

### Undefined

- whether storing an Outcome closes or transfers a failure route;
- what operation discharges the obligation;
- whether exhaustive branch analysis is required;
- whether copying duplicates, shares, or merely projects the obligation;
- whether an intentionally ignored failure requires an explicit policy sink;
- how an unaccounted Outcome is diagnosed at scope/function exit.

## Candidate A — tagged values transfer a must-account obligation

Producing `Outcome<T,E>` creates one disposition identity. Routing it into a
tagged value transfers the failure obligation to that value's semantic owner;
storage does not discharge it.

Before the owner leaves scope or crosses a boundary, every control path must do
one of the following:

```text
exhaustively inspect and handle success/failure
transform into another declared disposition
propagate through the enclosing declared result
route failure to an explicit policy sink
```

If not, compilation refuses with a dangling wire error at the ownership exit.
Copying the representation does not create independent semantic completions or
silently discharge the original obligation. Exact copy/alias mechanics remain
an ownership-stage question; canonical disposition identity must survive them.

**Pros**

- makes ignored tagged failures structurally impossible;
- preserves equivalence with explicit graph failure wires;
- catches the “works unless someone presses 1” maintenance class;
- permits Rust-like educational diagnostics at the exact scope exit;
- lets optimizers remove only outcomes proven fully accounted for.

**Cons**

- requires path-sensitive obligation analysis;
- generic containers and copying need later ownership rules;
- existing programs that construct and ignore Outcome values will be refused;
- concise propagation syntax remains unresolved.

**Consequences**

The canonical disposition fact records the obligation identity and its transfer
to the result symbol. A later consumption/propagation fact closes or transfers
that identity. The tagged carrier alone is not proof of completion.

**Migration impact**

The existing Text outcome example appears compatible because it branches on the
tag before using the value, but this must be proved against all paths rather
than assumed from source shape.

## Candidate B — tagged storage closes the route

Assigning `Outcome<T,E>` to any destination counts as a complete route. The
value may then be ignored like optional successful data.

**Pros**

- no new lifetime analysis;
- ordinary value semantics remain simple;
- easiest promotion of the existing implementation.

**Cons**

- an ignored failure becomes indistinguishable from unused data;
- recreates dangling failure paths inside values;
- weakens equivalence with graph wires;
- permits false operational confidence from merely retaining an outcome.

## Candidate C — only explicit destructuring receives failure authority

A tagged Outcome may be produced only directly into an exhaustive
destructuring/match construct. It cannot first be stored as an ordinary value.

**Pros**

- route completion is immediately visible;
- limits aliasing and lifetime complexity;
- simple dangling-wire validation.

**Cons**

- forbids useful storage, transport, return, and container use;
- requires new source syntax before reusing existing Outcome values;
- over-specializes representation around immediate handling.

## Candidate D — explicit unchecked/ignored outcome escape

Storage transfers an obligation as in Candidate A, but source may explicitly
discard it with a special unchecked or ignore operation.

**Pros**

- permits deliberate prototypes and compatibility code;
- makes the escape visible in review.

**Cons**

- an ordinary developer convenience becomes authority to lose failures;
- requires policy, profile, audit, and syntax decisions;
- risks normalizing the same maintenance hazard the dangling-wire law forbids.

An explicit top-level policy sink is already available conceptually and is
safer than a generic discard escape.

## Candidate E — tagged outcomes are not failure-flow projections

Keep `Outcome<T,E>` as an ordinary data convention. Canonical failure flow must
use separate graph/effect routes.

**Pros**

- avoids imposing obligation semantics on values.

**Cons**

- creates two failure systems;
- prevents tagged and graph projections from sharing one authority;
- undermines ADR 0058.

## Recommendation

Recommend **Candidate A — tagged values transfer a must-account obligation**.

It is the only candidate that preserves useful first-class Outcome values while
making ignored failure impossible by construction. It also gives Igor a precise
teaching diagnostic:

```text
error: tagged failure wire remains unaccounted at function exit
  produced here: Outcome<Text,TextFailure>
  stored in: outcome
  possible failure: TextFailure

  inspect both variants, propagate the outcome, or route the failure to an
  explicit policy sink
```

The diagnostic explains the legal repair classes without choosing program
intent.

## Decision

Candidate A was accepted as the current law on 2026-09-19. The first bounded
implementation is governed by
[Canonical Disposition Authority — Mission 02](../tasks/canonical-disposition-authority-mission-02.md).

Copy, alias, container, generic propagation, and future explicit handling
syntax remain revisitable only through later focused decisions.

## Historical choices presented

```text
A. Tagged Outcome storage transfers a must-account disposition obligation
B. Tagged Outcome storage itself closes the failure route
C. Outcome must be handled immediately by exhaustive destructuring
D. A generic explicit unchecked/ignore escape may discharge the obligation
E. Tagged Outcome is not a canonical failure-flow projection
F. Defer and define another prerequisite
```
