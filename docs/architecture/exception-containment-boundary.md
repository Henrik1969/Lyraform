# Stage 0 exception-containment boundary

**Status:** safety mission requirement; compiler/runtime/reference CLI slices implemented, wider boundary incomplete
**Date:** 2026-09-14

## Decision boundary

Lyraform language semantics do not contain exceptions, implicit unwinding, or
catch-all recovery. A failed language operation produces an explicit outcome,
diagnostic, trap, or lifecycle event according to its contract.

The current Stage 0 C++ implementation uses exceptions internally for parser,
runtime, bridge, and validation errors. That is a bootstrap implementation
mechanism, not a language feature. It is currently contained by top-level CLI
translation in the compiler and by selected runtime diagnostic capture before
rethrow.

## Required safety law

No C++ exception may cross any of these public boundaries:

- a Lyraform semantic or runtime value boundary;
- a versioned artifact or stage-process boundary;
- a provider/ABI invocation boundary;
- an embedding API boundary;
- a machine-readable CLI result boundary.

At each boundary, the implementation must translate the condition into a
structured result containing, where applicable:

```text
status or outcome
stable diagnostic code
stage and operation identity
source/provenance reference
resource and cleanup disposition
retry/cancellation disposition
correlation and attempt identity
```

Unknown `std::exception`, allocation failure, provider exception, and foreign
exception conditions must not become a successful result or an unlabelled
partial artifact. If translation itself cannot be completed, the boundary
must return a deterministic fatal/unresolved result and preserve the process
exit distinction.

## Current audit findings

- `flowmini_runtime.cpp` throws `flow::DiagnosticError` for semantic,
  payload, graph, provider, arithmetic, and resource failures; the graph loop
  catches it to attach routing context and rethrows.
- `main.cpp` converts `DiagnosticError` and generic `std::exception` into CLI
  diagnostics and a nonzero exit, but the generic fallback has no stable
  diagnostic code or structured machine-readable form.
- TokenTree catches allocation and unknown exceptions at its C API bridge.
- Frankencore provenance validation and ULID generation still use C++ standard
  exceptions for invalid records and entropy exhaustion.
- The Clock and Revision reference CLIs still use exception-based argument
  parsing internally, but their public failure boundary now supports
  `--diagnostics json` with stable failure codes, empty artifact stdout, and a
  `no_artifact` disposition.
- Package inventory, language, requirements, policy, and runtime capability
  projections already return explicit diagnostic/result structures for their
  normal provider failures, but their serialization APIs still need a common
  checked boundary if they become Stage 1 dependencies.
- The CUDA Flowparallel provider now supports `--diagnostics json`, translating
  argument, input, and provider failures to a stable failure code with a
  `no_artifact` disposition. Its normal human-readable diagnostics remain
  unchanged.
- The CPU Flowparallel provider provides the same tested projection through
  `--diagnostics json`, with a provider-specific stable failure code and no
  artifact on stdout.
- The Flowparallel runtime planner provides the same tested projection for
  malformed plans, capability snapshots, calibration reports, and argument
  failures.
- The plan-producing Flowparallel CLI provides a structured contract-failure
  projection through `--diagnostics json`; malformed semantic reports leave
  artifact stdout empty.
- The graph-reference Flowparallel consumer also provides a structured
  `no_artifact` failure projection for malformed or unsupported semantic graph
  input, preserving the independent artifact boundary.
- The graph-planner Flowparallel consumer provides the same tested projection
  for malformed graph, capability, calibration, and policy input.
- The native graph-CUDA consumer provides `--diagnostics json` for failures
  before or during native provider setup, with no graph artifact and an
  explicit `no_artifact` disposition. Hardware-dependent CUDA differential
  firetests remain separate evidence.
- The CUDA matrix execution consumer provides the same structured failure
  projection, including provider and cleanup failures. Cleanup remains
  explicit, and a cleanup failure cannot become a verified execution result.
- The CUDA matrix benchmark provides the same structured failure projection;
  calibration failure cannot be serialized as verified calibration evidence.
- The CPU execution API translates task and worker failures into explicit
  result metadata with stable codes and `no_artifact` disposition, publishing
  only after all already-launched workers have joined.
- The shared CUDA resource owner fails closed with `EINVAL` when a cleanup
  callback is absent, preventing an internal null-call from crossing a public
  provider boundary.
- These paths are covered by the hardware-independent
  `flowparallel_exception_containment` conformance test, which checks empty
  artifact stdout, stable failure codes, `no_artifact` disposition, and
  nonzero exit status across nine current Flowparallel boundaries. This does
  not replace native hardware fault injection.

The compiler CLI now accepts `--diagnostics json`. For a caught
`DiagnosticError`, allocation failure, or unexpected standard exception, it
emits a structured failure record on stderr, keeps artifact stdout empty, and
returns a nonzero status. The current CLI codes are intentionally conservative:
`FLOW_DIAGNOSTIC_ERROR`, `FLOW_RESOURCE_EXHAUSTED`, and
`FLOW_UNEXPECTED_EXCEPTION`, and `FLOW_UNKNOWN_FAILURE`. The Clock and
Revision reference CLIs provide the corresponding provider/provenance slice
with `FRANKENCORE_CLOCK_FAILURE`, `FRANKENCORE_CLOCK_UNKNOWN_FAILURE`,
`FRANKENCORE_REVISION_FAILURE`, and
`FRANKENCORE_REVISION_UNKNOWN_FAILURE`. These changes close only the tested
CLI/reference/provider projection slices; they do not make the internal exception
mechanisms themselves part of the language contract or close all provider/API
containment.

The runtime graph now also exposes `runModuleChecked`, which translates
runtime, allocation, and unexpected standard failures into an explicit
`RuntimeResult` before the compiler's public caller handles the result. The
legacy `runModule` and `RuntimeGraph::startAt` entry points remain available
for compatibility and still use internal exceptions; they are not yet the
preferred Stage 1 boundary.

## Required implementation slice

1. Define a small public `Outcome`/diagnostic boundary type without changing
   Lyraform syntax or adding language exceptions.
2. Add adapters at the compiler CLI, runtime graph, provider bridge, and
   Frankencore public API boundaries.
3. Preserve existing human diagnostics and exit statuses while adding stable
   machine-readable codes and dispositions.
4. Add hostile tests for known diagnostic errors, generic exceptions,
   allocation failure where injectable, provider failure, cleanup failure, and
   translation failure.
5. Build with exceptions enabled initially so legacy internals can be migrated
   incrementally; only consider `-fno-exceptions` after equivalent result
   coverage exists across every required provider and test target.

## Non-goals

This note does not require removing every C++ `throw` immediately, does not
turn process termination into a recoverable language outcome, and does not
claim that a catch-all handler is safe. Catching a condition is acceptable only
when the boundary assigns it an explicit semantic disposition and preserves
the relevant evidence.
