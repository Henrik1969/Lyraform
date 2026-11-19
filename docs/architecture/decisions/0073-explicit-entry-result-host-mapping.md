# 0073 — Explicit language entry result and selected host mapping

**Status:** accepted model; declaration syntax and executable mapping pending review  
**Date:** 2026-11-01  
**Mission:** `lyraform.contract-stabilization`, revision 1

## Owner authority

The owner approved the proposed second entry-point model and work order:
finish numeric/predicate stabilization, classify failures, migrate by intent,
verify both backends, and revalidate G1 before closing G2. The entry point has
an explicitly declared language result type. An explicitly selected host
mapping accounts for that result at the process boundary. The owner then
requested a concrete mapping proposal before adding declaration syntax.

The result type is not implicitly int64: the source chooses a supported,
explicit type. The host boundary cannot infer a mapping from equal machine
widths, successful compilation, target architecture or a backend convention.
ADR 0072 governs conversions, including widening and narrowing here.

A mapping must preserve the application's full result until the declared
mapping operation consumes it. An exact numeric conversion requires proof of
representability, or an admitted check and declared failure route. A deliberate
application classification, if explicitly written as ordinary computation,
is distinct from a numeric conversion; no compiler-generated classification
or fallback is authorized. Unsupported or missing mapping routes refuse.

## Current implementation and review boundary

Ordinary functions have explicit result types. MainBlock currently has no
result-type field; Flowanalyst inserts c_int for it. There is no accepted
source declaration or implemented target-policy field selecting a host-result
mapping. Existing implicit main behavior is compatibility evidence, not this
model's implementation. Do not add syntax or claim host mapping execution
until the [concrete proposal](../../tasks/entry-result-host-mapping-proposal.md)
has been reviewed. Existing tests remain evidence of the compatibility path.

This decision settles the model previously left open in ADRs 0070–0072.
It does not approve a new numeric failure type, handler, status code, target
schema, host ABI alias, or general narrowing implementation.
