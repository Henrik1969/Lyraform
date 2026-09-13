# Flowmini Testing

Flowmini uses a categorized integration test suite.

The project-wide [verification-gate policy](../development/verification-gates.md)
defines the binding Tier 1, Tier 2, and Tier 3 Firetest requirements. The
commands below are the current Flowmini realization of those gates.

## Categories

```text
examples/pass/*.flow
    runnable programs expected to succeed

examples/fail/*.flow
    examples expected to fail with diagnostics

examples/support/*.flow
    importable units / support files

examples/docs/*
    documentation
```

## Runner

```bash
cd Lyraform/compiler
../../tools/run-flowmini-test-suite.sh \
    --root . \
    --build-dir cmake-build-debug \
    --no-build
```

The canonical CMake-visible gates are:

```bash
cmake --build cmake-build-debug --target flowmini_ast_golden_tests
cmake --build cmake-build-debug --target flowmini_symbol_projection_tests
cmake --build cmake-build-debug --target flowmini_frontend_bundle_tests
cmake --build cmake-build-debug --target flowmini_suite
ctest --test-dir cmake-build-debug --output-on-failure
```

Expected current baseline:

```text
AST golden tests:          28 / 28
Symbol projection tests:   14 / 14
Frontend bundle tests:      8 golden / 1 isolated / 19 negative
fresh GCC 13.3 CTest:       104 / 104
fresh Clang 18.1 CTest:     103 / 103 (published pre-wide-aggregate baseline)
flowcat ELF example:        PASS
```

## Current categorized-suite note

On 2026-09-13, the normal compatibility-interpreter run had 138 cases: 83
passed and 55 migrated native-chain pass examples were refused as expected
interpreter incompatibilities. With support files included it had 154 cases:
99 passed and the same 55 expected native-chain refusals. The canonical
native-chain/support mode separately passed all 97 pass-corpus programs and
all 57 negative/support fixtures (57/57). These modes must not be collapsed
into one total.

The native pass-corpus driver selects its lowering path from semantic artifact
content: aggregate-layout reports use their manifest-independent path, while
other reports receive the exact policy binding. It does not maintain a list of
program or fixture names, so adding another program with an existing contract
does not require changing the driver.

These normal gates form the Flowmini Tier 2 integration baseline. Before
declaring a greater architectural border closed, run and record the additional
Tier 3 Firetest pressure checks defined by the project-wide policy.

## Expected files

```text
tests/expected/stdout/<name>.out
tests/expected/stderr/<name>.err
tests/expected/diagnostics/<name>.contains
```

Diagnostic `.contains` files are substring checks. They should contain stable diagnostic phrases, not full path-sensitive stderr.

## Optional modes

```bash
../../tools/run-flowmini-test-suite.sh --root . --build-dir cmake-build-debug --no-build --valgrind
../../tools/run-flowmini-test-suite.sh --root . --build-dir cmake-build-debug --no-build --gdb-failures
../../tools/run-flowmini-test-suite.sh --root . --build-dir cmake-build-debug --no-build --gdb-all
../../tools/run-flowmini-test-suite.sh --root . --build-dir cmake-build-debug --no-build --run-support
```

`--run-support` executes importable support units as expected failures when used
as root sources. The current native-chain support-inclusive firetest is 97 pass
programs plus 57/57 categorized negative/support checks. The legacy
interpreter-inclusive run is 154 total, 99 pass, and 55 expected native-chain
refusals.

The published Clang ASan/UBSan CTest gate is 103/103 when run with
`ASAN_OPTIONS=detect_leaks=0`; LeakSanitizer cannot run under this environment's
ptrace-based process supervision. The new `c_long` aggregate boundary is
verified by the fresh GCC run; the published Clang sanitizer result predates
that additional test and must not be reported as 104/104 until rerun. The
matching native/support categorized boundary is 57/57 under Valgrind 3.22.0.

Compiler, sanitizer, Valgrind, support-inclusive, and concurrency results are
checkpoint evidence rather than permanent properties of the branch. A new
greater-border claim requires a new Firetest report tied to the tested commit or
working-tree state.

The raw-frontend checkpoint evidence is recorded in the
[v0.24 frontend Firetest report](v0.24-firetest-report.md). The independent
export boundary has a separate
[frontend-border Firetest report](v0.24-frontend-border-firetest-report.md).

The historical v0.25 structural-origin contract and its negative attack surface are
documented in the [origin maturity audit](v0.25-origin-maturity-audit.md).

## Current build-isolation limitation

The legacy ABI example path still loads:

```text
build/libflowmini_testabi.so
```

The `flowmini_prepare_test_dependencies` target copies the provider from the
selected CMake build tree into that source-tree path. Consequently, separate
normal, sanitizer, or compiler build trees can overwrite one another's ABI test
provider. Do not run those preparation steps concurrently. Before testing with
a different build tree, restore its matching provider explicitly:

```bash
cmake --build <build-tree> --target flowmini_prepare_test_dependencies
```

This is a documented compatibility bridge, not the intended future
prerequisite/provider model.

The current struct-by-value ABI probe also shares the focused C declaration in
`subprojects/testabi/include/flowmini_testabi.h` between the test provider and
the runtime's existing hard-coded `Point` compatibility path. This keeps the
function-pointer type and carrier layout exact under UndefinedBehaviorSanitizer.
It is test-provider infrastructure, not a canonical Flowmini ABI type or the
future general ABI call mechanism.
