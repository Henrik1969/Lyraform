# TinyVM, cross-target and bootstrap maturation ledger

## Mission baseline — 2026-08-26

- Branch: `v25-symboltable-projection`
- Prior mission: v0.28 typed artifact contracts complete and pushed.
- New authority: `docs/tasks/tinyvm-cross-target-bootstrap.md`.
- Run state returned from `DONE` to `CONTINUE` for the new mission.
- Historical TinyVM source was absent; reconstruction provenance is documented
  under `subprojects/TinyVM`.

## Completed precursor checkpoints

### Recovered prototype — `d3bd592`

- Restored a buildable recovered-ISA C17 runner using GNU computed goto.
- Kept it isolated from canonical Graph IR and the installed Flowcore graph.
- Tag: `tinyvm-recovered-poc-2026-08-26`.

### Dispatch baseline — `9be3635`

- Added computed-goto, switch and function-pointer engines and comparative
  workloads.
- GCC 13 Release+LTO local observation favored computed goto across the three
  captured workloads; results remain non-universal exploratory evidence.
- Tag: `tinyvm-dispatch-baseline-2026-08-26`.

### Recovered ISA conformance — `614f045`

- Added 24 semantic/fault cases across all three dispatch engines.
- Covered all 27 recovered opcode identities; `CTX_*` consistently returns an
  explicit unsupported fault rather than invented historical semantics.
- GCC 13, Clang 18 and GCC ASan/UBSan suites passed. Leak detection was disabled
  because the managed traced host cannot run LeakSanitizer.
- Tag: `tinyvm-isa-conformance`.

### Artifact envelope slice — `73ba557`

- Added deterministic `flowcore.tinyvm_artifact` envelope format 1 carrying
  recovered ISA version 0.
- Added SHA-256 integrity and strict header, identity, size, resource, opcode,
  register, jump, reserved-field and terminal-HALT validation.
- Added separate `flowtinyvalidate` and `flowtinyrun` processes plus captured
  file, corruption, truncation and trailing-byte tests.
- Normal and GCC ASan/UBSan suites passed.
- This is an intermediate envelope, not the complete Flow-capable artifact.
- Tag: `tinyvm-artifact-envelope-v1`.

## Current work

Gate 1 is active: extend the executable artifact authority for typed constants,
strings, storage, imports and instruction/source provenance without weakening
the already captured recovered-ISA boundary.

### Sectioned artifact authority — `tinyvm-artifact-sections-v2`

- Specified format 2 without reinterpreting format 1 reserved bytes.
- Added ordered code, typed-constant, string, storage, authorized-import and
  per-instruction provenance sections.
- Added canonical alignment/padding, exact coverage, unique/sorted identities,
  carrier/storage/import/provenance validation and deterministic byte round-trip.
- `flowtinyvalidate` now independently identifies and validates formats 1 and 2.
- `flowtinyrun` revalidates format 2 and refuses declared imports until an
  authorized runtime resolver exists.
- Hostile tests recompute a valid SHA-256 after mutating identity padding,
  section uniqueness, boolean constants, import reserved bytes, provenance and
  alignment padding; every mutation is rejected structurally.
- GCC 13 and Clang 18 Debug suites passed 7/7 tests.
- GCC 13 ASan/UBSan passed 7/7 with leak detection disabled for the managed
  traced-host limitation.
- Format 2 currently admits recovered ISA 0. ISA 1 is explicitly rejected until
  its typed execution semantics and cross-section references are implemented.

### Flow-capable ISA v1 — `tinyvm-flow-isa-v1`

- Defined a separate ISA version rather than changing recovered ISA 0.
- Added typed virtual slots for `i1`, `i32`, `i64` and opaque handles.
- Added constant definition, move, conversion, modular arithmetic, signed
  division, six comparisons, absolute jump, conditional branch, return, trap,
  halt, string/storage handle and authorized-import call opcodes.
- Opaque handles encode stable tagged identities and never host addresses.
- `ADD`, `SUB` and `MUL` use modulo-width semantics to match the current LLVM
  lowerer's plain operations. Division faults are explicit rather than relying
  on C undefined behavior. Checked arithmetic remains a future distinct
  semantic operation.
- Added step-limit, uninitialized-slot, type, division and unresolved-import
  traps.
- Added portable switch and GNU computed-goto engines. Conformance compares
  complete slot state, carriers, bits, PC, step count, result and traps.
