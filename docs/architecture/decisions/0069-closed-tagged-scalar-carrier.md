# 0069 — Closed tagged scalar carriers

**Status:** accepted boundary; implementation in progress  
**Date:** 2026-10-17  
**Mission:** `lyraform.configresolve-tagged-value-boundary`

## Decision

Lyraform represents an exclusive family of named alternatives with a
`variant` declaration. The declaration is closed: every value has exactly one
declared alternative and no unnamed or extension alternative is admitted.

The source shape for the ConfigResolve carrier is:

```text
type Value variant {
    case Null
    case Bool : Bool
    case Int : int
    case Text : Text
    case Double : float64
}
```

`case Name` is a nullary alternative. `case Name : Type` is a unary
alternative whose payload has the declared type. This is a language carrier,
not an ordinary record convention and not a backend-specific ABI struct.

## Laws

- Construction selects one declared case exactly once.
- A case payload is copied as an independent language value; host pointer
  identity never becomes the value's identity.
- Layout is private to the language implementation and may differ between
  native LLVM and TinyVM.
- Reading a payload requires an explicit case test or a checked projection.
  A projection for the wrong case is an explicit failure; it never falls back
  to a default value or performs an implicit conversion.
- The declaration is closed. Adding a case is a source and compatibility
  change, not an incidental runtime extension.
- Member writes, resource lifetimes, collections, and implicit numeric
  conversions are outside this boundary.

## Admission slice

The first executable slice admits `Null`, `Bool`, `Int`, and `Text` only.
`Double` is retained in the reference shape but is refused until the language
specifies finite values, signed zero, NaN, infinity, and equality/ordering.
The refusal must identify the source case and preserve the mission evidence.

Native LLVM and TinyVM may use different physical representations, but an
admitted program must preserve the same active case, payload value, copy
behavior, and checked-projection result on both backends.

## Consequences

ConfigResolve's `Value` no longer needs to be approximated by a record with a
tag and optional payload fields. Ordinary records remain useful for `Fact`,
`Policy`, and `Resolution`, but they cannot claim the exclusivity invariant of
this carrier. The existing `Outcome<Text,TextFailure>` remains a separate
must-account carrier and is not silently generalized by this decision.
