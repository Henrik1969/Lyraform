# ADR-0077: Bounded native source-disposition callable bridge

Status: accepted bounded implementation slice — 2026-11-04

## Decision

The native LLVM lowering unit may emit one private callable bridge for the
already validated `TextOutcome` disposition specimen when all of these facts
are present:

- the lowering plan is callable-plan v2;
- the disposition projection is graph v3, validated, serial, and still
  marked `execution: unsupported`;
- exactly one policy-selected `recover` response exists;
- the producer, immutable failure envelope, response, and typed rejoin have
  exact callable identities and carriers;
- the response returns a statically declared `Text` string literal; and
- the source, policy, host-policy, wire, containment, commitment, and
  provenance evidence is available for embedding in the authority descriptor.

The emitter produces `flow.source.disposition`. The generated callable invokes
the existing producer, response, and rejoin symbols through the private
activation runtime. The runtime validates the copied authority, preserves the
failure envelope, closes the original obligation on recovery, and disposes a
provider-owned successful `Text` result exactly once.

## Boundary

This is a generated native callable probe, not public source-graph admission.
The semantic source disposition, lowering plan, and TinyVM projection remain
`execution: unsupported`. The public `flowlower` path continues to refuse the
blocked graph. The bridge is not emitted for multiple routes, missing
authority, non-static response ownership, different carriers, or a different
graph policy shape.

The first response contract is deliberately borrowed/static: the admitted
fixture returns a string literal, so the bridge does not dispose the recovered
fallback. General response-result ownership is not established by this ADR and
remains a lifecycle slice.

## Rationale

The compiler must carry the existing semantic identities into generated code;
a handwritten host driver must not become the implementation of source
recovery. The runtime remains a transition validator, not a policy selector or
source interpreter. Keeping the bridge private and shape-gated provides a
real native vertical slice while retaining the honest refusal boundary for the
general graph and for TinyVM.
