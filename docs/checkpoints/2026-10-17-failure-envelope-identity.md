# Failure-envelope identity preservation checkpoint

Branch: `experiment/configresolve-lyraform`. Mission:
`lyraform.disposition-graph-preservation`, revision 1. Partial progress; the
mission and overall ConfigResolve port remain active.

Source semantic analysis now emits each response's exact parameter symbol and
marks callable envelope parameters with their immutable input projection.
Shared validation checks the response against its exact ordinary definition,
parameter identity, failure payload type, projection and result type.

Disposition graph version 2 requires envelope parameter witnesses for the closed
response set and preserves the selected response's parameter identity,
immutable projection and incoming failure type. Binding, parallel scheduling,
optimization and preparation retain this graph. Historical version 1 remains
valid only with its historical non-executable claim; neither version grants
execution. Version markers must agree with the exact projected shape.

Hostile tests exposed a binding-boundary gap: Flowbind had accepted a mutated
callable signature despite the graph witness. It now invokes the same shared
disposition-fact validator as other consumers. A source topology also cannot
omit its operation facts to bypass callable validation.

Verification: build succeeded; focused 3/3 and full 191/191 CTests passed. The
expanded graph test covers 136 hostile mutations across four stages and two
policy selections, including missing/changed envelope evidence, contradictory
callable signatures and removed operation authority. Positive checks cover
identity preservation, historical version-1 inspection, ordinary envelope
forwarding and continued backend refusal of unsupported execution. The
ConfigResolve 96-case integer probe retained native/TinyVM parity and its
renaming/hostile-oracle controls. Logs are under
`experiments/configresolve/evidence/envelope-identity/`.

Authority: accepted ADR 0062/0066/0067. No application-specific compiler dispatch,
new source meaning or runtime shortcut was added. No commit or merge performed.

Next execution dependency: both backends currently load callable parameter
carrier spellings without an envelope representation. Native `TextFailure`
carrier lowering is `i32`, which must not be used as the ABI of `failure
TextFailure`. Implement and verify a generic immutable runtime envelope retaining
operation/disposition/obligation, attempt/correlation, commit evidence and
provenance before admitting source recovery execution. Source transformation
chains and explicit quarantine execution are still unfinished. Do not mark
Gate 4, the complete ConfigResolve port or the overall language goal achieved.
