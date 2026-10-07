# General effectful parallel scheduling — Gate 1

Date: 2026-10-07
Mission: [General Effectful Parallel Scheduling — Mission 01](../tasks/general-effectful-parallel-scheduling-mission-01.md)
Gate result: **PASS for bounded concurrent scalar observation**

## Baseline and scope

Work began on synchronized `main` with both `HEAD` and `origin/main` at:

```text
16a1cfff6e560ed343286af0f9f604edbb3d02e5
```

The worktree already contained the separately owned failure-flow maturation
work. It was preserved. `master` and `flowlfs-v0.1-alive` were not modified,
merged, rebased, or copied. No source scheduling syntax, `async` feature,
general FFI, mutation, fallible effect, cancellation, retry, backpressure, or
irreversible-effect behavior was admitted.

The completed family is exactly:

```text
one admitted scalar startup source
    -> two or more fresh receiver activations
    -> one exact read-only scalar provider observation per receiver
    -> complete-wave join
    -> deterministic activation-order publication
```

## Reconnaissance classification

| Concern | Classification and authority |
|---|---|
| external-call operation and complete provider tuple | canonical semantic fact produced by Flowanalyst |
| generated binding digest/evidence | canonical provider identity evidence; not scheduling policy |
| provider resource domain and concurrent-observation promise | reviewed provider-contract profile supplied explicitly to Flowanalyst |
| operation access and receiver effect summary | canonical semantic fact produced by Flowanalyst |
| graph dependencies and candidate waves | derived graph dependency |
| pairwise effect/resource independence | shared, recomputable conflict proof |
| serial versus parallel strategy | policy choice restricted to the canonical legal set |
| worker count and observed minimum benefit | runtime capability/calibration evidence |
| native threads and TinyVM serial iteration | backend mechanism |
| topology-only effect independence or `readonly` alone | accidental/insufficient behavior, now refused |
| pointer, aggregate, mutable, fallible, nested, or unprofiled calls | undefined for this family and explicitly refused |

Resource identity is not inferred from a function name, source node, library,
or hardware. It comes only from the reviewed exact-capability provider-effect
profile. Binding policy must independently authorize the same complete provider
tuple.

## Canonical authority implemented

`Flowcontracts/effect_scheduling.hpp` now defines and validates versioned:

- provider-effect profiles;
- operation access facts;
- pairwise effect-conflict facts;
- effect-schedule facts.

The admitted profile requires exact generated provider evidence, `readonly`,
`observe`, `concurrent_observation_v1`, `infallible_scalar_v1`, a nonempty
resource domain, and admitted by-value scalar carriers. `readonly` without the
other evidence has no concurrency meaning.

Flowanalyst accepts the reviewed profile set only through the explicit
`--effect-profiles` input. It binds profiles to exact external-call operations,
emits canonical access facts, and classifies a receiver as bounded observation
only when its direct result is exactly one matching provider call. Unknown,
mixed, mutated, and unprofiled receivers remain excluded.

Source-graph schedule version 6 carries `effect_conflict_waves_v1`, complete
pairwise independence evidence, a deterministic serial reference order,
`publish_after_wave_join_v1`, and `no_partial_wave_publication_v1`. Shared
validation regenerates the schedule from the canonical source graph; a changed
wave, conflict, provider tuple, resource identity, or operation set fails at
the next consumer.

Flowparallel, Flowoptimize, and backend preparation preserve the exact provider
profiles and access facts. The CPU policy selector counts the validated version-
6 access set and can select either `cpu.threadpool` or `cpu.serial` from the same
canonical plan according to observed benefit and minimum-speedup policy. It
cannot broaden the legal strategy set.

## Backend execution and provenance

LLVM reuses ordinary typed provider-call lowering inside the existing graph
worker functions. The runtime launches only the validated wave, joins every
worker, buffers outputs, and publishes them in activation order. Eight repeated
runs use a bounded two-call rendezvous in the deterministic fixture provider;
every run proves both observations were simultaneously in flight.

Each effect worker receives a compiler-serialized execution context containing
the canonical operation, activation, worker index, exact capability identity,
resource domain, and selected schedule policy. Execution and publication trace
records carry that context. If an unexpected worker fault reaches the existing
graph-failure boundary, the same context is the activation evidence and no
wave result has yet been published.

TinyVM validates the same profile, access, schedule, and exact provider tuple.
It executes each activation once in deterministic serial order and remains the
reference projection; it does not create host threads for this mission.

## Positive and hostile evidence

The focused end-to-end gate proves:

- two profiled scalar observations are admitted without source scheduling
  annotations;
- exact operation/resource conflict evidence produces one effect-aware wave;
- measured policy selects parallel at speedup `2.0` and serial at `1.0` for the
  same minimum `1.25` and the same canonical plan;
- native calls overlap in eight repeated runs;
- publication remains activation ordered and value equivalent;
- TinyVM executes the serial projection exactly once per activation;
- provider profiles and access authority survive optimization and preparation.

The hostile matrix refuses:

- no provider-effect profile;
- a changed provider symbol;
- a changed resource domain;
- a changed conflict relation;
- changed wave membership;
- malformed, duplicate, unsupported, pointer-bearing, non-read-only, fallible,
  or evidence-free profiles;
- previously refused receiver mutation and unsupported bodies.

Existing runtime gates retain worker-count bounds, worker-fault containment,
launch-failure cleanup, no publication after fault, and allocation-failure
coverage.

## Verification

GCC canonical build tree: `/tmp/lyraform-effect-scheduling`.

- configure: PASS;
- build: PASS;
- focused scheduling/backend suite: 9/9 PASS after the runtime provenance
  addition;
- complete CTest suite: 176/176 PASS;
- `igor doctor`: PASS;
- `igor build`: PASS;
- `igor test`: 176/176 PASS;
- `git diff --check`: PASS.

Clang 18.1.3 ASan+UBSan tree:
`/tmp/lyraform-effect-scheduling-asan`.

- configure/build: PASS;
- focused compiler/runtime suite: 9/9 PASS with
  `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and
  `UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`;
- LeakSanitizer: unavailable because the managed test environment runs under
  ptrace; LSan aborts before test execution.

Valgrind 3.22.0:

- strict effect-contract test: PASS, 0 errors, 0 bytes live;
- CPU parallel execution smoke test: PASS, 0 errors, all blocks freed;
- graph runtime bounds/fault test: PASS for memory errors and
  definite/indirect leaks. Its intentionally throwing forked child leaves
  exception/TLS allocations classified as possible/reachable at immediate
  process exit, so possible-leak-as-error mode changes the expected child exit
  code and is not a valid harness mode. No definite or indirect leak was
  reported.

The only build warning observed is the pre-existing unused `json_escape`
function in `Flowparallel/src/graph_cuda.cpp`; this mission did not introduce
it.

## Residual limits

Gate 1 does not claim general effectful parallel scheduling. The following
remain refused or future work:

- mutable, aliased, pointer-bearing, aggregate, owned/borrowed, opaque-output,
  fallible, nested, reentrant, or irreversible provider effects;
- effect-produced failure lanes, cancellation, retry, suspension,
  backpressure, queues, and partial-commit recovery;
- source-level scheduling or `async` syntax;
- TinyVM host-thread execution;
- signed provider profiles, stronger trust anchors, isolation, and broad
  platform assurance;
- a LeakSanitizer run outside the current ptraced environment.

Within the selected family, one semantic authority now crosses analysis,
conflict derivation, policy selection, LLVM parallel execution, and TinyVM
serial reference execution without a backend inventing semantic permission.
