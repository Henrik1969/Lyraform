# Aggregate cross-field invariant vertical slice — 2026-11-02

## Scope

This slice adds the first explicit aggregate-level invariant contract. A
record may declare one or more comparison laws over its own fields:

```text
record Range {
    lower : int32
    upper : int32
    invariant lower <= upper
}
```

The frontend preserves the invariant as a structured expression in the AST,
the SymbolTable projection retains its spelling as factual provenance, and
Flowanalyst checks it during finite record construction. The check occurs in
semantic admission, before any backend lowering or execution.

## Admitted proof boundary

The current bounded proof slice supports:

* direct integer field literals;
* unary `+` and `-`;
* `+`, `-`, `*`, and integer `div`;
* equality and ordered comparisons;
* multiple declared aggregate invariants.

An aggregate is admitted only when each invariant is statically evaluated to
true. A false law produces `FLOWANALYST_RECORD_INVARIANT_VIOLATION`.

The following remain explicit refusals:

* a law naming a field not declared by the record;
* a dynamic or otherwise unknown field value;
* an expression outside the bounded integer proof;
* runtime invariant enforcement;
* physical aggregate mutation and backend execution.

The corresponding diagnostics are
`FLOWANALYST_RECORD_INVARIANT_NAME` and
`FLOWANALYST_RECORD_INVARIANT_UNPROVEN`.

## Verification

Focused CTest:

```text
ctest --test-dir /tmp/lyraform-refinement-clean \
  -R lyraform_record_cross_field_invariants --output-on-failure
```

Result:

```text
100% tests passed, 1 test passed
```

The focused regression covers a proven law, a statically detected violation,
and a dynamic value that remains unproven. Existing nested refined-aggregate
tests also remain green in the focused matrix.

## Remaining boundary

This slice proves construction-time cross-field validity only. It does not yet
define how aggregate invariants compose with member updates, ownership
transfer, borrowing, lifetime termination, or backend storage. Those concerns
remain separate vertical slices.
