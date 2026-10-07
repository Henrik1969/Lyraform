# Canonical TextOutcome ownership authority — Stage 2D

Date: 2026-09-27.

## Objective

Audit the existing bounded TextOutcome, disposition, and ownership-transfer
path for duplicate semantic producers. Connect only already-accepted direct
return and one-forwarder unique-transfer behavior.

General failure propagation, policy sinks, additional forwarding hops, owned
parameters, aliases, recursion, shared ownership, containers, and new language
syntax remain excluded.

## Scout result

Flowanalyst is already the sole semantic producer for:

- atomic `Outcome<Text,TextFailure>` production;
- one must-account obligation;
- complementary success/failure branch accounting;
- success-only borrowed payload use and exactly-once disposal;
- explicit nonempty failure behavior;
- direct unique return transfer or exactly one forwarding owner.

Flowcontracts independently reconstructs the operation/function relation and
validates exact producer, return, call, owner, function, type, obligation,
branch, use, cleanup, recovery, and provenance relationships. Carrying stages
preserve the disposition fact exactly. LLVM and TinyVM validate the same
authority before lowering.

One small construction duplication remained: Flowanalyst independently rebuilt
the must-account identity string and owned carrier type while constructing
transfer facts and again while emitting the disposition obligation.

## Implementation

The producer's `OutcomeAccounting` record now establishes exactly once:

```text
obligation identity = operation:<producer operation>:outcome
owned value type    = producer return type
```

Every direct-return or forwarding transfer consumes those fields. The final
disposition obligation consumes the same identity. No schema, serialized
identifier, ownership meaning, failure code, or source behavior changed.

The positive corpus now explicitly proves that:

- disposition identity derives from its producer operation;
- success and failure routes name the producer's initial owner;
- each transfer retains the producer provider's return type;
- each transfer retains the exact disposition obligation identity.

## Initial focused evidence

- `text_outcome_boundary`: PASS;
- carrier-independent ownership-transfer contract: PASS;
- guard/disposition regression: PASS;
- combined focused result: **3/3 PASS**, 10.27 seconds;
- existing corpus retains eight LLVM/TinyVM execution cases, twenty-nine
  source refusals, fifty-one hostile mutations at seven format-correct
  consumers, three preserved carrying stages, and bounded forwarding.

## Final verification

- canonical Igor suite: **170/170 PASS**, 83.20 seconds;
- ASan/UBSan focused gate: **3/3 PASS**, 18.47 seconds, covering
  `text_outcome_boundary`, the ownership-transfer contract, and the
  guard/disposition lifecycle;
- Valgrind direct-return Flowanalyst probe: **0 errors**, 0 bytes in 0 blocks
  at exit;
- Valgrind one-forwarder Flowanalyst probe: **0 errors**, 0 bytes in 0 blocks
  at exit;
- Flowcontracts validation of both emitted semantic reports: **PASS**;
- repeated one-forwarder semantic emission: byte-identical;
- current-documentation drift guard: **PASS**;
- canonical-authority drift guard: **PASS**;
- legacy-deprecation drift guard: **PASS**;
- Igor doctor: **PASS**;
- repository diff whitespace check: **PASS**.

The sanitizer tree initially lacked executables required by the selected CTest
closure. They were built without changing source or configuration, after which
the unchanged focused command passed completely. This was an incomplete build
tree, not a semantic or sanitizer defect.

## Gate 2D

**PASS.** The bounded TextOutcome producer now establishes its obligation
identity and carrier type once. Direct-return and one-forwarder transfer facts,
the disposition obligation, validators, carrying stages, and both backends
consume or preserve that same authority. No new language or ownership meaning
was introduced.

Recommended next stage: audit provider/effect admission authority between
Flowanalyst requirements and Flowbind's exact provider selection. Keep ABI/FFI,
scheduling, and general failure-flow semantics outside that bounded audit.
