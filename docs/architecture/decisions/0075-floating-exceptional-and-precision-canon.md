# 0075 — Floating exceptional values and precision are a bounded binary64 contract

**Status:** accepted bounded floating arithmetic canon
**Date:** 2026-11-04
**Mission:** Lyraform numeric-semantics foundation

## Decision

The admitted `float64` carrier means IEEE-754 binary64. Floating arithmetic
uses round-to-nearest, ties-to-even, as its precision rule. Signed zero is
observable and preserved by the admitted lowering/runtime path. NaN and
positive/negative infinity are floating values that arithmetic may produce or
propagate; they are not silently reinterpreted as Lyraform failure or fault
dispositions.

The current bounded contract is:

```text
float64 arithmetic carrier: IEEE-754 binary64
rounding:                  nearest, ties-to-even
source float literals:     finite binary64 values only
1.0 / 0.0:                 +infinity
-1.0 / 0.0:                -infinity
0.0 / 0.0:                 NaN
```

The sign of a zero operand participates in the ordinary IEEE result sign. A
NaN payload is not part of the language-level contract; only the exceptional
classification and comparison behavior are semantically relevant.

## Admission and execution boundary

The frontend accepts exponent notation as part of a numeric literal. Semantic
admission rejects a source literal that cannot be represented as a finite
binary64 value, including overflow to infinity. This keeps source spelling
deterministic across providers while allowing arithmetic to produce exceptional
values after admission.

True division (`/`) is already a floating operation after numeric promotion,
so floating zero division follows this contract and does not require a dynamic
failure route. Integer `div` and `%` remain separate strict operations: their
zero-divisor and signed-minimum overflow rules remain refusal/failure-routing
boundaries under ADR 0074.

Ordered comparisons retain the backend-aligned IEEE behavior: equality with
NaN is false, inequality is true, and ordered less/greater comparisons are
false. A future language-level total-order operation may define a different
contract explicitly; it must not be inferred from the carrier.

LLVM and TinyVM are required to preserve the same admitted result class and
observable value for this slice. TinyVM has an explicit `FNEG` operation so
floating unary negation preserves negative zero instead of relying on a
zero-minus lowering that may erase its sign.

## Deliberate boundary

This decision does not define user-selected rounding modes, decimal exactness,
rational values, wider floating families, NaN payload identity, or provider-
specific math-library behavior beyond the admitted primitive arithmetic. It
also does not turn exceptional floating values into source-derived failure
routes. Those are separate future decisions and must be introduced as explicit
contracts.

The focused evidence is recorded in the
[floating exceptional/precision vertical slice checkpoint](../../checkpoints/2026-11-04-floating-exceptional-precision-slice.md).
