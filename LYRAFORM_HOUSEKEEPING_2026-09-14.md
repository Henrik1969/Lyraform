# Lyraform repository housekeeping — 2026-09-14

This bounded cleanup started from `main` at
`f543c4833e38a0f9b8568e778794da76e515fccd` in a clean clone of
`Henrik1969/Lyraform`. The primary checkout was inspected first and left
untouched.

## Classification

| Class | Audited material | Disposition |
| --- | --- | --- |
| Product/source | `Lyraform/`, stage directories, `tools/`, `igor`, `.github/` policy and guidance | Keep; update only directly affected current wording |
| Reproducible fixture | compiler fixtures, expected `.out` files, `Pattern_explored/src/*.cpp`, `art.*`, and test inputs | Keep; preserve the explicit stdout-fixture ignore exception |
| Historical evidence | `docs/checkpoints/`, `docs/tasks/`, dated architecture records, `BRANCH_CONSOLIDATION_2026-09-13.md`, `_archive/flowmini/previous-stages/` | Keep and label; no history rewrite |
| Generated reproducible output | five tracked `Pattern_explored/src` ELF binaries and root `flowparallel-smoketest-report.json` | Remove from the tree; ignore and regenerate in build/temporary locations |
| Local operational state | tracked `.codex-run-state` | Remove from the tree; runner state defaults below `.git/` |
| Ambiguous | no additional deletion candidates proved safe | Leave untouched and report rather than guess |

`_archive/` is approximately 8.6 MiB across 1,470 tracked files and remains
intentional source archaeology. It was not extracted, deleted, or rewritten.

## Changes

- `AGENTS.md` is now timeless Lyraform repository guidance and no longer
  activates a completed mission or grants indefinite task authority.
- The reusable-chain task is explicitly marked historical/superseded.
- `tools/run-autonomous-codex` requires `--task PATH`, accepts explicit state
  and context paths, validates repository-bound context, and defaults state to
  `git rev-parse --git-path codex/run-state` without granting authority.
- `Pattern_explored/src/build.sh` writes binaries/logs to `Pattern_explored/build`
  by default and accepts `--build-dir`.
- The Flowparallel smoketest validates its report before cleanup, retains it
  temporarily by default, and supports explicit `--output PATH` preservation.
- Current Lyraform identity and status language was corrected without changing
  historical compatibility identifiers or architecture semantics.

## Protected boundaries

The primary checkout's existing user work was not staged or modified. No
FlowLFS content, `master`, branches, tags, settings, or published history were
changed. No force push was used.

## Verification

Exact results from the current clean clone:

```text
bash -n tools/run-autonomous-codex tools/test-run-autonomous-codex.sh: PASS
tools/test-run-autonomous-codex.sh: PASS
Pattern_explored/src/build.sh --build-dir /tmp/lyraform-pattern-build-2: PASS (4 outputs; src remained clean)
cmake configure/build: PASS (150 targets; /tmp/lyraform-housekeeping-build)
ctest: PASS (107/107; 58.20 seconds)
./igor doctor: PASS
./igor build: PASS (150 targets; /tmp/lyraform-housekeeping-igor-build)
./igor test: PASS (107/107; 60.06 seconds)
Flowparallel smoketest default output: PASS (validated temporary report)
Flowparallel smoketest --output: PASS (validated preserved JSON report)
git diff --check: PASS
```

No GCC/Clang sanitizer orchestration is provided by this repository's current
housekeeping gates; no unsupported sanitizer result is claimed. This report
itself is not a source of semantic authority.
