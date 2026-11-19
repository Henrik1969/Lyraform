# Nested refined-aggregate vertical slice — 2026-11-02

## Scope

R5 proves composition through nested finite ordinary aggregates. The current
language has no direct syntax for declaring an invariant on a record itself,
so this slice does not invent one or change the parser contract. Instead, an
outer aggregate is admitted only when its nested refined fields are admitted.

The slice covers nested construction, projection, and the semantic boundary
for nested reconstruction-based member updates. It does not admit dynamic
invariant enforcement, physical member stores, ownership, or lifetime
transitions.

## Source shape

```text
type Positive32 refines int32 {
    invariant value > 0
}
record Balance { amount : Positive32 }
record Wallet { balance : Balance }
```

The valid `Wallet({balance: {amount: 7}})` specimen is recursively checked.
The record catalog carries `Wallet.balance` as the `Balance` record identity
and `Balance.amount` as its proven `int32` carrier. The nested construction and
projection path is lowered through the existing private finite-record
representation.

For `9 -> wallet.balance.amount`, the target fact preserves both member
segments and records the existing reconstruction law:

```text
members: balance, amount
assignability.state: assignable
assignability.update: reconstruct_and_rebind
destination_type: int32
execution: unsupported
```

The final physical member update remains backend-refused. The semantic fact is
not a permission to mutate a nested storage location.

Invalid and unknown nested field construction are refused with the existing
field-boundary diagnostics. An invalid nested member update is refused with
`FLOWANALYST_REFINED_MEMBER_VIOLATION`.

## Verification

Focused command:

```text
python3 tools/test-refinement-nested-aggregates.py \
  --build /tmp/lyraform-refinement-clean
```

Result:

```text
{"nested_construction":"admitted","nested_projection":"lowered","nested_member":"semantically_admitted_backend_refused","invalid_nested_field":"refused","unknown_nested_field":"refused","invalid_nested_member":"refused"}
```

The focused CTest matrix now covers the current R1–R5 refinement slices.

## Remaining boundary

The outer record still has no independently declared invariant. Aggregate-level
cross-field laws, ownership/lifetime composition, and executable nested
reconstruction remain later slices.
