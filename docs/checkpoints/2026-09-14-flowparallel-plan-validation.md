# Flowparallel execution-plan validation checkpoint

**Date:** 2026-09-14  
**Status:** implemented for the CPU provider boundary

`flowparallel_cpu` now parses the complete execution plan with the shared
JSON parser before policy selection. It requires the versioned top-level
format and status, reads `dependency_analysis.parallel_candidates` from its
declared object path, and refuses duplicate keys, wrong types, unsupported
versions, and malformed input. Scheduling controls are evaluated only at
their declared top level; nested lookalike fields cannot request or disable a
capability.

The existing refusal contract remains unchanged: effectful parallelism,
cancellation, asynchronous execution, and backpressure return structured
`unsupported` results with no fallback artifact. A non-ready plan returns an
explicit `blocked` result.

Evidence:

```text
CPU-provider positive/refusal/malformed/wrong-type/wrong-version/nested-field tests: PASS
normal CTest: 111/111 PASS
worktree: clean after publication
```

This closes typed input validation at the CPU selection boundary; it does not
admit cancellation, async execution, backpressure, effectful parallelism, or
general scheduling semantics.

The runtime planner now applies the same refusal at the provider-selection
boundary, so an unsupported scheduling request cannot bypass the CPU-provider
guard by entering through CUDA/runtime planning.
