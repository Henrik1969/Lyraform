# 0071 — Explicit integer width in concrete source types

**Status:** accepted owner decision; migration in progress  
**Date:** 2026-11-01  
**Mission:** `lyraform.contract-stabilization`, revision 1  
**Supersedes:** ADR 0070's default `int` alias decision only

## Decision and provenance

After discussing fixed aliases and policy-selected defaults, the owner explained
that a reader seeing `int` can presume the wrong width. The owner explicitly
approved enforcing width-bearing integer types and diagnosing bare `int`, with
migration based on intended ranges and boundaries rather than blind replacement.

Concrete integer declarations and signatures must state their width, using the
existing primitive identities such as `int32` and `int64`. Bare `int` must not
silently select a width, either from the target or from a default policy.
The compiler diagnoses it and asks the developer to choose a concrete width.
This includes nested type arguments, record fields and refinement base types.
Raw source/AST remains inspectable and preserves spelling; recognition does not
establish executable admission.

A target must implement the selected concrete contract or refuse compilation.
It cannot substitute its native width. No `short` spelling, new generic integer
syntax or source contract language is introduced. The conceptual family `intN`
is not by itself approval of a new executable generic/metatype facility.

ADR 0070's explicit conversion and overflow-routing requirements remain binding.
ABI contract types retain their separate declared contracts; they do not become
width-independent aliases for language integers. Literal contextual typing and
the host entry-result boundary need their own existing or explicitly approved
contracts; this decision does not choose an implicit process-status mapping.

## Migration and evidence

Preserve historical examples as dated evidence. Review active source declarations,
signatures, records and provider boundaries before selecting concrete widths.
Do not mechanically substitute int64 across the repository. Old captured plans
containing executable bare-int types cannot bypass current admission by omitting
optional facts or changing readiness/version claims.

G1's passed snapshot predates this source migration. G2 remains open, including
full backend width support, numeric safety, conversion intent and fixture migration.
The new admission tests are not proof that every explicit integer family executes.
