# ADR 0052: Ordinary aggregates are independent values

## Status

Accepted on 2026-09-19.

## Decision

An ordinary Lyraform aggregate is a compound value made from independently
typed subvalues. It does not acquire shared mutable object identity merely by
being bound, passed or placed.

Binding or placing an aggregate value into another ordinary destination
preserves value independence. A later update of one destination is not visible
through another binding that received the earlier value.

Member placement denotes recursive value reconstruction followed by rebinding
of the root destination under separately proven write authority. For example,
given a value `b` with members `x` and `a`:

```lyraform
7 -> b.x
```

produces a new value for `b` whose `x` is `7`; `b.a` and every other untouched
subvalue retain their prior values. Any other binding holding the earlier
aggregate value remains unchanged.

This decision defines source meaning, not physical implementation. A compiler
may use in-place storage update, copy elision, structural sharing or persistent
data structures only when the optimization is observationally equivalent to
reconstruction and rebinding.

Identity-bearing mutable state is not implicit in ordinary aggregates. A
future reference, cell, state or capability model must make identity and write
authority explicit.

## Consequences

Positive:

- aggregate fan-out does not introduce hidden mutable aliases;
- value reasoning, replay, provenance, serialization and distribution remain
  deterministic;
- parallel readers cannot observe an implicit member mutation;
- physical update optimizations remain possible after uniqueness proof;
- external resources and shared state can use explicit identity-bearing types.

Trade-offs:

- member placement requires a reconstruction/rebinding semantic fact;
- root binding write authority must be defined and proved separately;
- efficient large-value implementation needs copy elision, persistence or
  uniqueness analysis;
- aggregates containing future linear resources require additional admission
  rules.

## Invariants

```text
ordinary aggregate assignment does not create observable shared mutation
member update preserves all untouched subvalues
member update does not mutate previously produced aggregate values
physical in-place update is an optimization, never source authority
```

## Deferred

This ADR does not define declaration syntax, `const`, references, ownership,
borrowing, backend layout or executable member stores.
