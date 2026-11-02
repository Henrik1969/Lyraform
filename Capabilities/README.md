# Lyraform capability implementations

This collection provides capability implementations and integration adapters
used by Lyraform. It is not the FrankenCore architecture/policy authority,
which remains in Development's separate `FrankenCore/` project.

| Component | Responsibility |
|---|---|
| Contracts | Shared value types, validation and admission logic |
| LanguageMap | Language-map moniker resolution; not the language parser |
| Requirements | Restricted dotted-version requirement matching |
| Provenance | IDs, mutation/error history, persistence and JSON projections |
| ConfigResolveAdapter | Optional integration with ConfigResolve's public C ABI |
| DebianPackages | dpkg/APT package and repository fact discovery |
| RuntimeDiscovery | Linux CPU/memory and optional CUDA capability discovery |

These are distinct implementations, not automatically independent repositories.
Some contain platform-specific providers: DebianPackages and RuntimeDiscovery
must not be described as a universally substrate-neutral core. Provenance
currently uses Flowcontracts JSON and POSIX/filesystem persistence. The adapter
is disabled unless a ConfigResolve root is explicitly configured. LanguageMap
and ConfigResolveAdapter use Contracts.

Flowtools includes the components in the root build and owns most reference
probes/tests. Flowparallel consumes the runtime-capability snapshot. Preserve
those dependencies when changing or extracting components.

See [local policy](Policies/README.md) and the
[component review](../../Policies/reviews/2026-10-17-frankencore-implementations/README.md).
The review's old paths are historical; the
[move map](../../Policies/missions/2026-10-17-capabilities-health/moves.json)
resolves them. Existing `frankencore` namespaces, include prefixes, CMake aliases,
binary names and serialized identifiers are compatibility interfaces; the
folder rename does not change their public meaning.
