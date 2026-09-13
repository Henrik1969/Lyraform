# Lyraform tester and critic onboarding

This package is for people evaluating Lyraform from a clean checkout. It
provides the shortest reproducible path from prerequisites to a verified test
result, then explains what the result does and does not claim.

For adversarial evaluation missions—breaking assumptions, challenging
architecture, testing provenance, and comparing simpler alternatives—continue
with the [critical alpha-testing guide](../../ALPHA-TESTING.md).

## Quick start

The canonical repository is:

```text
https://github.com/Henrik1969/Lyraform
```

Use a clean clone and keep generated build output outside the checkout:

```bash
git clone https://github.com/Henrik1969/Lyraform.git
cd Lyraform
git switch --detach main

./tools/check-onboarding-prerequisites.sh
./igor doctor
./igor --build-dir /tmp/lyraform-onboarding-build test
```

Expected result:

```text
100% tests passed, 0 tests failed out of 91
```

Record the exact revision before reporting results:

```bash
git rev-parse HEAD
git status --short --branch
```

The expected clean state is a detached `main` revision with no worktree
changes. A normal named branch is also fine when testing a local patch.

## Install prerequisites

The complete canonical suite is currently maintained and verified on Linux
x86-64. On Debian, Ubuntu, or Pop!_OS, the usual prerequisite set is:

```bash
sudo apt update
sudo apt install \
  build-essential cmake ninja-build clang jq python3 file util-linux \
  libssl-dev libncursesw5-dev
```

For the optional memory-review gate, also install:

```bash
sudo apt install valgrind
```

The required baseline is:

- CMake 3.22 or newer and CTest;
- Ninja;
- GCC or another C/C++ compiler, plus Clang for the native LLVM checks;
- Git;
- `jq`, Python 3, `file`, `script`, `timeout`, and `sha256sum`;
- OpenSSL development headers and `libcrypto`;
- wide-character ncurses development/runtime support (`libncursesw`);
- a POSIX shell and a usable pseudo-terminal for the terminal tests.

CUDA hardware is not required for the canonical suite. CUDA-related provider
checks use their documented fallback/refusal paths when no CUDA device is
available.

## What Igor commands mean

```text
igor doctor   local tool and checkout readiness
igor check    configure and list the registered CTest graph
igor build    configure when needed, then build the root graph
igor test     build and run the canonical CTest suite
igor run      run a Flow source through the compatibility executable
```

The underlying stage binaries remain independently invocable for focused
investigation. `igor` is the canonical human-facing Lyraform driver.

## Tester path

1. Run the quick-start commands above.
2. Save the revision, platform, compiler versions, and complete test output.
3. If a test fails, rerun only that test with:

   ```bash
   ctest --test-dir /tmp/lyraform-onboarding-build \
     --output-on-failure -R '<test-name>'
   ```

4. Attach the failing test name and the first structured diagnostic. Do not
   replace it with a paraphrase.
5. Use the [bug-report template](BUG-REPORT-TEMPLATE.md).

For a focused source-chain check, the most representative tests are:

```bash
ctest --test-dir /tmp/lyraform-onboarding-build --output-on-failure \
  -R 'profile_free_generic_lowering|native_source_graph|flow_less_pager|namespace_ambiguity|text_outcome_boundary|math_library_boundary|strnlen_library_boundary'
```

## Critic path

Read these in order:

1. [root project README](../../README.md);
2. [current Lyraform version](../../Lyraform/CURRENT.md);
3. [migration record](../../LYRAFORM_MIGRATION_2026-09-13.md);
4. [verified reusable-chain result](../checkpoints/2026-09-07-reusable-flow-chain-result.md);
5. [verification-gate policy](../development/verification-gates.md).

Then challenge the claims with evidence:

- Does a clean checkout build with the installed dependencies?
- Does the complete 91-test suite pass at the reported revision?
- Are source identity, provider identity, native symbol identity, and contract
  identity kept distinct?
- Do malformed or unauthorized plans fail explicitly?
- Do renamed source units and programs remain independent of compiler profiles?
- Are platform limitations and compatibility surfaces stated honestly?
- Do the documented commands reproduce the claimed artifact or diagnostic?

Architectural criticism is welcome even when all tests pass. A green suite is
evidence for the covered contracts, not proof of language completeness,
production readiness, security certification, or universal portability.

## Scope and known limitations

The current public acceptance surface is experimental and not production-ready.
Native demonstrations target Linux x86-64 with LLVM/native linking and
`libncursesw.so.6`; the bounded `libm.so.6` math slice additionally requires
the host math library and is currently LLVM-only. The admitted graph surface is
scalar, bounded, acyclic FIFO expansion with fresh receivers. Aggregate graphs,
persistent node state, streams, asynchronous/reentrant delivery, and broader
target/provider coverage remain future work.

The `flowmini` executable name and `flowcore.*`/`flowmini.*` serialized artifact
identifiers are retained as technical compatibility surfaces. The current
project identity is Lyraform and the human-facing toolchain is Igor.

## Safety and evidence handling

The suite compiles native code, links shared libraries, loads declared
providers, and exercises terminal and filesystem paths. Run it in a disposable
or appropriately trusted environment. Do not feed secrets or sensitive source
into public issue reports. Keep generated build trees and temporary logs out of
commits unless a minimized fixture is intentionally part of the report.

See [BUG-REPORT-TEMPLATE.md](BUG-REPORT-TEMPLATE.md) for the evidence package
to attach to a finding.
