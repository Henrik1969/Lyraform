# Flowcontracts

`source_operation_coverage.hpp` validates the versioned statement-accounting
evidence carried by canonical lowering plans. This prevents a ready staged
execution path from treating omitted source behavior as an implementation
detail.

Flowcontracts is the public, compiler-stage-independent artifact contract
component introduced during the Flowcore v0.28 lineage and retained as a
Lyraform compiler-stage-independent contract surface.

Current surfaces:

- strict complete-input JSON parsing;
- duplicate object-key rejection;
- signed 64-bit integer identity and range preservation;
- finite floating-point values;
- complete JSON string escapes, including Unicode surrogate pairs;
- JSON-path validation diagnostics;
- deterministic serialization with lexicographically ordered object keys;
- typed artifact headers;
- typed `flowanalyst.semantic_report` v1 consumption for Flowparallel;
- typed `flowparallel.execution_plan` and
  `flowparallel.graph_provider_decision` v1 consumption for Flowoptimize;
- lowering-plan identity and matrix dimension/coordinate validation;
- canonical `lyraform.disposition_fact` v1 validation for the bounded proven
  guard-transition and `Outcome<Text,TextFailure>` slices, including operation,
  type, completion, commit, route, proof, owner, exact branch/value/disposal or
  guard linkage, and provenance consistency;
- captured `flowcore.bootstrap_seed` v1 tool/source/provider evidence;
- dependency-closed `flowcore.bootstrap_gap_inventory` v1 staged-bootstrap
  evidence;
- canonical `flowcore.target_policy` v1 validation for backend, architecture,
  ABI, capability, resource, lifecycle, evidence, and fallback authority;
- canonical `flowcore.backend_lowering_artifact` validation, including selected
  target identity and exact external-operation/authorization-set equality;
- independently invocable `flowvalidate` support for every main-chain artifact,
  stable valid/invalid/blocked/unsupported exits, machine or human diagnostics,
  source attribution, and canonical JSON round-tripping.
- structural validation for current ABI manifests, both runtime-capability
  variants, and matrix/graph calibration evidence consumed by provider planners.

Current guard and Text-outcome hostile gates use accepted semantic, optimized,
and backend-lowering controls and require a guard/disposition contract reason
for rejection. A backend rejecting a semantic report for the wrong format does
not count as disposition-validation evidence. See the
[consolidation checkpoint](../docs/checkpoints/2026-09-26-post-gate-2-consolidation.md).

`effect_scheduling.hpp` defines the backend-neutral version-1 provider-effect
profile, access, conflict, and schedule facts used by bounded effectful graph
scheduling. The admitted family is deliberately narrow: an exact generated
provider capability may authorize one read-only, infallible, by-value scalar
observation under `concurrent_observation_v1`. Shared validation recomputes
capability linkage, resource identity, pairwise independence, deterministic
serial order, complete-wave join, and no-partial-publication law. A `readonly`
label alone is never concurrency authority.

The generic `lyraform.ownership_transfer` v1 fact records a unique function
return, producer/return/call identities, source/destination owners and functions,
value type, and unchanged obligation identity. `ownership_transfer.hpp` owns
the shared legality checks used by source analysis and serialized projections.
The nested fact is preserved by carrying stages and checked by all seven
consumers: validation, binding, parallel planning, optimization, preparation,
LLVM lowering, and TinyVM lowering. TextOutcome is the first executable carrier;
the shared law has no Text-specific dispatch.

The bounded forwarding projection represents exactly two ordered version-1
transfer facts in `ownership_transfers`. Their adjacent owner, function, and
operation identities must connect, and their value type and obligation identity
must remain unchanged. The existing singular `ownership_transfer` projection
remains the compatible direct-return form.

`failure_consumer.hpp` defines the first carrier-independent declarative
failure-consumer boundary from ADR 0062. `lyraform.failure_consumer` version 1
binds a closed expected-failure payload-type set to exact ordinary function
identities whose semantic input is the canonical `failure_envelope` projection
for the exact routed payload type. The companion
`lyraform.failure_policy_selection` version 1 fact proves that policy selected
only a route authorized for that consumer and failure type. Both contracts are
explicitly `declarative`: they cannot authorize source admission or runtime
execution.

`failure_response_transition.hpp` adds the carrier-independent
`lyraform.failure_response_transition` version 1 refinement. Every route in a
validated set has exactly one successful-completion meaning: `recover`
produces `Success<T>` and closes the original expected-failure obligation;
`transform` produces a linked successor `Failure<E2>`. The response result
type must exactly equal the declared outgoing payload type. Origin commit
evidence remains immutable, response provenance links to the origin, and any
failure or fault of the response attempt remains a separate obligation. The
fact is also strictly `declarative`; it admits no source spelling, producer
wiring, response invocation, graph route, propagation, retry, or backend
execution.

