# Verification Gates

Lyraform uses tiered verification. Small changes should remain quick to test,
while claims that open or close an architectural border require broader pressure
testing.

## Binding rule

Firetest is required before declaring a greater architectural border closed or
publishing a release checkpoint.

Firetest supplements focused and integration tests. It does not establish that
a language design is correct, that every accepted construct is covered, or that
semantic contracts are valid.

## Gate tiers

### Tier 1: local patch gate

Use for a focused change during normal development:

- normal build;
- focused tests for the changed behavior;
- relevant golden tests where observable structure changes;
- `git diff --check`.

### Tier 2: standard integration gate

Use before integrating a coherent development checkpoint:

- normal build without new warnings;
- all AST golden tests;
- all SymbolTable projection golden tests;
- the categorized Flowmini suite;
- CTest;
- relevant documentation checks;
- `git diff --check`.

### Tier 3: greater-border and release gate (Firetest)

Run every Tier 2 check plus:

- a fresh GCC build;
- a fresh Clang build;
- AddressSanitizer and UndefinedBehaviorSanitizer runs where supported;
- Valgrind where meaningful and supported;
- the support-inclusive suite;
- repeated concurrent projection or structural-dump runs to probe determinism;
- documentation link, Markdown, and diff checks supported by the repository.

Use independent build trees for compiler and sanitizer configurations. Record
the exact commands, tool versions when relevant, results, and any skipped or
unavailable checks. A missing tool may be reported as unavailable with a reason;
an unexplained skip or unresolved failure does not satisfy Tier 3.

## Greater architectural borders

A greater border is a change or checkpoint that opens, closes, or materially
changes a major layer boundary. This includes:

- raw AST completion;
- SymbolTable projection maturity;
- the start or completion of semantic analysis;
- Graph IR introduction;
- ModuleSpec or FlowIR compatibility lowering;
- runtime-facing execution changes;
- ABI or contract-model changes;
- release tags and public checkpoints.

Firetest is also required for a checkpoint that materially changes:

- frontend acceptance or rejection rules;
- source-unit inspection or execution policy;
- AST structural ownership or stable IDs;
- the SymbolTable projection contract;
- runtime execution semantics;
- ABI test providers or their loading boundary.

Not every patch touching one of these areas is itself a closed border. The
requirement applies before the project makes a border-closure, release, or
public-maturity claim.

## Evidence and claims

A Tier 3 report must identify:

- the commit or exact working-tree state tested;
- each required pressure check and its result;
- unavailable checks and why they were unavailable;
- known infrastructure limitations that affected sequencing or isolation;
- unresolved risks and accepted-language coverage gaps.

The evidence supports portability, memory/tooling sanity, deterministic
projection, integration stability, and test-isolation findings. It does not by
itself prove semantic completeness or architectural correctness.

The rule for public claims is:

```text
small bricks:      Tier 1
integration:       Tier 2
greater borders:   Tier 3 Firetest
public closure:    recorded Tier 3 evidence, or no closure claim
```

Flowmini commands and current baselines are maintained in
[Flowmini Testing](../flowmini/testing.md).

## Current native-chain categorized gate

The historical categorized runner executes pass files in the compatibility
interpreter by default. Migrated native `return`, argv and ABI declarations must
use `tools/run-flowmini-test-suite.sh --native-pass-boundary --run-support` with
all six stage binary variables set. This validates the 91-program pass corpus
through the canonical native chain and keeps the 37 negative fixtures and 16
support fragments at their interpreter refusal boundary. `--valgrind` instruments
those negative/support runs; native compiler stages and linked applications can
be checked separately with `valgrind --error-exitcode=99 --leak-check=full`.
The root CTest suite includes the support-fragment gate.
