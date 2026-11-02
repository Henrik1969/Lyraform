# Recovered VM prototype — governance audit

Observed: 2026-10-17. Scope: `LyraForm/subprojects/TinyVM`.
Status: structural consolidation; implementation conformance is not certified.
Mission: `development.governance-consolidation`, revision 1.

## Owned boundary

Recovered prototype and bounded integration retain their declared limitations and provenance.

Local policy source: [map](README.md). Machine-readable resolution:
[context.json](context.json). The shared baseline is a declared dependency;
an isolated clone needs access to the referenced policy set or a deliberately
exported policy bundle. Missing shared policy must be reported, not invented.

## Evidence inspected

- [ISA-V2.md](../ISA-V2.md) — SHA-256 `e0666467d1b59946f1e4ea528e56d2baa80d5ced57187939bbabe4fbcca56b5d`.
- [README.md](../README.md) — SHA-256 `310ab664db9366759e2009d76ce2cd84810168c01eaa0ae52fb093723b30e35f`.
- [CMakeLists.txt](../CMakeLists.txt) — SHA-256 `9506d4c834116c61bbeddf9e2716cb197c5645098c3564af6854318bab07a024`.
- [ARTIFACT-V2.md](../ARTIFACT-V2.md) — SHA-256 `3a6ce0c56fd5053a9c6f19e4eafce82f26e26013ed0dd7270480b92d201dcb3d`.
- [ARTIFACT-V1.md](../ARTIFACT-V1.md) — SHA-256 `ca9848b15f7a0a6bf552ee819e2873a7c8f4bdbbf3a03213733b8031f31c8530`.
- [ISA-V1.md](../ISA-V1.md) — SHA-256 `a65fd5471c265f341802dce33d4bc5de8bf3513a052c2fddf118a314840c9c0c`.

## Development validation surface

The following commands were found in the project documentation. They were
not executed as product tests during this governance-only audit. Use the source
document for its working directory, prerequisites and full command.

- `README.md:40: cmake --build build/tinyvm-recovered`
- `README.md:41: ctest --test-dir build/tinyvm-recovered --output-on-failure`
- `README.md:55: cmake --build build/tinyvm-bench --target tinyvm_dispatch_benchmark`

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

2026-10-17 record unit review: added an uninstalled test-only compiler/VM adapter
that preserves blocked source plans and checks both engines and independent
record field storage. Public compiler admission is unchanged. Prior CMake hash:
`0a08a38a2100072448b0b2a9494529362e1c4ac645d619eb28160cda2dd5dd34`.
