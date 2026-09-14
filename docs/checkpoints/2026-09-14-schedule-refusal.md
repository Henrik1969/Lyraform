# Schedule refusal checkpoint

**Date:** 2026-09-14  
**Status:** implemented and verified on `main`

The `flowparallel_cpu` provider now checks schedule intent before provider
selection. It emits an explicit `unsupported` result and no fallback artifact
for:

```text
parallel_effectful_v1
cancellation = requested
async = requested
backpressure = requested
```

This prevents an unimplemented stronger contract from being silently treated
as serial-safe execution. The existing admitted pure independent-task path is
unchanged.

Verification:

```text
focused CPU-provider gate: PASS
hostile refusal cases: 4/4 passed
```

Cancellation, async execution, backpressure, and effectful parallelism remain
explicitly unimplemented; this checkpoint closes only the downgrade/refusal
boundary.
