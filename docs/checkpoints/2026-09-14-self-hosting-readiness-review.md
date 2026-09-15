# Lyraform Stage 1 self-hosting readiness review

**Date:** 2026-09-14  
**Status:** not ready; review recorded before any Stage 1 implementation  
**Authority:** `docs/tasks/pre-self-hosting-safety-assurance.md`,
`docs/architecture/product-scale-and-self-hosting-plan.md`, and the current
Stage 0 host-feature inventory

## Decision

Do not begin Stage 1 self-hosting yet. The first proposed slice is the pure
callable classifier shared by the compiler laboratory and headless document
model. Its callable artifact boundary is now evidenced by the version-2
catalog and the passing `callable_lowering_boundary` test. Stage 1 remains
closed because the surrounding Flow closure is incomplete: the authoritative
bootstrap inventory still lists the UTF-8 source reader, lossless tokenizer,
recursive data, canonical artifact I/O, ownership cleanup, target client, and
later compiler stages as unfinished. Starting Stage 1 before those contracts
are closed would still make the C++ implementation an unacknowledged semantic
authority.

This is a readiness decision, not a certification claim. Lyraform remains
experimental, unstable, and not safety-certified.

## First-slice admission contract

The first Stage 1 slice may be admitted only when all of these are true:

| Requirement | Required evidence | Current state |
|---|---|---|
| Pure scalar classifier has an explicit input/output contract | Flow source, typed lowering artifact, and positive/negative tests | planned slice exists; public Flow implementation not yet complete |
| Callable functions are represented completely | Function catalog, stable identities, parameters, entry root, calls, and returns survive lowering and validation | **PASS**; version-2 catalog and `callable_lowering_boundary` |
| Artifact consumers agree | LLVM and TinyVM execute the same captured artifact and produce equivalent result | **PASS** for the bounded scalar callable slice |
| Failure is explicit | malformed artifact, unsupported call shape, and runtime failure produce deterministic no-artifact/outcome diagnostics | partial Stage 0/provider evidence exists; Stage 1 Flow boundary is not closed |
| Bounds are explicit | source/input/output sizes, recursion/activation limits, and artifact limits are checked | bounds exist in several Stage 0 consumers; first Flow slice needs its own contract |
| No hidden resource effects | slice uses no filesystem, process, network, native ABI, mutation, async, or parallel effect | can be designed as pure; not yet proven by a Stage 1 artifact |

## Remaining Stage 0 privilege

The following privileges are still present and must remain visible until a
later stage replaces them through public files:

- C++ compiler, standard library containers, strings, maps, and recursive host
  object allocation build the current compiler and artifact consumers.
- C++ exceptions are used internally by parser, runtime, validation, and
  provider code. Tested CLI, runtime, artifact, and cleanup boundaries
  translate them into explicit failure results, but internal exception use is
  not a Lyraform language capability.
- The Stage 0 process owns filesystem access, subprocess execution, dynamic
  library loading, threads, clocks, and provider discovery. The pure first
  slice must not inherit any of these effects by ambient access.
- LLVM and TinyVM backend implementations are Stage 0 providers. Their output
  parity is evidence about the named artifact, not proof that Flow can yet
  construct the artifact without C++.
- Shell scripts, CMake, `jq`, sanitizers, Valgrind, and host test tools run the
  evidence campaign. They are not available as implicit language operations.

## Controls already available

The current public boundaries already provide useful controls for the eventual
slice: strict versioned artifact validation, identity/provenance preservation,
bounded activation and output paths, explicit unsupported scheduling, governed
ABI admission, structured no-artifact diagnostics, deterministic TinyVM/LLVM
parity tests, and durable error-state append/recovery behavior.

These controls close the bounded callable-artifact slice only. A green Stage 0
test cannot be promoted to a general Stage 1 guarantee until the remaining
Flow closure inventory is implemented and a Flow-written producer and the
independent consumers exercise the same public artifacts.

## Required next gate

Continue with `utf8-source-reader` in
`docs/bootstrap/remaining-bootstrap-inventory-v1.json`, then advance through
the dependency chain. The next safety-relevant closure evidence must include:

1. validated UTF-8/scalar input and exact spans;
2. lossless token and recursive-data contracts without private C++ hooks;
3. canonical artifact I/O with hostile mutation tests;
4. explicit ownership, cleanup, and boundedness outcomes for the slice;
5. equivalent LLVM and TinyVM execution from captured public artifacts; and
6. a Flow-written producer/consumer proof that does not use a private C++ hook.

The Stage 0 lexer now has independent UTF-8 ingress evidence through
`flowmini_utf8_source_boundary`; this does not satisfy the requirement for a
Flow-written source reader and therefore does not change the Stage 1 decision.

The focused boundary passes under Clang 18.1.3 ASan/UBSan with leak detection
disabled. The valid and invalid source cases also pass under Valgrind 3.22.0
Memcheck with `--error-exitcode=99` and no reported errors. These are focused
memory checks, not a claim that the complete sanitizer matrix is clean.

Until those checks pass, Stage 1 remains `not-started` and the safety mission
state remains `CONTINUE`.

## Safety-assurance reconciliation — 2026-09-15

Since this review was opened, the safety mission has added and regression-tested
bounded allocation-free structured diagnostics across the current compiler,
artifact, lowering, provider, graph, and kernel process boundaries. The shared
diagnostic primitive has direct hostile escaping/truncation and Memcheck
evidence. Durable error-state history now also covers child-process death after
partial write and after file synchronization but before directory
synchronization; incomplete tails remain refused and require explicit repair.

These closures strengthen the Stage 0 evidence boundary but do not alter the
Gate 9 decision. The first self-hosted slice still lacks a Flow-written
producer/consumer proof and the bootstrap closures listed above. The optional
ConfigResolve adapter is disabled in the canonical build and contributes no
provider evidence. Retention, deeper filesystem crash semantics, isolation,
signed trust profiles, cross-platform assurance, and general cancellation/
async/backpressure/effectful parallelism remain explicitly provisional or
future. Stage 1 remains `not-started`; the safety mission remains `CONTINUE`.

## Evidence boundary

This review deliberately records a not-ready decision rather than treating the
current Stage 0 C++ handling as inherited safety. It does not add a new
language feature, alter FlowLFS or `master`, or authorize self-hosting.
