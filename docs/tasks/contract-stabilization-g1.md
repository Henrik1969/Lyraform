# Stabilization G1 — source admission and shared execution validation

Parent: [contract stabilization revision 1](contract-enforcement-stabilization.md).
Selected by Henrik's 2026-11-01 execution instruction. This bounded addendum
selects T0/T1 only; parent state records progress. No new semantic law.

Endpoint: canonical source refuses executable refined values without invariant
enforcement and unproved integer division; supported constant-safe division
executes consistently. Shared artifact consumers independently enforce these
same admission bounds without an optional proof flag or new source vocabulary.

Authority: ADRs 0054/0055 establish refusal versus explicit runtime failure;
the current source-form audit marks refinement evaluation unsupported. Existing
scalar types, operand carriers and canonical operations supply value identities.
Flowanalyst establishes admission; Flowcontracts validates carried executable
operands; downstream stages preserve them; backends execute only admitted plans.

Reuse scalar/operand facts and the existing plan, without a new proof schema.
Conservative constant evaluation is an implementation technique for proving
division definedness, not new integer-overflow semantics. An unresolved operand
cannot become known through reassignment, branch scope or a call. Refined-type
declarations may remain inspectable; values requiring their invariants cannot
be executable. Reject unknown executable carriers rather than silently erasing
their contracts. Preserve recognized record/ABI carrier boundaries.

Positive: 6/2 and an unmodified locally initialized denominator; existing valid
scalars/static guards. Negative: Positive(-1), zero division, minimum/-1 and
dynamic unsafe denominator. Hostile: mutate divisor, carrier, owner, readiness
or version; delete optional facts and attempt downstream admission. Test
functions, records and graph expressions where applicable. No new trap route.

Reproduce with the canonical pipeline and fresh builds; compare O0/O2 native
and TinyVM values/dispositions. Tier 3 required before architectural closure.
Any accepted invalid refined value, unchecked unsafe division or optional-field
bypass falsifies G1. Record source hashes and exact cases. Retire unchecked
fallbacks only where their replacement is proven; reconcile support status and
tests before G2. Numeric choices without accepted authority remain unresolved.

## Required Firetest repairs found during G1

The pressure run exposed three pre-existing Text-example leaks, two allocation
fault tests loading vendor CUDA libraries before their injected throw, and a
callable-ABI harness failing to link sanitizer runtimes. Repair these bounded
verification dependencies before closure: explicit disposal in the existing
Text examples and grants, deterministic library fixtures with observed unload
for allocation-fault tests, and inherited sanitizer flags in the LLVM harness.
No new ownership model, graph route, numeric default or provider behavior is
introduced. These repairs do not claim G4 source recovery completion.
