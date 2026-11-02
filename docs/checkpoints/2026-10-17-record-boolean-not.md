# Record Boolean initializer execution — partial checkpoint

Mission `lyraform.ordinary-record-construction`, revision 1 remains active.
Question: can the already-defined Bool `not` initializer execute consistently
inside independent nested record values on both backends?

Scout: shared scalar authority accepts `not Bool -> Bool`, and typed record
operands preserve it. Both backend unary dispatchers refused it. A source probe
confirmed the TinyVM refusal before implementation. No new semantic decision was
needed. Native lowering now emits Boolean XOR with true; TinyVM uses existing
Boolean equality with false. Both require the Boolean carrier, reusing primitive
operations without new ISA or application-specific behavior.

Six real source variants cover original nested copies, renamed negative values,
false fields, `not false`, `not true`, and nested negation of a comparison. Twelve
native O0/O2 binaries and both VM engines agree. Copied storage checks remain in
place. Three hostile integer operands are rejected by both unit adapters through
shared record operand validation. Public source admission still refuses all six
record programs; unit adapters retain the exact blocked plan.

Verification: canonical build and doctor pass; 193/193 tests pass. Twelve native
ASan runs pass (actual O0 memory instrumentation checked; leak detection disabled
for the established host restriction). Eighteen Valgrind runs report zero errors:
twelve native binaries and six VM adapter runs. The scalar ConfigResolve probe
passes 96 cases, renamed-source and hostile-oracle checks, and verifies 911
protected baseline files. Policy audit: 41/41, zero errors/drift. Whitespace
verification passes. [Logs](../../experiments/configresolve/evidence/record-boolean-not/).

Gate: PASS for Boolean initializer backend parity only. Public record execution,
range/division admission, broader initializer forms and record parameter/return
ABIs remain unfinished. Next work should close public local-record admission by
using typed record facts before statement coverage, with enforceable arithmetic
bounds rather than inferring source arithmetic law from backend behavior. The
full ConfigResolve port remains incomplete; no commit, push or merge occurred.
