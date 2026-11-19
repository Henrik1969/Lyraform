# Lyraform component Flowparallel — governance audit

Observed: 2026-10-17. Scope: `LyraForm/Flowparallel`.
Status: structural consolidation; implementation conformance is not certified.
Mission: `development.governance-consolidation`, revision 1.

## Owned boundary

Component-specific contracts stay in their existing source documentation. Lyraform standing development rules apply; this record does not invent independent umbrella ownership.

Local policy source: [map](README.md). Machine-readable resolution:
[context.json](context.json). The shared baseline is a declared dependency;
an isolated clone needs access to the referenced policy set or a deliberately
exported policy bundle. Missing shared policy must be reported, not invented.

## Evidence inspected

- [README.md](../README.md) — SHA-256 `171e67bb87a75df64da8df4ebd94de14bd534166636a750c4ae14f5eec0a35ba`.
- [CMakeLists.txt](../CMakeLists.txt) — SHA-256 `9f147eb3250f22e3b4d1e93f90a54516351a5544f536a157c18b15ea0b33da6d`.

## Development validation surface

The following commands were found in the project documentation. They were
not executed as product tests during this governance-only audit. Use the source
document for its working directory, prerequisites and full command.

- `README.md:154: cmake --build build --target flowparallel_cuda_execute`

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
