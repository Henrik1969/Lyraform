# Lyraform component Lyraform/compiler — governance audit

Observed: 2026-10-17. Scope: `LyraForm/Lyraform/compiler`.
Status: structural consolidation; implementation conformance is not certified.
Mission: `development.governance-consolidation`, revision 1.

## Owned boundary

Component-specific contracts stay in their existing source documentation. Lyraform standing development rules apply; this record does not invent independent umbrella ownership.

Local policy source: [map](README.md). Machine-readable resolution:
[context.json](context.json). The shared baseline is a declared dependency;
an isolated clone needs access to the referenced policy set or a deliberately
exported policy bundle. Missing shared policy must be reported, not invented.

## Evidence inspected

- [README.md](../../../../Policies/history/2026-10-17-lyraform-placement/Flowcore/Lyraform/compiler/README.md) — SHA-256 `4a9d24e494cf04461a7a33fdd13cee17a52d5807e75515a1e8e6fd7e1b22d112`. Preserved pre-placement source.
- [CMakeLists.txt](../CMakeLists.txt) — SHA-256 `ed2ac1e35d2ac36a539e337f33c3fcee5b0842540c820db228ccfba169e3b5e9`.

## Development validation surface

The following commands were found in the project documentation. They were
not executed as product tests during this governance-only audit. Use the source
document for its working directory, prerequisites and full command.

- `README.md:63: cmake --build cmake-build-debug -j20`
- `README.md:69: cmake --build cmake-build-debug --target flowmini_frontend_bundle_tests`
- `README.md:70: cmake --build cmake-build-debug --target flowmini_symbol_projection_tests`
- `README.md:71: cmake --build cmake-build-debug --target flowmini_ast_golden_tests`
- `README.md:72: cmake --build cmake-build-debug --target flowmini_suite`
- `README.md:73: ctest --test-dir cmake-build-debug --output-on-failure`

## Nested scope accounting

- `LyraForm/Lyraform/compiler/examples/apps/flow_less` — consumer-or-test-fixture; remains under this context without a new independent ownership claim.
- `LyraForm/Lyraform/compiler/subprojects/SymbolTable/SymbolTable_v1_1` — owned-component; remains under this context without a new independent ownership claim.
- `LyraForm/Lyraform/compiler/subprojects/testabi` — owned-component; remains under this context without a new independent ownership claim.
- `LyraForm/Lyraform/compiler/subprojects/TokenTree/tokentree_v1` — owned-component; remains under this context without a new independent ownership claim.

## Consolidation and remaining decisions

- Local AGENTS.md resolves this map and the shared executor model.
- The nearest source repository excludes private mission-state directories.
- Instructions and evidence do not silently approve overrides or broaden operational permissions.
- No local GAP file was established by this scope inspection; this is not full code compliance evidence.

Review this record when the context, instructions, local policies or source
evidence change. Source hashes describe this audit snapshot; future drift must
be reviewed, not silently re-labelled as verified.
