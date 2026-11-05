# Provider-result cleanup slice — 2026-11-05

## Result

Provider-owned `Text` results now carry an explicit cleanup contract through
the Flowmini chain. The `Text` ABI type declares
`flow_text_dispose`. Plan-v2 provider operations publish the matching
`result_resource`, and non-entry providers publish the destination owner
symbol used for transfer checking.

The bounded `text_runtime` specimen now proves the complete one-hop path:

```text
text_runtime.concat -> join() -> main owner -> dispose(owner)
```

Flowbind requires a direct return transfer from `join`, checks every static
call boundary, and matches each cleanup to its specific owner. The LLVM and
TinyVM paths both lower and execute the same contract. Missing result
metadata, cleanup authority, or owner identity is refused by hostile tests.

## Verification

Build directory:

```text
.tmp/build
```

Focused commands:

```text
cmake --build .tmp/build -j2
ctest --test-dir .tmp/build -R '^(text_runtime_boundary|tinyvm_runtime_text_parity|text_outcome_boundary)$' --output-on-failure
tools/test-text-runtime.sh
tools/test-tinyvm-runtime-text-parity.sh
```

Focused result:

```text
build: PASS (70/70 targets)
CTest: 3/3 passed
LLVM Text runtime boundary: PASS
TinyVM/LLVM Text parity: PASS
```

Full canonical verification:

```text
cmake --build .tmp/build -j2: PASS
ctest --test-dir .tmp/build --output-on-failure: 207/207 passed
git diff --check: PASS
```

## Honest boundary

The cleanup proof is deliberately bounded to owned `Text` provider results,
direct return transfer, one static caller boundary, and explicit disposal. It
does not claim general resource-flow analysis, arbitrary multi-hop forwarding,
loop cleanup, asynchronous cleanup, or complete exception unwinding.

