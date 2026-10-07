# ADR-0061: Legacy implementation is deprecated and oracle-only

Date: 2026-09-26.

Status: accepted.

## Decision

The Flowmini parser, `ModuleSpec` model, direct runtime, and their fallback path
are deprecated legacy implementation. They are excluded from Lyraform's
canonical source, semantic, artifact, planning, and execution authority.

Legacy behavior may be observed to:

- recover historical intent;
- identify semantic questions and compatibility obligations;
- provide differential and regression evidence;
- guide the design of explicit canonical contracts and negative tests.

An observed legacy behavior does not become Lyraform semantics merely because
the old runtime accepts or executes it. Promotion requires an explicit language
decision, canonical structural representation, semantic facts, versioned
contracts, fail-closed validation, and applicable backend evidence.

## Operational consequences

- `igor run` uses the bounded canonical staged compiler path and never falls
  back to legacy execution.
- `igor run-canonical` remains an explicit alias during convergence.
- `igor run-legacy` is the only Igor entry point to the deprecated runtime and
  emits a deprecation warning.
- Legacy plan v1, `parseModule`, `ModuleSpec`, and direct-runtime results are
  oracle evidence only. They cannot authorize a canonical artifact or backend.
- Existing legacy tests remain useful when clearly classified. A historical
  pass result means the oracle exhibited that behavior, not that canonical
  Lyraform admits it.
- No new language feature is implemented in the legacy path. Maintenance is
  limited to preservation, containment, diagnostics, and evidence collection.

Stable historical protocol identifiers and source extensions are separate
compatibility contracts and are not renamed by this decision.

## Rationale

Keeping the legacy runtime as a default execution route leaves two semantic
authorities and allows implementation accidents to masquerade as language law.
Deleting it immediately would discard valuable evidence. Explicit deprecation
preserves that evidence while making the authority boundary unambiguous.
