# Decision brief — canonical disposition fact and carrier projections

Date: 2026-09-19.

Status: **DECIDED — Candidate A accepted on 2026-09-19.**

Durable authority:
[ADR 0058](../architecture/decisions/0058-canonical-disposition-fact-projections.md).

## Problem

ADR 0055 defines one exclusive completion:

```text
Success<T> | Failure<E> | Fault<F>
```

ADR 0056 requires every failure and fault to have an explicit, inspectable
route. Lyraform must now decide whether tagged values, function signatures,
graph ports, provider responses, and execution records are separate failure
mechanisms or projections of one canonical semantic fact.

## Why the decision is required

Without one authority, a sequential backend could represent a guard violation
as a tagged value while a graph backend treats it as an output port and a
provider reports only a process diagnostic. Each could then disagree about:

- which dispositions are possible;
- whether one was consumed or left dangling;
- commit atomicity;
- provenance and route identity;
- whether a backend may terminate instead of producing the declared result.

Runtime guard lowering cannot proceed safely until those representations are
known to carry the same meaning.

## Current evidence

### Canonical facts

- One semantic authority may have multiple independently inspectable
  projections.
- One admitted attempt has exactly one success, failure, or fault completion.
- Every failure and fault route must be statically inspectable.
- Backends execute established semantics and do not invent routing.

### Implementation behavior

- `Outcome<Text,TextFailure>` proves a bounded tagged-value representation in
  both LLVM and TinyVM.
- Source graphs already preserve typed ports, wires, signal identity, fan-out,
  and failure provenance.
- Compiler stages and providers expose structured boundary diagnostics and
  `no_artifact` dispositions.
- These mechanisms are useful evidence but none is yet the universal semantic
  carrier.
- ADR 0057 requires human diagnostics and debugger/machine views to project the
  same canonical evidence without becoming disposition destinations.

### Undefined

- whether an outcome value and success/failure/fault ports are equivalent
  projections;
- where the canonical disposition identity and route live;
- whether functions declare closed failure/fault sets or another bounded
  summary;
- how provider-selected implementations prove conformance to the declaration;
- concrete source syntax and serialized contract versions.

## Candidate A — one canonical disposition fact, contextual projections

Semantic analysis creates one versioned operation-disposition fact containing
the operation identity, mutually exclusive possible dispositions, payload type
identities, commit law, origin, and route identities.

Different contexts project that same fact appropriately:

```text
sequential/value context    tagged outcome projection
function boundary           declared result/disposition signature
graph context               typed success/failure/fault ports and wires
provider boundary           versioned response contract
execution/audit boundary    disposition record and observations
```

Every projection must preserve the discriminant, type identities, route
completeness, and provenance. A projection that cannot represent the fact is
refused rather than weakened.

**Pros**

- directly realizes one semantic authority with many projections;
- prevents backend-specific failure meaning;
- permits natural syntax in both value and graph contexts;
- gives dangling-wire validation one canonical source;
- supports independently testable stage artifacts.

**Cons**

- requires versioned projection-equivalence validators;
- needs a stable minimal semantic fact before surface syntax;
- adapters must prove they preserve exclusivity and route identity;
- richer than reusing one concrete carrier everywhere.

**Consequences**

Runtime guard violation is one canonical
`Failure<GuardViolation>` disposition. A tagged result and a graph failure port
are not different semantics; they are validated projections of that fact.

**Migration impact**

The existing bounded `TextOutcome`, graph failure evidence, and provider
results become evidence toward projection adapters. They are not silently
promoted to complete authority.

## Candidate B — universal tagged outcome value

Every fallible operation produces a concrete tagged value such as
`Disposition<T,E,F>`. Graph ports and provider responses transport that value
without separate success/failure/fault lanes.

**Pros**

- one concrete representation;
- familiar value typing and generic composition;
- bounded `TextOutcome` provides implementation evidence.

**Cons**

- graph routing must inspect values to discover control-relevant disposition;
- dangling-wire analysis becomes less structural;
- fault containment can look like ordinary value handling;
- forces one carrier shape onto providers and execution records.

**Consequences**

All graph nodes receive and switch on the universal tagged value unless
compiler transformations split it later.

**Migration impact**

Requires generalizing `TextOutcome` far beyond its current bounded authority.

## Candidate C — universal disposition ports

Every operation exposes distinct success, failure, and fault output ports.
Sequential expressions and functions are syntax over those graph ports.

**Pros**

- route completeness and dangling wires are visually and structurally clear;
- graph execution is the direct canonical form;
- containment boundaries can be explicit edges.

**Cons**

- makes ordinary sequential composition graph-heavy;
- value-oriented APIs and providers need artificial port adapters;
- source ergonomics and generic algorithms become harder;
- prematurely makes one graph projection the language definition.

**Consequences**

The graph model, rather than the semantic operation fact, becomes primary.

**Migration impact**

Requires broad function, outcome, and provider redesign before runtime guards.

## Candidate D — declared failure effect as the universal carrier

Functions and operations declare success results plus typed failure/fault
effects. Tagged values and graph ports are lowering choices for those effects.

**Pros**

- concise sequential source;
- signatures expose unsuccessful possibilities;
- can support generic propagation.

**Cons**

- effect handling and propagation semantics are not canonical;
- risks recreating hidden exceptional control flow;
- graph route identity is indirect;
- fault containment does not fit ordinary effects without additional law.

**Consequences**

The effect system becomes the semantic authority before Lyraform has selected
its handler and propagation model.

**Migration impact**

Requires a general effect-language stage first.

## Candidate E — independent mechanisms

Tagged outcomes, graph lanes, provider responses, and execution faults remain
separately specified mechanisms connected by ad hoc adapters.

**Pros**

- smallest immediate change to each subsystem.

**Cons**

- permits semantic disagreement between projections;
- duplicates validation and category systems;
- weakens backend equivalence;
- conflicts with compiler convergence toward one semantic authority.

## Recommendation

Recommend **Candidate A — one canonical disposition fact with contextual,
validated projections**.

It chooses semantic unity without forcing one surface representation onto every
context. It also permits the next implementation stage to be deliberately
small: define and validate the minimal fact first, then project one bounded
guard failure through existing artifacts without yet inventing recovery syntax.

This decision would not itself choose concrete source spelling, serialized
field names, failure-type hierarchy, or top-level policy syntax.

## Decision

Candidate A was accepted on 2026-09-19. The bounded implementation mission is
[Canonical Disposition Authority — Mission 01](../tasks/canonical-disposition-authority-mission-01.md).

## Historical choices presented

```text
A. One canonical disposition fact with validated contextual projections
B. One universal tagged outcome value
C. One universal success/failure/fault port model
D. One universal declared failure-effect model
E. Keep independent mechanisms
F. Defer and define another prerequisite
```