- Added independent file emission, validation and execution proving typed
  `i64(42)` with the producer absent.
- Format 2 now validates ISA 1 constant/string/storage/import references,
  virtual-slot ranges, import argument spans, control targets and terminal
  behavior.
- GCC 13 and Clang 18 passed 9/9 tests. GCC ASan/UBSan passed 9/9 with the
  documented LeakSanitizer exclusion.
- Runtime provider resolution for `CALL_IMPORT` remains Gate 5; both engines
  explicitly trap and the command-line runner refuses unresolved imports.

### Backend-neutral lowering artifact — `backend-neutral-lowering-v1`

- Added independently invocable `flowprepare`, which consumes captured
  optimization and optional binding files and emits canonical
  `flowcore.backend_lowering_artifact` version 1 JSON.
- The artifact preserves source, complete target catalog and selected target,
  ABI contracts, external operations, operation/block/symbol identities, exact
  authorization capabilities and optimization-transform provenance.
- Public validation rejects target drift, duplicate authorization identities
  and any difference between authorized tuples and external-call tuples.
- LLVM `flowlower` and TinyVM `flowtinylower` consume the same captured file;
  replay tests invoke the producer neither in-process nor as a binary.
- TinyVM deterministically lowers the first provider-free empty program to ISA
  v1 returning typed `i32(0)`. It reports structured `unsupported` for valid
  non-empty plans pending Gate 4 rather than falsely admitting them.
- Direct optimization-report input to LLVM remains a documented temporary
  corpus compatibility path, not the public backend boundary.
- The complete GCC superbuild and all 68 canonical tests passed. Focused Clang
  18 ASan/UBSan builds and both backend-boundary tests passed with leak
  detection disabled for the managed traced-host limitation.

### Provider-free scalar parity — `tinyvm-provider-free-scalar-parity`

- TinyVM lowering now admits typed integer and Boolean literals, identifiers,
  i32/i64 conversions, unary negation, modular arithmetic, signed division,
  comparisons, local definitions, assignment, structured branches, loops and
  typed returns.
- Symbol storage and expression temporaries lower to distinct virtual slots;
  control references resolve to artifact-local instruction indexes.
- Every emitted instruction retains source, lowering-plan, operation, block and
  symbol derivation. Compiler-generated control/return operations use explicit
  reserved provenance identities rather than pretending to be source nodes.
- Differential tests run the same public backend-neutral files through LLVM
  and TinyVM for empty, literal-return, expression, local-symbol, comparison
  branch and integer-loop programs and compare observable return values.
- Unsupported operation and expression kinds still produce structured
  `unsupported` results without creating a partial executable artifact.
- The complete GCC superbuild and all 69 canonical tests passed. Focused Clang
  18 ASan/UBSan builds and the boundary/parity tests passed with the documented
  LeakSanitizer exclusion.

### Provider-free execution inputs and opaque values — `tinyvm-provider-free-parity`

- Added typed ISA 2 as a versioned execution-input extension; ISA 1 bytecode
  meaning remains frozen.
- ISA 2 slot 0 receives process-style argument count and subsequent reserved
  slots receive tagged opaque argument identities, never host pointers or
  serialized argument bytes.
- Lowering selects ISA 2 only for argument intrinsics, calculates the required
  static index span and emits a checked count guard returning `i32(64)` before
  access. Dynamic indexing remains explicitly unsupported.
- No-argument and one-argument executions of the current LLVM argument-count
  fixture are differentially equal through TinyVM.
- Switch and computed-goto engines compare complete ISA 2 execution state,
  including input metadata and opaque argument slots.
- String constants and writable-storage compatibility declarations now lower
  into validated artifact sections and tagged opaque handles. Host resolution
  remains governed provider work in Gate 5.
- The complete GCC superbuild and all 69 canonical tests passed. Focused Clang
  18 ASan/UBSan conformance, boundary and parity tests passed with the
  documented LeakSanitizer exclusion.

### Governed pure calls — `tinyvm-governed-pure-calls`

- TinyVM lowering admits only two exact initial import tuples:
  `libc.so.6:abs(c_int)->c_int` and
  `libc.so.6:strlen(c_string)->c_size_t`, both under contract `libc`, C calling
  convention and `pure` effect.
- The executable import section preserves contract, library, symbol,
  convention, effect, parameter/result carriers and stable authorization
  evidence identity.
- `flowtinyrun` refuses import-bearing artifacts without an explicit active
  policy and rechecks the complete policy tuple before library loading.
