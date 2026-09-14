# Lyraform pre-self-hosting safety-assurance mission

**Status:** active mission, started 2026-09-14 by Henrik
**Scope:** safety mechanisms required before beginning staged self-hosting
**Date:** 2026-09-14

## Objective

Harden and evidence the Lyraform/Frankencore safety boundaries before any
Stage 1 self-hosting implementation begins.

The mission must produce a reproducible assurance case for the admitted
language and runtime surface. It must also make every unsupported safety
feature explicit, so that absence of a feature cannot be mistaken for a safe
default.

This mission advances safety readiness. It does **not** claim NASA, avionics,
medical, industrial, security, or other formal safety certification.

## Governing safety principle

Lyraform does not use hidden exception handling as a safety mechanism.
Failures, cancellation, resource exhaustion, provider loss, and recovery are
explicit semantic results or lifecycle events with deterministic disposition,
diagnostics, provenance, and policy authority.

No operation may silently continue, retry, downgrade assurance, substitute a
provider, publish partial state, or convert unknown evidence into acceptance.

## Binding laws

- Constitutional laws cannot be overridden by project, provider, or operation
  policy.
- Discovery is evidence; authorization is a separate exact policy decision.
- Unknown, contradictory, malformed, expired, unsupported, or unverifiable
  input fails closed or becomes explicitly unresolved.
- Capability is not authority; authentication is not authorization; policy
  outcome is not evidence.
- Published artifacts and state revisions are immutable once exposed.
- Every state-changing action records actor, operation, policy, provenance,
  before/after evidence, outcome, and recovery disposition.
- Resource ownership, cleanup, cancellation, and failure disposition are part
  of the contract, not runtime folklore.
- Safety properties must be checked at every independent artifact boundary.
- A bounded implementation must reject work outside its contract rather than
  infer broader semantics.
- The FlowLFS branch `flowlfs-v0.1-alive` and historical `master` remain
  untouched and separate.

## Execution gates

### Gate 0 — freeze the safety baseline

- Confirm `main`, worktree, remote, branch topology, and current revision.
- Run `igor doctor`, `igor build`, `igor test`, and the canonical sanitizer
  and memory-review gates where available.
- Inventory every existing safety test and its exact contract claim.
- Record toolchain, host, kernel, library, sanitizer, and environmental
  exclusions.

**Exit:** the baseline is reproducible and every claimed green result has an
exact command, revision, and evidence location.

### Gate 1 — create the safety-case inventory

For each admitted operation, provider, artifact, resource, and lifecycle,
record:

- hazard and possible unsafe outcome;
- controlling contract and policy;
- enforcement point;
- diagnostic and provenance fields;
- cleanup/recovery disposition;
- positive, negative, malformed, mutation, exhaustion, and provider-loss tests;
- residual risk and whether it blocks self-hosting.

Classify each item as **implemented**, **compatibility**, **provisional**,
**future**, or **not claimed**. Do not use a test count as a substitute for
coverage of hazards.

### Gate 2 — explicit outcome and error-state model

- Inventory all current error, trap, status, outcome, and process-exit paths.
- Define the smallest shared semantic vocabulary for success, failure,
  unresolved, denied, cancelled, exhausted, unavailable, and unsupported.
- Preserve the distinction between a rejected attempt, an operational error
  state, and a committed state transition.
- Define deterministic lifecycle transitions, correlation identities,
  operator-action requirements, and temporary recovery artifacts.
- Ensure LLVM, TinyVM, CLI, and machine-readable projections preserve the same
  semantic result.

No catch-all exception or implicit unwind mechanism may be introduced as a
shortcut.

### Gate 3 — ownership, cleanup, and bounded resources

- Audit every owned, borrowed, observed, provider-owned, and runtime-owned
  value and resource.
- Prove cleanup on normal return, explicit failure, cancellation, exhaustion,
  provider disappearance, and partial initialization.
- Add boundedness checks for stacks, queues, buffers, handles, recursion,
  activation records, streams, and output sizes.
- Reject double cleanup, use-after-release, aliasing violations, stale handles,
  partial ownership transfer, and unbounded allocation.
- Keep compatibility pointer/storage behavior visibly separate from permanent
  language semantics.

### Gate 4 — cancellation, async, backpressure, and parallel effects

Define and implement only the smallest independently testable contracts:

- cancellation request, observation point, acknowledgement, and final state;
- whether cancellation is cooperative, preemptive, or unsupported;
- queue admission, capacity, overflow, drain, and backpressure behavior;
- effect/resource classes permitted in asynchronous or parallel execution;
- commit/abort rules for partial work and provider failure;
- ordering, joins, retries, idempotence, and reentrancy;
- deterministic serial projection where parallel execution is not admitted.

Effectful, nested, reentrant, distributed, or irreversible work remains
refused until its complete contract and fault-injection evidence exist. A
requested stronger schedule must never silently downgrade to serial execution.

### Gate 5 — provenance and durable error-state storage

