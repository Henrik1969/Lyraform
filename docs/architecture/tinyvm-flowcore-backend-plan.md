# TinyVM as a Flowcore backend

**Status:** active maturation plan
**Date:** 2026-09-13
**Scope:** parity with the currently admitted LLVM backend surface, followed by
shared expansion  
**Recovered baseline:** `tinyvm-recovered-poc-2026-08-26`  
**Benchmark baseline:** `tinyvm-dispatch-baseline-2026-08-26`

## Outcome

TinyVM becomes a governed Flowcore lowering and runtime provider:

```text
Flow source
  -> [frontend bundle file]
  -> [semantic report file]
  -> [binding authorization file]
  -> [execution plan file]
  -> [optimization report file]
  -> [backend-neutral lowering artifact file]
       |-> flowlower-llvm  -> [LLVM IR file]  -> native executable
       `-> flowlower-tinyvm -> [TinyVM file] -> flowtinyvalidate -> flowtinyrun
```

Canonical Flow/Graph meaning remains above both backends. TinyVM bytecode is a
versioned executable projection. GNU computed goto is one runtime engine for
that projection, not part of its semantics. A portable `switch` engine remains
the reference and fallback implementation.

“LLVM parity” initially means the behavior currently accepted by Flowlower:

- integer, boolean, string, pointer and declared pointer-like ABI carriers;
- literal, identifier, list-length/list-index, conversion, unary and binary
  expressions;
- value definitions, assignments, branches, loops and integer returns;
- checked process arguments;
- exactly authorized external calls and typed result placement;
- writable-storage compatibility behavior and cleanup laws;
- explicit target selection, artifact identity and source provenance;
- deterministic refusal of malformed, unauthorized or unsupported input.

It does not mean implementing LLVM's full language, optimizer, platform, ABI,
debugger or object-file surface.

## Binding laws

1. No Flowmini syntax, application name, fixture name or profile name may
   select TinyVM behavior.
2. LLVM and TinyVM consume the same typed lowering model and binding facts.
3. Provider discovery or a host symbol address is evidence, not authorization.
4. Portable artifacts contain no raw host pointer or computed-label address.
5. Every opcode has typed operands, validation rules, failure disposition and
   provenance behavior.
6. Malformed artifacts fail before execution; runtime faults are explicit and
   deterministic.
7. The switch engine defines executable reference behavior. Computed goto must
   remain differentially equivalent.
8. LLVM remains the conservative fallback until a parity gate explicitly
   admits TinyVM for the requested artifact and target.
9. Every arrow is a durable, versioned artifact boundary. A downstream tool
   must operate from the upstream file without the upstream executable,
   process, private headers or in-memory objects being present.
10. Every stage is independently invocable and replayable. Files may be passed
    through stdin/stdout for convenience, but their complete serialized form is
    the contract and must be persistable, inspectable and independently
    validated.
11. Shared libraries may implement public contract parsing and value types;
    they must not collapse producer and consumer into one authority boundary or
    expose private producer state as required input.

## Separate-tool checkpoint discipline

TinyVM integration preserves the existing Flowcore chain architecture. The
minimum eventual tool boundary is:

```text
flowlower-tinyvm
  input:  validated optimization/lowering and binding artifacts
  output: flowcore.tinyvm_artifact file

flowtinyvalidate
  input:  flowcore.tinyvm_artifact file
  output: structured validation report and optional canonical artifact

flowtinyrun
  input:  independently validated or internally revalidated TinyVM artifact
  output: structured execution record plus declared program outputs
