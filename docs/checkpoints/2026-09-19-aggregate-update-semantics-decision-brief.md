# Decision brief — canonical aggregate update semantics

Date: 2026-09-19.

Status: decided — Candidate A accepted on 2026-09-19. See ADR 0052.

## Problem

When Lyraform source places a value into a resolved member target such as:

```lyraform
7 -> p.a.b
```

what happens to the aggregate represented by `p`?

The compiler can already prove which declarations `p.a.b` denotes and that its
final type is, for example, `int`. It cannot yet prove assignment legality
because the language has not established whether aggregates are immutable
values, uniquely owned mutable values, explicit references, or shared mutable
objects.

## Why the decision is required

Gate 9 cannot safely derive member assignability from member existence alone.
This decision determines:

- whether member placement mutates an object or produces a new value;
- whether the root binding must be rebound or merely grants access to storage;
- whether two bindings can observe the same update;
- what parameters receive and may update;
- whether copy, move, borrow, reference or alias facts are required;
- what provenance represents the state transition;
- which transformations preserve meaning;
- whether independent graph branches can process aggregates without races;
- what evidence Flowanalyst and Flowcontracts must carry;
- what LLVM, TinyVM and future distributed backends must eventually execute.

Implementing member admission before this decision would make runtime storage
behavior the accidental language authority.

## Current evidence

### Canonical facts

- Parser authority preserves identifier, field-path and indexed target
  structure.
- Gate 8 resolves the root, every member declaration/type and the final target
  type.
- `lyraform.target_fact` v2 explicitly records
  `assignability: unresolved` and `execution: unsupported`.
- Local bindings, function parameters and structural fields have identity,
  type and origin facts, but no mutable, immutable, readonly or writable fact.
- No `const` declaration syntax is admitted by the active structural model.
- The bounded source-graph aggregate contract uses immutable, provider-verified,
  by-value payloads. That is a canonical rule for that graph boundary only, not
  yet a general local-language aggregate law.
- Published semantic and stage artifacts are immutable snapshots.

### Canonical implementation behavior

- The canonical analyzer can prove `p.x : int` and `p.a.b : int` but leaves
  member assignability unresolved.
- A Bool source placed toward an `int` member currently remains unresolved in
  canonical analysis; the future admission authority must reject it before a
  ready executable plan.
- LLVM and TinyVM validate the resolved target and refuse member execution.

### Legacy behavior

- The compatibility parser checks target type and lowers member placement to
  `record.field.set` using a root payload path and textual member path.
- The compatibility runtime mutates nested `RecordPayload` storage in place.
- Ordinary legacy aggregate placement uses `record.copy`; an existing probe
  observed destination independence after later source mutation.
- Legacy records are value-owned C++ variants in that implementation.

These observations are useful oracles. They are not canonical Lyraform
semantics.

### Undefined areas

- general local aggregate value versus object identity;
- root binding write/rebind authority;
- parameter update authority;
- field-level writability;
- initialization versus subsequent update;
- aliasing and shared observation;
- copy/move/borrow/reference rules;
- aggregate update failure and allocation behavior;
- mutation/revision provenance for language values;
- semantics of resources or handles stored inside future aggregates.

## Candidate A — immutable aggregate values with reconstruction

**Meaning**

Ordinary aggregates are values without observable mutable identity. Placement
into `p.a.b` computes a new `a`, then a new `p`, and rebinds `p` under explicit
root write authority:

```text
new_a = reconstruct(p.a, b = 7)
new_p = reconstruct(p, a = new_a)
rebind p = new_p
```

Existing values and aliases remain unchanged. An implementation may optimize
reconstruction through uniqueness, persistence or copy elision only when the
result is observationally identical.

**Pros**

- deterministic, local reasoning with no hidden alias mutation;
- natural fit for graph/dataflow evaluation and immutable stage facts;
- safe fan-out and parallel reads without races;
- straightforward serialization, caching, replay and distributed execution;
- optimizations can use persistent structures, structural sharing or elision
  without changing semantics;
