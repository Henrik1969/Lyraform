# Lyraform component Flowtools — governance audit

Observed: 2026-10-17. Scope: `LyraForm/Flowtools`.
Status: structural consolidation; implementation conformance is not certified.
Mission: `development.governance-consolidation`, revision 1.

## Owned boundary

Component-specific contracts stay in their existing source documentation. Lyraform standing development rules apply; this record does not invent independent umbrella ownership.

Local policy source: [map](README.md). Machine-readable resolution:
[context.json](context.json). The shared baseline is a declared dependency;
an isolated clone needs access to the referenced policy set or a deliberately
exported policy bundle. Missing shared policy must be reported, not invented.

## Evidence inspected

- [README.md](../README.md) — SHA-256 `71410c2e975e19881d39fa8f2cf4df052b1eeb6050e0073835703a97563f5f81`.
- [CMakeLists.txt](../../../Policies/history/2026-10-17-capabilities/LyraForm/Flowtools/CMakeLists.txt) — SHA-256 `580e9fdbac03aa686b681729a493f813c754b5462450052187680acb9db32e2f`. Pre-rename evidence preserved.

## Development validation surface

No repeatable test command was identified in the inspected top-level documentation. This is a documentation/evidence limit, not a claim that testing does not exist. Select applicable checks before implementation.

## Nested scope accounting

- `LyraForm/Flowtools/examples/flowcat-clion` — consumer-or-test-fixture; remains under this context without a new independent ownership claim.
- `LyraForm/Flowtools/reference/packages` — owned-component; remains under this context without a new independent ownership claim.
- `LyraForm/Flowtools/reference/text` — owned-component; remains under this context without a new independent ownership claim.
- `LyraForm/Flowtools/reference/requirements` — owned-component; remains under this context without a new independent ownership claim.
- `LyraForm/Flowtools/reference/contracts` — owned-component; remains under this context without a new independent ownership claim.
- `LyraForm/Flowtools/reference/policy` — owned-component; remains under this context without a new independent ownership claim.
- `LyraForm/Flowtools/reference/language` — owned-component; remains under this context without a new independent ownership claim.
- `LyraForm/Flowtools/reference/revision` — owned-component; remains under this context without a new independent ownership claim.
- `LyraForm/Flowtools/reference/clock` — owned-component; remains under this context without a new independent ownership claim.
- `LyraForm/Flowtools/facades/ls` — owned-component; remains under this context without a new independent ownership claim.

## Consolidation and remaining decisions

- Local AGENTS.md resolves this map and the shared executor model.
- The nearest source repository excludes private mission-state directories.
- Instructions and evidence do not silently approve overrides or broaden operational permissions.
- No local GAP file was established by this scope inspection; this is not full code compliance evidence.

Review this record when the context, instructions, local policies or source
evidence change. Source hashes describe this audit snapshot; future drift must
be reviewed, not silently re-labelled as verified.

2026-10-17 activation-runtime review: `reference/disposition` remains owned by
this component and adds the private C frame runtime plus source-derived CTest.
CMake prior hash `c71f09b19b40e7556e19af091fe1f870c066ca6f1f0747d9730fa5ad2227cde1`; reviewed current hash `825deea2473bd238ba418e9ee81b578be7585ce1df9b53356db474de3ab4c4eb`.
Validation: `ctest --test-dir <build> -R flowactivation_source_runtime`.
No policy or language-semantics change; source execution remains unsupported.