- Runtime dispatch uses named typed thunks, not a generic FFI. Host symbol
  addresses exist only after policy admission and are never serialized.
- Artifact string and process-argument handles resolve inside the typed string
  thunk. Temporary NUL termination is provider-local and bounded.
- LLVM/TinyVM differential tests prove `abs(-42) == 42` and
  `strlen("Flowcore") == 8`; missing and effect-mismatched policies fail closed.
- Switch and computed-goto engines differentially execute the same resolved
  import callback and complete slot/result state.
- The complete GCC superbuild and all 70 canonical tests passed. Focused Clang
  18 ASan/UBSan conformance and both parity suites passed with the documented
  LeakSanitizer exclusion.

### Governed readonly identity calls — `tinyvm-governed-readonly-calls`

- Added exact typed zero-argument thunks for `getpid`, `getuid`, `getgid`,
  `geteuid`, `getegid`, `getppid` and `getpgrp` under their declared
  `kernel`/`linux`, `libc.so.6`, C, `readonly`, `()->c_int` authorities.
- Empty parameter lists serialize canonically as `none`; the ISA validator and
  runtime agree that they consume zero argument slots.
- Existing four-field Flowbind policy grants remain accepted only for
  zero-argument imports. Typed result and contract admission are still checked
  by the artifact and named thunk, so this compatibility does not become a
  generic symbol grant.
- Differential executions compare normalized process exit results. Parent-ID
  execution is invoked without a command-substitution intermediary so LLVM and
  TinyVM observe the same parent process.
- The complete GCC superbuild and all 70 canonical tests passed. Focused Clang
  18 ASan/UBSan provider, scalar and ISA conformance suites passed with the
  documented LeakSanitizer exclusion.

### Governed output call — `tinyvm-governed-output-call`

- Added the exact typed `libc.so.6:puts(c_string)->c_int` thunk under contract
  `libc`, C calling convention and `io` effect.
- The same artifact-string handle used by `strlen` resolves through the
  provider-local bounded NUL-terminated view; no pointer enters the artifact.
- Parity now runs a plan containing `strlen`, `abs` and `puts` together and
  compares program stdout byte-for-byte separately from the structured TinyVM
  execution record.
- Missing and mismatched active policy checks remain applied to the I/O fixture
  as well as pure and readonly fixtures.
- The complete GCC superbuild and all 70 canonical tests passed. Focused Clang
  18 ASan/UBSan governed-provider and ISA conformance tests passed with the
  documented LeakSanitizer exclusion.

### Current governed-provider inventory — `tinyvm-governed-provider-inventory`

- Added exact differential parity for `labs`, `getpgid`, `getsid`,
  `getpriority` and checked indexed argument access through `puts`.
- Runtime policy is preflighted for every import before the first instruction,
  so an unreachable call cannot bypass authorization.
- Published `tinyvm-current-llvm-parity-inventory.md`, separating every
  executed tuple from readonly-output, filesystem, IPC, loopback, namespace,
  memory, ncurses and aggregate tuples that still lack safe runtime laws.
- A validated authorized `fork` plan proves structured unsupported exit 2 and
  proves that no partial executable is created.
- Legacy ISA 0 payloads inside the v2 artifact envelope remain executable;
  governed import preflight applies to ISA 1/2 artifacts with imports and does
  not silently change the frozen recovered format's behavior.
- The complete GCC superbuild and all 70 canonical tests passed. Focused Clang
  18 ASan/UBSan governed-provider, scalar, artifact-boundary and ISA
  conformance tests passed with the documented LeakSanitizer exclusion.

### Target-policy artifact contract — `target-policy-artifact-v1`

- Added the independently validated `flowcore.target_policy` version 1 file
  boundary covering backend, architecture, ABI, capabilities, resources,
  lifecycle, evidence and fallback.
- Added canonical named policies for `llvm-host` and `tinyvm-portable`; both
  explicitly prohibit fallback.
- Added `flowtarget`, which resolves only an exact safe target name beneath an
  explicit fixed policy root, validates the file, checks internal/requested
  identity equality and emits canonical JSON.
- Boundary tests prove canonical round trips and refusal of missing, path-like
  and identity-mismatched policy requests without invoking either backend.
- The complete GCC superbuild and all 71 canonical tests passed. Focused Clang
  18 ASan/UBSan contract, validator and target-policy boundary tests passed
  with the documented LeakSanitizer exclusion.

