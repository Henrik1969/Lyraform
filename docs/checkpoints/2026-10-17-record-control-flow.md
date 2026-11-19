# Record control flow — partial checkpoint

Mission `lyraform.ordinary-record-construction`, revision 1 remains active.
Authority remains ADR 0068 and ADR 0052; this checkpoint adds no source law.

Shared validation now proves definite construction before typed local record
reads across structured blocks. Branch and loop bodies receive incoming
bindings, but their local bindings cannot escape. Reads are checked before a
value definition becomes available. Record overwrites, orphan record operations,
cyclic blocks and multiply owned blocks are refused. Unrelated scalar functions
and unused catalogs do not acquire additional control-flow requirements.

Two real-source positive scenarios (branches and a possibly zero-iteration loop)
remain valid blocked plans. Eight hostile mutations test forward/sibling reads,
branch/loop escapes, orphan operations, overwrites, cycles and shared blocks.
Existing native/TinyVM record value units continue to pass.

Canonical build/doctor and all 193 tests pass. The ConfigResolve scalar probe
passes 96 cases, renamed-source and hostile-oracle checks, and verifies 911
protected files. Policy audit: 41/41 contexts, zero errors and evidence drift.
Whitespace verification passes. [Logs](../../experiments/configresolve/evidence/record-control-flow/).

Public source execution remains blocked. Arithmetic/range admission, broader
initializer forms and function record parameter/return ABIs remain dependencies.
The complete ConfigResolve port is unfinished. No application-specific compiler
logic, commit, push or merge was introduced.
