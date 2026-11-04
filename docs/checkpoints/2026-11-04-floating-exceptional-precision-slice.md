# Floating exceptional and precision vertical slice — 2026-11-04

## Scope

This slice closes the first bounded exceptional/precision rules for Lyraform's
`float64` carrier. It follows the existing numeric-promotion and true-division
slices through the frontend, semantic admission, artifact validation, LLVM
lowering, native execution, TinyVM lowering, validation, and execution.

The authority is [ADR 0075](../architecture/decisions/0075-floating-exceptional-and-precision-canon.md).

## Canon

```text
float64: IEEE-754 binary64
rounding: nearest, ties-to-even
source literals: finite binary64 only
arithmetic: may produce and propagate NaN and +/-infinity
signed zero: preserved and observable
```

Floating true division by zero is therefore a value-level arithmetic result:
`1.0 / 0.0` produces positive infinity, `-1.0 / 0.0` produces negative
infinity, and `0.0 / 0.0` produces NaN. Integer `div` and `%` remain strict
operations with their existing zero-divisor and overflow boundaries.

## Implementation evidence

The vertical slice includes:

- exponent-aware frontend numeric tokenization and float-literal classification;
- finite-binary64 source-literal admission in Flowcontracts and Flowanalyst;
- removal of the TinyVM floating zero-divisor trap;
- explicit TinyVM `FNEG` lowering/runtime support to preserve negative zero;
- LLVM/TinyVM witnesses for positive and negative infinity, NaN, negative zero,
  ties-to-even, next representable rounding, and overflow to infinity;
- a negative witness for a non-finite source literal.

The focused gate passed:

```text
lyraform_floating_exceptional_precision: PASS
```

The related arithmetic admission gates also passed:

```text
lyraform_numeric_promotion_canon: PASS
lyraform_unreachable_invalid_branch: PASS
```

Clean configure and build also passed:

```text
cmake -S . -B /tmp/lyraform-refinement-clean -G Ninja: PASS
cmake --build /tmp/lyraform-refinement-clean -j2: PASS
```

The complete current CTest result is:

```text
207/207 passed
100% tests passed, 0 tests failed
```

`git diff --check` passed. The repository remains experimental and not
production-ready.

## Boundary

NaN payload identity, user-selected rounding modes, decimal exactness, rational
values, wider floating families, and provider-specific transcendental behavior
remain open. Exceptional floating values are not implicit failure routes, and
dynamic integer-divisor failure routing remains a separate slice.
