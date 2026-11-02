# Arithmetic division canon

## Objective

Implement and verify the first bounded division semantics for Lyraform under
[ADR 0074](../architecture/decisions/0074-true-division-and-integer-quotient.md).
This task is deliberately narrower than the full numeric migration and does
not close floating-point, refinement, aggregate, or general conversion
semantics.

## Acceptance matrix

| Area | Positive evidence | Refusal or negative evidence |
|---|---|---|
| Integer quotient | `7 div 3 = 2`; signed operands truncate toward zero | no silent floor-versus-truncate ambiguity |
| Integer remainder | `a = (a div b)*b + a % b`; remainder sign follows `a` | `%` with zero divisor is refused |
| True division | `8 / 2` has `float64` type and value `4.0`; `1 / 3` is non-integral float | `/` never narrows to integer because the value is integral |
| Float division | `8.0 / 2.0` remains `floatN`, value `4.0` | float result is not reclassified as integer |
| Zero divisor | statically known zero is rejected before lowering | dynamic zero requires an admitted failure route |
| Integer overflow | `INT_MIN div -1` is rejected or explicitly routed | no wrapping, backend trap, or silent process exit |
| Width identity | same-width integer quotient preserves width | mixed widths require explicit conversion |
| Numeric mixing | explicit int/float conversion is accepted only when defined | implicit int/float mixing is refused initially |
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

The owner decision is accepted. The current published experiment has partial
integer-width and conversion machinery, but the canonical suite is not green
and the current shared scalar classifier does not yet constitute the complete
`/` versus `div` value-level contract. No implementation-complete claim is
made until the matrix is executable on both backends.

