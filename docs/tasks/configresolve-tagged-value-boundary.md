# ConfigResolve tagged scalar Value boundary

ID: `lyraform.configresolve-tagged-value-boundary`; revision 1. This mission
resumes the ConfigResolve port after the finite record and Text slices.

## Measurable goal

Establish an evidence-backed, generic Lyraform carrier for the five scalar
`Value` alternatives required by ConfigResolve: `Null`, `Bool`, `Int`,
`Double`, and `String/Text`. The carrier must preserve one active alternative,
typed access without implicit fallback, independent copies, and explicit
failure for invalid access. It must be representable in the backend-neutral
artifact and either execute identically on native LLVM and TinyVM or refuse
before execution with a source-linked reason.

## Why ordinary records are insufficient

The reference implementation uses an exclusive `std::variant`. A record with
one tag plus several payload fields would admit invalid states unless the
language and every consumer enforce the invariant. It would also obscure
ownership and floating-point policy. The existing `Outcome<Text,TextFailure>`
carrier is related evidence, but it is not a general scalar union.

## Initial boundary

The first implementation slice may cover only `Null`, `Bool`, `Int`, and
`Text`, with `Double` retained as an explicit refused alternative until its
finite-value, signed-zero, NaN, and infinity policy is decided. It must not
silently use presentation formatting for equality or turn host pointers into
language values. Loaders, collections, schemas, diagnostics, stores, views,
and C ABI handles remain outside this mission.

## Evidence

The reference contract is `experiments/configresolve/contract-matrix.json`,
`../ConfigResolve/include/configresolve/value.hpp`, and
`docs/architecture/text-value-v0.1-proposal.md`. Existing
`Outcome<Text,TextFailure>` evidence is documented in
`docs/architecture/text-outcome-v0.1.md`.
