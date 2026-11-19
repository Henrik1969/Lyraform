# Refinement function-boundary vertical slice — 2026-11-02

## Scope

R3 extends the bounded scalar refinement lifecycle through function
arguments and returns. This slice admits only values whose refinement
predicate is statically provable at the source boundary. It does not claim
dynamic refinement checking or general aggregate, ownership, or lifecycle
composition.

## Source specimen

The focused specimen declares `Positive32` as a refinement of `int32`, passes
the statically valid literal `7` into a refined parameter, and returns `7`
from a refined-returning function. The same specimen is exercised through the
Flowanalyst, binding, planning, optimization, preparation, validation, LLVM,
native, and TinyVM stages.

The generated callable contracts use the explicit `int32` carrier required by
the current lowering boundary. That carrier normalization does not erase the
source refinement contract: the refined argument and return checks occur
before the callable is admitted.

## Boundary evidence

The valid specimen was admitted and executed on both LLVM/native and TinyVM.
The focused result was:

```text
{"valid":"admitted","bad_argument":"refused","bad_return":"refused","unknown_return":"refused","llvm":"checked","tinyvm":"checked"}
```

The rejected cases preserve source provenance and use these boundary
diagnostics:

* `FLOWANALYST_REFINED_ARGUMENT_VIOLATION` for `accept(-1)`;
* `FLOWANALYST_REFINED_RETURN_VIOLATION` for a statically invalid `-1`
  return;
* `FLOWANALYST_REFINED_RETURN_UNPROVEN` for an unknown return value.

## Verification

Focused command:

```text
python3 tools/test-refinement-function-boundaries.py \
  --build /tmp/lyraform-refinement-clean
```

Result: pass.

The focused CTest matrix, including R1 construction, R2 assignment, and R3
function boundaries, passed `10/10`.

The full repository CTest run registered `202` tests and passed `164/202`;
`38` existing integration, explicit-width migration, provider, text,
graph, and record/corpus tests remain red. The R3 test itself passed and did
not add a new failure class.

## Remaining boundary

Dynamic refined arguments and returns remain refused because their predicates
are not yet executable at this semantic boundary. Refined aggregate fields,
member updates, ownership/lifetime composition, and invariant preservation
across nested values remain later lifecycle slices.
