# Typed record construction operands — partial implementation checkpoint

Mission `lyraform.ordinary-record-construction`, revision 1 remains active.
Valid local literal records now project as typed `record_construct` operands.
Each references the private type catalog and carries exactly its declared named
fields in declaration order, regardless of source initializer order. Finite nested
literal construction and int/Bool literals and scalar unary/binary expressions
use the same representation and existing scalar operator rules.

Shared validation checks catalog presence, field count/order/identity, nested
type correspondence, scalar operator types, literal syntax and expression IDs.
These operands currently require a blocked version-2 plan. Neither a ready flag,
a removed catalog nor changed operand meaning can authorize execution.

Validation exposed an existing coverage defect: refused record statements still
listed executable operation IDs. The analyzer now leaves those IDs empty for
refused statements. Typed inspection operations remain present without claiming
source execution. Actual blocked construction plans now pass structural validation.

Verification: canonical Igor build, doctor and all 193 tests pass. Focused tests
inspect actual nested construction, reordered fields and arithmetic/comparison
operand types, and reject eleven hostile operand mutations. Existing shape/type/
bound tests and forty hostile catalog cases also pass. The separate scalar
ConfigResolve probe passes 96 cases, renamed-source and hostile-oracle checks;
911 protected files verified. Policy audit: 41/41, zero errors/drift; diff
whitespace check passes. [Logs](../../experiments/configresolve/evidence/ordinary-record-construction-operands/).

Limits: identifiers, field projection, independent copies, general initializers,
function value ABIs and native/TinyVM record execution remain unfinished. Typed
scalar expression preservation is not proof of execution range or division safety.
No complete record or ConfigResolve port claim is made. No application-specific
compiler/runtime behavior, commit, push or merge was introduced.
