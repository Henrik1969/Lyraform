# 0074 — True division and integer quotient are distinct operations

**Status:** accepted initial arithmetic canon; implementation and backend parity pending
**Date:** 2026-11-02
**Mission:** Lyraform numeric-semantics foundation

## Owner decision

The `/` spelling denotes true division. The `div` spelling denotes an integer
quotient. They are different operations with different value and result-type
contracts; `/` must not silently change an expression's type because a
particular quotient happens to have no fractional part.

The initial bounded type rules are:

```text
intN div intN     -> intN
intN / intN       -> float64
floatN / floatN   -> floatN
```

The first executable integer quotient law truncates toward zero. For integer
operands `a` and non-zero `b`, the result `q` and remainder `r` satisfy:

```text
q = a div b
r = a % b
a = q*b + r
|r| < |b|
```

The sign of a non-zero remainder follows the dividend. Examples:

```text
 7 div  3 =  2
-7 div  3 = -2
 7 div -3 = -2
-7 div -3 =  2
```

True division produces a floating-point value even when the mathematical
result is integral:

```text
8 / 2       = 4.0
1 / 3       = 0.333...
8.0 / 2.0   = 4.0
```

The display form may omit a trailing `.0`, but representation and type remain
floating-point. An exact ratio is not implied by `/`; an exact rational type is
a separate future decision.

## Safety and conversion laws

- Division by zero is never a successful operation. A statically provable zero
  divisor is rejected during admission; a dynamic divisor needs an explicitly
  declared failure route before execution can be admitted.
- `INT_MIN div -1` is overflow and must be rejected or follow an authorized
  failure route. It must not wrap or trap through an unselected backend policy.
- Integer widths remain distinct identities. Mixed-width arithmetic requires
  explicit value-preserving conversion.
- Integer/float mixing requires an explicit conversion in this initial slice.
- `%` belongs to the integer-quotient family and is not initially defined for
  floating-point operands.
- LLVM and TinyVM must preserve the same result type, value, and refusal
  behavior for every admitted case.

## Boundary

This decision does not complete floating-point semantics. NaN, infinity,
signed zero, precision guarantees, rounding modes, literal suffixes, rational
values, dynamic numeric failure routes, and general refinement composition
remain separate decisions. It also does not claim that the current canonical
compiler already accepts the `div` spelling or executes the complete matrix.

The focused acceptance matrix is recorded in
[the arithmetic division canon task](../../tasks/arithmetic-division-canon.md).