`failure_flow_execution.hpp` supplies the first deliberately narrow executable
projection over those unchanged authorities. `lyraform.failure_flow_plan`
version 1 binds one established producer `Failure<E>`, one explicit typed wire,
one closed consumer route, one policy selection, and one response transition
under `serial_explicit_failure_route_v1`. Dispatch uses the exact resolved
function-symbol identity; there is no textual-name lookup or dynamic nearest
handler. `lyraform.failure_envelope` version 1 carries the original disposition,
obligation, producer attempt, correlation, no-commit evidence, payload, and
provenance read-only into the selected function. A completed execution emits a
separately validated `lyraform.failure_flow_receipt` version 1 proving either
typed recovery with original-obligation closure or typed transformation with a
distinct successor obligation. This is a shared reference execution boundary,
not source admission or LLVM/TinyVM support. Propagation, retry, sinks,
response-attempt failure, faults, mutation, and concurrent failure routing
remain refused.

`failure_flow_closed_set.hpp` generalizes only the cardinality of that
reference projection. `lyraform.failure_flow_plan` version 2 admits one finite
producer failure set when the consumer accepts exactly the same semantic set,
every declared type has one explicit wire and one policy selection, every
authorized route has one response transition, and every response function is
referenced. A type may have several authorized routes, but policy selects
exactly one before execution. The concrete version-1 envelope selects its
prevalidated type entry; execution still dispatches by exact function identity
and emits the unchanged version-1 receipt. Open sets, wildcard/subtype lookup,
dynamic handlers, and source or backend integration remain absent.

`failure_flow_chain.hpp` adds one fixed two-stage reference chain. An explicit
`lyraform.failure_flow_chain` version 1 handoff binds the selected transforming
transition to the next plan's producer disposition, operation, and failure
type. The successor envelope is constructed from the transformation receipt:
it reuses the linked successor obligation, response attempt, response payload,
correlation, no-commit evidence, and response provenance. The second stage
must recover and close that exact obligation. Arbitrary depth, configurable
evidence budgets, compaction, cycles, retry, and general propagation remain
outside this contract.

`failure_response_attempt.hpp` closes one bounded response-attempt failure
case. A selected recovering response may produce one declared expected
failure. That result opens a distinct linked obligation while the original
obligation remains live; it cannot masquerade as successful recovery. One
exact closed-set recovery plan may then consume the response failure and
rejoin the original transition only when its successful payload type exactly
matches the original promised result. The final receipt proves both the nested
obligation closure and the explicit rejoin before closing the original
obligation. Faults, host exceptions, recursively failing responses, and
inferred or type-changing rejoins are refused.

`fault_containment.hpp` supplies the first bounded integrity-fault destination.
`lyraform.fault_containment_plan` version 1 binds one exact `Fault<F>` producer,
typed fault wire, activation-scope containment authority, and policy selection
to `halt_and_quarantine`. The reference executor dispatches by exact authority
identity and accepts only a result proving that the declared activation is
halted and quarantined, normal publication is suppressed, and local
continuation is absent. Its versioned receipt accounts for containment while
preserving the fault type, obligation, correlation, commit evidence, and
provenance; it never claims recovery or success. Other scope kinds,
escalation, restart, repair, and implicit or top-level termination remain
outside this contract.

`failure_response_fault.hpp` binds one selected recovering response to that
containment destination when the response attempt itself produces a declared
`Fault<F>`. The response fault receives a distinct obligation and exact fault
envelope. Its activation is halted and quarantined through the selected
authority; normal publication, continuation, and rejoin are all absent. The
original expected-failure obligation is preserved as `unresolved_contained`
rather than being forged closed. Expected failure, success, obligation aliasing,
and fault recovery are refused on this projection.

ADR 0063 governs the later envelope's evidence lifetime: live evidence remains
must-account until proven closure or transfer, may compact only through a
validated closure receipt, and may never be silently truncated to satisfy a
retention budget. This contract does not yet serialize or execute those epochs.

## Version-1 unknown-field policy

Version-1 artifact objects are additive: unknown fields are retained when the
containing `json::Value` is preserved and otherwise ignored by a consumer that
does not claim their semantics. Unknown fields never satisfy a required field,
select an artifact format, or authorize an operation. Authority-bearing nested
objects are validated through their public typed surface before use.

This policy permits additive evidence fields while failing closed on missing,
malformed, duplicate, unsupported, or conflicting authority.

The component intentionally does not depend on Flowmini AST internals,
Flowlower emitters, provider implementations, or application names.

`flowcore.source_graph` v1 is independently validated, canonical analysis evidence.
It retains complete explicit node/wire endpoints, source locations, literal
provider policies, and resolved receiver identities in `lowering_plan.source_graph`.
Its status is `non_executable`: validation does not authorize providers or admit
native graph execution. Every execution consumer rejects this retained graph even
if an outer status has been changed to ready. Captured graph evidence is tested
without running its producer, with deterministic canonical round-trips and hostile
identity, port, policy, provenance and activation-contract mutations.