- legacy copy-independence evidence points in this direction;
- resource mutation can remain a separate explicit capability rather than
  contaminating ordinary values.

**Cons**

- requires a canonical aggregate reconstruction operation and root rebinding
  law;
- naïve implementations may copy large nested aggregates;
- allocation/resource-exhaustion behavior must eventually be explicit;
- aggregates containing linear resources or opaque handles need separate
  admissibility rules;
- current legacy in-place field update cannot be treated as normative, even if
  it may later implement an optimized unique case.

**Consequences**

- Language: member placement is a value update plus root rebinding, not object
  mutation.
- Compiler contracts: Flowanalyst must prove root rebind authority, final type
  compatibility and reconstructibility for every segment.
- Optimization: in-place update is legal only as an unobservable optimization.
- Parallelism: independent old values remain race-free; competing rebindings of
  one root still require graph/state ordering.
- Backends: eventually lower explicit reconstruction/update facts, never infer
  offsets from target spelling.
- Future features: explicit references/cells can be added separately for
  identity-bearing state.

**Migration impact**

Existing field-placement syntax can remain, but its canonical meaning changes
from undefined to reconstruction. Legacy tests that rely only on final values
may remain valid; tests relying on shared in-place observation would be refused
or moved to an explicit reference facility. Frontend declaration facts and
target artifacts need additive write-authority/update evidence before backend
work.

## Candidate B — uniquely owned mutable aggregates

**Meaning**

An aggregate has one owner. `p.a.b` may update storage in place only while the
compiler proves exclusive ownership of the entire path. Transfer consumes or
moves the source; aliases and borrows restrict mutation.

**Pros**

- permits efficient in-place updates without shared mutable aliasing;
- can provide strong race and lifetime safety;
- resembles the legacy runtime's physical update when uniqueness is proven;
- supports deterministic cleanup for owned resources.

**Cons**

- requires ownership, move, borrow and lifetime semantics before this small
  member feature can be admitted;
- parameters and returns need transfer/borrow modes;
- greatly expands semantic analysis and diagnostics;
- graph fan-out requires copies, borrows or explicit sharing;
- serialization and suspension need ownership-state contracts.

**Consequences**

- Language: assignment and calls acquire move/borrow meaning.
- Compiler contracts: plans must carry ownership state and exclusive update
  proof, not merely target identity.
- Optimization: strong uniqueness enables in-place update, but proof must be
  preserved across transformations.
- Parallelism: safe when exclusivity is statically maintained; scheduling must
  respect borrows and transfers.
- Backends: require owned storage/lifetime lowering and deterministic cleanup.
- Future features: a full resource-safe ownership system becomes foundational.

**Migration impact**

Current placements and parameter passing may need new diagnostics or syntax.
Legacy copy behavior would need classification as explicit copy versus move.
This is a substantially larger language commitment than Gate 9 alone.

## Candidate C — immutable ordinary values plus explicit reference mutation

**Meaning**

Ordinary aggregates remain immutable values. Mutable identity exists only
through an explicit reference, cell, state or capability type. Member mutation
is legal only when the root is such an identity-bearing value and carries write
authority. Ordinary `p.a.b` either means Candidate A reconstruction or is
refused unless `p` is explicitly a mutable reference.

**Pros**

- makes mutation and aliasing visible in types and effects;
- preserves simple value semantics for ordinary aggregates;
- supports controlled shared state, resources and foreign objects;
- capability/write authority can be explicit and auditable;
- parallel conflict analysis has concrete identities to reason about.

**Cons**

- needs new reference/cell/capability types and likely syntax;
- requires lifetime, alias and synchronization policy for references;
- does not by itself define whether plain member placement reconstructs or is
  illegal;
- raises the immediate language-design scope substantially.

**Consequences**

- Language: value updates and identity mutation become distinct operations.
- Compiler contracts: reference identity, access mode and effects must be
  explicit.
