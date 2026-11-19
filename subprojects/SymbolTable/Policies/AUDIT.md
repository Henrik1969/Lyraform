# Lyraform-scoped SymbolTable — governance audit

Observed: 2026-10-17. Scope: `LyraForm/subprojects/SymbolTable`.
Status: structural consolidation; implementation conformance is not certified.
Mission: `development.governance-consolidation`, revision 1.

## Owned boundary

Raw symbol facts are separated from views and semantic law; distinct from standalone SymbolTable v2.

Local policy source: [map](README.md). Machine-readable resolution:
[context.json](context.json). The shared baseline is a declared dependency;
an isolated clone needs access to the referenced policy set or a deliberately
exported policy bundle. Missing shared policy must be reported, not invented.

## Evidence inspected

- [README.md](../README.md) — SHA-256 `01dd5478c7e841a4d22f86b18fa30f02f163be934ec12b227076cd94f9fc426c`.
- [CHANGELOG.md](../CHANGELOG.md) — SHA-256 `4d493ad5c521acedede84a59608b50addfa2baefe99bed073b1ca6233125491e`.

## Development validation surface

No repeatable test command was identified in the inspected top-level documentation. This is a documentation/evidence limit, not a claim that testing does not exist. Select applicable checks before implementation.

## Nested scope accounting

- `LyraForm/subprojects/SymbolTable/SymbolTable_v1` — owned-component; remains under this context without a new independent ownership claim.
- `LyraForm/subprojects/SymbolTable/SymbolTable_v1_1` — owned-component; remains under this context without a new independent ownership claim.

## Consolidation and remaining decisions

- Local AGENTS.md resolves this map and the shared executor model.
- The nearest source repository excludes private mission-state directories.
- Instructions and evidence do not silently approve overrides or broaden operational permissions.
- No local GAP file was established by this scope inspection; this is not full code compliance evidence.

Review this record when the context, instructions, local policies or source
evidence change. Source hashes describe this audit snapshot; future drift must
be reviewed, not silently re-labelled as verified.
