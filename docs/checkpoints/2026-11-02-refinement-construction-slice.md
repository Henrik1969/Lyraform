# Refinement construction vertical slice — 2026-11-02

## Scope

This checkpoint records R1 of the refinement lifecycle: direct construction of
one scalar refinement from a statically known integer value. The specimen is:

```lyraform
type Positive32 refines int32 {
    invariant value > 0
}
```

The slice admits a direct typed initializer only when every invariant is
statically proved. It carries the admitted value through the canonical
Flowanalyst → Flowbind → Flowparallel → Flowoptimize → Flowprepare →
Flowcontracts → Flowlower/TinyVM path.

## Evidence

Focused command:

```text
python3 tools/test-refinement-construction.py \
  --build /tmp/lyraform-refinement-clean
```

Result:

```json
{"valid":"admitted","invalid":"refused","unknown":"refused",
 "llvm":"checked","tinyvm":"checked"}
```

The valid case constructs `Positive32(7)` and preserves its `int32` carrier in
the ready lowering plan; the surrounding specimen returns `0` through the
existing `c_int` entry boundary on TinyVM. LLVM lowering is also produced
successfully. The invalid `Positive32(-1)` case is refused by
Flowanalyst with `FLOWANALYST_REFINED_INVARIANT_VIOLATION`, including source
file, line, column and statement AST provenance. An unknown initializer remains
refused with `FLOWANALYST_REFINED_VALUE_UNSUPPORTED`.

The clean-build focused regression set passed 8/8 tests. The complete CTest
run measured 162/200 passed and 38 failed. Those failures match the existing
explicit-width, provider/integration, record-boundary and corpus migration
baseline; the new refinement test and all preceding numeric and branch slices
passed. This slice does not claim whole-repository green status.

## Boundary

This slice does not admit refined values in assignments, function parameters or
returns, aggregate fields, member mutation, inherited refinements, dynamic
runtime validation, or failure routing. Those remain explicit follow-up slices.
The implementation is construction-only and does not invent a runtime trap or
failure disposition.
