# ADR 0053: Named guards use lifecycle-statement syntax

## Status

Accepted on 2026-09-19.

## Decision

A named Lyraform live invariant is activated and selectively deactivated with
keyword-led lifecycle statements:

```lyraform
guard positive : x > 0
unguard positive
```

`positive` is the guard identity. `x > 0` is its predicate. The activation is
not a variable declaration, type construction, assertion, or conditional
branch. The guard remains active in its lexical scope until the matching named
`unguard` event or lexical scope exit.

The parser represents activation and deactivation with dedicated statement
forms. Semantic projection gives the guard a contract identity with an
explicit guard role. Later semantic stages resolve dependencies, classify
threatening transitions, and preserve guard evidence independently of backend
choice.

`guard` and `unguard` are contextual statement introducers. The established
typed-binding form remains valid, including a binding whose name is `guard`:

```lyraform
guard : int(1)
```

An expression statement beginning with the spelling `guard` or `unguard` is
reserved by this grammar and requires a different identifier.

## Rationale

The keyword-led form makes the lifecycle relationship visible and removes the
ambiguity present in name-first candidates such as:

```lyraform
positive : guard x > 0
positive : guard(x > 0)
```

Those forms resemble ordinary value/type declarations and obscure whether
`positive` names a variable or the invariant. The selected form states plainly
that the operation guards the named identity `positive`, and pairs directly
with `unguard positive`.

## Consequences

- Guard syntax maps directly to dedicated AST variants and provenance.
- Guard identity is distinct from guarded variables and predicate
  dependencies.
- Multiple named guards can compose and be selectively deactivated.
- `guard` is not introduced as a first-class runtime type.
- Runtime overwatch and guard-failure routing remain separate semantic
  decisions.

## Deferred

A lexical block form may be considered later:

```lyraform
guard positive : x > 0 {
    // guarded region
}
```

That form would make `}` an implicit lifetime end. Whether long blocks also
permit or require an explicit named terminator is not decided here. This ADR
does not define aggregate guards, aliases, cross-call preservation,
concurrency, runtime failure objects, or handler routing.
