# Source-graph persistent receiver state

Status: implementation phase opened by Henrik after the finite-stream
checkpoint (2026-09-13).

## v0.31 bounded contract

Persistent state is an explicit receiver contract. It is not inferred from a
function name, a provider effect, or an outer artifact status.

The first surface is one scalar state slot per receiver node:

```text
state receiver : c_long = 0
node receiver : fn accumulate persistent
```

The persistent receiver function has the typed shape
`fn accumulate(input_type, c_long): c_long`. The first argument is the delivered
wire payload; the second is the current node-local state. Its returned `c_long`
is both the logical output payload and the candidate next state. The initial
literal is captured in the source graph with source provenance.

The activation and commit laws are:

- one delivery creates one fresh activation frame;
- the frame reads the receiver's current state exactly once;
- the function runs exactly once with `(input, state)`;
- a successful return emits one output and atomically commits that return value
  as the next state;
- a failed activation emits no output and leaves the prior state unchanged;
- fan-out reuses the committed output signal and never re-executes the receiver;
- state belongs to the receiver node, not the wire, root, process-global scope,
  or another receiver;
- delivery order remains scheduler policy, and this first phase is synchronous
  FIFO only.

The initial state and every successful before/after state are part of the graph
activation trace. State is an in-process execution value in this phase: there is
no crash recovery, durable storage, snapshotting, cross-process sharing,
serialization, joins, async suspension, or parallel mutation.

State declarations must be unique, target a source receiver, use `c_long`, and
fit the signed 64-bit literal contract. Persistent receivers cannot be used in
the v0.30 finite-stream direct template. The state-aware schedule carries the
state transition contract explicitly, and the bounded direct native lowering
now proves commit/rollback execution. Aggregate payloads and reentrant/parallel
delivery remain later phases.

## Required implementation evidence

The implementation must add independent positive and hostile evidence for:

1. repeated ordered deliveries observe the prior committed state;
2. the initial value is used exactly once;
3. receiver failure does not commit a candidate state or activate fan-out;
4. a forged/missing/duplicated state declaration is rejected by every artifact
   consumer;
5. optimization and backend preparation preserve state identity and initial
   value; and
6. a native execution trace records before/after state for each successful
   activation.

The frontend, source graph, schedule, and native lowering now carry these
identities through the bounded direct path. Receiver pipelines, aggregate
payloads, and reentrant/parallel delivery remain outside this phase.
