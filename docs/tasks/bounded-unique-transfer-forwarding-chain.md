# Bounded unique-transfer forwarding chain

Date: 2026-09-26.

## Objective

Extend ADR 0060's existing unique owned-obligation transfer authority through
one non-entry forwarding function without changing ownership meaning.

```text
owned producer
    -> direct return
    -> one forwarding owner
    -> direct return
    -> one entry-caller owner
    -> existing complete local outcome accounting
```

## Baseline

- branch: `main`;
- HEAD and local `origin/main`: `ecdac7184236dab24423ca7495e882f771cdf287`;
- uniform direct-return checkpoint: PASS;
- current-truth consolidation: PASS in the existing unstaged worktree;
- preserve all current documentation work and unrelated `meta-discusions.md`;
- no staging, commit, push, branch mutation, `master`, or FlowLFS work.

## Established authority

ADR 0060 already decides the semantics: every hop preserves one completion,
payload type, commit law, and obligation identity while replacing exactly one
owner. A hop does not clone or discharge the obligation, and an old owner may
not be reused or cleaned up.

This stage therefore introduces no new ownership candidate or syntax.

## Bounded admitted shape

- one nullary function produces the owned tagged outcome;
- one direct return transfers it to one nullary forwarding function;
- that function directly returns the received owner without inspecting,
  copying, replacing, or branching it;
- one entry caller receives the final owner;
- the entry caller completes the already-admitted local success/failure proof;
- exactly one static caller exists at each hop;
- the same obligation identity and value type cross both hops.

## Refused shapes

- missing, reordered, disconnected, duplicated, or cyclic transfer hops;
- different obligation or value type at any hop;
- multiple callers or destinations at either hop;
- forwarding-owner copy, reuse, overwrite, inspection, or cleanup;
- parameters, recursion, containers, aliases, loops, branches, and nested
  forwarding control flow;
- a chain that does not terminate at the admitted entry-owner accounting proof;
- implicit propagation, policy sinks, shared ownership, or fan-out.

## Contract and compatibility

Use the existing `lyraform.ownership_transfer` version 1 relation for each hop.
Represent a chain as an ordered collection of those same relations. Preserve
validation of the existing singular direct-return projection so previously
emitted artifacts remain consumable. No backend-specific ownership state or
Text-specific transfer law is permitted.

## Evidence

- generic carrier-independent two-hop contract tests;
- forwarding success and provider-failure execution on LLVM and TinyVM;
- direct-return regression coverage;
- deterministic source refusals for invalid chain shapes;
- hostile ordered-chain mutations at every current format-correct consumer;
- exact chain preservation through semantic, parallel, optimized, and lowering
  artifacts;
- canonical Igor build/test, focused ASan/UBSan, Valgrind, shell syntax, current
  documentation, and diff checks.

## Gate

PASS only when the two-hop chain has one uniform independently validated
authority, both backends execute the same admitted semantics, every unsupported
shape fails before execution, direct-return compatibility remains green, and
the full canonical suite passes.

## Result

PASS on 2026-09-26. See
[the bounded forwarding checkpoint](../checkpoints/2026-09-26-bounded-unique-transfer-forwarding-chain.md)
for exact implementation, refusal, sanitizer, Valgrind, and canonical-suite
evidence.
