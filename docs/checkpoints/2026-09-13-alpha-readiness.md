# Lyraform alpha-readiness checkpoint

**Date:** 2026-09-13
**Status:** verified onboarding plus Text-slice checkpoint
**Canonical branch:** `main`

## Purpose

Lyraform now has a public path for testers and critics to reach the verified
baseline from a clean checkout and to report failures or architectural
counterexamples with reproducible evidence. This checkpoint records the
readiness work separately from compiler-language maturity; a green baseline is
not a claim that the design is complete or correct.

## Delivered surface

- `igor.png` is the canonical mascot asset and remains unchanged;
- [`docs/onboarding/README.md`](../onboarding/README.md) contains prerequisites,
  clean-clone commands, tester and critic workflows, scope limits, and safety
  guidance;
- [`tools/check-onboarding-prerequisites.sh`](../../tools/check-onboarding-prerequisites.sh)
  checks the required commands, CMake version, CMake dependency resolution,
  native C++ linking, and `libncursesw` linkability;
- [`docs/onboarding/BUG-REPORT-TEMPLATE.md`](../onboarding/BUG-REPORT-TEMPLATE.md)
  defines the minimum evidence package;
- [`ALPHA-TESTING.md`](../../ALPHA-TESTING.md) defines adversarial missions for
  testers and critics and is preserved as a first-class repository document.

## Exact verification

From the canonical checkout:

```text
./tools/check-onboarding-prerequisites.sh   PASS
./igor doctor                              PASS
./igor --build-dir /tmp/lyraform-onboarding-build test
                                            103/103 PASS
bash -n tools/check-onboarding-prerequisites.sh
                                            PASS
git diff --check                           PASS
```

The fresh GCC onboarding build compiled 148 targets and the canonical CTest
suite passed all 103 tests in 50.57 seconds on the verified Linux x86-64
environment. A fresh Clang 18.1.3 ASan/UBSan build also passed 103/103 with
`ASAN_OPTIONS=detect_leaks=0`; LeakSanitizer is unavailable under this
environment's ptrace-based process supervision.

The categorized compatibility-interpreter run is intentionally split from the
native chain: normal mode is 83/138 with 55 expected native-chain refusals;
support-inclusive mode is 99/154 with the same 55 refusals. Native-chain mode
passed all 97 pass-corpus programs and all 57 negative/support fixtures. The
matching 57/57 boundary suite passed under Valgrind 3.22.0.

## Current scope

The onboarding baseline exercises the current experimental Lyraform surface:
source-driven lowering, exact ABI/provider evidence, durable scalar graph
activation, Flow-owned paging, TinyVM boundaries, malformed-artifact refusal,
and native execution. It does not establish production readiness, formal
verification, memory safety, universal portability, aggregate/streaming graph
semantics, or complete language closure.

## Text maturation slice

The first bounded Text slice is now implemented and tested through the generic
chain. The focused command was:

```text
tools/test-text-value.sh
  PASS: semantic Text artifacts, Flowbind authorization, LLVM/native output,
        empty/non-ASCII values, c_string confusion, dynamic concat, and invalid UTF-8 refusals
tools/run-flowcore-pass-corpus.sh
  Flowcore pass corpus: 97 programs passed semantic and lowering boundaries
tools/test-tinyvm-text-parity.sh
  PASS: the admitted compile-time Text slice has LLVM/TinyVM output parity
tools/test-text-runtime.sh
  PASS: provider-owned bounded runtime concat and explicit exhaustion trap
tools/test-tinyvm-runtime-text-parity.sh
  PASS: bounded runtime-created Text has LLVM/TinyVM output and failure parity
```

The native fixture prints an empty line followed by `Lyraform — Igor`, proving
that empty Text is not lowered as the legacy null c_string compatibility value.
It also returns a constant-backed Text value from an ordinary function before
printing it, covering the callable boundary.
The implementation is intentionally bounded: concatenation is admitted only
when operands are initializer-known constants, and `puts_text(Text)` provides a
declared borrowed view at the native call boundary. Runtime owned allocation,
explicit failure/exhaustion, and general owned Text returns remain open gates;
the native bounded provider path now covers those behaviors for one concat
shape, while the runtime fixture also chains a provider-owned result into a
second concat and prints it repeatedly; the compile-time slice has TinyVM
parity.

The existing `c_string` ABI carrier remains unchanged and is not silently
promoted into the language contract.

That distinction is the next design-and-test checkpoint. Until it is specified,
the current string ABI compatibility path remains unchanged.
