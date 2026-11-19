# Ordinary record type projection, version 1

Authority: ADR 0068 complete construction and ADR 0052 independent values.
This document describes a compiler artifact contract, not additional source law.

`lowering_plan.ordinary_record_types` carries a
`lyraform.ordinary_record_types` version-1 object when the source contains types
within the currently projected bound: int, Bool and finite nested ordinary
records. Recursive, resource-bearing and otherwise unsupported records are not
projected; omission does not admit them for execution. Existing construction
refusal remains in force.

The catalog records `layout: private`, `semantics: independent_value` and
`status: non_executable`. Each entry carries its source symbol ID, declaration
ID, human-readable name and declared named fields. Scalar references carry the
accepted source type name; record references carry the declaration's symbol ID.
Identity is not inferred from a backend name. A field list is source declaration
order, not a public byte-layout or foreign calling-convention promise.

The analyzer resolves field type references in the declaring record's lexical
scope. It projects only finite supported dependency closures. Shared subrecords
remain distinct from recursive active dependency paths. The shared validator
checks exact version/meaning, unique declarations and field names, closed type
references, finite dependencies and the supported scalar bound. Foreign ABI
`aggregate_abi_layouts` remain a separate contract with their existing authority.

The catalog travels unchanged inside the lowering plan through scheduling,
optimization and preparation. It has no ready/executable form in version 1.
Unknown fields, including byte offsets, are refused so public layout cannot be
introduced through an unchecked artifact extension.

`record_construct` operands now refer to this catalog by `record_symbol_id`.
Their fields carry names and nested operands in declaration order, independently
of source initializer order. The first operand projection supports nested
literal records and int/Bool literals and scalar unary/binary expressions whose
operator types follow existing scalar authority. Other initializer forms remain
unsupported by this projection. Typed literal operands do not establish integer
execution bounds, division safety or backend support.

Shared validation requires the catalog, exact field identities and counts,
matching scalar/record types, valid scalar operators and expression IDs. The
proven finite scalar/nested bound carries a separate
`lyraform.ordinary_record_execution` version-1 contract. It is emitted only for
one sane source target after typed facts, control-flow ownership and coverage
are complete. The contract states `finite_scalar_nested`, `independent_value`,
and `private` layout. A ready plan with record operands must carry that exact
contract; blocked plans may retain record operands as inspection evidence.
Removing or mutating the contract is rejected by shared validation.

`record_value` operands now identify a typed local record definition by variable
symbol and record type identity. A compatible local copy is a value-definition
operation consuming that operand; the catalog supplies independent-value meaning.
`record_project` operands carry a typed base, named field, base record identity
and the exact declared result type, including nested record results. The analyzer
projects these reads only from previously recognized typed local definitions
and filters source declaration-before-use. It does not infer a new conversion.

Shared validation checks local definition identity/type, same-function ownership,
acyclic value definitions, field existence and exact result type. All three
record operand kinds still require a blocked version-2 plan. Structured control-flow validation additionally requires each record read to be
dominated by its local definition. Branch and loop bodies inherit incoming
definitions but never export their local definitions. Unreachable record
operations, cyclic/shared block ownership and record overwrites are refused.
Only functions containing record operands require this additional proof.
General initializers, parameter/return values and public execution admission
remain unfinished; unit-level value independence is described below. Removing an unused catalog does not invalidate an otherwise scalar
program; actual record operands require the exact type identities they depend on.
Refused source statements do not claim executable operation IDs, even when their
inspection plan contains typed operand evidence.

## Native unit implementation

Structured native lowering maps catalog record identities to private LLVM named
aggregate types. It constructs values with `insertvalue`, reads and copies whole
aggregate values through separate local slots, and projects fields with
`extractvalue`. No foreign layout or application-specific dispatch is introduced.

An uninstalled unit helper adapts the outer semantic-report envelope for the
internal emitter while preserving the exact blocked lowering plan. Six unit
binaries cover nested copies, renamed/changed source, signed values and Boolean
branches at O0/O2. This validates the native value implementation independently
of public execution admission. It is not an alternate accepted source compiler.
Public admission for local finite scalar/nested records now reaches the LLVM and
TinyVM production boundaries. Broader initializers, unsupported function record ABIs, and
complete range/division admission remain outstanding.

## TinyVM unit implementation

TinyVM lowering represents finite ordinary records as private trees of existing
scalar VM slots. Construction snapshots field values, copies recursively allocate
distinct slots and emit existing MOVE instructions, and projection selects the
typed field slot. Record anchors are compiler metadata; no new ISA operation,
foreign ABI packing rule or runtime reference identity is introduced.

A test-only build exposes an uninstalled unit adapter that compiles the unchanged
blocked source plan. Both switch and computed VM engines match the native units.
After source execution, the unit inspects distinct original/copied slots and
perturbs original storage to confirm copied fields remain unchanged. This host
unit observation does not introduce source member-write authority.

Record rebinding, unsupported function record parameter/return forms, complete
range/division admission and broader initializer forms remain unsupported by
this implementation bound. The finite pure scalar/nested parameter/return slice
is now public.

## Boolean initializer execution

Both backend value implementations now execute the established scalar Bool `not`
operator. Six source variants, including true/false/nested negation, match at
native O0/O2 and in both VM engines. Three malformed negations with integer
operands are refused by each unit adapter through shared contract validation.
This extends the unit implementation; public record admission is still blocked.

## Analyzer fact ordering

Catalog, local type and typed operand facts are now prepared before statement
coverage/admission. The immutable operand map preserves both expression identity
and expected-type context; report serialization consumes it without resolving
record types again. The existing public refusal remains until an executable
bound and its downstream validation are implemented.
