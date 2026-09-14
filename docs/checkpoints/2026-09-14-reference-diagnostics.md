# Reference-tool structured diagnostics checkpoint

**Date:** 2026-09-14  
**Status:** implemented and verified on `main`

## Scope

The Clock and Revision reference CLIs now expose a small, explicit machine
readable failure boundary:

```text
--diagnostics json
```

Known and unknown C++ failures are translated to stable diagnostic codes on
stderr. Successful or failed diagnostic modes never place a failure artifact on
stdout, and the process exits nonzero on failure. The normal human-readable
mode remains compatible.

Implemented codes:

```text
FRANKENCORE_CLOCK_FAILURE
FRANKENCORE_CLOCK_UNKNOWN_FAILURE
FRANKENCORE_REVISION_FAILURE
FRANKENCORE_REVISION_UNKNOWN_FAILURE
```

## Verification

The focused CTest exercises an unknown clock and a non-monotonic revision,
checking exit status, empty stdout, JSON validity, stable codes, and the
`no_artifact` disposition.

```text
focused reference tests: 5/5 passed
complete Igor test gate: 109/109 passed
```

## Residual risk

This is a boundary-hardening slice, not removal of all internal C++ exception
use. Checked serialization for every public projection, cancellation and
async/backpressure semantics, durable error-state storage, isolation/trust
anchors, and arbitrary native ABI/FFI remain separate safety-mission work.
