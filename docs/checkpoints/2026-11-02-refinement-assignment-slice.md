# Refinement assignment vertical slice — 2026-11-02

## Scope

This checkpoint records R2 of the refinement lifecycle. It extends the bounded
R1 scalar construction proof with direct assignment to the same refined local:

```lyraform
type Positive32 refines int32 {
    invariant value > 0
}

main {
    x : Positive32(7)
    9 -> x
    0 -> return
}
```

An assignment is admitted only when the complete invariant set is statically
proved for the assigned value. No runtime trap, implicit failure route or
backend-specific enforcement is introduced.

## Evidence

Focused command:

```text
python3 tools/test-refinement-assignment.py \
  --build /tmp/lyraform-refinement-clean
```

Result:

```json
{"valid":"admitted","invalid":"refused","unknown":"refused",
 "llvm":"checked","tinyvm":"checked"}
```

The valid assignment `9 -> x` reaches a ready semantic plan and is lowered
through LLVM and TinyVM. The invalid assignment `-1 -> x` is refused with
`FLOWANALYST_REFINED_ASSIGNMENT_VIOLATION`. An unknown assignment remains
refused with `FLOWANALYST_REFINED_ASSIGNMENT_UNPROVEN`. Both diagnostics carry
source file, line, column and statement provenance.

The combined focused regression set passed 9/9 tests, including the preceding
construction, numeric, branch-admission and documentation gates.

The complete CTest run measured 163/201 passed and 38 failed. The failed set
is the same known explicit-width, provider/integration, record-boundary and
corpus migration baseline; both refinement lifecycle tests passed. This slice
does not claim whole-repository green status.

## Lifecycle boundary

This slice covers one statically proven scalar local after direct construction.
Function arguments and returns, aggregate fields, member mutation, inherited
refinement composition, dynamic runtime validation and failure routing remain
outside the admitted lifecycle.
