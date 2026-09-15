# TinyVM recovered prototype

This directory recovers Henrik's 2025 TinyVM learning experiment as a small,
buildable baseline for future mutation. It is experimental prior art, not part
of Flowcore's canonical Graph IR or a standalone graph runtime. Its governed
Flowcore integration is documented below.

The original `~/Development/tinyvm` source tree is no longer present. This
edition was reconstructed on 2026-08-26 from two surviving backtraces and the
imported ChatGPT archive. It therefore preserves the demonstrated architecture
and core dispatch loop, but does not claim byte-for-byte identity with the lost
tree.

## Preserved ideas

- C17 with the GNU labels-as-values extension.
- Direct-threaded `goto *dispatch[opcode]` execution.
- Four signed 64-bit fields per `InstrWord`: `opcode`, `a`, `b`, and `pad`.
- Eight general registers, a program counter, flags, and a running state.
- The recovered arithmetic, bitwise, comparison, control, memory, stack,
  transaction-context, and halt opcode identities.
- Instruction handlers kept separate from the dispatch loop.

The transaction-context operations are retained as explicit unsupported stubs
until their original storage and rollback semantics can be recovered or newly
specified. The `pad` field is reserved; the historical plan was to divide part
of it into instrumentation flags, an expansion index, and source/debug identity.

`tinyvm_isa_conformance` covers all 27 recovered opcode identities and the
defined fault boundaries. Each case runs through computed goto, `switch`, and
function-pointer dispatch and compares registers, memory, stack, program
counter, flags, running state, result and fault text. The five `CTX_*` opcodes
currently conform by producing the same explicit unsupported fault; passing
their tests is not a claim that their historical semantics were recovered.

## Build the isolated prototype

```sh
cmake -S subprojects/TinyVM -B build/tinyvm-recovered
cmake --build build/tinyvm-recovered
ctest --test-dir build/tinyvm-recovered --output-on-failure
```

## Dispatch benchmark

`tinyvm_dispatch_benchmark` executes identical instruction arrays through the
recovered computed-goto loop, an ordinary `switch`, and a function-pointer
table. It reports a NOP-heavy dispatch workload plus periodic and deterministically
shuffled mixed-handler workloads. Results are measurements of one
host/compiler/configuration, not a portable performance promise.

```sh
cmake -S subprojects/TinyVM -B build/tinyvm-bench \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc
cmake --build build/tinyvm-bench --target tinyvm_dispatch_benchmark
build/tinyvm-bench/tinyvm_dispatch_benchmark
```

The first recovered-baseline observations are recorded in
[`benchmarks/RESULTS-2026-08-26.md`](benchmarks/RESULTS-2026-08-26.md).

## Provenance

- `/home/henrik/Dokumenter/tinyWM_backtraced.md`
- `/home/henrik/Dokumenter/tinyWM_backtraced2.md`
- Chat archive conversation `68dd3d80-fc2c-832c-bb22-bbf76265f30f`,
  "Binary executor implementation", created 2025-10-01.
- Chat archive conversation `6905dbb5-97b8-8327-b4c6-6352af7b2234`,
  "GCC computed goto explanation", created 2025-11-01.

Future Flowmini work should lower a validated canonical artifact into a
versioned TinyVM executable artifact. GNU dispatch is an execution mechanism;
it is not semantic authority.

The staged integration and LLVM-parity plan is documented in
[`docs/architecture/tinyvm-flowcore-backend-plan.md`](../../docs/architecture/tinyvm-flowcore-backend-plan.md).
The first binary boundary is specified in [`ARTIFACT-V1.md`](ARTIFACT-V1.md).
The sectioned Flow-capable successor contract is specified in
[`ARTIFACT-V2.md`](ARTIFACT-V2.md).
The first typed execution semantics are specified in [`ISA-V1.md`](ISA-V1.md).
The process-argument execution-input extension is specified in
[`ISA-V2.md`](ISA-V2.md).

