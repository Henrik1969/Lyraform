# Lyraform component Flowlower — governance audit

Observed: 2026-10-17. Scope: `LyraForm/Flowlower`.
Status: structural consolidation; implementation conformance is not certified.
Mission: `development.governance-consolidation`, revision 1.

## Owned boundary

Component-specific contracts stay in their existing source documentation. Lyraform standing development rules apply; this record does not invent independent umbrella ownership.

Local policy source: [map](README.md). Machine-readable resolution:
[context.json](context.json). The shared baseline is a declared dependency;
an isolated clone needs access to the referenced policy set or a deliberately
exported policy bundle. Missing shared policy must be reported, not invented.

## Evidence inspected

- [README.md](../README.md) — SHA-256 `d62cec43f4360f80bd4db81b19ca7c2d0b009121bbb671e1173afcd7339db99d`.
- [CMakeLists.txt](../CMakeLists.txt) — SHA-256 `a0a3e94215f614c1d162a872b31815c77319100c331a6bb4eeaa39dbff9f3898`.

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

2026-10-17 native callable ABI review: added an uninstalled unit emission helper
and `flowlower_source_callable_abi` CTest. Prior CMake hash `f385dcb7128ef8d627e8af85cdf2e3b4dee35b5162ef36b52e33c17c6a3bade5`.
The helper verifies individual source callables while public blocked-graph
admission remains enforced; it is not an alternate runnable graph compiler.
