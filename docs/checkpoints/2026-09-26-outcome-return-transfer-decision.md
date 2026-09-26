# Decision brief: returning a bounded owned outcome

Status: candidate A accepted by Henrik on 2026-09-26, with the explicit
requirement that all compiler stages use one uniform mechanism.
[ADR 0060](../architecture/decisions/0060-uniform-owned-obligation-transfer.md)
records the accepted law. The scout and candidate discussion below describe
the evidence at proposal time; the
[implementation checkpoint](2026-09-26-uniform-owned-transfer.md) records current support.

## Question

When a function returns a locally produced `Outcome<Text,TextFailure>`, does
the return transfer its existing must-account obligation and owned success
payload to the caller? What uses of the old owner remain legal?

## Established facts

- ADR 0059 says tagged storage transfers rather than discharges the obligation.
  Every reachable ownership exit must account for the disposition.
- ADR 0058 requires one completion identity to survive contextual projections.
- The current admitted shape handles a local outcome completely with two
  sibling branches and exactly one success cleanup.
- ADR 0052 makes ordinary aggregates independent values but explicitly leaves
  resource ownership and aggregates containing linear resources unresolved.
  It therefore does not authorize copying owned Text handles.

## Implementation at proposal time and scout evidence

The parser recognizes `fn make(): TextOutcome` and `outcome -> return`.
The semantic report retains callable result spelling and function identity.
LLVM and TinyVM already recognize TextOutcome carriers. These representation
facts are not a proof of source ownership transfer.

A fresh source probe produced a local tagged outcome, returned it from `make`,
and bound its result in `main`. Frontend export succeeded; Flowanalyst returned
exit 2 with `FLOWANALYST_DANGLING_OUTCOME_WIRE` because the producer's failure
was never inspected. Current source admission therefore does not implement
function-return propagation. No unsupported probe was executed.

The required additions are an explicit semantic transfer relation, caller
obligation admission, lifetime/cleanup authority, independently validated
cross-function identities, and preservation at both backend boundaries.

## Candidates

### A — unique transfer through return (recommended)

Returning the existing owned outcome transfers its obligation and success
payload to exactly one caller destination. The callee does not dispose or
continue using the transferred value. The caller must handle the outcome or
perform a separately admitted transfer; return does not manufacture a second
semantic completion.

Advantages: consistent with the current must-account law and exactly-once
cleanup; no implicit deep copy or shared ownership; a small inspectable first
implementation can retain existing return spelling.

Costs: requires an explicit invalidation/transfer rule and caller/callee
contract linkage. This is a new ownership decision, not merely a new backend
instruction. Optimizations must preserve the transfer identity. Parallel
fan-out remains refused without its own semantics.

First slice after acceptance: one producer function, one direct return, one
caller destination, and existing local handling in the caller. No recursion,
multiple returns, forwarding chains, parameters carrying outcomes, containers,
aliases, or concurrency. An already-disposed success payload cannot be
transferred. No new general `move` syntax is implied.

### B — permit copied/shared outcomes at the boundary

More than one destination may retain the result. This requires deciding
whether to clone the payload, share ownership, and how multiple consumers
account for one semantic completion without duplicate cleanup or lost failure.

Advantages: easier fan-out for some callers. Costs: clone behavior, allocation
failure, shared lifetime, obligation composition, synchronization, and backend
runtime support become prerequisites. Ordinary aggregate copying does not
settle these questions. This is substantially broader than the current slice.

### C — keep return transfer refused for now

Require local handling while defining a general ownership/lifetime model first.
This avoids a narrow rule that might later need generalization, but delays
composable fallible functions and broadens the design task.

## Recommendation and conditional stages

Choose A for the next bounded source-language decision. It carries forward the
existing single-obligation law without prematurely defining shared ownership.
This recommendation was subsequently accepted with the unification constraint.

After acceptance:

1. Define canonical transfer evidence and owner invalidation; test source
   admission and refusals before backend enablement.
2. Independently validate producer/return/call/destination linkage and retained
   completion identity; preserve it through carrying stages.
3. Execute the bounded return case on LLVM and TinyVM with positive/negative,
   hostile, sanitizer, and leak evidence.
4. Reassess forwarding chains and richer control flow as separate gates.

Runtime guard routes and top-level policy sinks remain later independent
decisions. No exception, implicit unwind, discard, or implicit termination is
introduced by this proposal.

## Original decision choices (resolved: A)

A. Unique transfer through function return, starting with the bounded slice.
B. Design copied/shared outcome semantics first.
C. Defer return support and define the general ownership model first.
