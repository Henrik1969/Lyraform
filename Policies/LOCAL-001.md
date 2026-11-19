# LOCAL-001 — Lyraform and Igor: policy scope

- Identifier: `LyraForm/LOCAL-001`
- Version: 1.1
- Status: active policy map and scope record; not a conformance certificate
- Owner: project maintainer; Henrik Sørensen is accountable for this workspace organization
- Recorded/effective: 2026-10-17
- Parent: [GOV-001 v1.1](../../Policies/rules/GOV-001.md); the complete [shared baseline index v1.3](../../Policies/HANDBOOK.md) applies according to each rule's scope.
- Decision provenance: Henrik requested explicit project policy files, upward references, provenance and evidence on 2026-10-17. Authorization covers organization and truthful scope records; it does not approve unresolved exceptions.
- Local override: none authorized by this record.

## Purpose and scope

Language semantics remain owned by accepted language law. Compiler missions and current dirty work are not changed by this policy organization.

## Applicable clusters

- [Libraries and public interfaces index v1.0](../../Policies/capabilities/libraries.md)
- [Command-line tools index v1.0](../../Policies/capabilities/cli.md)

For C++ source, consult [C++ supplement v1.1](../../Policies/languages/CPP.md). Its modular scaffold is conditional on adopting that profile; this map does not claim adoption or compliance with it.

## Local wording and review

Inherit applicable shared rules without duplicating their requirements. Preserve
the source-owned boundaries identified above. Any local change must be recorded
in a separate policy file with the exact affected parent and evidence. A known
mismatch remains a gap until explicitly resolved; no implicit waiver applies.

Review this map when parent versions, capability scope or source contracts
change. Validate implementation claims separately; no build or runtime behavior
was tested for this documentation-only mapping.

## Evidence

- [Flowcore/README.md](../../Policies/history/2026-10-17-lyraform-placement/Flowcore/README.md) — observed 2026-10-17; SHA-256 `42f43ba83b84bf0cf133e374a2de546de6766f616b4470e1b9375ccaafac5568`. Preserved pre-placement source.
- [Flowcore/AGENTS.md](history/2026-10-17-agent-neutral/AGENTS.md) — observed 2026-10-17; SHA-256 `9518897d015eff8303d917e4738c81a2d6477220622fb4c50d55cdf497bd7784`.
- [Flowcore/CMakeLists.txt](../../Policies/history/2026-10-17-capabilities/LyraForm/CMakeLists.txt) — observed 2026-10-17; SHA-256 `79706ca075f4f989413aba7b21db2ee26a411ba1827efb6aae4c71e2ba669d9a`. Pre-rename evidence preserved.

## Revision history

- 2026-10-17: created scoped map from existing evidence; no semantic override or maturity promotion.

- 2026-10-17 (v1.1): shared work/state/executor definitions adopted through the project policy map; existing live records and semantic contracts remain unchanged.

Parent review, 2026-10-17: C++ supplement v1.1 sets four-column indentation and tab width; this map points to the revised shared rule. Implementation conformance remains separately assessed.