### Target-policy lowering admission — `target-policy-cross-compile-v1`

- Added backend-lowering artifact version 2, which captures the complete
  independently resolved target policy. Historical version 1 artifacts remain
  valid and replayable.
- `flowprepare` keeps source named-target projection separate from backend
  target policy and never resolves or substitutes a backend itself.
- LLVM and TinyVM consumers independently admit exact backend, artifact format,
  architecture, ABI and required provider capabilities before emission.
- A single cross-compilation test keeps source, optimization artifact, resolver,
  policy root and preparation/lowering commands fixed while changing only
  `llvm-host` to `tinyvm-portable`.
- Wrong-backend, wrong-ABI and unavailable-provider cases return structured
  `unsupported` exit 2 and prove that no partial output artifact exists.
- The complete GCC superbuild and all 72 canonical tests passed. Focused Clang
  18 ASan/UBSan resolver, preparation, LLVM/TinyVM admission and captured-file
  boundary tests passed with the documented LeakSanitizer exclusion.

### Stage 0 host and language closure inventory — `stage0-language-closure-inventory`

- Declared the exact current host-language, standard-library, provider and tool
  privileges for every compiler-chain component and assigned each gap to
  language, library, provider, tooling or application ownership.
- Added deterministic `flowcore.bootstrap_seed` capture containing the tracked
  seed digest, revision, compiler/build tool evidence, language standards and
  provider surface, with deterministic and environmental fields separated.
- Identified the first shared compiler/document slice as an ordinary pure
  scalar classifier and preserved the actual opening gap: public lowering does
  not yet describe complete callable functions, and TinyVM cannot execute the
  existing `fn_demo.flow` plan.
- The complete GCC superbuild and all 73 canonical tests passed. Focused Clang
  18 ASan/UBSan bootstrap-contract, validator and deterministic-capture tests
  passed with the documented LeakSanitizer exclusion.

### Non-empty target-policy replay correction — `target-policy-nonempty-replay`

- Corrected the LLVM structured-plan loader to populate backend-lowering
  artifact version 2 rather than treating it as an empty non-applicable plan.
- Strengthened name-only cross-target proof with a nonzero return plan: LLVM
  and TinyVM both execute result 37 from their independently admitted v2 files.
- The test now prevents an empty source fixture from masking loss of lowering
  operations at the target-policy boundary.
- The complete GCC superbuild and all 73 canonical tests passed. The strengthened
  cross-target execution test passed under Clang 18 ASan/UBSan with the
  documented LeakSanitizer exclusion.

### Callable lowering plan v2 — `callable-lowering-plan-v2`

- Added a complete function catalog with stable identities, parameter types,
  body roots, return types and exactly one entry to the backend-neutral plan.
- Every lowering operation now names its owning function; ordinary calls and
  nested call expressions preserve exact callee and typed argument authority.
- Flowparallel and Flowoptimize pass the complete file forward without needing
  Flowanalyst or frontend internals, and `flowprepare` captures it unchanged.
- Public validation and mutation tests reject duplicate function identity,
  unknown operation ownership and unknown callees before either backend is
  invoked; named-target entry candidates remain explicit until selection.
- Flowanalyst exposes v2 through explicit `--lowering-plan-version 2` during
  staged consumer migration; v1 remains the default until both backends pass
  callable parity, avoiding a flag-day break of historical captured pipelines.
- The complete GCC superbuild and all 74 canonical tests passed. Focused Clang
  18 ASan/UBSan frontend, analyst, callable boundary and public-validator tests
  passed with the documented LeakSanitizer exclusion.

### LLVM callable consumer — `llvm-callable-lowering-v2`

- LLVM lowering now consumes callable plan v2 as distinct function definitions
  with stable parameter-slot binding, exact ordinary callee identities and
  expression-keyed call results; v1 lowering remains unchanged.
- Added the first shared compiler/document probe: an ordinary pure scalar
  classifier called through tokenizer-shaped and headless-document-shaped
  functions without compiler intrinsics or source-name dispatch.
- The captured v2 plan is prepared under `llvm-host`, emitted, compiled and
  executed with observable result 1.
- The complete GCC superbuild and all 74 canonical tests passed. The full
  callable v2 capture/validation/LLVM execution slice passed under Clang 18
  ASan/UBSan with the documented LeakSanitizer exclusion.

### TinyVM callable consumer — `tinyvm-callable-lowering-v2`

