---
title: Flowanalyst current status
status: active-development
---

# Flowanalyst current status

The first implementation establishes the sibling boundary and the
`flowanalyst.semantic_report` version 1 output contract.

Implemented checks:

- preserve and report frontend diagnostics from Flowmini;
- duplicate symbol names within one exported scope;
- declared type spelling resolution against built-ins and exported type-like
  symbols;
- identifier and call-name resolution through exported scope ancestry;
- resolved-name facts and semantic dependency edges;
- callable arity checking for resolved calls;
- refined-type base resolution and invariant binding checks;
- each named target must expose exactly one `main` procedure.
- bounded named-guard lifecycle and straight-line scalar preservation proofs;
- canonical disposition facts for statically proven guarded scalar
  transitions, with exact operation, type, commit, route, guard-proof, owner,
  and provenance linkage.
- canonical must-account disposition facts for the bounded
  `Outcome<Text,TextFailure>` source path, including exact tagged owner,
  code-projection, complementary branch, success-value/disposal,
  failure-recovery, failure-code, and provenance linkage;
- deterministic early refusal of unhandled or incompletely handled bounded
  outcomes as `FLOWANALYST_DANGLING_OUTCOME_WIRE`.

The report also exposes the first semantic analysis graph and Boolean sparse
matrix view. It is the current green-flag input for downstream stages when its
status is `ok`; the final integrity pass and broader semantic check family are
still expansion work.

Flowanalyst does not construct Graph IR, optimize, select named target
artifacts, or execute code. It establishes the semantic facts and consumer
contract those later stages use.

Runtime-dependent guard transitions remain refused. Flowanalyst does not yet
define general failure routing, recovery syntax, top-level failure policy, or
fault containment. Outcome copy, alias, container, loop, and nested
control-flow accounting also remain outside the admitted slice.

Gate 2 hardening additionally checks statement order, sibling control-flow
entry, exits before accounting, unchanged tag/zero operands, and cleanup after
the final direct value use. Non-cleanup uses must be direct read-only call borrows;
local payload aliases are refused. These bounded execution checks are shared with the
independent artifact validator. Call status destinations must not overwrite the
outcome tag used by subsequent branch tests.

The consolidation stage maps guard/outcome fact and diagnostic locations through
frontend import source maps. Dangling-outcome messages distinguish supported
local handling and direct unique return transfer from further propagation and
policy sinks, which remain unavailable.
See [the consolidation checkpoint](../docs/checkpoints/2026-09-26-post-gate-2-consolidation.md)
and [the uniform transfer checkpoint](../docs/checkpoints/2026-09-26-uniform-owned-transfer.md).
One nullary producer may directly return its owned outcome to one entry caller.
The shared ownership contract preserves obligation identity and changes its owner;
the caller must complete the existing local accounting proof.
