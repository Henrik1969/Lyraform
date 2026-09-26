# ADR 0059: Tagged outcomes transfer a must-account obligation

## Status

Accepted as the current law on 2026-09-19.

## Decision

Routing `Failure<E>` into an `Outcome<T,E>` value transfers the canonical
disposition obligation to that value's semantic owner. Storage does not
discharge the obligation.

Before the owner leaves its governed lifetime or crosses a semantic boundary,
every reachable path must account for the disposition by:

```text
exhaustively handling success and failure
transforming to another declared disposition
propagating through the enclosing declared result
routing failure to an explicit policy sink
```

Otherwise the compiler refuses the program with a dangling wire error at the
ownership exit.

## Identity and representation

Copying or lowering a tagged representation must not create independent
semantic completions or erase the original obligation. The disposition
identity survives every validated projection. Exact copy, move, borrow, alias,
container, and generic rules remain separate ownership decisions.

## Diagnostics

The diagnostic identifies the producer, tagged owner, possible failure type,
unaccounted exit, and legal repair classes. It follows ADR 0057 and does not
guess which repair the developer intended.

## Bounded first implementation

Mission 02 may recognize the existing `Outcome<Text,TextFailure>` recovery
shape only when it can prove:

- `.code` is projected from the exact outcome;
- complementary success and failure branches cover the tag;
- `.value` is consumed and disposed only on the success branch;
- the failure branch performs explicit recovery behavior;
- the result is not copied, returned, aliased, or stored in a container.

Unknown or incomplete shapes fail closed. This does not freeze future match or
propagation syntax.

## Consequences

- Ignored tagged failures are inadmissible.
- Tagged and graph projections obey the same no-dangling-wire law.
- Dead-value elimination requires proof that the disposition obligation was
  accounted for.
- Runtime backends receive an already-validated handling fact and cannot infer
  handling from carrier layout.

## Deferred

- general path-sensitive outcome analysis;
- copy/move/borrow/alias behavior;
- exhaustive match syntax;
- propagation operators;
- generic and container-held outcomes;
- explicit policy-sink syntax;
- concurrent outcome composition.
