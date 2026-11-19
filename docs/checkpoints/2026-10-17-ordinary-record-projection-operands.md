# Typed local record reads and projections — partial implementation checkpoint

Mission `lyraform.ordinary-record-construction`, revision 1 remains active.
Recognized typed local definitions now support `record_value` reads and compatible
copy operands. `record_project` preserves named field, typed base and exact result
type, including nested record projection. Source declaration-before-use and
lexical symbol resolution determine which known local records can be projected.
No conversion is inferred by this representation.

Shared validation ties a read to a typed local definition in the same function,
checks record identities, field existence and exact result types, and rejects
cyclic or duplicate value definitions. All record operand kinds require a blocked
version-2 plan. These facts carry independent-value authority from the catalog;
they do not prove runtime storage independence or control-flow dominance.

Verification: actual source constructs a nested record, copies it, extracts a
record-valued field, then reads int and Bool fields through both the extracted
record and a nested field chain. Ten hostile mutations test local identity, type,
field, result category, function ownership, cyclic/duplicate definitions,
catalog removal and premature ready status. Focused tests and all 193 canonical
Igor tests pass; build and doctor pass. The independent scalar ConfigResolve probe
passes 96 cases with renamed-source and hostile-oracle checks; 911 protected
files verified. Policy audit 41/41, zero errors/drift; diff whitespace check passes.
[Logs](../../experiments/configresolve/evidence/ordinary-record-projection-operands/).

Remaining: general initializer operands, function parameter/return record ABIs,
control-flow admission, native/TinyVM value representation and verified independent
runtime copies. The record mission and full ConfigResolve port are incomplete.
No application-specific compiler/runtime behavior, commit, push or merge was added.
