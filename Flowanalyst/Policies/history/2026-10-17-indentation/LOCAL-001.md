# LOCAL-001 — Lyraform component Flowanalyst: policy scope

- Identifier: `Flowcore/Flowanalyst/LOCAL-001`
- Version: 1.1
- Status: active policy map and scope record; not a conformance certificate
- Owner: project maintainer; Henrik Sørensen is accountable for this workspace organization
- Recorded/effective: 2026-10-17
- Parent: [GOV-001 v1.1](../../../Policies/rules/GOV-001.md); the complete [shared baseline index v1.3](../../../Policies/HANDBOOK.md) applies according to each rule's scope.
- Decision provenance: Henrik requested explicit project policy files, upward references, provenance and evidence on 2026-10-17. Authorization covers organization and truthful scope records; it does not approve unresolved exceptions.
- Local override: none authorized by this record.

## Purpose and scope

Component-specific contracts stay in their existing source documentation. Lyraform standing development rules apply; this record does not invent independent umbrella ownership.

## Applicable clusters

- [Libraries and public interfaces index v1.0](../../../Policies/capabilities/libraries.md)

Containing-project policy: [LOCAL-001 v1.1](../../Policies/LOCAL-001.md). This is an explicit relationship, not an inference from folder nesting.

For C++ source, consult [C++ supplement v1.0](../../../Policies/languages/CPP.md). Its modular scaffold is conditional on adopting that profile; this map does not claim adoption or compliance with it.

## Local wording and review

Inherit applicable shared rules without duplicating their requirements. Preserve
the source-owned boundaries identified above. Any local change must be recorded
in a separate policy file with the exact affected parent and evidence. A known
mismatch remains a gap until explicitly resolved; no implicit waiver applies.

Review this map when parent versions, capability scope or source contracts
change. Validate implementation claims separately; no build or runtime behavior
was tested for this documentation-only mapping.

## Evidence

- [Flowcore/Flowanalyst/README.md](../README.md) — observed 2026-10-17; SHA-256 `bb482007e16110268c6b652d49a0a03309f64b1a625561d57b1f7c94a6bc2e40`.
- [Flowcore/Flowanalyst/CMakeLists.txt](../CMakeLists.txt) — observed 2026-10-17; SHA-256 `e6cb575e89f5e436d429375d138df8b3958d6782a203d3940d433ae58a18c7e2`.

## Revision history

- 2026-10-17: created scoped map from existing evidence; no semantic override or maturity promotion.

- 2026-10-17 (v1.1): shared work/state/executor definitions adopted through the project policy map; existing live records and semantic contracts remain unchanged.
