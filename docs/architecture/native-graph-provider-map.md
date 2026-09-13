# Native graph provider selection

`flowcore.graph_provider_map` v1 is an explicit, non-authorizing selection file.
Flowanalyst accepts it with `--graph-providers path`. An entry maps the arbitrary
implementation name already present in a graph declaration to one qualified
external function in the captured frontend symbol table:

```json
{"format":"flowcore.graph_provider_map","version":1,"providers":[
  {"implementation":"input.batch","source_callable":"host.read_batch",
   "activation":"startup_once","output_port":"out"}
]}
```

The bounded adapter requires a producer node, a zero-argument external function,
and one returned value on `out`. Startup invokes a producer once, consistent with
the existing serial interpreter's initial producer activation. It defines no
streaming, repeated output, asynchronous activation, persistent receiver state or
new source-function trigger. Provider behavior remains external and explicitly
selected; a source receiver still obeys the owner's fresh-single-input contract.

Selection resolves source callable identity, provider/library/native symbol,
convention, carrier/effect facts and generated evidence independently. Neither a
factory-like name nor successful resolution grants capability authority.
`source_graph.providers` retains resolved facts and `provider_selection` retains
the selection file for independent validation. Import aliases belong to the
source reference, never the native symbol. Unused selections have no activation.

## Executable scalar graph boundary

Request `--lowering-plan-version 2 --graph-plan-version 2` with the explicit
provider map to publish `flowcore.source_graph` v2. Admission requires complete
producer/receiver resolution, compatible port types, one native entry, no graph
cycles, and no provider policies on this initial scalar adapter. Version 1
remains non-executable evidence and cannot be upgraded by changing its status.
External callable catalog entries retain their provider tuple; producer identity
must match that catalog and an exact Flowbind capability grant.

Flowparallel publishes `flowcore.graph_schedule` v1 with FIFO delivery per root
in source order. Every step records activation, input/output signals, delivery,
wire, full endpoints and its input activation reference. The one-successful-output
contract permits static scheduling: fan-out reuses the originating result rather
than invoking the function again. The admitted expansion is bounded at 65,536
activations. The native scalar carrier set includes the exact `c_int`, `c_long`,
`c_ulong`, `c_size_t`, and `c_string` provider results admitted by the graph
contract; `c_long` remains an LLVM `i64` payload through receiver calls and
fan-out. Optimization and backend preparation preserve and independently
validate the complete schedule against the graph. Multiple independent startup
roots are processed as separate FIFO sequences; each root creates fresh receiver
activations and receives distinct output-signal identities. Scheduling remains
separate from the fresh-single-input receiver contract.

LLVM emits actual function invocations with fresh native stack storage. Link the
emitted object with `libflowgraph_runtime.so` (installed under `lib/flowcore`) and
the explicitly selected provider libraries. Lowering reports declare that runtime
requirement. `FLOWCORE_GRAPH_TRACE=1` produces JSON activation/output records;
unconnected outputs always produce a drop diagnostic. Records retain source and
wire provenance. Division by zero or signed division overflow produces a
`flowcore.graph_failure` with the current activation and operation identity, exits
70, and never publishes a normal receiver result or invokes its fan-out.

The `native_source_graph` gate generates a new provider after the tools are built,
checks unchanged compiler hashes, executes repeated receivers and fan-out, changes
input selection and wire order, verifies an unused selection has no LLVM effect,
and rejects missing grants, forged schedules, cycles and mutated provider identity.
This initial native graph surface is Linux x86-64 and scalar; aggregate payloads,
provider streaming/policies and native TinyVM graph execution remain unsupported.
The pager uses this scalar graph with an input status and selected page as wire
payloads; its input provider owns an immutable raw data batch accessed through
explicit read-only capabilities. Navigation, bounds, key interpretation, failure
selection and rendering belong to Flow. No persistent receiver state is used.

The runtime also exports the explicitly bindable C capability
`flow_graph_raise(c_int code): c_int`. It terminates the active graph invocation
with a structured `source_failure`, the source-selected code, current operation
and activation provenance, and exit 70. Calling it requires the same generated
provider evidence and explicit grant as any other external capability. Its
implementation never returns a normal result. It permits Flow-owned validation
without encoding application-specific error cases in the compiler or runtime.
