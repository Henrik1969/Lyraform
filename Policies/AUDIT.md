# Lyraform and Igor — governance audit

Observed: 2026-10-17. Scope: `LyraForm`.
Status: structural consolidation; implementation conformance is not certified.
Mission: `development.governance-consolidation`, revision 1.

## Owned boundary

Language semantics remain owned by accepted language law. Compiler missions and current dirty work are not changed by this policy organization.

Local policy source: [map](README.md). Machine-readable resolution:
[context.json](context.json). The shared baseline is a declared dependency;
an isolated clone needs access to the referenced policy set or a deliberately
exported policy bundle. Missing shared policy must be reported, not invented.

## Evidence inspected

- [DISCLAIMER.md](../DISCLAIMER.md) — SHA-256 `88edc39bfba3261d87cc0c206a21f902bdf3861a8bb92073209210642404c158`.
- [REFERENCES.md](../REFERENCES.md) — SHA-256 `cf9c99b8d110943558dfdf2f7fa20e4c28444ff0a76c159119a74262553482aa`.
- [BRANCH_CONSOLIDATION_2026-09-13.md](../BRANCH_CONSOLIDATION_2026-09-13.md) — SHA-256 `a65b4dc181f53c0e9b1afa97dce7549e66edc76221c6a85544004f8c8fc5476f`.
- [ALPHA-TESTING.md](../ALPHA-TESTING.md) — SHA-256 `0cea08e8271649daecea64232c9ecc775a60a4ea8268e01f5f3932edbf240d58`.
- [LICENSE.md](../LICENSE.md) — SHA-256 `616aca9ed9f84d0ad2c5d9b1324fed253e9310e34993a0f2896ce4d108b19f89`.
- [BACKLOG.md](../BACKLOG.md) — SHA-256 `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`.
- [meta-discusions.md](../meta-discusions.md) — SHA-256 `c31c285cc191f06f455f8eade4a4b29bac4efa4b3c5059f1bfb3637b419dc47b`.
- [LYRAFORM_MIGRATION_2026-09-13.md](../LYRAFORM_MIGRATION_2026-09-13.md) — SHA-256 `f2d31393002b5565756fe8d1fcd2da16b48226533ccb45ba7677f994f2980d19`.
- [ACKNOWLEDGEMENTS.md](../ACKNOWLEDGEMENTS.md) — SHA-256 `29a9f1cadf96a57e95ee10adbf59ba842625d42171b47cfc241d93b70f417f04`.
- [README.md](../../Policies/history/2026-10-17-lyraform-placement/Flowcore/README.md) — SHA-256 `42f43ba83b84bf0cf133e374a2de546de6766f616b4470e1b9375ccaafac5568`. Preserved pre-placement source.
- [CMakeLists.txt](../../Policies/history/2026-10-17-capabilities/LyraForm/CMakeLists.txt) — SHA-256 `79706ca075f4f989413aba7b21db2ee26a411ba1827efb6aae4c71e2ba669d9a`. Pre-rename evidence preserved.
- [CHANGELOG.md](../CHANGELOG.md) — SHA-256 `3c12636c3f9bb3b2a857c383e55f3f2405c34b96b84174aa9df64ca15ff995e7`.
- [LYRAFORM_HOUSEKEEPING_2026-09-14.md](../LYRAFORM_HOUSEKEEPING_2026-09-14.md) — SHA-256 `850f1b7c0fa874e48b633592d192222c2733ad22ca54a5add8e537c069946707`.
- [copilot-instructions.md](history/2026-10-17-consolidation/.github/copilot-instructions.md) — SHA-256 `5a7b7133f098d6b521681241cc217b08785ee8926d3c797ff0e7fe51a7430815`.
- [project-presentation.instructions.md](history/2026-10-17-consolidation/.github/instructions/project-presentation.instructions.md) — SHA-256 `161fa0cac1f02d4263c6c832dd64c16851c758fcb26bc6def9e5527f805f7239`.

## Development validation surface

The following commands were found in the project documentation. They were
not executed as product tests during this governance-only audit. Use the source
document for its working directory, prerequisites and full command.

- `BRANCH_CONSOLIDATION_2026-09-13.md:176: cmake --build build -j`
- `BRANCH_CONSOLIDATION_2026-09-13.md:177: ctest --test-dir build --output-on-failure`
- `BRANCH_CONSOLIDATION_2026-09-13.md:434: cmake --build build -j4                                       PASS (146/146)`
- `BRANCH_CONSOLIDATION_2026-09-13.md:435: ctest --test-dir build --output-on-failure                     PASS (81/81)`
- `BRANCH_CONSOLIDATION_2026-09-13.md:443: cmake --build /tmp/flowcore-flowmini-subbuild -j4              PASS (34/34)`
- `BRANCH_CONSOLIDATION_2026-09-13.md:458: cmake --build /tmp/flowcore-flowmini-asan -j4                               PASS (146/146)`
- `BRANCH_CONSOLIDATION_2026-09-13.md:460:   ctest --test-dir /tmp/flowcore-flowmini-asan --output-on-failure          PASS (81/81)`
- `README.md:105: cmake --build /tmp/lyraform-build`
- `README.md:106: ctest --test-dir /tmp/lyraform-build --output-on-failure`

## Nested scope accounting

- `LyraForm/Lyraform/flowmini_v24_explicit_ast` — historical-prototype; remains under this context without a new independent ownership claim.
- `LyraForm/Lyraform/flowmini_v24_explicit_ast/subprojects/SymbolTable/SymbolTable_v1_1` — historical-prototype; remains under this context without a new independent ownership claim.
- `LyraForm/Lyraform/flowmini_v24_explicit_ast/subprojects/testabi` — historical-prototype; remains under this context without a new independent ownership claim.
- `LyraForm/Lyraform/flowmini_v24_explicit_ast/subprojects/TokenTree/tokentree_v1` — historical-prototype; remains under this context without a new independent ownership claim.
- `LyraForm/Pattern_explored/refactored_flow` — historical-prototype; remains under this context without a new independent ownership claim.

## Consolidation and remaining decisions

- Local AGENTS.md resolves this map and the shared executor model.
- The nearest source repository excludes private mission-state directories.
- Instructions and evidence do not silently approve overrides or broaden operational permissions.
- [GAP-001.md](GAP-001.md): resolved on 2026-10-17 by Henrik through FrankenCore IDENTITY-001; architecture authority and possible system implementation are distinct.
- [GAP-002.md](GAP-002.md): resolved by the shared C++ v1.1 four-column indentation ruling; existing compiler formatter matches.

Review this record when the context, instructions, local policies or source
evidence change. Source hashes describe this audit snapshot; future drift must
be reviewed, not silently re-labelled as verified.

## Executor adapter inspection

The existing Codex runner remains a legacy loop controller, classified by
[RUNNER-001](RUNNER-001.md). It does not validate portable goal records.
The hygiene gate now checks portable state placement and local policy resolution;
existing live legacy state and the runner implementation are unchanged.
