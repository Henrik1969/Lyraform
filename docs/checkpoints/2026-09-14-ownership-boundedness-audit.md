# Ownership, cleanup, and boundedness audit

**Date:** 2026-09-14  
**Status:** inventory in progress; Gate 3 remains open

## Existing controls

- TinyVM artifact destruction releases every owned section and reinitializes
  the artifact, including malformed-load and partial-load paths.
- TinyVM runtime-provider teardown closes tracked file descriptors, releases
  owned strings and text outcomes, and frees storage bytes and initialization
  maps. Allocation failure during storage setup leaves the provider destroyable
  and does not publish the incomplete storage handle.
- Graph execution has explicit activation-record and pending-queue bounds;
  overflow is rejected rather than silently drained or downgraded.
- Text and file providers expose explicit outcome/failure values and the
  canonical suite covers normal cleanup, invalid handles, exhaustion, and
  provider-loss behavior for the admitted paths.
- The CUDA provider wrappers close their dynamic libraries and perform
  explicit device-resource cleanup on their currently admitted linear paths.

## Implemented in this slice

- The CUDA graph executor now tracks device buffers and the cuBLAS handle in
  one explicit cleanup owner. Cleanup is idempotent, runs on both success and
  failure, and reports the first cleanup error instead of silently discarding
  it. The public process boundary also labels an unknown non-standard failure.
- The CUDA matrix executor now uses the same explicit owner and reports the
  first cleanup failure on both exceptional and normal paths.
- The shared owner is exercised without CUDA hardware using injected provider
  callbacks for complete, partial, failing, and repeated cleanup sequences.
- CPU worker ownership uses `std::jthread` in a scope that joins all launched
  workers before publishing `ExecutionResult`. Launch failure is translated to
  an explicit error, including when only a partial worker set was started.

## Gaps requiring Gate 3 work

- CUDA/device allocations still need injected provider-failure tests covering
  every allocation, transfer, and execution operation; the owner-level
  cleanup cases are now covered.
- The runtime-provider and artifact allocators have bounded behavior checks,
  but do not yet expose systematic allocation-failure injection at every
  ownership-transfer point.
- Generic ownership categories (owned, borrowed, provider-owned,
  runtime-owned, and observed) are not yet represented in one cross-provider
  contract matrix.
- Thread-launch fault injection is not yet available; the join-safe behavior
  is covered by the normal and task-failure execution gates.
- Cancellation cleanup, asynchronous queue admission, and backpressure are
  intentionally not admitted; Gate 4 must define those contracts before they
  can be counted as covered.

## Safety disposition

No new ownership semantics are admitted by this audit. Existing bounded paths
remain available, while the identified CUDA cleanup and fault-injection gaps
remain explicit residual risk and block claiming Gate 3 complete. No FlowLFS
or `master` content was touched.
