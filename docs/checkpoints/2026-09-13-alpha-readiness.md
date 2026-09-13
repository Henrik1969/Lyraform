# Lyraform alpha-readiness checkpoint

**Date:** 2026-09-13
**Status:** verified onboarding/documentation checkpoint
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

## Current scope

The onboarding baseline exercises the current experimental Lyraform surface:
source-driven lowering, exact ABI/provider evidence, durable scalar graph
activation, Flow-owned paging, TinyVM boundaries, malformed-artifact refusal,
and native execution. It does not establish production readiness, formal
verification, memory safety, universal portability, aggregate/streaming graph
semantics, or complete language closure.

## Next maturation slice

The next language-closure candidate is the [explicit Text semantics
proposal](../architecture/text-value-v0.1-proposal.md): a documented `Text`
value contract, Text literals/concatenation, and a provider-independent print
path. Before implementation, the acceptance boundary must define ownership,
encoding, concatenation allocation/failure behavior, source provenance,
lowering representation, and native/TinyVM parity. The existing `c_string` ABI
carrier is not silently promoted into that language contract.

That distinction is the next design-and-test checkpoint. Until it is specified,
the current string ABI compatibility path remains unchanged.
