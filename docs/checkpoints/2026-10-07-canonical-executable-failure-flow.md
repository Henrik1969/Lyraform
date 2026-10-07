# Canonical executable failure flow — Gate 1

Date: 2026-10-07.
Mission: [Canonical Executable Failure Flow — Mission 01](../tasks/canonical-executable-failure-flow-mission-01.md)
Gate result: **PASS for one explicit serial recover or transform route**

## Baseline and protected scope

The stage began on synchronized `main` at
`16a1cfff6e560ed343286af0f9f604edbb3d02e5`, equal to `origin/main`.
The working tree already contained the uncommitted Canonical Failure Response
Transition Gate 2 and General Effectful Parallel Scheduling Gate 1 work, plus
the unrelated untracked `meta-discusions.md`; all were preserved.

`master`, `flowlfs-v0.1-alive`, source syntax, Flowanalyst emission, compiler
plans, LLVM/TinyVM backends, runtime guards, mutation, and parallel failure
execution were not modified.

## Scout findings

- ADRs 0054--0056 and 0062 already establish explicit typed failure flow,
  exactly one accountable route, closed consumers, policy selection among
  authorized routes, exact ordinary function identities, and no dynamic
  nearest-handler lookup.
- `lyraform.failure_consumer/v1`,
  `lyraform.failure_policy_selection/v1`, and
  `lyraform.failure_response_transition/v1` already provide declarative
  consumer, policy, recovery, and transformation authority.
- No shared component previously associated one concrete producer disposition
  and envelope with those authorities or invoked the selected function.
- Existing source and graph notation remains provisional. Promoting it during
  this stage would have expanded the language surface.

## Bounded question and selected model

The stage asked what exact shared contract is sufficient to execute one serial
expected-failure route without source syntax, dynamic lookup, backend
inference, or runtime-created policy.

The selected model composes the existing authorities in one versioned plan:

```text
producer Failure<E>
    -> explicit typed wire
    -> closed consumer route
    -> policy-selected exact function identity
    -> declared recover or transform transition
    -> independently validated completion receipt
```

The reference executor dispatches only by resolved function symbol identity.
The runtime cannot select another route or infer transition meaning from a
function name, body, or return value.

## Implemented contracts

`flowcontracts/failure_flow_execution.hpp` now defines:

- `lyraform.failure_flow_plan/v1`, status `ready`, with the exact serial
  schedule `serial_explicit_failure_route_v1`;
- one producer disposition with exactly-one completion and the law
  `failure_publishes_no_normal_state`;
- one explicit `failure` to `failure_envelope` wire;
- exact reuse of the existing consumer, policy-selection, and
  response-transition authorities;
- `lyraform.failure_envelope/v1`, preserving disposition, obligation,
  operation, attempt, correlation, payload, no-commit proof, and provenance;
- exact function-identity dispatch through the bounded reference executor;
- `lyraform.failure_flow_receipt/v1`, proving either original-obligation
  closure through `Success<T>` or a distinct linked successor
  `Failure<E2>` obligation;
- independent plan, envelope, and receipt validation and deterministic JSON
  round trips.

The callback boundary covers only successful completion of an authorized
response attempt. A missing function, thrown host callback, contradictory
result, or malformed successor identity produces no completion receipt. This
does not define the still-deferred Lyraform response-attempt failure or fault
route.

## Evidence

Positive evidence covers:

- serial `ReadFailure -> SensorReading` recovery;
- serial `ReadFailure -> SensorUnavailable` transformation;
- exact typed envelope delivery;
- closure of the original recovery obligation;
- creation of one distinct successor transformation obligation;
- preservation of correlation, no-commit evidence, and origin provenance;
- deterministic plan, envelope, and receipt round trips.

Negative and hostile evidence refuses:

- invalid plan or schedule identity;
- changed producer failure type or commit law;
- diagnostic/payload ports substituted for the canonical failure-envelope
  wire;
- wrong consumer, route, policy, function, transition, or failure type;
- empty, duplicate, or multiple bounded routes;
- mismatched envelope disposition, operation, obligation, attempt,
  correlation, type, commit evidence, or provenance;
- unavailable response functions;
- response results contradicting the canonical transition;
- recovery that creates a successor obligation;
- transformation without a distinct successor obligation;
- host callbacks that throw instead of completing the bounded response;
- altered receipt plan, obligation, disposition, or successor evidence;
- missing required plan fields, non-ready plans, and future plan/receipt
  versions.

## Verification

- Focused GCC consumer/transition/execution suite: **3/3 PASS**.
- Focused Clang 18 ASan/UBSan execution test: **1/1 PASS** with LeakSanitizer
  disabled for the managed ptraced environment.
- Valgrind 3.22.0 execution test: **0 errors**, **0 bytes in use at exit**,
  2,760 allocations and 2,760 frees.
- Canonical-language authority freeze: **PASS**.
- Current-documentation drift guard: **PASS**.
- `git diff --check`: **PASS**.
- `./igor doctor`: **PASS**.
- `./igor build`: **PASS**.
- `./igor test`: **177/177 PASS**, 60.28 seconds.

The build retains the pre-existing unused `json_escape` warning in
`Flowparallel/src/graph_cuda.cpp`; this stage did not introduce it.

## Deliberate boundary

This gate does not claim:

- source syntax or source-level consumer declarations;
- Flowanalyst producer/wire emission;
- integration with callable-plan-v2 or source-graph artifacts;
- LLVM or TinyVM response-function execution;
- runtime guard failure routing;
- response-attempt failure, propagation, retry, sinks, or fault containment;
- durable evidence storage, mutation, or parallel failure composition.

The declarative consumer and transition facts remain declarative. Only the new
bounded plan may compose them into this exact executable reference projection.

## Next decision

The next engineering step is no longer merely internal propagation. It must
choose where the producer-to-consumer association first becomes visible:

1. an artifact-first compiler projection using existing operation/function
   identities and no new source spelling; or
2. a reviewed source-level consumer/association spelling projected into the
   same canonical plan.

Because that choice establishes a public language/compiler boundary, it
requires human review before implementation. Fallible parallel observations
must wait until the serial compiler/backend path exists and response-attempt
failure accounting is closed.

## Gate 1

**PASS.** One explicit typed expected failure can be routed and executed
serially through one exact authorized response identity with inspectable,
fail-closed obligation accounting. No new syntax or hidden failure mechanism
was introduced.