Graph artifacts use ISA 2 activation records to expose validated wire, signal,
port and delivery identity through an optional runtime observer. Use
`flowtinyrun --trace-graph` to write those activation events as JSONL to
standard error; a host embedding the ISA may also install a scheduling hook to
refuse an activation before it is observed. The host can also set a
deterministic activation-record limit on the execution context; the default is
`SIZE_MAX`. The observer is diagnostic and does not change program output.
Accepted activation records are available through the context's FIFO pending
count and pop APIs; this is a metadata scheduling boundary, not effectful
graph dispatch.

When built in the Flowcore superbuild, `flowtinylower` consumes the public
backend-neutral lowering artifact and deterministically emits ISA v1 for
provider-free scalar programs and ISA 2 for argument or graph artifacts. Its first
admitted slices cover empty programs plus provider-free typed literals,
conversions, unary/binary arithmetic, comparisons, local definitions,
assignments, structured branches, loops and returns. Other valid plans receive
a structured unsupported result until their lowering rules land.
`--diagnostics json` reports malformed contracts, invalid or unavailable input,
output publication failure, resource exhaustion, runtime failure, and unknown
non-standard failure as distinct staged `no_artifact` records on standard
error. A failure of the post-rename parent-directory durability barrier instead
reports `artifact_published_durability_uncertain`, because the new artifact is
already visible and must not be described as absent. Successful and explicitly
unsupported lowering results remain on standard output.
Version 2 artifacts are encoded before publication, written and synchronized
through a private sibling file, and atomically renamed over the destination
only after a successful close. The parent directory is opened before
publication and synchronized after rename. Pre-rename failure preserves any
previous destination and removes the temporary file during ordinary failure
handling; post-rename directory synchronization failure returns the explicit
`TINYVM_ARTIFACT_WRITE_DURABILITY_UNCERTAIN` result. Abrupt-death orphan
cleanup, adversarial parent-directory replacement, and cross-platform
durability are not claimed.
The retained recovered-VM v1 compatibility writer exposes the same publication
sequence and three-state parent-directory durability result through
`tinyvm_artifact_write_result`; its boolean entry point likewise reports
success only after the directory barrier.

The current Gate 6 slice additionally admits executable source graphs with one
authorized startup provider and serial fresh receiver activations. Receiver
pipelines and fan-out are specialized from graph schedule v1 and compared
differentially with LLVM. The bounded finite scalar stream template is also
admitted through exact typed count/item provider thunks; its current delivery
shapes include direct root-to-receiver fan-out with shared source signal
identity and distinct delivery identities per receiver, plus one type-continuous
acyclic receiver pipeline. Persistent scalar
state is admitted through a typed state slot. Pure independent parallel waves
are admitted as a deterministic serial projection; effectful/nested parallel,
branching/merging stream graphs and larger/non-packed aggregate graph contracts
remain explicit unsupported results. Packed, no-padding verified integer
aggregates (`c_int`, `c_long`, `c_ulong` or `c_size_t`) up to 8 bytes are
carried as typed 64-bit payloads; the dedicated `c_long` differential test
also covers provider return and aggregate-parameter calls. These boundaries are not silently converted
to a different contract.

The first governed runtime-provider slice admits only the exact pure tuples
`libc.so.6:abs(c_int)->c_int` and
`libc.so.6:strlen(c_string)->c_size_t`. `flowtinyrun` requires an explicit
active policy file for import-bearing artifacts, matches the complete tuple,
loads the named library only after that check, and dispatches through typed
thunks. A missing or non-matching policy fails closed. Argument and artifact
string handles are resolved inside the thunk and never serialized as pointers.

The readonly slice additionally provides exact zero-argument typed thunks for
`getpid`, `getuid`, `getgid`, `geteuid`, `getegid`, `getppid`, and `getpgrp`
under their declared `kernel`/`linux` contracts. Legacy four-field Flowbind
policy grants are accepted only for zero-argument imports; the artifact still
carries and the thunk still checks the result carrier and complete authority.

The first observable I/O thunk is the exact
`libc.so.6:puts(c_string)->c_int` tuple under `libc`, C, `io`. Program stdout is
kept distinct from the final structured execution-record line and is compared
byte-for-byte with LLVM in the parity suite.

```sh
flowtinyrun --policy active.policy program.tvm
```
