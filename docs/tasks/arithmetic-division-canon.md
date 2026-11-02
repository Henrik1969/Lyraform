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
| Integer remainder | reserved for the integer family | `%` is explicitly refused until a backend opcode contract exists |
| True division | `8 / 2` has `float64` type and value `4.0`; `1 / 3` is non-integral float | `/` never narrows to integer because the value is integral |
| Float division | `8.0 / 2.0` remains `float64`, value `4.0` | float result is not reclassified as integer |
| Zero divisor | statically known zero is rejected before lowering | dynamic zero requires an admitted failure route |
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
8 / 2
1 / 3
8.0 / 2.0
1 / 0
INT_MIN div -1
x div y
```

## Current status

The owner decision is accepted. The integer-quotient vertical slice is complete
for statically established compatibility-width operands: frontend keyword,
semantic admission, zero/overflow refusal, artifact validation, LLVM lowering,
TinyVM lowering, execution, and backend parity are covered by
`lyraform_arithmetic_division_canon`.

The first mixed-numeric vertical slice is also complete for `+`, `-`, `*`, and
`/`: integer operands are promoted to the `float64` carrier when required,
`/` always returns `float64`, and LLVM/TinyVM execute the same admitted cases.
Its focused acceptance gate is `lyraform_numeric_promotion_canon`.

The remaining matrix is intentionally open: immutable-local proof reuse,
dynamic failure routing, `%`, floating exceptional values and precision rules,
broader conversion policy, and the repository's explicit-width migration still
require later slices.

