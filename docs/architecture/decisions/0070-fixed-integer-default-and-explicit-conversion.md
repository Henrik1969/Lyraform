# 0070 — Fixed integer default and explicit conversion

**Status:** default alias superseded by [ADR 0071](0071-explicit-integer-width.md); conversion and overflow law retained  
**Date:** 2026-11-01  
**Mission:** `lyraform.contract-stabilization`, revision 1

## Decision and provenance

The owner's direct answers are recorded in the
[numeric decision record](../../tasks/contract-stabilization-numeric-decision.md).
The owner selected 64 bits for standard `int` and prefers explicit-width names
because target-dependent `int` meanings have caused porting problems.

`int` is the compatibility/prelude alias of signed `int64`, on every target.
Its range is −9223372036854775808 through 9223372036854775807. Prefer `int32`
and `int64` in explanations where width matters. Preserve written spelling in
the raw AST, as required by the binding type policy. This does not introduce
`short`, change ABI contract identities, or claim support for every primitive.

Conversion requires explicitly written intention or explicitly invoked policy.
Assignment, argument passage and return are not by themselves authorization to
change a value's type. An alias resolving to the same identity is not conversion.
A wider host carrier is not authority to widen a source-language value.

Integer overrun must take an authorized conversion route to a larger integer
type or the declared failure route. Silent wrap, truncation and implicit widening
are forbidden. A missing route refuses admission under ADR 0056; a backend trap
or private process exit cannot replace the route. This decision introduces no
numeric failure type, source syntax, handler or executable widening mechanism.

## Limits and implementation duties

This resolves the historical provisional `int` mapping. It does not resolve
floating-point rules or an implicit mapping from a language entry result to an
operating-system exit status. The latter needs an explicit host boundary; the
current compiler's `c_int` main carrier is implementation evidence, not authority.

Flowanalyst and shared scalar rules must resolve identities and validate intent.
Flowcontracts must reject inconsistent imported artifacts independently of
optional facts. LLVM and TinyVM must preserve the approved range and meaning.
G2 must test boundaries, exact full results and hostile artifacts before claiming
conformance. Current G1 binaries have not yet acquired this new alignment.
