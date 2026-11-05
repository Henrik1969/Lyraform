# ADR-0079: Provider-owned results carry cleanup through ABI boundaries

Status: accepted bounded implementation slice — 2026-11-05

## Decision

Provider-owned `Text` results must publish their cleanup authority at the ABI
boundary. The plan-v2 `result_resource` must contain the exact resource facts
for the provider return type, including:

- `type: Text`;
- `ownership: owned`;
- value lifetime and non-nullability as declared by the ABI type contract;
- `opaque` as declared by the ABI type contract;
- `cleanup_capability: flow_text_dispose`.

The semantic producer also publishes the destination owner symbol when the
provider result is assigned to a value. A provider result produced inside a
non-entry function must be transferred by a direct `return value`. The current
Flowbind proof admits one bounded transfer boundary: every static call site
must receive the returned owner, and each entry-function caller must dispose
that specific owner. Cleanup is matched by owner identity, not merely by the
capability name.

The lowerer and TinyVM backend admit only this exact owned-`Text` contract for
the current provider-result slice. Missing result metadata, missing owner
identity, missing direct transfer, or an unresolved/wrong cleanup capability
is refused before execution.

## Boundary

This is not general resource analysis or implicit unwinding. It does not admit
arbitrary ownership graphs, multiple forwarding functions, asynchronous
transfers, exceptional cleanup paths, or cleanup inside loops without a
lifetime proof. It also does not change the separate source-disposition
response ownership contract. Provider failure remains a runtime fault in the
bounded `text_runtime` specimen; an admitted result still requires an explicit
source cleanup action.

## Rationale

An ABI result can cross a function boundary before its cleanup obligation is
visible at the final caller. Requiring the producer, transfer edge, receiving
owner, and cleanup action to agree keeps the ownership law inspectable through
semantic analysis, binding, lowering, and both runtime backends. It prevents a
backend from silently treating a provider-owned pointer as borrowed or from
accepting a cleanup capability without proving which result it disposes.

