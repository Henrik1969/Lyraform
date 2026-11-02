# Native record value implementation — unit checkpoint

Mission `lyraform.ordinary-record-construction`, revision 1 remains active.
Structured native lowering now derives private LLVM aggregate types from catalog
identities. Construction inserts all named fields, record reads/copies load and
store aggregate values in distinct local slots, and projection extracts the
catalog-selected field. Source names do not dispatch backend behavior; no foreign
ABI layout or ConfigResolve-specific logic is introduced.

The uninstalled emitter unit helper accepts the real blocked semantic report and
changes only its outer artifact envelope for internal emission. Its lowering plan,
including refusal, remains unchanged. The public pipeline still refuses these
source programs. This is a unit implementation proof, not public source admission
or completion of the ConfigResolve port.

Six compiled unit binaries cover nested construction/copies, renamed and changed
source, negative values and Boolean-controlled branches at O0 and O2. Expected
exit results are 12, 3 and 73. Tests inspect aggregate load/store and at least three
separate aggregate slots. Source, LLVM and binary hashes are recorded in the logs.

Verification: Igor build, doctor and 193/193 tests pass; integrated native ABI test
passes. All six record binaries pass Valgrind with zero errors and no remaining
allocations. All six pass AddressSanitizer mode with LeakSanitizer disabled for
the ptrace environment. The unit instrumentation adds ASan function attributes
without changing source operations and disables skipping proven-safe/promotable
stack accesses. O0 emitted instrumented IR is checked for real ASan memory-check
calls. Merely linking the runtime initially produced no checks, so that was not
accepted as sanitizer evidence. No UBSan coverage is claimed for generated IR.

The scalar ConfigResolve probe passes 96 cases, renamed-source and hostile-oracle
checks, with 911 protected files verified. Policy audit 41/41, zero errors/drift;
diff whitespace check passes. [Logs](../../experiments/configresolve/evidence/native-record-values/).

Remaining: TinyVM record value implementation/parity, complete executable
admission (including control-flow and arithmetic bounds), general initializer
forms and source function parameter/return record ABIs. Runtime resource-bearing
records remain outside the accepted bound. No commit, push or merge was made.
