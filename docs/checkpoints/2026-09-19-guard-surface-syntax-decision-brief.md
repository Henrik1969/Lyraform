# Decision brief — canonical `guard` / `unguard` surface syntax

Date: 2026-09-19.

Status: decided — Candidate B accepted on 2026-09-19. See ADR 0053.

## Problem

What source spelling creates a named scoped guard without representing that
guard as an ordinary data value or type declaration?

The semantic meaning is already established: a guard is a named live invariant
with selective deactivation. The current grammar recognizes none of the
candidate spellings correctly.

## Why the decision is required

The choice fixes public language syntax and influences:

- whether activation is structurally a declaration or a lifecycle statement;
- whether `guard` occupies the type grammar;
- where the predicate begins and ends;
- how guard identities enter lexical scope;
- how activation and `unguard` are paired in the AST;
- diagnostics and source completeness;
- whether guards might later be mistaken for first-class runtime values;
- compatibility with a future general contract/refinement surface.

Implementation convenience cannot decide these language consequences.

## Current evidence

### Canonical facts

- Guard identity is named and lexically scoped.
- Activation and deactivation are semantic events.
- A guard is distinct from `if`, `assert`, `requires`, and a refined type.
- `unguard` removes exactly one resolved guard identity.

### Current grammar

- `name : Type(initializer)` is the established initialized-binding form.
- Keyword-led statements represent lifecycle/control events such as `if`,
  `while`, and `return`.
- `guard` and `unguard` are currently identifiers, not keywords.
- The AST has no guard statement variants.

### Probe behavior

- `positive : guard x > 0` becomes an uninitialized variable of unknown type
  `guard`; its predicate is lost.
- `positive : guard(x > 0)` becomes a variable of unknown type `guard` whose
  initializer happens to be a Bool expression.
- `guard positive : x > 0` is skipped and partially misprojected.
- `unguard positive` is omitted entirely.

These are parser accidents, not compatibility commitments.

## Candidate A — name-first contextual declaration

```lyraform
positive : guard x > 0
unguard positive
```

**Meaning**

After a name and colon, the contextual word `guard` switches from type parsing
to a dedicated guard-activation statement whose predicate consumes the rest of
the statement.

**Pros**

- matches the preferred direction in the semantic brief;
- foregrounds the guard identity;
- visually resembles other named declarations;
- concise for multiple composed guards.

**Cons**

- colon currently means that a type reference follows;
- needs a contextual grammar exception and reserves `guard` in that position;
- creates ambiguity with a legitimate user type named `guard`;
- the boundary between the marker and predicate is whitespace/context based;
- visually suggests `positive` is an ordinary bound value unless tooling makes
  the distinction clear.

**Consequences**

The AST must still use a dedicated guard node, not `LetStatement`. Symbol
projection must distinguish guard identity from variable identity. A future
first-class `guard` type would require different syntax or disambiguation.

**Migration impact**

Potential source incompatibility for a user-defined type named `guard` in
initialized or uninitialized bindings. No current canonical guard source needs
migration.

## Candidate B — keyword-led lifecycle statement

```lyraform
guard positive : x > 0
unguard positive
```

**Meaning**

`guard` explicitly activates a named invariant; `unguard` explicitly
deactivates that same kind of lexical identity.

**Pros**

- activation and deactivation are visibly symmetric lifecycle statements;
- maps directly to dedicated AST variants;
- does not reuse or distort type/declaration grammar;
- gives an unambiguous predicate boundary after the colon;
- avoids implying that guards are ordinary values;
- leaves a user-defined type named `guard` conceptually separate if contextual
  keyword handling is retained.

**Cons**

- differs from the semantic brief's preferred name-first direction;
- foregrounds the construct rather than its identity;
- reserves or contextually recognizes `guard` and `unguard` at statement start;
- is slightly more verbose when scanning a list of named declarations.

**Consequences**

