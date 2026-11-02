# Unreachable invalid branch vertical slice — 2026-11-02

## Decision

The compiler validates the complete source tree during semantic admission. A
branch that cannot execute for a statically known condition is still part of
the program contract: invalid names, invalid operand types, and statically
provable arithmetic faults must be rejected before binding, lowering, or
execution.

This is a validation boundary, not an instruction to execute dead code. A
valid unreachable branch remains legal source and may later be optimized while
preserving the already-established semantic facts.

## Evidence

`lyraform_unreachable_invalid_branch` covers:

```text
if false { 1 / 0 -> return } else { 0 -> return }
if true  { true + 1 -> return } else { 0 -> return }
if false { missing_name -> return } else { 0 -> return }
```

Each case is rejected by Flowanalyst with a source-provenanced diagnostic and
a blocked lowering plan. Flowbind and Flowparallel also refuse the blocked
artifact. The source-operation coverage remains complete, proving that the
invalid branch was inspected rather than silently omitted.

A valid `if false` specimen is carried through binding, scheduling,
optimization, preparation, Flowcontracts validation, LLVM lowering, TinyVM
lowering, TinyVM validation, and TinyVM execution.

The focused gate passed on the clean numeric-slice build:

```text
lyraform_unreachable_invalid_branch: PASS
```

No execution result from a valid branch is used to excuse an invalid sibling
branch.
