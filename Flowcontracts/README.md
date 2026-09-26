# Flowcontracts

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

The generic `lyraform.ownership_transfer` v1 fact records a unique function
return, producer/return/call identities, source/destination owners and functions,
value type, and unchanged obligation identity. `ownership_transfer.hpp` owns
the shared legality checks used by source analysis and serialized projections.
The nested fact is preserved by carrying stages and checked by all seven
consumers: validation, binding, parallel planning, optimization, preparation,
LLVM lowering, and TinyVM lowering. TextOutcome is the first executable carrier;
the shared law has no Text-specific dispatch.

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
