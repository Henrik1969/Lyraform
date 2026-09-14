# Source-defined graph activation: approved bounded contract

Status: approved by Henrik in the autonomous mission (2026-09-07).
Implementation is in progress; approval does not imply native graph support.

## Recovered gap

The reusable-chain ledger previously equated Flow wiring with Flow-owned pager
behavior. `PagerNavigateNode::run` in `flowmini_runtime.cpp` implements command
interpretation, page state, bounds and page extraction in C++. Both pager Flow
sources select this built-in atom; neither defines those algorithms.

`NodeDecl` currently contains only role, id and built-in kind. `buildCheckedGraph`
requires that kind to exist in `AtomRegistry`. There is no function-body identity,
input-to-parameter mapping or return-to-output mapping. Callable lowering v2
provides ordinary function calls, but defines no graph activation contract.

Before this audit, the frontend bundle silently omitted graph declarations and
Flowanalyst marked the remaining marker initializer ready for native lowering.
The new `FLOWMINI_GRAPH_LOWERING_UNSUPPORTED` diagnostic prevents that projection
and retains original source locations. Interpreter graph demonstrations remain
supported. This refusal is a correctness fix, not a completed graph backend.

## Approved v0.29 contract

Admit a single-input/single-output source receiver with fresh activation-local state:

- A node explicitly references an ordinary Flow function by semantic identity.
- Delivery on its declared input activates that function once with the payload
  as its typed parameter. One result emits on its declared output.
- Each activation has fresh local state; persistence between deliveries requires
  an explicit future state contract. Batched pager commands can be a payload.
- A failure emits no successful result and retains node, port, wire and signal
  provenance. No implicit retry or activation on an absent input is introduced.
- Scheduling chooses delivery order; it does not change the activation rule.
  Existing deterministic fan-out and unconnected-output laws still apply.
- Multi-input joins, repeated outputs, cycles and persistent receiver state
  remain unsupported until their own explicit activation contracts are defined.

One delivered input creates exactly one fresh activation frame and invokes the
receiving function exactly once. A successful return creates one logical output
activation. Fan-out copies deliveries, preserves that output's signal identity,
and never re-executes the function. Failure emits no normal result. `guard`,
`when` and selector evaluation retain their existing intra-activation contracts.
Scheduling policy remains separate from activation semantics.

Multi-input joins, persistent/shared state, streams, repeated outputs, suspension,
reentrant or parallel scheduling, cancellation, retries, backpressure, zero-output
sinks, generalized multi-result functions, distributed execution and durable
signal identity are outside this bounded mission.

The first explicit syntax spelling is `node receiver : fn function_name`; the
function is resolved semantically, never as an AtomRegistry factory. Existing
provider nodes retain their spelling. `flowmini.graph_syntax` v1 captures node
roles, implementation references and full wire endpoints with source provenance.
It is syntax evidence, not authorization or canonical Graph IR. Native export
continues to refuse graphs until semantic validation and execution are admitted.

## Alternatives checked

- Renaming the built-in pager or moving its C++ into a provider leaves application
  semantics outside Flow and fails Gate 6.
- Calling a function sequentially from main does not preserve `=>` delivery,
  input selection or wire/signal identity and is prohibited by the mission.
- Existing callable v2 does not connect functions to AtomRegistry or ports.
- Implementing a general stateful/join-capable receiver now would select broader
  public semantics without a documented activation contract.

Implement the approved contract through durable frontend,
semantic, execution and lowering artifacts; add independently replayed boundary
tests; then express pager navigation in Flow and remove the built-in algorithm
after equivalent positive, negative, order and native execution coverage passes.
Keep graph projection refusal until that full route is admitted.

## Verified implementation boundary — 2026-09-07

Frontend capture and semantic receiver analysis now preserve function/parameter
identity, source provenance and explicit `in`/`out` port mapping. Receiver-to-
receiver wires require matching declared types. Wrong ports, unconnected inputs,
missing/ambiguous functions, invalid roles and duplicate graph identities receive
specific diagnostics even while execution remains refused. Provider port contracts
and executable activation frames are still required before native admission.

Compatibility runtime fan-out now records a separate delivery identity alongside
wire and shared output-signal identities. Tests verify one producer invocation,
distinct deliveries, and no successful output from a failed activation. This
does not yet prove source-function execution or fresh function-local storage.

## Compatibility receiver execution — 2026-09-07

The compatibility interpreter now executes `node name : fn function_name` for
`int`, `Bool`, and `c_string` payloads. The function's existing placement/control-
flow operations execute in a new record and runtime instance for each delivery.
Only a single successfully captured result becomes an outer output activation;
fan-out retains that signal and never re-executes the function. Internal signal
and delivery identities are scoped to the incoming delivery. Missing conditional
results cannot reuse an earlier activation's result. Cyclic receiver graphs,
producer bodies, wrong roles/ports/types, and unsupported carriers fail explicitly.

This is the compatibility parser's existing function syntax, including `-> return`;
it does not extend that parser to native `return`, `guard`, or `when` statements.
Native source graph execution and its full approved expression surface are still
unfinished. Legacy FlowIR export explicitly refuses receiver frames to avoid
silently omitting their bodies. The durable frontend syntax remains available.
`source_receiver_frames` covers repeated input, fresh state, fan-out, failure,
carrier conversion boundaries, forward references, and hostile connections.

## Durable graph evidence — 2026-09-07

Frontend graph syntax now retains literal policies as well as explicit nodes,
wires, endpoint locations and implementation references. Flowanalyst embeds those
facts and receiver resolutions as `flowcore.source_graph` v1 in the lowering plan.
The standalone Flowcontracts reader validates this non-executable evidence;
execution consumers refuse it independently of an outer artifact's status. This
prevents scalar projection when captured reports are altered. Native provider
contracts, executable graph planning and activation lowering remain unfinished.

## Native scalar activation — 2026-09-07

Explicit graph v2 now admits selected zero-argument external startup providers
and source-defined scalar receivers. Flowparallel's runtime-owned deterministic
activation-record FIFO keeps
wire, port, signal and delivery identity; LLVM invokes each receiver once per
delivery with a fresh native frame. Successful fan-out reuses the output value.
Missing function results are rejected, and arithmetic failure publishes a
structured activation diagnostic without a normal output. See
[native graph provider selection](native-graph-provider-map.md) for exact commands,
runtime linking and scope. Aggregate receivers remain outside this scalar surface;
this checkpoint does not broaden the approved activation contract.

## Flow-owned pager migration — 2026-09-07

The pager now uses ordinary native source receivers for navigation and rendering.
Selected input providers return one startup status and retain an immutable raw
batch. Explicit accessor capabilities expose lines and raw commands; all command
classification, page bounds, page extraction and output text live in Flow.
The terminal provider acquires ncurses, reads a bounded raw-key batch, and closes
it before returning. The default is one key, with final rendering after the batch;
this is not a streaming or continuously redrawing pager. C++ pager navigation and
rendering atoms have been removed after native positive/negative coverage passed.
