# Current Lyraform version

```text
project:        Lyraform
implementation: Lyraform/compiler
toolchain:      Igor
authority:      main
lineage:        v0.29 reusable native language chain
status:         experimental / unstable / not production-ready
```

The current verified boundary includes generic scalar/control-flow lowering,
exact generated ABI evidence, durable source graphs, fresh receiver frames,
Flow-owned paging, canonical guard/disposition authority, bounded tagged
outcome accounting, and uniform ownership transfer through at most one
forwarding owner. The first typed failure-consumer and policy-selection
contracts are declarative only; response-transition execution remains refused.
Accepted disposition-envelope evidence uses bounded epochs and proven closure
before compaction. See the
[current-status landing page](../docs/current-status.md) and its linked latest
checkpoint for exact revisioned evidence.

The `flowmini` executable name and `flowmini.*` artifact namespace remain where
existing scripts and serialized compatibility contracts require them. The
legacy Flowmini parser, `ModuleSpec`, and direct runtime are deprecated,
non-canonical behavior oracles. They cannot admit Lyraform source or define
canonical semantics, artifacts, plans, or execution.

Future work remains explicitly scoped: additional forwarding hops and owned parameters,
response-transition integration, runtime guard routes, failure policy sinks,
evidence storage/reclamation, broader aggregate and streaming
graph activation, additional targets, permanent writable-storage syntax,
broader standard-library coverage, optimizer expansion, and self-hosting.
