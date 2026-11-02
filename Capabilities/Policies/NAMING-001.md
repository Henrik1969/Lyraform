# NAMING-001 — Capability implementation directory names

- Identifier: Lyraform/Capabilities/NAMING-001
- Version: 1.0
- Status: active owner-authorized naming and placement ruling
- Owner: Henrik Sørensen
- Effective: 2026-10-17
- Parents: [GOV-001 v1.1](../../../Policies/rules/GOV-001.md), [ENG-005 v1.1](../../../Policies/rules/ENG-005.md), [PLACEMENT-001 v1.0](../../Policies/PLACEMENT-001.md)
- Authority: Henrik explicitly approved Capabilities and the seven proposed component names in this conversation.

## Scope and ruling

The implementation collection is `LyraForm/Capabilities/`, replacing the
misleading `Frankencore/Core` nesting. Its directories are Contracts,
LanguageMap, Requirements, Provenance, ConfigResolveAdapter, DebianPackages
and RuntimeDiscovery. These names describe implementations and adapters;
FrankenCore remains the separate architecture/policy authority.

No component is merged with another or declared independently packaged by this
naming change. Existing public namespaces, include names, CMake aliases,
binaries and serialized identifiers remain compatibility interfaces. Their
migration would need separately scoped work and compatibility evidence.

## Evidence and review

[Move map](../../../Policies/missions/2026-10-17-capabilities-health/moves.json)
and [component review](../../../Policies/reviews/2026-10-17-frankencore-implementations/README.md)
record purpose, source evidence and prior/current paths. Before-edit navigation
and build references are preserved under Development's
`Policies/history/2026-10-17-capabilities/`.

Review when a component changes responsibility, is extracted, or a public
compatibility identifier is deliberately migrated. Preserve the old identity
and consumer evidence. 2026-10-17 v1.0: owner-approved directory naming only.
