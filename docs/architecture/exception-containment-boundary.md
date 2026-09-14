# Stage 0 exception-containment boundary

**Status:** safety mission requirement; CLI slice implemented, wider boundary incomplete
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
- These paths are covered as implementation behavior, but the repository does
  not yet have one conformance test proving exception containment at every
  public boundary.

The compiler CLI now accepts `--diagnostics json`. For a caught
`DiagnosticError`, allocation failure, or unexpected standard exception, it
emits a structured failure record on stderr, keeps artifact stdout empty, and
returns a nonzero status. The current CLI codes are intentionally conservative:
`FLOW_DIAGNOSTIC_ERROR`, `FLOW_RESOURCE_EXHAUSTED`, and
`FLOW_UNEXPECTED_EXCEPTION`. This closes only the CLI projection slice; it does
not make the internal exception mechanisms themselves part of the language
contract or close provider/runtime/API containment.

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
