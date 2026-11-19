# TinyVM record values and native parity — unit checkpoint

Mission `lyraform.ordinary-record-construction`, revision 1 remains active.
TinyVM lowering now represents finite ordinary records with private trees of
scalar slots. Construction snapshots fields, copies recursively allocate distinct
field storage using existing MOVE instructions, and projection selects the typed
field. No new ISA or foreign-layout packing rule is needed. Record return ABIs
and rebinding are explicitly unsupported in this bound.

The uninstalled test-only adapter compiles the actual blocked source plan without
rewriting it, validates the resulting VM artifact and executes both switch and
computed engines. Native and VM units return 12, 3 and 73 for nested-copy,
renamed-negative and false-field source variants. The VM unit checks all four
primitive leaves of the whole-record copy: distinct slot identities, equal values,
and unchanged copied values when original slots are perturbed after execution.
That unit observation supplies no source member-write authority.

Verification: canonical build/doctor and 193/193 tests pass. Parity covers three
source variants, both VM engines and six native O0/O2 binaries. Valgrind reports
zero errors and zero remaining allocations for all three VM adapter runs and six
native binaries. The scalar ConfigResolve probe passes 96 cases, renamed-source
and hostile-oracle checks, with 911 protected files verified. Policy audit 41/41,
zero errors/drift; diff whitespace check passes. The new CMake unit target is
uninstalled and its policy evidence hash was reviewed. [Logs](../../experiments/configresolve/evidence/tinyvm-record-values/).

Public admission remains blocked on both backends. Complete source/control-flow/
arithmetic admission, general initializer forms and record function parameter/
return ABIs remain unfinished. The record mission and ConfigResolve port remain
active; these unit binaries do not satisfy the complete port objective. No
application-specific compiler/runtime logic, commit, push or merge was introduced.
