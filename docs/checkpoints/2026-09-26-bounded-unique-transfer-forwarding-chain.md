# Bounded unique-transfer forwarding chain — 2026-09-26

## Baseline and scope

This stage continued on `main` from synchronized HEAD and local
`origin/main` `ecdac7184236dab24423ca7495e882f771cdf287`. Existing current-truth
documentation changes were retained. No file was staged, committed, or pushed;
`master`, `flowlfs-v0.1-alive`, and the unrelated untracked
`meta-discusions.md` were untouched.

ADR 0060 already established uniform unique transfer. The bounded question was
whether the same owned obligation could cross one non-entry forwarding owner
without copying, discharge, backend-specific ownership, or a second transfer
meaning:

```text
producer -> direct return -> forwarder -> direct return -> entry caller
         -> existing complete local accounting
```

## Implementation and authority

The direct-return `lyraform.ownership_transfer` version 1 fact remains
unchanged and compatible. The one-forwarder projection uses an ordered
two-element `ownership_transfers` collection of that same fact. The shared
Flowcontracts law requires:

- exactly two hops;
- identical value type and obligation identity;
- exact adjacency of destination/source owner and function identities;
- the first call operation to be the second producer operation;
- non-entry producer and forwarding functions and one final entry caller;
- one static caller, nullary calls, direct return, and no old-owner reuse at
  each hop.

Flowanalyst discovers and validates at most two hops, then performs the existing
outcome-accounting proof at the final owner. Flowvalidate, Flowbind,
Flowparallel, Flowoptimize, Flowprepare, LLVM lowering, and TinyVM lowering
reconstruct and validate the ordered relation. The carrying semantic, parallel,
optimized, and lowering projections preserve it exactly.

The generic shared-law tests use `OwnedBuffer`, `OwnedSocket`, and
`Outcome<Packet,IoFailure>` labels. These are type-independence contract tests,
not new executable source-type claims. `TextOutcome` remains the first
end-to-end executable carrier.

## Evidence

- Eight bounded programs execute with matching LLVM and TinyVM behavior,
  including forwarded success and forwarded provider failure.
- Twenty-nine invalid source programs are refused. The forwarding additions
  cover copy, stale-owner use, inspection, producer fan-out, caller fan-out,
  and a third transfer hop.
- Fifty-one hostile mutations are rejected at each of seven format-correct
  consumer boundaries. Ten forwarding-chain mutations cover missing, empty,
  reversed, overlong, shared-mode, changed-obligation, changed-type,
  changed-owner, changed-function, and changed-operation relations.
- Generic contract tests cover valid chains and disconnected obligation,
  stale-owner, and branched-caller refusals for three carrier labels.
- The existing singular direct-return projection remains in the same gate and
  passes unchanged compatibility checks.

## Verification

- `./igor doctor`: PASS.
- `./igor build`: PASS; the pre-existing unrelated `graph_cuda.cpp`
  unused-function warning remains.
- First canonical run: **166/167 PASS** in 84.69 seconds. The only failure was
  the current-documentation drift guard still naming the preceding semantic
  checkpoint after this page became current; no compiler, contract, runtime,
  or backend test failed.
- Focused `text_outcome_boundary`, `flowcontracts_ownership_transfer`, and
  current-documentation tests before checkpoint rotation: **3/3 PASS**.
- Clang 18 ASan/UBSan focused outcome, ownership-transfer, and guard tests:
  **3/3 PASS**, 22.40 seconds, with LeakSanitizer enabled outside sandbox
  tracing.
- Valgrind forwarding probes for analysis, semantic validation, LLVM execution,
  and TinyVM execution: **4/4 clean**, zero errors and zero bytes in use at exit.
- Final `./igor test` run after checkpoint rotation: **167/167 PASS**,
  83.49 seconds.

**Gate: PASS** for the bounded one-forwarder unique-transfer projection.

## Boundary and next stage

Supported: one nullary owned producer, either direct return or exactly one
nullary forwarding owner, one entry caller, and complete existing local outcome
accounting by that caller.

Still refused: a second forwarding owner, owned parameters, recursion,
containers, aliases, loops or branches inside forwarding, inspection or reuse
by the forwarding owner, fan-out, implicit propagation, and policy sinks.
Shared ownership and policy destinations remain separate semantic decisions.
