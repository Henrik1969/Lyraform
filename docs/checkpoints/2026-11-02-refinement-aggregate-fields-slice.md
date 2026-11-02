# Refinement aggregate-field vertical slice — 2026-11-02

## Scope

R4 carries a statically proven scalar refinement into a finite ordinary
record. The slice covers field construction, field projection, and the
semantic boundary for reconstruction-based member updates. It does not admit
dynamic invariant enforcement, physical member stores, aggregate-level
invariant composition, ownership, or lifetime transitions.

## Construction and projection

For:

```text
type Positive32 refines int32 {
    invariant value > 0
}
record Wallet { balance : Positive32 }
```

a statically valid `balance: 7` is checked against `Positive32` before the
ordinary-record artifact is admitted. The public record catalog preserves the
source field through its proven `int32` carrier, so the existing private
finite-record lowering can construct and project the value without inventing a
new aggregate representation.

The invalid field value `-1` is refused with
`FLOWANALYST_REFINED_RECORD_FIELD_VIOLATION`. An unknown field value is refused
with `FLOWANALYST_REFINED_RECORD_FIELD_UNPROVEN`; both diagnostics retain
source provenance.

The valid construction and projection cross Flowanalyst, Flowbind,
Flowparallel, Flowoptimize, Flowprepare, Flowvalidate, LLVM/native, and
TinyVM. The native and TinyVM executions complete successfully.

## Member-update boundary

The existing ordinary-aggregate law remains reconstruction and root rebinding.
For a statically valid update such as `9 -> wallet.balance`, the target fact
records:

```text
assignability.state: assignable
assignability.update: reconstruct_and_rebind
assignability.destination_type: int32
execution: unsupported
```

The fact is preserved through Flowbind, Flowparallel, Flowoptimize, and
Flowprepare. LLVM and TinyVM reject the final member execution because the
backend reconstruction/store contract is not implemented yet. Invalid and
unknown updates are refused before that backend boundary with
`FLOWANALYST_REFINED_MEMBER_VIOLATION` and
`FLOWANALYST_REFINED_MEMBER_UNPROVEN` respectively.

This keeps semantic admission separate from executable availability. A field
carrier is not permission for a backend to mutate storage or to bypass the
aggregate reconstruction law.

## Verification

Focused command:

```text
python3 tools/test-refinement-aggregate-fields.py \
  --build /tmp/lyraform-refinement-clean
```

Result:

```text
{"field_construction":"admitted","field_projection":"executed","member_update":"semantically_admitted_backend_refused","invalid_field":"refused","unknown_field":"refused","invalid_member":"refused","unknown_member":"refused"}
```

The focused CTest matrix, including R1 through R4, passed `11/11`.

## Remaining boundary

The containing record has no invariant of its own in this slice. Composing
field refinements with aggregate invariants, nested ownership, lifecycle
obligations, and executable reconstruction remains a later vertical slice.
