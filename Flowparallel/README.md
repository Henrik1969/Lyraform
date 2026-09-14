# Flowparallel

Flowparallel is the conservative execution-plan sibling after Flowanalyst:

```text
Lyraform source -> Flowmini-compatible frontend -> Flowanalyst -> Flowparallel -> Flowoptimize -> Flowlower
```

It consumes an accepted `flowanalyst.semantic_report` v1 and emits an
inspectable `flowparallel.execution_plan` v1. The first implementation carries
the dependency-analysis boundary, preserves source provenance, requires a
runtime capability snapshot, and always declares a serial CPU fallback.

Contract and input failures can be requested as machine-readable
`--diagnostics json` records with a `no_artifact` disposition; human-readable
diagnostics remain the default.

It emits only runtime-deferred candidates whose current proof includes pure
callee behavior, disjoint inputs, and distinct outputs. Dependency structure
alone does not prove purity, absence of mutation, ordering freedom, or safe
external effects. The CPU execution API accepts only this explicitly approved
task boundary; all other work remains serial or deferred.

Provider selection is deferred to runtime policy. A deployment may eventually
ship the plan with a Frankencore runtime/JIT layer that discovers local CPU,
memory, and CUDA capabilities, then selects a permitted strategy without
recompiling the source artifact.

The first provider is `flowparallel_cpu`. It consumes the plan, discovers the
local logical-processor ceiling, caps the requested worker count, and applies
the observed-speedup threshold. It emits a selection record but does not yet
execute user program regions:

```text
execution plan + runtime capacity + calibration + policy
→ cpu.threadpool or cpu.serial
```

Its execution API accepts only an explicitly approved vector of independent
tasks. The execution smoke test obtains its task count from the proven
`parallel_candidates` count, compares serial and threaded results, and verifies
failure propagation. User regions are not executed unless they cross this
approved-task boundary.

The execution API publishes explicit result metadata: invalid worker counts,
task failures, non-standard task failures, and worker-launch failures carry a
stable code and `no_artifact` disposition. All launched workers are joined
before the result is published; no partial success is silently promoted.

The CPU provider refuses stronger scheduling requests rather than silently
falling back to serial execution. `parallel_effectful_v1`, cancellation,
asynchronous execution, and backpressure requests produce an explicit
`unsupported` selection with no fallback artifact. Those capabilities remain
outside the admitted pre-self-hosting contract.

The provider parses and validates the complete execution-plan JSON before
making that decision. Duplicate keys, wrong field types, unsupported versions,
and nested lookalike fields cannot alter the top-level scheduling policy.
`flowparallel_cpu --diagnostics json` exposes malformed-input and provider
failures as structured `no_artifact` diagnostics; the default remains a
human-readable error. Numeric worker and speedup policy arguments must be
complete finite tokens; malformed suffixes are rejected before selection.

The optional `flowparallel_cuda` provider currently probes the CUDA driver and
emits a linear-algebra workload contract for matrix multiplication. It includes
host/device transfer costs and always requires a CPU fallback. On a host without
a usable CUDA device it reports `unknown` or `unavailable`; it never claims a
kernel executed. A future CUDA backend can consume the same contract. It
refuses `parallel_effectful_v1`, cancellation, asynchronous execution, and
backpressure before probing the driver, returning an explicit unsupported
selection without a fallback artifact. `--diagnostics json` translates
argument, input, and provider failures to a machine-readable `no_artifact`
diagnostic; normal human-readable diagnostics remain available by default.

## Parallel smoke test

The provider smoke test uses a deterministic segmented reduction. It compares
one-worker serial execution with a multi-worker execution, repeats the serial
run, verifies exact result equality, rejects an invalid worker count, and runs
under a timeout plus a bounded virtual-memory limit.

```sh
FLOWPARALLEL_SMOKETEST_ITEMS=2000000 \
FLOWPARALLEL_SMOKETEST_WORKERS=4 \
./Flowparallel/tests/run-parallel-smoketest.sh
```

The validated JSON report is temporary by default. Preserve one deliberately
with an explicit path, for example:

```sh
./Flowparallel/tests/run-parallel-smoketest.sh --output /tmp/flowparallel-report.json
```

`FLOWPARALLEL_MIN_SPEEDUP` overrides the smoke-test observation threshold;
the default is `1.25`. In the eventual runtime this value belongs to resolved
policy, not to the source program.

The report is an inspectable `flowparallel.smoketest_report` artifact. Timing
is recorded for observation only; correctness and safe failure are the gates.
The report also calculates an observed speedup and applies a deliberately
visible 1.25 minimum-speedup threshold. That threshold is measurement output,
not yet an automatic compiler decision.

Flowparallel also preserves Flowanalyst's `region_dependency` graph-to-matrix
projection as metadata in the execution plan. The graph remains canonical;
the Boolean COO view is an acceleration projection. Runtime policy decides
whether CPU or CUDA is appropriate.

## Host GPU execution gate

