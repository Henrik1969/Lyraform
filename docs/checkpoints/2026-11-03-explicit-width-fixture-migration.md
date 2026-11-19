# Explicit-width fixture migration — 2026-11-03

## Scope

This checkpoint updates the active Lyraform/FlowMini fixture and test surface
after the owner decision that bare `int` has no executable width. It does not
silently turn a newly exposed implementation gap into a passing expectation.

The migration covers active `.flow` examples, Flowanalyst/Flowlower record
fixtures, scalar and target tests, generated AST/frontend/symbol goldens, and
the current programmer manual. Archive trees and historical reports remain
historical evidence.

Record literals retain contextual numeric literals, for example:

```flow
record Point { x : int32
y : int32
}
point : Point({x: 10, y: 20})
```

`int32(10)` is a typed declaration initializer, not an expression cast inside
a record literal. Explicit conversion at a host result boundary remains a
separate operation, such as `result : c_int(value)`.

## Verification

The generated active goldens pass:

```text
AST golden tests: PASS (29)
Frontend bundle tests: PASS (8 golden, 1 isolated, 19 negative)
Symbol projection tests: PASS (14)
Flowanalyst tests: PASS
```

The focused numeric/refinement gates also pass:

```text
lyraform_explicit_integer_width
lyraform_integer_conversions
lyraform_arithmetic_division_canon
lyraform_numeric_promotion_canon
lyraform_unreachable_invalid_branch
lyraform_refinement_construction
lyraform_refinement_assignment
lyraform_refinement_function_boundaries
lyraform_refinement_aggregate_fields
lyraform_refinement_nested_aggregates
lyraform_record_cross_field_invariants
```

The full clean build directory was `/tmp/lyraform-refinement-clean`. After the
compatibility-parser admission, the full CTest run registered 205 tests:

```text
176 passed
29 failed
86% passed
```

The repository is therefore intentionally not reported as globally green.

The deprecated compatibility parser was then taught to recognize explicit
`int32`/`int64` names and explicit-width list/array element spellings while
retaining its historical runtime carrier. That is compatibility support for
the oracle workflow, not a second canonical semantic authority.

## Remaining implementation boundaries exposed by the migration

The remaining failures include:

- canonical scalar/runtime-path tests that still lack explicit-width admission;
- parse-validity and target-preservation paths whose current implementation
  does not yet carry the new width contract through every path;
- static guard proof over explicit-width scalar bindings;
- ordinary record execution admission, while declarative record operands and
  the native record unit remain covered;
- the contract-stabilization guard specimen;
- older provider, text, graph, and pass-corpus integrations whose source
  operations or numeric contracts are not yet admitted under the current
  rules.

These are implementation or contract follow-ups, not reasons to restore bare
`int` to active fixtures. Historical and hostile-artifact tests that
intentionally mention bare `int` remain unchanged where their purpose is to
prove rejection.

## Commands

```bash
FLOWMINI_BIN=/tmp/lyraform-refinement-clean/flowmini/flowmini \
  Lyraform/compiler/tools/run-flowmini-ast-golden-tests.sh

FLOWMINI_BIN=/tmp/lyraform-refinement-clean/flowmini/flowmini \
  Lyraform/compiler/tools/run-flowmini-frontend-bundle-tests.sh

FLOWMINI_BIN=/tmp/lyraform-refinement-clean/flowmini/flowmini \
  Lyraform/compiler/tools/run-flowmini-symbol-projection-tests.sh

ctest --test-dir /tmp/lyraform-refinement-clean --output-on-failure
```

