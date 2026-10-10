# ADR 0067: Fault containment returns an explicit quarantined activation result

## Status

Accepted architectural and execution decision on 2026-10-10.

## Context

ADR 0065 requires a fault-containment authority to halt and quarantine its
exact scope, suppress normal publication, preserve evidence, permit no local
continuation, and emit a receipt. The reference contract proves that model,
but the native runtime's current `flow_graph_fail` implementation records a
diagnostic and calls `exit(70)`. Process exit is not an activation-containment
receipt and hides the boundary that made the decision.

Returning normally from the failed operation, throwing a host-language
exception, using non-local jumps, or allowing the activation to resume would
also violate the established model.

## Decision

Every executable graph activation completes with one explicit, structured
activation disposition:

```text
Published<T>
Failed<FailureEnvelope<E>>
Quarantined<FaultEnvelope<F>, ContainmentReceipt>
```

These are backend projections of one canonical disposition fact, not
independent backend error systems.

For a fault, execution follows this exact transition:

```text
Fault<F>
    -> explicit typed fault port
    -> declared activation containment authority
    -> halt and quarantine that activation
    -> suppress all normal publication from it
    -> emit a validated containment receipt
    -> return ContainedFault to the graph executor
```

The quarantined activation cannot resume or be converted into success or
expected failure. Its receipt preserves the activation, producer,
disposition, fault type, route, containment authority, commit state, and
provenance identities.

## Source and graph association

The first containment source projection is explicit:

```lyraform
node reader     : fn read_sensor
node quarantine : containment activation

wire reader.fault => quarantine.fault
```

`containment activation` names the bounded authority already established by
ADR 0065. It is not an ordinary function and has no success output. The graph
executor owns the validated containment receipt as control/evidence output;
observing or storing that receipt does not consume or reclassify the fault.

Missing, duplicate, incompatible, broader-scope, or ordinary-recovery fault
routes are admission refusals.

## Backend and runtime law

Native lowering must use explicit status/result branches and out-of-band
storage owned by the generated activation ABI. It must not use C++ exceptions,
`longjmp`, ambient thread-local handler lookup, or a low-level call to
`exit`. The generated caller checks the disposition and transfers control to
the declared containment path.

TinyVM returns the same structured activation disposition to its interpreter
loop. LLVM/native and TinyVM must agree on classification, scope, publication
suppression, receipt facts, and absence of continuation.

`flow_graph_fail` may remain temporarily as a compatibility terminal while no
artifact claims ADR 0067 execution. It must be removed from the admitted path
before that path is marked executable.

## Graph-executor and host boundary

The graph executor returns a structured `ContainedFault` result to its host.
That return is explicit fault propagation across the graph-execution boundary;
it is not success, ordinary recovery, or implicit process termination.

A versioned host policy selects only an authorized host response. The first
admitted command-host response is:

```text
report_and_fail
    render the structured fault and containment receipt
    return a documented non-zero command status
```

An embedded host receives the structured result through its API. It receives
no authority to resume the quarantined activation. Restart, replacement,
process quarantine, escalation to hardware or distributed containment, and
continued execution of independent graph regions require separately admitted
contracts.

The same host-policy artifact may be external or embedded. Missing or stale
host policy is an admission refusal; low-level runtime code never invents a
default exit path.

## First executable bound

The first executable projection is one serial activation with no independently
committable sibling work and no normal publication before the fault result is
known. This bound makes publication suppression and no-continuation directly
provable without inventing general cancellation or concurrent fault
composition.

The semantic result model is final for this bound. Later stages may widen the
set of schedulable graphs only after proving how independent work, irreversible
effects, cancellation, and outer containment preserve it.

## Consequences

- Fault containment becomes observable and testable without terminating from
  inside the activation runtime.
- Backends share one disposition and receipt contract.
- Command exit is a host-policy projection, not language-level unwinding.
- Embedded runtimes can inspect the contained fault without acquiring local
  continuation authority.
- Normal output after a contained fault is structurally impossible in the
  admitted path.

## Explicit non-decisions

This ADR does not admit fault recovery, activation restart, graph-region or
process containment, general cancellation, parallel fault composition,
distributed quarantine, or implicit top-level termination.