`flowparallel_cuda_execute` is the first execution projection. It dynamically
loads CUDA Runtime and cuBLAS, performs a small matrix multiplication, copies
the result back, and verifies the result numerically. It does not make CUDA a
mandatory build dependency for the rest of Flowparallel.

Build it with:

```sh
cmake --build build --target flowparallel_cuda_execute
```

Run the host-only gate through a context that can access the real NVIDIA device
namespace:

```sh
FLOWPARALLEL_CUDA_EXECUTE_BIN="$PWD/build/flowparallel_cuda_execute" \
  tests/run-cuda-execution-host-test.sh
```

The ordinary CTest suite does not run this gate because IDEs, containers, and
restricted test runners may not expose `/dev/nvidia*`. A successful report has
format `flowparallel.cuda_execution`, status `verified`, provider
`cuda.cublas`, at least one device, and maximum numerical error below `0.001`.

The implementation is deliberately narrow: it proves the runtime boundary and
numerical execution. Scheduling, workload thresholds, memory policy, and
lowering from a Flowparallel plan remain separate capabilities.

`--diagnostics json` translates argument, provider, and cleanup failures into
a stable `no_artifact` record on stderr; normal human-readable diagnostics
remain the default.

`flowparallel_matrix_benchmark` compares an optimized single-thread CPU
baseline with cuBLAS and reports both compute-only and end-to-end timings. Its
purpose is calibration evidence, not a universal compile-time threshold: small
matrices may lose to CUDA because transfer and launch costs dominate.

The benchmark uses the same explicit device-resource owner as the execution
paths. Allocation, transfer, warm-up, timed execution, verification, and
cleanup failures are checked; `--diagnostics json` produces a stable
`no_artifact` failure record without presenting calibration as execution proof.

## Runtime provider planner

`flowparallel_runtime_planner` is the first explainable selection boundary. It
consumes an execution plan, a `frankencore.runtime_capabilities` snapshot, an
optional verified calibration report, and the active minimum-speedup policy.
It selects `cuda.cublas` only when all evidence is present and the measured
end-to-end benefit meets policy. Otherwise it selects `cpu.serial` and reports
the reason. CPU fallback is always retained.

This planner deliberately consumes snapshots rather than discovering hardware
inside the optimizer. Compile-time legality, runtime facts, policy, and
provider execution remain separate.

Malformed plans, capability snapshots, calibration reports, and argument
failures do not produce a provider selection. `flowparallel_runtime_planner
--diagnostics json` exposes those failures as stable machine-readable
`no_artifact` diagnostics; human-readable errors remain the default.

Before provider selection it also refuses execution plans that request
cancellation, asynchronous execution, backpressure, or effectful parallel
scheduling. These requests produce an explicit unsupported decision with no
fallback artifact; the planner never turns an unimplemented capability into a
serial or CUDA execution choice.

`flowparallel_graph_reference` and `flowparallel_graph_cuda` provide the first
paired graph-algebra operation: Boolean reachability over the
`region_dependency` matrix. The CUDA provider uses a dense cuBLAS projection
with thresholded Boolean semantics and is accepted only when its reachable-pair
count matches the CPU reference. Sparse/dense selection and cost policy remain
the next calibration gate.

The Flowparallel execution plan preserves the COO entries in its graph
projection so downstream optimizer stages can inspect and transform the
derived view without losing the canonical graph boundary.

`flowparallel_graph_planner` makes that gate explicit. It consumes the graph
report, runtime capability facts, and optional verified calibration, then emits
`flowparallel.graph_provider_decision` v1. The default density threshold is
`0.25` and the default minimum measured end-to-end speedup is `1.25`; both are
policy inputs. Sparse graphs remain on `cpu.reference`. Dense graphs may use
`cuda.cublas.boolean_threshold` only when CUDA is available and verified
calibration clears the threshold. Every decision records its representation,
reason, and mandatory CPU fallback.

The CUDA graph provider also emits `cpu_reference_ms`,
`cuda_end_to_end_ms`, and `end_to_end_speedup`. That report is an accepted
calibration input to the graph planner. Small graphs are expected to lose: the
host test currently measures a verified 28-pair result but only about
`0.000005x` end-to-end speedup on the four-node probe because launch and
transfer overhead dominate. The planner therefore keeps the CPU provider when
the measured evidence does not clear policy.

`tests/run-graph-cuda-firetest.sh` exercises empty, chain, cyclic, and dense
four-node graphs. Each shape must agree with the CPU reference; the current
host run passed all four. This is deliberately a correctness gate, not a claim
that small graphs benefit from CUDA.

The representative calibration sweep covers sparse and dense graphs at sizes
4, 8, and 16. It passed all six CPU/CUDA differential cases; every measured
speedup was far below `1.25x`, so the conservative runtime decision remains
`cpu.reference`. The complete evidence is recorded in
`docs/flowparallel/2026-08-21-graph-calibration-report.md`.
