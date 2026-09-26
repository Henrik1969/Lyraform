# General explicit failure-flow scout

Date: 2026-09-19.

Gate: **PASS** — the refusal/failure/fault model was accepted on 2026-09-19
and recorded in ADR 0055.

## Baseline and protected scope

- branch and synchronized baseline: `main` at
  `6ba9db303db453af952327069814c3a607b26d03`;
- bounded static guard lifecycle: PASS;
- runtime guard disposition: Candidate D, fail closed until general failure
  flow is canonical;
- `master` and `flowlfs-v0.1-alive`: untouched;
- no staging, commit or push authority.

## Bounded question

What dispositions exist before and during execution, and which belong to the
program's ordinary failure graph?

This scout does not attempt to settle recovery syntax, retries, cancellation,
timeouts, backpressure or top-level execution policy.

## Canonical facts

The following laws already exist and are promoted by ADR 0054:

- failure is explicit flow;
- no language exceptions, implicit unwinding or silent failure;
- invalid compile-time-known behavior is refused before execution;
- runtime failure must carry structured, typed information;
- recovery and propagation are graph behavior;
- rejected attempts, operational error states and committed mutations remain
  distinct;
- a failed state transition publishes no partially committed normal state.

For guards this yields:

```text
candidate transition
        ↓
pre-commit guard check
   ├── success ──> commit
   └── failure ──> general failure flow
```

The right-hand branch is architectural intent, not yet admitted language
semantics.

## Existing implementation evidence

### Bounded tagged outcome

`Outcome<Text,TextFailure>` is implemented for owned Text construction across
LLVM and TinyVM. It proves that a backend-neutral tagged success/failure value
can be serialized and executed. It does not prove a universal source-language
Outcome type, graph routing law or ownership rule.

### Structured stage/process failures

Compiler, validator, provider and planning boundaries emit stable diagnostic
codes, dispositions and nonzero exits. These are public boundary projections,
not ordinary values flowing inside a Lyraform graph.

### Durable operational error state

`ErrorStateEvent` records opened, diagnosed, recovery-attempted, resolved,
escalated and reopened lifecycle facts. It is an audit/recovery family, not a
source operation's failure carrier.

### Mutation rejection

`MutationRejection` records a failed state-transition attempt separately from
both committed mutation and operational error lifecycle. This distinction is
directly relevant to guard refusal but does not decide graph routing.

### Distributed graph direction

The design notes already require error-lane behavior, diagnostics carried with
envelopes, and explicit failure/timeout/retry/backpressure contracts. They do
not freeze local language syntax or success/failure cardinality.

## Resolved by ADR 0055

The repository now establishes:

- construction/admission refusal is outside execution;
- one admitted attempt completes with exactly one of `Success<T>`,
  `Failure<E>`, or `Fault<F>`;
- expected runtime failure is ordinary typed flow;
- execution-integrity fault is explicit evidence governed separately from
  ordinary recovery;
- tagged values, graph ports, provider responses, and execution records may be
  projections of the same discriminated completion;
- diagnostics and events may accompany a completion but do not create another
  semantic completion;
- the prove/refuse/runtime-enforce precondition rule is canonical.

## Still undefined

The repository does not yet establish:

- the minimal common failure envelope;
- whether every failure category is a distinct type or uses an extensible
  category field;
- concrete syntax for the now-required explicit disposition route;
- how recovery transforms rejoin normal flow;
- how multiple simultaneous upstream failures compose;
- who owns retry, cancellation and timeout policy.

Normal data cannot be a second successful completion beside failure or fault.
Observations such as diagnostics, metrics, audit events, and explicitly
described partial-work evidence may accompany the selected disposition.

## Authority boundary

The future authority should follow:

```text
language semantic model
    operation result algebra and legality

versioned contracts
    result/failure consistency and provenance

graph projection
    explicit success/failure ports and wires

execution policy
    explicit top-level failure sinks and fault-containment authorities

backend
    execute the established branch; never invent routing
```

Durable history may record a failure after the semantic result exists. It must
not become the source-language authority merely because it already has storage.

## Resolved decision

Refusal, expected runtime failure and execution fault are distinct. One
admitted attempt completes with exactly one of success, failure, or fault.
This determines guard commit atomicity and the common completion cardinality;
concrete carriers and routing remain future bounded work.

Routing completeness was subsequently resolved by ADR 0056: every failure and
fault needs an explicit, inspectable destination; otherwise admission fails
with a dangling wire error. Concrete carrier and routing syntax remain future
bounded work.

See
[the operation-result flow decision brief](2026-09-19-operation-result-flow-decision-brief.md).

## Verification

This scout changes documentation only after the verified guard implementation:

- preceding canonical suite: **165/165 PASS**;
- preceding focused ASan/UBSan guard suite: **1/1 PASS**;
- preceding Valgrind guard probes: zero errors and zero leaks;
- `git diff --check`: PASS after this scout and associated ADR updates.

GATE: PASS — REFUSAL / FAILURE / FAULT MODEL
