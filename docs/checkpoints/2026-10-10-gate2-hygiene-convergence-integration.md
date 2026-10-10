# Gate 2 and hygiene convergence integration

Date: 2026-10-10

Mission: `docs/tasks/gate2-hygiene-convergence-integration.md`

Review branch: `codex/gate2-hygiene-convergence`

Integrated branch: `main`

## Result

**PASS — MERGED INTO `main`.**

The review branch contains the complete ordinary merge ancestry of the
mission/workspace hygiene branch and the source-disposition Gate 2 branch.
It provides one coherent repository authority chain and one source-derived,
closed typed disposition topology for the bounded `TextOutcome` specimen.
That topology remains declarative and execution-unsupported.

Henrik approved continuation after reviewing the branch result. The reviewed
tip was merged into `main` through ordinary ancestry as
`9f3c40ea36047bae4eb788356b4c30e67ba686fc`. Gate 3 has not started and is not
authorized by this checkpoint.

## Provenance and ancestry

```text
starting origin/main:
    86a5c69d58bacb09f393cef2a3d4e798ac9fe3ad

hygiene source tip:
    6566cd8dbff0d29fd4ff549a52b90120bf536cb7

Gate 2 source tip:
    418e1e2172a8605436de34391b6e8d1729f803b5

hygiene merge commit:
    304ccc9

Gate 2 merge commit:
    d5d6bf607c9b1788cf9b7f449604b8cb2aa63acf

review branch tip:
    5feb23d760636dd234b341f891c20cfb8ed81069

main integration merge:
    9f3c40ea36047bae4eb788356b4c30e67ba686fc
```

Both source tips are ancestors of the integration head. Their common verified
base is the recorded `origin/main`. Neither `master` nor
`flowlfs-v0.1-alive` enters the integration ancestry.

## Preserved boundaries

- semantic analysis remains the sole legality producer for
  `lyraform.source_disposition_topology/v1`;
- the bounded source specimen still emits a blocked lowering plan with
  `execution: unsupported` and every execution claim false;
- policy selection, executable Graph IR, LLVM/TinyVM disposition routing, and
  runtime activation containment remain absent;
- mission selection requires one explicitly selected tracked direct child of
  `docs/tasks/` with a convergence contract;
- runner state and immutable launch manifests remain mission-specific and
  Git-local under `.git/codex/runs/<mission-id>/`;
- the tracked root `.codex-run-state` is absent from the current tree;
- the 21 archived development artifacts retain their provenance and verified
  checksums;
- historical task and checkpoint text remains evidence, not ambient authority.

No exception unwinding, dynamic nearest-handler lookup, implicit propagation
or termination, diagnostic-and-drop, policy-created success, or ordinary
fault recovery was introduced.

## Integration defects closed

Two operational defects were exposed by the combined verification and repaired
without changing language meaning:

1. The autonomous-runner test assumed every temporary repository began on
   `master`. It now records and verifies the branch actually created by Git,
   making the hostile test independent of host `init.defaultBranch` policy.
2. Seven aggregate parity/graph tests copied a generated provider library into
   `Lyraform/compiler/build`. They now use each test's disposable temporary
   workspace, preserving the declared `./build/...` provider path without
   writing generated artifacts into the source tree.

The first canonical attempt consequently reported 180 passes and seven
source-tree-write failures. After the root-cause repair, all seven focused
regressions and the complete suite passed.

## Focused evidence

- autonomous runner hostile tests: PASS;
- agent/workspace hygiene checker: PASS;
- current-documentation checker: PASS;
- archive `SHA256SUMS`: all 21 payloads PASS;
- Gate 2 source disposition: PASS — one deterministic source topology, ten
  early semantic refusals with verified source locations, 23 topology
  mutations rejected, six embedded semantic-report mutations rejected, and
  execution unsupported;
- corrected aggregate source-tree isolation regressions: 7/7 PASS;
- `git diff --check`: PASS.

## Clean verification

Normal external build tree:
`/tmp/lyraform-gate2-hygiene-build-20261010`.

- `./igor --build-dir ... doctor`: PASS;
- `./igor --build-dir ... build`: PASS, 324 Ninja steps;
- `./igor --build-dir ... test`: **187/187 PASS** in 59.25 seconds;
- focused Clang 18.1.3 ASan/UBSan build and Gate 2 test: PASS with
  `halt_on_error=1` and leak detection disabled for the host environment;
- Valgrind 3.22.0 frontend export: 1,513 allocations/frees, zero bytes live,
  zero errors;
- Valgrind semantic analysis: 25,817 allocations/frees, zero bytes live, zero
  errors;
- Valgrind contract validation: 1,098 allocations/frees, zero bytes live, zero
  errors.

The normal build retains two pre-existing `-Wunused-function` warnings in
`Flowparallel/src/graph_cuda.cpp`. They are unrelated to this integration.

## Fresh-clone verification

Final pushed review tip:
`5feb23d760636dd234b341f891c20cfb8ed81069`.

The candidate was cloned from
`git@github.com:Henrik1969/Lyraform.git`, selecting only
`codex/gate2-hygiene-convergence`, into a brand-new temporary directory.

- clean clone and exact remote branch tip: PASS;
- `./igor ... doctor`: PASS;
- clean configure and 324-step build: PASS;
- canonical suite: **187/187 PASS** in 58.46 seconds;
- source checkout remained clean after the suite.

An earlier prepublication proof attempt at `388bf956` stopped when the host
`/tmp` filesystem reached zero free space. Only disposable directories created
by this mission were removed; no repository or user data was touched. The
candidate and final-tip clean-clone proofs then built successfully, confirming
an environmental capacity failure rather than source or semantic evidence.

## Review-branch safety accounting

```text
Gate 3 work started:   NO
child branches created: NO
main written directly: NO
master touched:         NO
FlowLFS touched:        NO
force used:             NO
```

## Main integration accounting

```text
human merge approval:                  YES
ordinary merge ancestry preserved:     YES
unreviewed implementation added direct: NO
master touched:                        NO
FlowLFS touched:                       NO
force used:                            NO
Gate 3 work started:                   NO
```

The convergence loop is closed on `main`. Any adjacent disposition stage still
requires a newly selected bounded mission.
