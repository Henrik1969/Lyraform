# ADR-0080: Provider-owned results may cross one bounded forwarding function

Status: accepted bounded implementation slice — 2026-11-05

## Decision

The cleanup contract from ADR-0079 may cross one additional non-entry function
before reaching an entry caller:

```text
provider-owned Text result
    -> direct return
    -> one forwarding function owner
    -> direct return
    -> entry caller owner
    -> owner-specific flow_text_dispose
```

The compiler and binder require every static call boundary to have a
destination owner. The forwarding function must directly return that owner;
it may not inspect, copy, replace, branch on, clean up, or otherwise reuse the
owned value. The final caller must be an entry function and must clean up the
specific destination owner. Direct provider-to-entry calls remain valid.

The same `result_resource` contract and `flow_text_dispose` capability are
preserved through semantic analysis, binding, lowering, LLVM, and TinyVM.
Owner identity remains the authority for cleanup matching.

## Boundary

This admits only one forwarding function for the current owned `Text` provider
result slice. A third function boundary, missing return transfer, multiple
return paths, missing destination owner, forwarding inspection, loops, async
transfer, shared ownership, fan-out, and owned function parameters remain
refused. This decision does not define source syntax for borrowed or consuming
parameters; that is a separate lifecycle decision.

External/global resources such as the ncurses window retain their existing
capability-based cleanup semantics and are not made owner-sensitive by this
slice.

## Rationale

One-hop forwarding is the smallest useful extension after direct provider
result cleanup. It reuses the existing unique-transfer law instead of adding a
Text-specific ownership meaning, while the explicit hop bound keeps the proof
finite and prevents accidental general resource-flow inference.

