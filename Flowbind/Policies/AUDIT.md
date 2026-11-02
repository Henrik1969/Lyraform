# Lyraform component Flowbind — governance audit

Observed: 2026-10-17. Scope: `LyraForm/Flowbind`.
Status: structural consolidation; implementation conformance is not certified.
Mission: `development.governance-consolidation`, revision 1.

## Owned boundary

Component-specific contracts stay in their existing source documentation. Lyraform standing development rules apply; this record does not invent independent umbrella ownership.

Local policy source: [map](README.md). Machine-readable resolution:
[context.json](context.json). The shared baseline is a declared dependency;
an isolated clone needs access to the referenced policy set or a deliberately
exported policy bundle. Missing shared policy must be reported, not invented.

## Evidence inspected

- [README.md](../README.md) — SHA-256 `dd46f074fafc5b6d5efa5a220d90c2727c49767e6ea4d28041f52c1e3e01521b`.
- [CMakeLists.txt](../CMakeLists.txt) — SHA-256 `b59955a03474fa7e23b3e263032b83776a49bbba963d41720110bec79377f389`.

## Development validation surface

No repeatable test command was identified in the inspected top-level documentation. This is a documentation/evidence limit, not a claim that testing does not exist. Select applicable checks before implementation.

## Nested scope accounting

No additional unregistered build boundary was discovered beneath this registered context.

## Consolidation and remaining decisions

- Local AGENTS.md resolves this map and the shared executor model.
- The nearest source repository excludes private mission-state directories.
- Instructions and evidence do not silently approve overrides or broaden operational permissions.
- No local GAP file was established by this scope inspection; this is not full code compliance evidence.

Review this record when the context, instructions, local policies or source
evidence change. Source hashes describe this audit snapshot; future drift must
be reviewed, not silently re-labelled as verified.
