# Source-graph finite stream activation

Status: implementation phase opened by Henrik after the bounded v0.29 native
graph checkpoint (2026-09-13).

## v0.30 bounded contract

The first stream surface is a finite, ordered scalar stream selected explicitly
in the graph provider map. It is not a reinterpretation of `startup_once` and
does not introduce persistent receiver state.

- A stream provider has one zero-argument count callable and one indexed item
  callable. The item callable receives a `c_size_t` index and returns one of the
  existing admitted scalar carriers.
- The count callable is invoked exactly once for the stream root. Its result
  must be non-negative and no greater than the selected hard bound of 4,096
  items. A zero count emits no item activation.
- Item indices are delivered in ascending order from zero. Each successful item
  is one logical output activation with its own signal identity; each connected
  wire receives a distinct delivery while retaining that item's signal identity.
- A receiver still gets one fresh activation frame per delivered item and one
  invocation per delivery. Fan-out never re-executes the receiver.
- A count or item failure emits no normal result, records the stream root/index
  and provider provenance, and stops the stream without retrying or emitting
  later items.
- The stream is finite and synchronous within the existing ordered scheduler.
  It has no joins, persistent/shared state, repeated output from one receiver
  activation, async suspension, cancellation, backpressure, or parallelism.

The provider map carries the count/item identities and cap as explicit
selection evidence. Flowbind must authorize both exact callable ABI tuples;
symbol existence or a stream-shaped name is never authority. The schedule
artifact records the stream template and cap rather than fabricating one static
step for an unknown runtime count. Downstream consumers independently validate
that template before lowering.

The first implementation is intentionally scalar and direct: stream items may
enter the existing single-input receiver contract, and stream-root per-wire
fan-out remains valid. Receiver pipelines, aggregate items,
persistent state, and reentrant/parallel delivery follow as separate phases.
