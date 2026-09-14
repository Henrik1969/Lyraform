# Graph-reference input validation checkpoint

**Date:** 2026-09-14  
**Status:** implemented for the CPU graph-reference and diagnostic boundaries

`flowparallel_graph_reference` now consumes `flowanalyst.semantic_report` via
the shared typed artifact parser. It validates the complete report, including
duplicate keys, declared analysis-graph format/version, matrix dimensions,
entry types, bounds, and duplicate coordinates, before computing reachability.
It no longer discovers matrix fields through substring scans.

Evidence:

```text
graph-reference positive and blocked tests: PASS
duplicate-key malformed input refusal: PASS
normal CTest: 111/111 PASS
```

The reference provider remains CPU-only and does not imply admission of
unsupported graph scheduling, effectful parallelism, cancellation, or async
semantics.

The consumer also supports `--diagnostics json`. Malformed semantic graph
input produces no graph artifact on stdout and a stable
`FLOWPARALLEL_GRAPH_REFERENCE_FAILURE` record with `no_artifact` disposition.

The paired graph planner also now exposes `--diagnostics json` for malformed
graph, capability, calibration, and policy input, with no provider decision on
stdout and a stable `FLOWPARALLEL_GRAPH_PLANNER_FAILURE` record.

The native CUDA graph consumer now supports the same diagnostic mode. A direct
malformed-input check passes before CUDA driver loading; the hardware-dependent
CUDA graph firetest remains a separate gate and is not represented as a CTest
pass on hosts without the required device.
