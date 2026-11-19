# Arithmetic division canon

## Objective

Implement and verify the bounded division and first mixed-numeric semantics for Lyraform under
[ADR 0074](../architecture/decisions/0074-true-division-and-integer-quotient.md).
This task is deliberately narrower than the full numeric migration and does
not close floating-point, refinement, aggregate, or general conversion
semantics.

## Acceptance matrix

| Area | Positive evidence | Refusal or negative evidence |
|---|---|---|
| Integer quotient | `7 div 3 = 2`; signed operands truncate toward zero; LLVM/TinyVM parity | no silent floor-versus-truncate ambiguity |
| Integer remainder | `7 % 3 = 1`; sign follows the dividend; LLVM/TinyVM parity | zero divisor and `INT_MIN % -1` are refused before lowering |
| True division | `8 / 2` has `float64` type and value `4.0`; `1 / 3` is non-integral float | `/` never narrows to integer because the value is integral |
| Float division | `8.0 / 2.0` remains `float64`, value `4.0` | float result is not reclassified as integer |
| Integer zero divisor | statically known zero for `div`/`%` is rejected before lowering | dynamic integer zero requires an admitted failure route |
| Floating exceptional division | `1 / 0` produces `+infinity`, `-1 / 0` produces `-infinity`, and `0 / 0` produces `NaN` under the binary64 contract | non-finite source literals are rejected; integer `div`/`%` do not use this rule |
| Integer overflow | `INT_MIN div -1` is rejected or explicitly routed | no wrapping, backend trap, or silent process exit |
| Width identity | same-width integer quotient preserves width | mixed widths require explicit conversion |
| Numeric mixing | `+`, `-`, `*`, and `/` promote mixed operands to `float64`; the artifact carries an explicit promotion node | unsupported numeric families and unproved conversions remain refused |
| Backend parity | LLVM and TinyVM agree on value, type, and refusal | any divergence blocks the slice |

## Required source specimens

The implementation test matrix should include constant expressions, immutable
locals, function arguments, function results, record fields where already
admitted, and a dynamic divisor. Each source specimen must be traced through:

```text
AST → symbol facts → semantic admission → validated artifact
→ lowering plan → LLVM/TinyVM result or explicit refusal
```

At minimum:

```text
7 div 3
-7 div 3
7 div -3
-7 div -3
7 % 3
-7 % 3
8 / 2
1 / 3
8.0 / 2.0
1 / 0
1 div 0
INT_MIN div -1
x div y
```

## Current status

The owner decision is accepted. The integer-quotient/remainder vertical slice is complete
for statically established compatibility-width operands: frontend keyword,
semantic admission, zero/overflow refusal, artifact validation, LLVM lowering,
TinyVM lowering, execution, and backend parity are covered by
`lyraform_arithmetic_division_canon`.

The first mixed-numeric vertical slice is also complete for `+`, `-`, `*`, and
`/`: integer operands are promoted to the `float64` carrier when required,
`/` always returns `float64`, and LLVM/TinyVM execute the same admitted cases.
Its focused acceptance gate is `lyraform_numeric_promotion_canon`.

The remaining matrix is intentionally open: immutable-local proof reuse,
dynamic integer failure routing, broader conversion policy, user-selected
rounding modes, and the repository's explicit-width migration still require
later slices. Floating exceptional values and the bounded binary64 precision
rules are now covered by `lyraform_floating_exceptional_precision`.

