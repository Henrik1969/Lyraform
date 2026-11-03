# Integer remainder vertical slice — 2026-11-03

## Scope

This checkpoint closes the first executable integer-remainder (`%`) slice
through the Flowmini/Lyraform language pipeline. It does not change true
division (`/`), floating-point exceptional-value policy, precision rules, or
dynamic failure routing.

The admitted contract is:

```text
integer % integer -> integer
```

The divisor must be statically established as non-zero. The remainder follows
the dividend's sign, matching truncation-toward-zero quotient semantics:

```text
 7 %  3 =  1
-7 %  3 = -1
 7 % -3 =  1
-7 % -3 = -1
```

`INT_MIN % -1` is refused as an overflow boundary. Floating-point remainder
is refused as a type error. A divisor that cannot be proven non-zero remains
refused until an explicit runtime failure route is admitted.

## Vertical coverage

The slice is carried through each current layer:

- Flowanalyst admits integer `%`, evaluates constant cases, and emits distinct
  diagnostics for floating operands, unproven divisors, zero, and overflow.
- Flowcontracts validates integer remainder artifacts with the same zero and
  signed-minimum safety law as integer quotient.
- LLVM lowering maps `%` to signed remainder (`srem`).
- TinyVM ISA v1 adds `TV1_SREM` after the existing opcode range, preserving
  prior opcode numbers. Both switch and computed dispatch execute it.
- TinyVM and Flowmini conformance tests cover ordinary results, signed cases,
  and zero-divisor trapping.
- The arithmetic canon and G2 inventory now describe remainder as implemented
  rather than pending backend support.

## Verification

Clean build directory:

```text
/tmp/lyraform-refinement-clean
```

Build command:

```text
cmake --build /tmp/lyraform-refinement-clean -j2
```

Result:

```text
PASS
```

Focused gate:

```text
ctest --test-dir /tmp/lyraform-refinement-clean --output-on-failure -R '^(lyraform_scalar_semantics|lyraform_arithmetic_division_canon|lyraform_numeric_promotion_canon|tinyvm_isa_v1_conformance|flowcore_pass_corpus|flowcore_pipeline_matrix)$'
```

Result:

```text
8/8 passed
```

Full gate:

```text
ctest --test-dir /tmp/lyraform-refinement-clean --output-on-failure
```

Result:

```text
205/205 passed
100% tests passed, 0 tests failed
```

`git diff --check` also passed. The remaining numeric boundaries are dynamic
divisor failure routing, floating exceptional values and precision, broader
conversion policy, and the still-open explicit-width migration outside this
focused slice.