```

`flowtinyrun` must still validate untrusted input itself; a prior validator
report is evidence, not permission to skip validation. The emitter must not be
linked into the runner, and the runner must not need Flowmini, Flowanalyst,
Flowbind, Flowparallel, Flowoptimize or Flowlower installed. A captured artifact
and its declared runtime providers are sufficient to reproduce execution.

Tests must exercise every consumer in two modes:

1. piped directly from the previous stage; and
2. started independently against a previously captured file after the producer
   process has exited.

The file boundary is a deliberate checkpoint and audit surface, not temporary
serialization between components that secretly depend on one another.

## Gate 0 — freeze and test the recovered baseline

- Preserve the reconstructed runner and its provenance as the historical
  baseline.
- Add unit tests for every implemented opcode, invalid opcode/register/address,
  division by zero, stack bounds, jumps, missing halt and unsupported context
  operations.
- Make computed-goto, switch and function-pointer engines execute the same
  conformance vectors and produce the same final state or fault.
- Add repeated benchmark samples, randomized engine order, medians and spread;
  record CPU affinity and optimization/LTO settings.

**Exit:** all engines are semantically equivalent on the recovered ISA and the
benchmark can detect regressions without being treated as a correctness gate.

## Gate 1 — define the executable artifact contract

Specify `flowcore.tinyvm_artifact` version 1 with deterministic serialization:

- magic, format version, ISA version, endianness and word representation;
- artifact, source, target, lowering-plan and optimization identities;
- code, constants, strings, writable storage and entry-point sections;
- typed slot/register requirements and declared resource limits;
- external capability import table using contract/evidence identities;
- operation-to-instruction and instruction-to-source provenance maps;
- optional instrumentation metadata derived from the historical `pad` idea;
- content digest and explicit unknown-section/field policy.

Add a public typed reader/writer and independent validator. Validate complete
input, duplicate identities, section bounds, opcode/operand legality, jump
targets, stack/resource declarations, import references and provenance links.
Never serialize computed-label or process pointer values.

**Exit:** canonical round-trip is byte deterministic; every single-field
authority mutation is rejected; the runtime only accepts validated artifacts.

## Gate 2 — make the ISA capable of carrying Flow values

Version the recovered opcode set rather than silently changing it. Define a
new ISA revision with:

- explicit `i1`, `i32`, `i64`, pointer-like handle and void carriers;
- typed slots or virtual registers independent of the eight physical scratch
  registers;
- constant/string loading and deterministic sign extension/truncation;
- comparison results as values, not only implicit flags;
- validated local memory and writable-storage handles;
- block labels, conditional/unconditional branches, call, return and halt;
- explicit trap/outcome codes for arithmetic, bounds, import and resource
  failures;
- reserved instrumentation fields that do not affect program semantics.

Use opaque handles for artifact-visible pointers. Resolution into a host
address occurs only inside an authorized runtime provider.

**Exit:** a hand-built artifact can express every internal value, expression
and control-flow form currently emitted by the generic LLVM lowerer.

## Gate 3 — extract one backend-neutral lowering model

- Publish Flowlower's validated structured-plan semantics as a deterministic,
  versioned backend-neutral lowering artifact. Parsing, typed expressions,
  blocks, operations, carriers, provider tuples and authorization facts must
  cross this file boundary rather than an in-memory producer object.
- Keep artifact-contract parsing in `Flowcontracts`; do not duplicate JSON
  interpretation in TinyVM.
- Give independently invocable LLVM and TinyVM tools consumers for the same
  public artifact contract. They may share its public parser/value library but
  neither may depend on the process or private implementation that emitted it.
- Preserve operation, block, symbol, provider, source and derivation identity
  in both emission reports.
- Reject an operation before backend selection if its semantics are unresolved.

**Exit:** LLVM output remains green and a skeleton TinyVM emitter can consume a
captured lowering artifact in a fresh process without any earlier compiler
stage present. The file round-trips deterministically and is independently
validated before either backend uses it.

## Gate 4 — internal computation parity

Lower, in small vertical slices:

1. empty program and explicit integer return;
2. integer/boolean/string constants and symbol placement;
3. unary operations and integer conversions;
4. arithmetic and comparisons;
5. assignments and mutable slots;
6. branch/join control flow;
7. loops and bounded backward edges;
8. checked argument count and indexed argument access;
9. writable-storage compatibility values.

For each slice, execute the same Flow source through LLVM and TinyVM and compare
exit status, stdout/stderr, declared outputs, faults and preserved identities.

**Exit:** all provider-free programs accepted by current generic LLVM lowering
have differential TinyVM parity.

## Gate 5 — governed external calls

Define a TinyVM import/call boundary rather than embedding arbitrary native
addresses:

- artifact imports identify the exact contract, library, convention, symbol,
  parameter/result carriers, effects, resources and authorization evidence;
- loading revalidates the import against the active policy and installed
  provider evidence;
- runtime call slots are resolved only after authorization and are not part of
  the portable artifact;
- the first bridge supports the current admitted C carriers (`c_int`,
  `c_long`, `c_ulong`, `c_size_t`, `c_string`, `c_pointer` and declared
  pointer-like carriers);
- generated typed thunks are preferred over an unrestricted generic FFI;
- resource acquisition, cleanup, failure outcome and terminal disposition
  remain explicit in bytecode and runtime traces.

Start with pure calls (`abs`, `strlen`), then readonly identity calls, output,
filesystem/resource operations, and finally the existing ncurses and TUI
acceptance programs.

**Exit:** every external-call fixture currently admitted through LLVM either
executes equivalently through TinyVM or receives a structured, documented
unsupported result. No authorized tuple can be substituted by symbol name.

## Gate 6 — functions, graphs and runtime activation

- Define program/library entry points, parameter/result placement and a bounded
  call-frame model.
- Lower selected graph roots without changing graph connection semantics.
- Keep wire/signal/port identity in runtime activation records.
- Add explicit scheduling hooks without encoding scheduling policy into the
  bytecode's semantic meaning.
- Revisit the recovered `CTX_*` family only after context snapshot, commit,
  abort, isolation and failure laws are specified. Do not infer these semantics
  from their historical names.

**Exit:** reusable functions and admitted graph regions run through TinyVM with
the same observable contracts as the reference CPU/LLVM path.

### Current admitted slice — 2026-09-13

`flowtinylower` now admits a bounded serial source-graph projection when the
backend-neutral artifact carries graph schedule v1
(`fifo_per_root_source_order_v1`, `fresh_single_input_v1`). The slice requires
one exactly authorized `startup_once` provider and invokes the existing typed
callable lowering for each fresh one-input receiver activation. Static
pipelines and fan-out are covered by the schedule, and the resulting TinyVM
artifact is deterministic and differentially checked against LLVM. Finite
scalar or verified one-64-bit aggregate stream schedule v2 is also admitted through a typed count/item provider
bridge and a bounded TinyVM loop. Verified aggregate items may also traverse the
existing linear schedule-v5 pipeline; both paths have dedicated differential
coverage. The admitted delivery shape is direct
root-to-receiver fan-out: each stream item is delivered independently to every
root-connected receiver with the shared source signal and distinct delivery
identity. A versioned linear stream-pipeline schedule v5 is also admitted:
one root delivery may traverse an acyclic chain of type-continuous fresh
receivers, carrying each receiver result into the next activation. Persistent
scalar and verified one-64-bit aggregate schedule v3 are admitted through a
typed mutable state slot updated between receiver activations.
Pure parallel schedule v4 is admitted as a deterministic serial projection of
validated dependency waves, since pure independent activations have no
observable ordering contract in this slice.
Verified packed, no-padding aggregate layouts containing `c_int`, `c_long`,
`c_ulong` or `c_size_t` fields and occupying at most 8 bytes are admitted as
typed 64-bit payloads for provider returns and aggregate-parameter calls. The
`c_long` path is covered by a dedicated LLVM/TinyVM differential test.

This remains a bounded graph specialization, not yet a general TinyVM
scheduler: ISA 2 graph artifacts now carry validated wire, signal, port and
delivery identity in optional activation metadata and expose it through the
`--trace-graph` runtime observer; the ISA context retains copied records with
execution sequence and stream index. The observer is diagnostic; it does not yet
provide effectful scheduling, reentrancy, cancellation or runtime-owned
activation queues; an embedding may install the explicit schedule hook to
refuse an activation before observation and may impose a deterministic limit on
runtime-owned activation records. Effectful or nested-call parallel activations,
branching/merging stream pipelines, larger/non-packed aggregate layouts and
external aggregate-to-aggregate calls remain explicit unsupported boundaries
until their runtime delivery and aggregate contracts are implemented. Gate 7
coverage now also proves that mutated aggregate evidence, graph scheduling and
exact runtime policy tuples are refused without a partial TinyVM artifact or
provider call.

## Gate 7 — parity corpus and adversarial proof

Build a single backend-neutral acceptance corpus containing:

- values, expressions, conversions and assignments;
- results, branches, loops, checked arguments and explicit return;
- an authorized fallible/resource-bearing external operation and cleanup;
- multiple named targets;
- source, operation, block, provider, ABI/effect/resource and provenance
  identities.

For every fixture:

- compare LLVM and TinyVM observable behavior;
- compare computed-goto and switch runtime behavior;
- mutate each authority/identity category independently and prove refusal;
- cover truncation, duplicate keys/sections/identities, malformed jumps,
  invalid carriers, missing imports, resource-limit violations and corrupted
  provenance;
- run normal, ASan/UBSan and deterministic round-trip suites.

**Exit:** current LLVM-supported Flowcore behavior has documented TinyVM parity,
with no silent fallback or semantic discrepancy.

## Gate 8 — provider selection, packaging and fallback

- Publish TinyVM as an explicit lowering/runtime provider capability.
- Extend target policy so `llvm` and `tinyvm` are deliberate selections.
- Allow portable artifacts to carry TinyVM code plus a conservative native or
  serial fallback as described by ADR 0050.
- Record source artifact, plan revision, runtime capability snapshot, policy
  revision, provider, fallback and emitted-artifact digest.
- Package the portable switch runtime by default; offer computed-goto as a
  GCC/Clang optimized build where supported.
- Never change backend because one fails at runtime without an attributable
  policy-approved fallback decision.

**Exit:** installed Flowcore can select, validate, execute and explain the
TinyVM backend while preserving LLVM fallback.

## Gate 9 — optimization after parity

Only after semantic parity:

- superinstructions and opcode fusion with derivation provenance;
- register/slot allocation;
- constant pooling and block layout;
- direct-threaded tail dispatch;
- optional runtime specialization/JIT;
- Flowparallel region placement and safe concurrent contexts;
- profile-specific resource and boundedness admission.

Every optimization emits a derived artifact, passes switch/computed/LLVM
differential tests, and retains a non-specialized valid fallback.

## Checkpoint sequence

Use small buildable checkpoints rather than one long backend rewrite:

1. `tinyvm-isa-conformance`
2. `tinyvm-artifact-contract-v1`
3. `tinyvm-typed-values-control`
4. `flowlower-backend-neutral-model`
5. `tinyvm-internal-parity`
6. `tinyvm-authorized-hostcalls`
7. `tinyvm-graph-runtime-parity`
8. `tinyvm-backend-parity`
9. `tinyvm-provider-packaging`

At every checkpoint run focused tests, `git diff --check`, the complete
canonical Flowcore suite when shared contracts change, sanitizer tests, and
the relevant LLVM/TinyVM differential corpus. Commit and annotate the exact
tested boundary; do not tag a partially failing state.

Every checkpoint must also retain captured positive and adversarial boundary
artifacts sufficient to test the next consumer without running its producer.

## Deliberately deferred decisions

- Permanent writable-storage syntax remains outside this backend plan.
- The exact canonical Graph IR shape is not selected here.
- `CTX_*` semantics are not reconstructed from incomplete historical evidence.
- General arbitrary native ABI/FFI, exceptions, garbage collection, coroutines,
  distributed execution and safety certification are not implied by parity
  with today's LLVM backend.

The recovered implementation began with Gate 0: complete ISA conformance and
engine differential tests. The current graph checkpoint above is the first
Gate 6 implementation slice; it preserves the same refusal-first discipline
while expanding only the admitted serial contract.
