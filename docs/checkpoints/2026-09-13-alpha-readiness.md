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
                                            81/81 PASS
bash -n tools/check-onboarding-prerequisites.sh
                                            PASS
git diff --check                           PASS
```

The fresh onboarding build compiled 146 targets and the canonical CTest suite
passed all 81 tests in 35.89 seconds on the verified Linux x86-64 environment.

After the Text slice, a fresh `/tmp/lyraform-text-build` compiled 115 targets
and the complete CTest graph passed 82/82 in 34.51 seconds. The two pager tests
were rerun after a compatibility repair for empty legacy `c_string` values and
also passed.

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
  Flowcore pass corpus: 92 programs passed semantic and lowering boundaries
```

The native fixture prints an empty line followed by `Lyraform — Igor`, proving
that empty Text is not lowered as the legacy null c_string compatibility value.
It also returns a constant-backed Text value from an ordinary function before
printing it, covering the callable boundary.
The implementation is intentionally bounded: concatenation is admitted only
when operands are initializer-known constants, and `puts_text(Text)` provides a
declared borrowed view at the native call boundary. Runtime owned allocation,
explicit failure/exhaustion, Text returns, and TinyVM parity remain open gates.

The existing `c_string` ABI carrier remains unchanged and is not silently
promoted into the language contract.

That distinction is the next design-and-test checkpoint. Until it is specified,
the current string ABI compatibility path remains unchanged.
