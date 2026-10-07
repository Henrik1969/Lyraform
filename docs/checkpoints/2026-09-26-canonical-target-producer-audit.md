# Canonical target producer audit — Stage 2B

Date: 2026-09-26.

## Objective

Determine whether target identity, member-chain resolution, destination type,
assignability, or provenance is independently reconstructed after canonical
target analysis.

This was an authority audit, not permission to add member/index execution or
aggregate semantics.

## Finding

`flowanalyst::target::resolve` is the sole producer of
`lyraform.target_fact`. It receives the already-established operation identity
and, for bounded scalar identifier targets, the destination identity and type
from the scalar authority connected at Stage 2A.

Shared Flowcontracts validation enforces:

- operation and statement identity equality;
- identifier base identity equality with `result_symbol_id`;
- identifier identity/type equality with retained scalar evidence;
- member source-expression identity;
- exact member-chain owner, declaration, and resulting type relationships;
- target provenance shape;
- refusal cannot appear in a ready plan;
- field/index targets cannot collapse into scalar `result_symbol_id`;
- field/index execution remains explicitly unsupported.

Flowbind copies target evidence for inspection. Carrying artifacts invoke the
shared scalar/target validation boundary, and Flowlower invokes
`require_executable_targets` before backend lowering. No later stage resolves
member names or types again.

## Decision

No implementation change is justified. Refactoring a single existing producer
would create churn without removing an authority defect. The authority map now
records this bridge as `single_producer_validated`.

## Evidence

- Stage 2A target-preservation and field-path gates: PASS;
- existing hostile target-artifact corpus remains active;
- complete canonical suite after the immediately preceding Stage 2A code
  change: **170/170 PASS**, 81.60 seconds;
- authority and documentation drift guards after this audit: PASS;
- `git diff --check`: PASS.

## Gate

Stage 2B: **PASS**. Target facts already satisfy the Stage 2 producer rule.

Recommended next stage: audit the relationship among guard facts, disposition
facts, and their lowering-operation identities. Implement only if that scout
finds an actual duplicated semantic decision.