- TinyVM lowering consumes the same callable v2 catalog through deterministic
  non-recursive call specialization, typed parameter slots, expression-keyed
  results and explicit return-to-continuation jumps.
- Callable artifacts remain host-address-free and byte-deterministic across two
  lowerings from the same captured input.
- The shared tokenizer/document classifier executes with result 1 under both
  `llvm-host` and `tinyvm-portable`, selected only by target-policy file.
- `flowtinyrun --engine switch|computed` exposes both governed engines for file
  artifacts; the callable probe produces identical complete result under each.
- The complete GCC superbuild and all 74 canonical tests passed. The full
  cross-backend callable slice and switch/computed ISA conformance passed under
  Clang 18 ASan/UBSan with the documented LeakSanitizer exclusion.

### Precise remaining bootstrap inventory — `bootstrap-gap-inventory-v1`

- Added a canonical, publicly validated bootstrap gap graph separating proven
  foundations from language, library, compiler-kit, package, resource and
  staged-bootstrap work.
- Every remaining gap has a stable identity, owner, gate, dependency set and
  executable acceptance statement; dependencies must resolve to another gap or
  a completed foundation.
- Stage 1, 2 and 3 name their actual producer, current state and missing
  evidence. None is reported started or complete merely because the shared
  scalar compiler/document slice now executes.
- The complete GCC superbuild and all 75 canonical tests passed. Focused Clang
  18 ASan/UBSan seed/inventory contracts, adversarial mutations and public
  validator tests passed with the documented LeakSanitizer exclusion.

### Mission closure — `tinyvm-cross-target-bootstrap-complete`

- The mission definition of done is satisfied and summarized in
  `2026-08-26-tinyvm-cross-target-bootstrap-result.md`.
- Closure preserves explicit non-claims: complete self-hosting, Stage 1 and
  FlowOpenOffice remain future gates described by the validated gap inventory.

## Gate 6 reactivation — 2026-09-13

### Serial source-graph parity — `tinyvm-serial-graph-runtime-parity-v1`

- TinyVM now consumes the executable source graph and its independently
  validated graph schedule v1 from the backend-neutral artifact.
- The admitted contract is one exactly authorized `startup_once` provider plus
  fresh one-input source-function receivers. Static receiver pipelines and
  fan-out execute through the existing typed callable compiler, preserving
  provider authorization and deterministic artifact identity.
- LLVM and TinyVM differential parity covers observable graph output and result;
  TinyVM lowering is deterministic across repeated lowering of the same file.
- A valid parallel schedule v4 is rejected with a structured `unsupported`
  result and no partial TinyVM artifact. Persistent v3, finite-stream v2 and
  aggregate payload contracts remain future slices.
- This checkpoint does not claim a general TinyVM activation runtime: wire,
  signal, port, queue, persistent-state and parallel-wave identities remain
  outside the current bytecode runtime contract.

### Finite scalar stream parity — `tinyvm-finite-scalar-stream-parity-v1`

- TinyVM now admits the validated graph schedule v2 finite scalar stream
  template: one authorized count provider returning `c_size_t`, one item
  provider accepting `c_size_t`, and bounded fresh receiver deliveries.
- The lowerer emits a checked count bound and a deterministic ISA loop; the
  runtime resolves only the exact typed stream thunks after active policy
  admission.
- LLVM and TinyVM produce identical observable receiver output and result, and
  repeated lowering produces byte-identical TinyVM artifacts.
- Persistent schedule v3, parallel schedule v4, stream receiver pipelines and
  aggregate payloads remain explicit future gates.

### Persistent scalar graph parity — `tinyvm-persistent-scalar-parity-v1`

- TinyVM now admits validated graph schedule v3 with one startup provider and
  repeated fresh deliveries to persistent scalar receivers.
- Each receiver carries a typed `c_long` state slot initialized from the graph
  contract, passed as the second callable parameter, and updated from the
  callable result before the next delivery.
- A two-delivery fixture observes the state transition `5 -> 6` identically
  through LLVM and TinyVM; lowering remains deterministic and artifact-free on
  failure.
- Effectful/nested parallel schedule v4, stream receiver pipelines and aggregate payloads
  remain explicit future gates. The current state model is serial and static;
  it is not yet a general runtime activation-record system.

### Pure parallel graph parity — `tinyvm-pure-parallel-graph-parity-v1`

- TinyVM now admits validated schedule v4 dependency waves when all receiver
  bodies are provider-free/pure and contain no nested calls.
