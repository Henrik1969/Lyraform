# Lyraform capability implementations and adapters — governance audit

Observed: 2026-10-17. Scope: `LyraForm/Capabilities`.
Status: structural consolidation; implementation conformance is not certified.
Mission: `development.governance-consolidation`, revision 1.

## Owned boundary

Component-specific contracts stay in their existing source documentation. Lyraform standing development rules apply; this record does not invent independent umbrella ownership.

Local policy source: [map](README.md). Machine-readable resolution:
[context.json](context.json). The shared baseline is a declared dependency;
an isolated clone needs access to the referenced policy set or a deliberately
exported policy bundle. Missing shared policy must be reported, not invented.

## Evidence inspected

- [README.md](../../../Policies/history/2026-10-17-capabilities/LyraForm/Frankencore/README.md) — SHA-256 `7b1642e251edba6396030fe4e20602d65bd16f64ac3df78b516792bdc62d62c9`. Pre-rename evidence preserved.

## Development validation surface

No repeatable test command was identified in the inspected top-level documentation. This is a documentation/evidence limit, not a claim that testing does not exist. Select applicable checks before implementation.

## Nested scope accounting

- `LyraForm/Capabilities/Core/Contracts` — owned-component; remains under this context without a new independent ownership claim.
- `LyraForm/Capabilities/Core/Provenance` — owned-component; remains under this context without a new independent ownership claim.
- `LyraForm/Capabilities/Core/Packages` — owned-component; remains under this context without a new independent ownership claim.
- `LyraForm/Capabilities/Core/Runtime` — owned-component; remains under this context without a new independent ownership claim.
- `LyraForm/Capabilities/Core/Policy` — owned-component; remains under this context without a new independent ownership claim.
- `LyraForm/Capabilities/Core/Language` — owned-component; remains under this context without a new independent ownership claim.
- `LyraForm/Capabilities/Core/Requirements` — owned-component; remains under this context without a new independent ownership claim.

## Consolidation and remaining decisions

- Local AGENTS.md resolves this map and the shared executor model.
- The nearest source repository excludes private mission-state directories.
- Instructions and evidence do not silently approve overrides or broaden operational permissions.
- No local GAP file was established by this scope inspection; this is not full code compliance evidence.

Review this record when the context, instructions, local policies or source
evidence change. Source hashes describe this audit snapshot; future drift must
be reviewed, not silently re-labelled as verified.

## Component ownership review

[2026-10-17 implementation review](../../../Policies/reviews/2026-10-17-frankencore-implementations/README.md)
separates reusable contracts, platform providers and integration adapters, with
observed dependencies and placement recommendations. It authorizes no move.

Naming follow-up, 2026-10-17: [NAMING-001](NAMING-001.md) implements the approved component names.
[Development-health verification](../../docs/development/workspace-health.md) records
the fresh build and 189-test suite; no standalone packaging claim is made.
