# LOCAL-001 — Lyraform component Flowtools: policy scope

- Identifier: `Flowcore/Flowtools/LOCAL-001`
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

- [Flowcore/Flowtools/README.md](../README.md) — observed 2026-10-17; SHA-256 `71410c2e975e19881d39fa8f2cf4df052b1eeb6050e0073835703a97563f5f81`.
- [Flowcore/Flowtools/CMakeLists.txt](../CMakeLists.txt) — observed 2026-10-17; SHA-256 `580e9fdbac03aa686b681729a493f813c754b5462450052187680acb9db32e2f`.

## Revision history

- 2026-10-17: created scoped map from existing evidence; no semantic override or maturity promotion.

- 2026-10-17 (v1.1): shared work/state/executor definitions adopted through the project policy map; existing live records and semantic contracts remain unchanged.
