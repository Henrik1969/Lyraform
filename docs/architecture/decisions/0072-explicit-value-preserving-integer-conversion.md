# 0072 — Explicit, value-preserving integer conversion

**Status:** accepted owner decision; bounded implementation under G2  
**Date:** 2026-11-01  
**Mission:** `lyraform.contract-stabilization`, revision 1

## Owner decision

The owner explicitly required visible conversion semantics and no widening or
narrowing unless explicitly stated, then instructed implementation in the language.
This clarifies ADR 0070's conversion rule; ADR 0071's explicit widths remain binding.

A conversion needs both authorization and exact representability. Authorization
comes from explicit source intention or an explicitly invoked, admitted policy.
Equal carrier widths, lossless widening, assignment, argument passage, return,
arithmetic and the host boundary do not grant conversion authority themselves.

A conversion preserves the exact original value. Narrowing may succeed when the
value fits the destination range. Otherwise it takes its declared failure route.
If safety cannot be proved and no admitted runtime check/failure route exists,
compilation refuses. Silent truncation, wrapping and backend-selected fallback
are forbidden. Width alone cannot justify signedness changes.

## Bounded implementation

Existing typed initialization expresses conversion intention, for example:

```text
x : int32(42)
y : int64(x)
```

The implementation preserves distinct identities even when their storage width
matches. Flowanalyst projects the existing explicit-initializer marker only for
an actual typed initializer, including an ordinary call or expression result.
Assignments and returns cannot acquire that marker merely from a destination type.

Shared Flowcontracts checks the marker's operation context, expression identity,
source type, known declaration identities, function signatures, argument/result
boundaries and destination range. Constant-safe signed narrowing and signed
widening are admitted. Known nonnegative values and unchanged signed values in a directly proven
nonnegative branch can convert exactly to the selected 64-bit size/unsigned ABI
carrier. Unknown narrowing and unproved signedness conversions refuse; no numeric handler, failure type or policy vocabulary is invented here.
Compatibility artifacts cannot infer a wider return-to-host conversion from an
absent function-result contract. Native and TinyVM preserve int32/int64 carriers.

This is not a claim that all numeric families, predicates, record conversions,
dynamic narrowing routes or legacy oracle execution are complete. G2 tracks
those limits. The source main/host-status model remains a separate decision;
whatever its selected contract, conversion obeys this rule.
