# Numeric promotion vertical slice — 2026-11-02

## Scope

This checkpoint records the second bounded arithmetic slice after the integer
quotient (`div`) slice. It establishes one coherent numeric rule across the
frontend, shared scalar semantics, Flowanalyst admission, Flowcontracts
artifact validation, LLVM lowering, TinyVM lowering, and execution.

The admitted source rules are:

```text
intN + float64 -> float64
intN - float64 -> float64
intN * float64 -> float64
intN / intN    -> float64
float64 / ...  -> float64
```

The mirrored operand orders are admitted for `+`, `-`, and `*`. Integer
operands are represented as explicit `numeric_promotion` nodes in the lowering
artifact. `div` remains integer-only, `%` remains refused, and a statically
known zero divisor is refused before backend lowering.

## Vertical evidence

The focused CTest gates passed:

```text
lyraform_scalar_semantics       PASS
lyraform_integer_conversions    PASS
lyraform_arithmetic_division_canon PASS
lyraform_numeric_promotion_canon  PASS
```

`lyraform_numeric_promotion_canon` traces these cases through frontend,
analysis, binding, scheduling, optimization, preparation, Flowcontracts
validation, LLVM lowering/native execution, and TinyVM lowering/validation/
execution:

```text
2.5 + 5       -> 7.5
5 - 2.5       -> 2.5
2.5 * 5       -> 12.5
5 / 2.5       -> 2.0
8 / 2         -> 4.0
1 / 3         -> 0.3333333333333333...
```

Negative evidence covers float operands to `div`, the still-unadmitted `%`,
and `1 / 0`.

## Whole-suite status

The full CTest run was not green:

```text
160/198 passed
38 failed
```

The failures remain outside the focused slice’s acceptance claim and include
the active bare-`int` to explicit-width migration, older provider/pipeline and
corpus fixtures, and existing artifact-size/record-boundary pressure tests.
The focused gates above are the evidence for this slice; the repository is not
represented as globally complete or production-ready.

## Backend boundary

TinyVM ISA v1 now has an `f64` carrier and distinct `FADD`, `FSUB`, `FMUL`,
and `FDIV` opcodes. Integer and floating arithmetic cannot cross carriers
without the explicit lowering conversion. The implementation does not yet
settle NaN, infinity, signed-zero, rounding-mode, precision, or dynamic failure
route policy.
