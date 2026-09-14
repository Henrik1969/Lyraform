# Lyraform Stage 1 self-hosting readiness review

**Date:** 2026-09-14  
**Status:** not ready; review recorded before any Stage 1 implementation  
**Authority:** `docs/tasks/pre-self-hosting-safety-assurance.md`,
`docs/architecture/product-scale-and-self-hosting-plan.md`, and the current
Stage 0 host-feature inventory

## Decision

Do not begin Stage 1 self-hosting yet. The first proposed slice is the pure
callable classifier shared by the compiler laboratory and headless document
model. Its semantic contract is small enough to admit, but the current
backend-neutral lowering boundary does not yet carry a complete function
catalog, parameter bindings, entry root, or call-return structure. Starting
Stage 1 before that boundary is closed would make the C++ implementation an
unacknowledged semantic authority.

This is a readiness decision, not a certification claim. Lyraform remains
experimental, unstable, and not safety-certified.

## First-slice admission contract

The first Stage 1 slice may be admitted only when all of these are true:

| Requirement | Required evidence | Current state |
|---|---|---|
| Pure scalar classifier has an explicit input/output contract | Flow source, typed lowering artifact, and positive/negative tests | planned slice exists; public Flow implementation not yet complete |
| Callable functions are represented completely | Function catalog, stable identities, parameters, entry root, calls, and returns survive lowering and validation | **blocking gap** in `bootstrap_gap_inventory` |
| Artifact consumers agree | LLVM and TinyVM execute the same captured artifact and produce equivalent result | scalar callable evidence exists, complete call structure does not |
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

These controls do not close the callable-function gap. A green Stage 0 test
cannot be promoted to a Stage 1 guarantee until a Flow-written producer and
the independent consumers exercise the same public artifact.

## Required next gate

Close `callable-scalar-slice` in
`docs/bootstrap/remaining-bootstrap-inventory-v1.json` by adding:

1. a versioned function catalog and stable function identities;
2. explicit parameter binding and return-carrier records;
3. a validated entry root and call graph with bounded depth/activation rules;
4. canonical serialization and hostile mutation tests;
5. equivalent LLVM and TinyVM execution from the captured artifact; and
6. a Flow-written producer/consumer proof that does not use a private C++ hook.

Until those checks pass, Stage 1 remains `not-started` and the safety mission
state remains `CONTINUE`.

## Evidence boundary

This review deliberately records a blocker rather than treating the current
Stage 0 C++ handling as inherited safety. It does not add a new language
feature, alter FlowLFS or `master`, or authorize self-hosting.
