# Lyraform development health — 2026-10-17

Observed: 2026-10-17T13:33:10.475819+02:00. Branch: `codex/disposition-policy-selection-gate3`. Base commit: `b50534c380879a5a70a77ed5c1dd442bfc4142bd`.
Scope: the current working tree including pre-existing uncommitted compiler work.
This is a development verification snapshot, not a release or semantic maturity claim.

## Current layout and authority

The implementation collection is `Capabilities/`, containing Contracts,
LanguageMap, Requirements, Provenance, ConfigResolveAdapter, DebianPackages and
RuntimeDiscovery. The former `Frankencore/Core` directory is retired. Top-level
Development `FrankenCore/` remains architectural/policy authority.
See [NAMING-001](../../Capabilities/Policies/NAMING-001.md).

The components remain distinct. Public C++ namespaces/include prefixes, CMake
aliases, executable names and serialized format identities are unchanged.
Their old spelling is a compatibility boundary, not a second current authority.

## Verified development path

```sh
./igor --build-dir /tmp/lyraform-capabilities-health-20261017 doctor
cmake -S . -B /tmp/lyraform-capabilities-health-20261017 -G Ninja
cmake --build /tmp/lyraform-capabilities-health-20261017 -j 4
ctest --test-dir /tmp/lyraform-capabilities-health-20261017 --output-on-failure -j 4
```

Fresh configure and build passed. **189/189 tests passed**. The suite includes
five Igor cache-identity cases: foreign source, current source, equivalent path
alias, missing source identity and fresh build directory. Native file-producer
and scheduling checks use the new paths. Documentation, workspace hygiene,
strict architecture-boundary checks and shared governance validation pass.
The architecture checker now reports its actual scope instead of claiming
constitutional certification from a source-layout check.

Igor refuses to build from a cache whose CMake source identity differs from its
checkout. Select a fresh build directory after relocation. The old local `build/`
cache still names the retired Flowcore path; it was preserved and the refusal
was verified. Do not relabel that cache or infer readiness from its old outputs.

## Explicit limits and remaining maintenance

- ConfigResolveAdapter was disabled in this default build. Its optional enabled
  configuration was not built or certified during this task.
- Follow-up, 2026-10-17: the obsolete `json_escape` helper was removed with
  Henrik's approval. Both CUDA graph targets rebuilt without warnings; the
  exception-containment and CUDA allocation-fault tests passed (2/2). The
  original 189-test run remains the earlier, separately scoped observation.
- Passing tests do not establish real CUDA/device availability or all provider
  configurations. No production-readiness claim is made.
- Existing Gate 3 compiler work remains on its original branch and uncommitted;
  this task neither completes nor resumes that separate mission.
- Provenance still depends on Flowcontracts JSON; platform-specific providers
  remain platform-specific. Renaming does not claim standalone packaging.

## Maintenance procedure

Use Igor doctor and a build directory belonging to the selected checkout. Run
focused tests for edits and the canonical suite at integration gates. Keep
policy mappings, source inventories and component documentation aligned when
moving code. Preserve historical snapshots and mission identities; update this
health record with new dated evidence rather than reusing these test counts.

[Mission evidence](../../../Policies/missions/2026-10-17-capabilities-health/README.md)
records preservation and validation details. No commit or publication occurred.
