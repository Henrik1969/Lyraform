# Lyraform language-maturation publication — 2026-11-02

## Purpose and authority

This report records the publication of the current Lyraform language-development
wave from the experimental branch `experiment/configresolve-lyraform`. The
current user explicitly authorized committing and pushing this work, which
supersedes the earlier local mission wording that prohibited publication of
the experiment. That authorization applies to this publication only.

The branch remains an experiment. It is not merged into `main`, does not claim
completion of the language-maturation criteria, and does not claim a green
integration baseline.

Base and pre-publication tip:

```text
branch: experiment/configresolve-lyraform
tip:    b50534c380879a5a70a77ed5c1dd442bfc4142bd
main:   b50534c380879a5a70a77ed5c1dd442bfc4142bd
```

## Published scope

The working wave contains the current Lyraform contract and language
maturation work, including source-disposition policy and graph evidence,
ordinary-record analysis and admission, explicit integer-width and
value-preserving conversion rules, Flowanalyst/Flowbind/Flowcontracts/
Flowlower/Flowparallel/Flowtools/TinyVM integration, focused regression
tests, policy files, architecture decisions, checkpoints, and ConfigResolve
experiment evidence.

The obsolete duplicate `Frankencore/Core` implementation copies are removed
in favor of the current canonical Flowtools arrangement. This publication
does not modify FlowLFS, Flowcore, `master`, or any other repository line.

## Clean verification

All checks below used the fresh external build directory
`/tmp/lyraform-publication-20261102`:

```text
./igor --build-dir /tmp/lyraform-publication-20261102 doctor
PASS — required toolchain and repository paths found

./igor --build-dir /tmp/lyraform-publication-20261102 check
PASS — CTest graph configured with 196 tests

./igor --build-dir /tmp/lyraform-publication-20261102 build
PASS — 332/332 build steps completed

./igor --build-dir /tmp/lyraform-publication-20261102 test
166/196 passed; 30 failed
```

The failed tests are retained as current evidence, not relabeled as passing.
They cluster around the open bare-`int` to explicit-width migration and its
provider, pipeline, lowering, and legacy-fixture fallout. Representative
boundaries include `abi_strlen_main`, record fixtures that still use bare
`int`, `flowcore_pass_corpus`, and the Flowanalyst/Flowlower/Flowparallel
pipeline families. The complete failed-test list is in the build's
`Testing/Temporary/LastTestsFailed.log` and is summarized by the existing
[explicit integer conversion checkpoint](../checkpoints/2026-11-01-explicit-integer-conversions.md).

Additional repository hygiene:

```text
git diff --check (before staging the new report)
PASS for the tracked working-tree edits
```

The staged historical Markdown records retain a small number of intentional
trailing spaces used for hard line breaks. No generated build or test output
was staged.

The build emitted only existing compiler warnings about omitted
`TypeDef::variants` initializers; no build failure resulted from them.

## Interpretation

This is a truthful development snapshot: compilation and configuration are
green, while the full canonical test suite is not. The open failures are
consistent with the documented explicit-integer migration and contract
stabilization work. No G1–G5 or C1–C5 completion is claimed, and no
production-readiness claim is made.

The next work should migrate the remaining bare-integer fixtures and resolve
the provider/host-result and pipeline compatibility boundaries before any
claim of a green integration baseline.
