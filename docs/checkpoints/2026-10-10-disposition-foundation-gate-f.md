# Disposition foundation Gate F checkpoint

Date: 2026-10-10  
Mission: `docs/tasks/disposition-foundation-and-source-authority-gate2.md`  
Branch: `codex/disposition-foundation-source-gate2`  
Starting authority: `origin/main` at
`86a5c69d58bacb09f393cef2a3d4e798ac9fe3ad`  
Reviewed branch: `origin/mission/canonical-disposition-lanes-wire-closure` at
`375e36fcf545c36241f0cb046bf2d458daa571d7`  
Integration commit: `965127a306fbd84705c689bc33ed005c178f5986`

## Result

Gate F: **PASS**.

The reviewed branch was zero commits behind and four commits ahead of the
starting authority. The integration commit has the starting authority and the
reviewed branch tip as its two parents, preserving all four published commits
and their accepted ADR text.

The bridge remains a declarative validator and projection. It consumes
independently parsed graph and plan artifacts, validates endpoint direction,
lane, exact payload type, producer, disposition, route, consumer, policy
selection, scope, provenance and graph-to-plan wire identity, and rejects
dangling, duplicate, forged and executable/`ready` claims. No compiler,
lowerer or backend treats the disposition bundle as executable. Hand-authored
fixtures remain test inputs, not source producers.

The base `.codex-run-state` is byte-for-byte identical to `origin/main`
(`CONTINUE\n`) and was excluded from the integration. This mission uses the
untracked Git-local state file
`.git/codex/disposition-foundation-source-gate2-run-state`.

Unrelated pre-existing untracked paths `meta-discusions.md` and `output/`
were not modified or staged. `main`, `master`, `flowlfs-v0.1-alive` and the
Flowselection line were not modified.

## Independent repair

Review found that machine-readable diagnostic repairs named unimplemented
propagation and policy sinks. The integration commit replaces those hints
with an existing declarative regeneration route or an explicit unsupported
source-association result. A regression checks that machine and human output
remain the same fact and that the unavailable repair names cannot return.
No syntax, policy or runtime execution was added.

## Verification evidence

Tool versions:

- CMake 3.28.3
- Ninja 1.11.1
- Ubuntu Clang 18.1.3
- Valgrind 3.22.0

Fresh external build directory:
`/tmp/lyraform-disposition-foundation-build`.

- `./igor --build-dir /tmp/lyraform-disposition-foundation-build doctor`:
  PASS.
- `./igor --build-dir /tmp/lyraform-disposition-foundation-build build`:
  PASS, 324 build steps.
- `./igor --build-dir /tmp/lyraform-disposition-foundation-build test`:
  PASS, 184/184 tests.
- Focused CTest selector `^flowcontracts_disposition_route_`: PASS, 2/2.
- Clang ASan/UBSan focused test: PASS. The test reported 6 diagnostic
  classes, 12 evidence refusals, 2 declarative lane round-trips, and 47
  hostile mutations rejected by 2 consumers.
- Valgrind focused test: PASS, 0 errors, 0 bytes in use at exit, 476,892
  allocations and frees.
- `git diff --check` and `git diff --cached --check`: PASS before commit.
- `.codex-run-state` comparison with `origin/main`: PASS after commit.

The normal build retains two pre-existing `-Wunused-function` warnings for
`json_escape` in `Flowparallel/src/graph_cuda.cpp`; they are outside this
gate's scope and did not affect the build or tests.

## Boundary retained

This checkpoint proves a reviewed declarative foundation only. It does not
prove source-derived disposition facts, source admission, route execution,
policy execution, LLVM/TinyVM failure routing, propagation, retry, or fault
recovery. Gate 0 must now establish an inspectable source-to-future-fact owner
and identity map without claiming those later capabilities.