- The bytecode projection executes those independent waves in deterministic
  serial order. This is equivalent for the admitted effect-free contract and
  does not claim a parallel TinyVM runtime or reorder effectful work.
- LLVM and TinyVM both complete the pipeline/fan-out fixture under switch and
  computed engines; effectful parallel graphs remain rejected.

### Verified aggregate payload parity — `tinyvm-verified-aggregate-parity-v1`

- TinyVM now admits verified packed aggregate layouts containing only `c_int`
  fields and occupying at most 8 bytes.
- The current artifact projection carries these values in typed 64-bit slots;
  the runtime uses exact policy-authorized typed aggregate return and parameter
  thunks, without application-name dispatch.
- Provider-return and aggregate-parameter graph calls produce identical
  observable output and result under LLVM and TinyVM, with deterministic
  repeated lowering.
- Larger/non-packed layouts, non-`c_int` fields and aggregate-to-aggregate
  calls remain explicit future boundaries.

### Aggregate authority and evidence refusal proof — `tinyvm-aggregate-boundary-adversarial-v1`

- Mutating aggregate verification status, packed size, field type or graph
  scheduling policy refuses TinyVM lowering; no partial artifact is emitted.
- Schema-invalid evidence produces the existing contract-error refusal, while
  semantically unsupported evidence produces the structured `unsupported`
  lowering result with a reason.
- Removing or changing the exact aggregate provider policy tuple fails runtime
  preflight before the provider is opened or called.
- The aggregate payload fixture now produces the same observable output and
  result through both switch and computed-goto TinyVM engines.
- The provider-free scalar corpus now compares both TinyVM engines with LLVM
  for returns, expressions, branches, loops and dynamic argument length.
- The serial source-graph fixture now compares both TinyVM engines with LLVM
  for startup provider delivery, pipeline evaluation and fan-out output.
- The governed provider corpus now compares both TinyVM engines with LLVM for
  libc, ctype and kernel calls, negative results, and selected arguments.
- Text and bounded-memory parity fixtures now compare both TinyVM engines with
  LLVM for tagged text calls, storage mutation and result propagation.
- A fresh Clang 18 ASan/UBSan TinyVM-focused run exposed and fixed a
  zero-length text copy that passed a null pointer to `memcpy`; the runtime
  text linker now receives the sanitizer flags so instrumented shared runtimes
  load consistently. The corrected sanitizer run passed all 20 TinyVM-focused
  tests.

### TinyVM installed backend surface — `tinyvm-provider-packaging-v1`

- The canonical install now includes `flowtinylower`, `flowtinyvalidate` and
  `flowtinyrun`, the TinyVM public headers, both target-policy JSON files and
  the TinyVM artifact/ISA documentation.
- This keeps `tinyvm-portable` an explicit selectable backend in an installed
  toolchain instead of leaving the runtime only in the source/build tree.
- The staged install-shape test passes for all three TinyVM tools, both target
  policies, the public headers and separated TinyVM documentation; it also
  resolves `tinyvm-portable` and executes an installed empty artifact. No
  fallback is inferred when a target or provider is unavailable.
- The focused aggregate parity test and complete canonical suite passed;
  canonical CTest is now 103/103 green.

### Finite stream fan-out parity — `tinyvm-finite-stream-fanout-parity-v1`

- The TinyVM finite-stream fixture now covers two independent receivers wired
  directly from the stream root, not only the single-receiver case.
- LLVM and TinyVM produce the same six ordered outputs under both switch and
  computed engines for three source items delivered to both receivers.
- The validated schedule proves both receiver activations retain the shared
  source activation/input signal and `$index` carrier while using separate
  delivery identities. This records the current direct fan-out law without
  widening the contract to stream receiver pipelines.

### Linear stream pipeline parity — `tinyvm-finite-stream-pipeline-parity-v1`

- Graph schedule v5 now admits one finite-stream root delivery followed by an
  acyclic, type-continuous chain of fresh receivers. Branching and merging
  stream shapes remain refused by the schedule contract.
- LLVM and TinyVM produce identical output for a three-item pipeline where the
  first receiver transforms each item and the second receiver observes the
  transformed value; switch and computed engines agree as well.
- The schedule records the prior receiver as each next activation's input and
  advances the signal identity, while retaining the per-item `$index` carrier.
- The refreshed Clang 18 ASan/UBSan TinyVM-focused run passed all 21 tests,
  including the new pipeline path.
