# LOCAL-001 — Lyraform-scoped TokenTree: policy scope

- Identifier: `LyraForm/subprojects/TokenTree/LOCAL-001`
- Version: 1.1
- Status: active policy map and scope record; not a conformance certificate
- Owner: project maintainer; Henrik Sørensen is accountable for this workspace organization
- Recorded/effective: 2026-10-17
- Parent: [GOV-001 v1.1](../../../../Policies/rules/GOV-001.md); the complete [shared baseline index v1.3](../../../../Policies/HANDBOOK.md) applies according to each rule's scope.
- Decision provenance: Henrik requested explicit project policy files, upward references, provenance and evidence on 2026-10-17. Authorization covers organization and truthful scope records; it does not approve unresolved exceptions.
- Local override: none authorized by this record.

## Purpose and scope

Reusable structural bridge, scoped to this containing project; not a parser or semantic checker.

## Applicable clusters

- [Libraries and public interfaces index v1.0](../../../../Policies/capabilities/libraries.md)

Containing-project policy: [LOCAL-001 v1.1](../../../Policies/LOCAL-001.md). This is an explicit relationship, not an inference from folder nesting.

For C++ source, consult [C++ supplement v1.1](../../../../Policies/languages/CPP.md). Its modular scaffold is conditional on adopting that profile; this map does not claim adoption or compliance with it.

## Local wording and review

Inherit applicable shared rules without duplicating their requirements. Preserve
the source-owned boundaries identified above. Any local change must be recorded
in a separate policy file with the exact affected parent and evidence. A known
mismatch remains a gap until explicitly resolved; no implicit waiver applies.

Review this map when parent versions, capability scope or source contracts
change. Validate implementation claims separately; no build or runtime behavior
was tested for this documentation-only mapping.

## Evidence

- [Flowcore/subprojects/TokenTree/README.md](../README.md) — observed 2026-10-17; SHA-256 `5b08e93918ebd4789f5f2a753e794ae844d6f80c468d55f5468d2ed0ba72673c`.

## Revision history

- 2026-10-17: created scoped map from existing evidence; no semantic override or maturity promotion.

- 2026-10-17 (v1.1): shared work/state/executor definitions adopted through the project policy map; existing live records and semantic contracts remain unchanged.

Parent review, 2026-10-17: C++ supplement v1.1 sets four-column indentation and tab width; this map points to the revised shared rule. Implementation conformance remains separately assessed.