- Implement or formally close the mutation-provenance contract before adding
  broader mutation.
- Preserve event, attempt, correlation, actor, provider, policy, revision,
  before/after evidence, atomicity, and rollback/recovery references.
- Add a project-local durable error-state/history boundary with validation,
  atomic append, incomplete-tail handling, quarantine, and explicit repair.
- Prove that complete committed history is never silently rewritten or erased.
- Test crash, torn-write, duplicate-event, replay, branch-reconciliation, and
  uncertain-recovery cases.

### Gate 6 — isolation, trust, profiles, and platform assurance

- Define the minimum practical assurance profile and its honest limitations.
- Implement provider-neutral isolation claims with independently checked
  resource, identity, filesystem, network, privilege, and teardown bounds.
- Add signed profile/trust declarations only with explicit key scope,
  expiry, revocation, rotation, and downgrade behavior.
- Separate supplier authentication, owner attestation, operator override, and
  execution permission.
- Build a platform matrix for Linux x86-64 first, then add platforms only
  through explicit adapters and evidence; never infer one host's guarantees
  from another.

Missing isolation or trust evidence must produce quarantine, unresolved, or
rejection according to policy—not a reassuring label.

### Gate 7 — governed ABI/FFI boundary

- Define the exact admitted ABI tuple: provider, library, symbol, convention,
  parameter carriers, return carrier, effect, ownership, layout, lifetime,
  cleanup, failure, and versioned evidence.
- Validate the tuple independently before lowering and again before runtime
  invocation.
- Keep raw addresses, host pointers, guessed layouts, and symbol existence out
  of portable authority artifacts.
- Add hostile tests for signature drift, library substitution, layout changes,
  missing symbols, invalid pointers, partial transfers, provider replacement,
  and unsupported carriers.
- Maintain an explicit unsupported inventory for arbitrary native ABI/FFI.

### Gate 8 — adversarial verification campaign

Run a fault-injection matrix covering:

- malformed and duplicate-key artifacts;
- identity, provenance, policy, version, and digest mutation;
- provider disappearance and conflicting provider claims;
- allocation, stack, queue, buffer, and activation exhaustion;
- cancellation at every admitted observation point;
- cleanup failure and partial initialization;
- scheduler reorder, duplicate delivery, dropped delivery, and reentrancy;
- crash/torn-write/replay/recovery scenarios;
- isolation setup and teardown failure;
- sanitizer, fuzz, differential, deterministic-output, and native execution
  evidence.

Every fault must have one deterministic disposition and a useful diagnostic.

### Gate 9 — self-hosting readiness review

Before Stage 1 begins, verify that the safety contracts needed by the first
self-hosted compiler slice are already available in the public language,
standard library, provider contracts, and artifact boundaries.

The review must identify any remaining Stage 0 privilege explicitly. A safety
gap is not closed merely because the Stage 0 C++ implementation handles it.

## Non-goals

Do not begin self-hosting, claim certification, implement arbitrary FFI,
generalize all scheduling, add hidden exception semantics, replace Frankencore
policy with a second policy engine, merge FlowLFS, modify `master`, rewrite
history, or force-push.

Do not expand the language surface without a concrete safety contract and
adversarial evidence.

## Evidence and checkpoint discipline

Each gate must leave:

- focused positive and negative tests;
- structured machine-readable evidence;
- exact commands and environment details;
- updated architecture/status documentation;
- a recorded residual-risk list;
- `git diff --check`, clean generated-output inspection, and a coherent commit.

At major checkpoints, run the complete canonical suite and the applicable
ASan/UBSan and Valgrind gates. Keep build trees and logs outside the checkout.

## Definition of done

- The admitted safety surface has a complete hazard/control/evidence matrix.
- All admitted failures have explicit, deterministic outcomes and provenance.
- Cleanup and bounded-resource behavior is proven across return, failure,
  cancellation, exhaustion, and provider loss.
- Cancellation, async, backpressure, and effectful parallelism are either
  implemented with complete contracts or explicitly refused.
- Mutation provenance and durable error-state handling are implemented for the
  scope claimed, or the exact remaining boundary blocks self-hosting.
- Isolation, trust, signed profiles, and platform limitations are honest,
  versioned, and testable.
- ABI/FFI admission is exact; arbitrary native ABI/FFI remains refused.
- Fault injection, sanitizer, fuzz, differential, determinism, and recovery
  evidence pass for the admitted scope.
- The self-hosting readiness review identifies no hidden Stage 0 safety
  privilege in the first planned Stage 1 slice.
- Documentation matches executable evidence and states clearly that no safety
  certification is claimed.

## Legitimate stopping conditions

Stop only when the mission is complete or when progress is blocked by a genuine
semantic safety decision, missing authority/key, unavailable isolation or
verification mechanism, or incompatible external dependency. Preserve all
passing work and record the exact blocker and smallest required decision.

Ordinary implementation effort, test failures that can be diagnosed, or an
unfinished future feature are not blockers; they are evidence for the next
gate or an explicit refusal boundary.