- Optimization: value paths remain freely optimizable; reference operations
  become effect barriers according to their contract.
- Parallelism: shared writes can be rejected or governed by explicit policy.
- Backends: need separate value reconstruction and reference-store lowering.
- Future features: maps well to provider state, devices and external resources.

**Migration impact**

Existing unqualified member placement cannot silently become reference
mutation. New forms or type qualifiers would be required. Legacy in-place
behavior would migrate behind explicit mutable-reference authority.

## Candidate D — shared mutable object semantics by default

**Meaning**

Aggregate bindings refer to objects. Copying a binding aliases the same object,
and `p.a.b` mutates that shared object for every observer.

**Pros**

- direct implementation with conventional object layouts and stores;
- efficient mutation without reconstruction;
- familiar to users of mainstream object-oriented runtimes;
- superficially resembles the compatibility runtime's field-set operation.

**Cons**

- hidden aliasing and action-at-a-distance;
- race and scheduling hazards conflict with deterministic graph execution;
- requires identity, lifetime, synchronization and memory-model rules;
- complicates optimization, caching, replay, serialization and distribution;
- legacy aggregate copy independence contradicts default shared observation;
- weakest fit with Lyraform's safety and explicit-authority goals.

**Consequences**

- Language: bindings carry object identity and assignment aliases by default.
- Compiler contracts: alias sets, effects and synchronization become central.
- Optimization: transformations must preserve observable alias/mutation order.
- Parallelism: mutable-sharing conflicts require conservative refusal or
  synchronization.
- Backends: need stable object layout, allocation, identity and memory model.
- Future features: distributed and provenance-aware execution become harder.

**Migration impact**

Would likely change observed legacy copy-independence and require broad new
runtime/backend contracts. It should not be inferred from one in-place runtime
atom.

## Candidate E — defer and define prerequisites first

**Meaning**

Keep member assignability unresolved while separately defining binding modes,
aggregate construction, resource containment and mutation provenance before
choosing an update model.

**Pros**

- avoids premature commitment;
- permits targeted research into resource values and declaration authority;
- preserves all current compiler truth.

**Cons**

- Gate 9 and member execution remain blocked;
- source/destination compatibility cannot become complete admission evidence;
- prolonged ambiguity may allow legacy behavior to keep influencing design by
  accident.

**Consequences**

No compiler or backend contract changes. The next work is another decision or
reconnaissance stage rather than implementation.

**Migration impact**

None immediately. Future impact depends on the later choice.

## Recommendation

Recommend **Candidate A: immutable aggregate values with reconstruction** for
ordinary Lyraform aggregates, while preserving Candidate C as the future model
for explicitly identity-bearing mutable state and external resources.

Candidate A best matches the existing evidence and project constraints:

- bounded graph aggregates are already immutable by-value payloads;
- legacy aggregate copy probes show destination independence;
- explicit value semantics improve determinism, provenance, replay,
  serialization and backend independence;
- graph fan-out and effectful parallelism do not acquire hidden alias races;
- physical in-place update remains available later as a proven-unique
  optimization rather than source meaning;
- reference mutation can remain explicit instead of becoming an invisible
  default.

The accepted bounded follow-up uses the already established rebinding authority
for explicitly initialized local `Variable` declarations. It does not infer
authority for parameters, uninitialized declarations, fields as independent
storage, or the currently unrecognized `const` spelling.

## Decision

Henrik selected:

**Candidate A — immutable aggregate values with reconstruction.** A member
placement reconstructs the containing aggregate path and rebinds the root.
Untouched subvalues retain their values, and bindings that hold the earlier
aggregate remain unchanged. This decision is recorded normatively in
[ADR 0052](../architecture/decisions/0052-independent-aggregate-values.md).

```text
A. Immutable aggregate value / reconstruction and root rebinding
```

`b.x` receives the new subvalue while every untouched subvalue in `b` retains
its prior value. Other bindings that hold the earlier aggregate value remain
unchanged. The durable language decision is recorded in ADR 0052.