The frontend gains `GuardActivateStatement` and `GuardDeactivateStatement` (or
equivalent) with explicit name, predicate, and provenance. Semantic identity
can use the existing contract concept with a guard role or a dedicated internal
kind. Runtime first-class guard values are not implied.

**Migration impact**

Statement-start uses of identifiers named `guard` or `unguard` may need
contextual disambiguation. No admitted canonical guard syntax changes.

## Candidate C — parenthesized name-first form

```lyraform
positive : guard(x > 0)
unguard positive
```

**Meaning**

The name-first form uses parentheses to delimit the predicate, but the parser
special-cases `guard(...)` into a guard activation rather than an ordinary
typed binding.

**Pros**

- has an explicit predicate boundary;
- remains close to the preferred name-first form;
- existing expression parsing already preserves the comparison inside the
  parentheses;
- allows multiline predicates later if expression grammar permits them.

**Cons**

- is currently exactly the visual and structural shape of a typed initialized
  binding;
- strongly suggests `guard` is a type and the predicate is a stored value;
- requires a special exception in the declaration parser;
- conflicts directly with a user-defined `guard` type and constructor;
- encourages accidental treatment of guard identity as first-class data.

**Consequences**

Despite surface similarity, the AST would have to reject the existing
`LetStatement` interpretation. Tooling and users would need to remember that
one apparent type constructor is actually a lifecycle construct.

**Migration impact**

Largest collision risk with future or existing ordinary type syntax. No
canonical guard source currently requires migration.

## Candidate D — lexical guard block

```lyraform
guard positive : x > 0 {
    4 -> x
}
```

**Meaning**

Guard lifetime is the body block; explicit `unguard` is unnecessary inside the
basic form.

**Pros**

- lifetime is structurally obvious;
- scope exit is automatic;
- control-flow analysis begins from an explicit region.

**Cons**

- cannot naturally express the brief's selective deactivation among composed
  guards without adding another construct;
- introduces nesting solely to control lifetime;
- materially changes the requested model;
- complicates guards activated conditionally or deactivated before block exit.

**Consequences**

Guard scope becomes a region construct rather than an active-set fact over the
enclosing lexical block. This is a different language model, not mere syntax.

**Migration impact**

The examples in the semantic brief would require restructuring.

## Recommendation

Recommend **Candidate B — keyword-led lifecycle statement**.

It best reflects the already chosen meaning: activation and deactivation are
events governing a semantic lifetime, while the guard identity is a scoped
contract name rather than a stored value. It fits the existing distinction
between typed bindings and control/lifecycle statements, avoids claiming that
`guard` is a type, gives the parser a complete predicate boundary, and pairs
cleanly with `unguard`.

Candidate A remains viable if identity-first visual consistency is more
important than preserving the colon/type grammar. If selected, it must still
produce a dedicated guard AST node and must not be implemented as a special
variable of type `guard`.

## Deferred runtime decision

This syntax decision does not choose the runtime guard-failure object or
routing mechanism. After syntax and a static-only slice are complete, a later
decision must select how not-provable transitions lower to pre-commit overwatch
and how failure carries guard identity, predicate, candidate state,
dependencies, operation, and provenance.

Until then, not-provable transitions must fail closed at compilation.

## Decision

Candidate B was accepted:

```lyraform
guard positive : x > 0
unguard positive
```

The identity follows `guard`, so `positive` is unambiguously the thing being
guarded. Candidate A and Candidate C leave the reader asking whether
`positive` names a variable, a type-like value, or the guard itself.

A future lexical form such as `guard name : predicate { ... }` remains
possible, but is not part of this decision. A closing brace could provide
automatic lifetime termination; an explicit named closing form may still be
valuable for readability in long regions. That question requires its own
decision and must not alter the statement form selected here implicitly.

The durable decision is recorded in
[ADR 0053](../architecture/decisions/0053-named-guard-lifecycle-syntax.md).

## Historical choice presented

```text
A. positive : guard x > 0
B. guard positive : x > 0       (recommended)
C. positive : guard(x > 0)
D. lexical guard block
E. defer syntax and define another prerequisite first
```
