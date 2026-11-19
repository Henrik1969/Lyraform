# Ordinary record implementation map

Observed 2026-10-17 under `lyraform.ordinary-record-construction`, revision 1.
Authority: ADR 0068 and ADR 0052. This map is engineering evidence, not new law.

The first source probes exposed a frontend boundary defect: after parsing one
record, the top-level scanner skipped another `record` declaration. Adding the
already-recognized record token to the boundary set preserves consecutive record
declarations. The nested-construction tests assert that both declarations survive.

`Flowanalyst/src/record_analysis.hpp` now checks named-field completeness for
ordinary record literals in typed local declarations and nested field literals.
It resolves record declarations through lexical symbols and origin identities;
nested field types use the declaring record's scope. Missing, duplicate and
unknown fields receive source-located diagnostics in both plan versions.
Duplicate declared fields are diagnosed when that record is constructed.
The pass now also reuses the established scalar type/operator analysis for
`int` and `Bool` fields, including nested literals. It rejects incompatible
known scalar results and scalar values supplied for a record field. It uses
existing declaration-before-use analysis. Expressions outside that analysis
remain unproved, rather than becoming accepted through an invented conversion.
This pass does not establish whole-program record typing or execution admission.

Executable-plan requests now additionally check actual record literals for
recursive by-value record dependencies and initializer purity. Repeated type
identity on the active dependency path is refused; shared finite subrecords are
not cycles. Calls require the analyzer's existing proven-pure function fact.
Unknown effects are refused without inventing an effect order. These checks do
not apply to provider-result bindings, which are not named-field construction.
They neither admit resource fields nor certify general value execution.

The lowering plan now carries a validated
[private type catalog](../architecture/ordinary-record-type-projection.md) for
finite int/Bool/nested records. This preserves source type and field identity
across stages, independently of foreign aggregate ABI layout. The catalog is
non-executable. Typed literal construction operands now reference its type IDs,
normalize named fields to declaration order and preserve nested scalar expressions.
Shared validation checks that correspondence and rejects premature ready status.
Typed local record reads/copy operands and nested field projections now retain
local symbol/type/field identity. Shared validation binds reads to typed definitions
in the same function and refuses cyclic value definitions. Structured dataflow
now checks definition-before-read through branches and loops, rejects escaped
branch/loop-local definitions, and refuses orphan operations, record overwrites
and cyclic/shared block ownership. Public execution admission is established
for the finite scalar/nested bound; general initializers and broader function
value ABIs remain unfinished.

## Remaining implementation owners

- Flowanalyst must produce typed record facts and operands, check declared field
  types, initializer purity, finite type structure, construction in calls/returns,
  and independent copies. Existing scalar rules live in
  `Flowcontracts/include/flowcontracts/scalar_semantics.hpp` (`int`, `Bool`);
  reuse those rules rather than infer source semantics from LLVM carriers.
- `Flowanalyst/src/main.cpp` now emits typed construction/read/projection operands
  for the proved local bound; broader initializer and function ABI forms remain
  unsupported. Maintain
  the explicit v2 construction refusal until the supported bound is executable.
- Shared Flowcontracts validators must validate new record facts at each stage
  boundary. Provider-verified `aggregate_abi_layouts` describe foreign ABI, not
  ordinary records; do not turn the private layout into a public ABI contract.
- Native structured lowering now implements unit-tested ordinary value operations
  and the finite record function parameter/return slice; finish the remaining admission checks and broader
  function value ABIs while preserving the existing
  ownership-specific TextOutcome carrier law.
- TinyVM now has unit-tested independent scalar-slot record values and the finite
  record function parameter/return slice alongside provider aggregate carriers.
  Remaining admission checks still require completion.

Source tests cover field shape, declaration retention, bounded scalar field
typing, finite/pure construction checks and typed record control flow. Backend
units provide bounded copy/parity evidence. They do not establish general field
typing, general purity, public execution admission or a ConfigResolve port.

Native structured lowering now implements private aggregate construction,
whole-value local copies and field extraction. The finite scalar/nested bound
also crosses the public LLVM path for one sane target and produces a real binary.
TinyVM now implements private scalar-slot record trees with recursive value
copies and typed projection. Both VM engines match the native unit results;
post-execution unit checks verify distinct copied field storage. Remaining
admission checks and broader source function forms remain dependencies. The public
status changes only when the exact `ordinary_record_execution` contract
accompanies the typed facts; unsupported records remain refused.

Record facts now resolve before statement coverage: catalog, local record types,
and operands are captured in an immutable map keyed by expression identity and
expected type. Report serialization performs lookup only. Operation argument
expected-type selection has one shared helper, retaining the later specialized
TextOutcome producer projection. This removes late semantic resolution as a
prerequisite to execution admission; it does not itself admit records.
