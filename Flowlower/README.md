# Flowlower

Flowlower is the target-lowering sibling in the Lyraform chain after
Flowoptimize. `flowprepare`
publishes the backend-neutral boundary first:

```text
source -> Flowmini -> Flowanalyst -> Flowparallel -> Flowoptimize -> flowprepare -> backend lowerer
```

The first target provider is `llvm`. Profile-free lowering covers scalar and
pointer ABI calls, typed result placement, integer and string values,
expressions, returns, comparisons, checked entry arguments, and selected
structured control-flow plans. The file-copy and terminal examples are chosen
from source-derived plan operations and exact authorized capabilities rather
than application names; their profile-free native error and cleanup laws are
covered by the current pipeline tests.

The lowering report preserves the source path carried by the upstream semantic
and optimization reports. This keeps emitted IR attributable to its source
artifact without making the lowerer depend on Flowmini internals.

`flowcore.backend_lowering_artifact` version 1 is canonical JSON containing the
complete selected target, lowering plan, ABI contracts, external operations,
exact authorization capabilities and optimization provenance. Both
`flowlower` (LLVM) and `flowtinylower` (TinyVM) consume a captured instance from
disk. TinyVM admits the bounded scalar, graph, aggregate, stream, persistent,
parallel, Text, and memory surfaces covered by its parity tests, and returns a
structured unsupported result for features outside that documented boundary.
Direct optimization-report input to `flowlower` remains a temporary corpus
compatibility path and is not the public backend boundary.

```sh
flowprepare --binding-report binding.json --target cli optimization.json > lowering.json
flowvalidate --canonical lowering.json
flowlower --emit-llvm output.ll lowering.json
flowtinylower lowering.json output.tvm
```

Multiplexed reports are preserved through Flowparallel and Flowoptimize. When
multiple named targets exist, Flowlower requires explicit selection:

```sh
flowlower --target cli < optimization-report.json
```

An absent or unknown target is rejected. A report without named targets keeps
the compatibility default `main`. This establishes target selection as a
separate artifact boundary. For a generic empty lowering plan, independent
target artifact emission is proven with separate
`--emit-llvm` paths. Each output contains an attributable target marker and
the lowering report records `artifact.target_specific: true`. Unsupported
target profiles remain blocked.

Report-only invocations validate the optimization envelope and lowering-plan
authority, including plan readiness, versions, operand arrays and operation
identity. They cannot bypass those checks by omitting `--emit-llvm`. Contract
failures return a structured `FLOWLOWER_CONTRACT` diagnostic with a JSON artifact
path; emission/CLI refusals return `FLOWLOWER_REFUSAL`. A ready report without
an emitted artifact is still boundary validation, not native execution evidence.

With `--diagnostics json`, malformed or missing input is reported on stderr as
`FLOWLOWER_INPUT_INVALID` at the `input` stage, with empty stdout and no
artifact disposition. This keeps hostile input outside the lowering artifact
boundary while preserving the human-readable compatibility path.
LLVM IR is rendered completely before publication, written and synchronized
through a private sibling file, and atomically renamed over the requested path
only after a successful close. Write, sync, close, or rename failure preserves
an existing destination and returns `FLOWLOWER_OUTPUT_FAILURE` at stage
`output`; parent-directory crash durability and abrupt-death orphan cleanup are
not claimed.

The companion `flowprepare` and `flowtarget` tools use the same structured
input boundary. Missing artifacts, incomplete options, unavailable policies,
and invalid target names return `FLOWPREPARE_INPUT_INVALID` or
`FLOWTARGET_INPUT_INVALID` at stage `input`, with empty stdout and
`disposition: no_artifact`. Validly parsed artifacts that violate their
versioned contract remain separately classified as `*_CONTRACT_FAILURE`.
