# ADR 0060: Uniform transfer of owned obligations

Status: accepted by Henrik on 2026-09-26.

## Decision

Use one canonical ownership-transfer mechanism throughout the compiler chain.
An owned value's transfer preserves its semantic completion identity, payload
type, commit law, and outstanding obligations while replacing its current
owner. It neither clones the obligation nor discharges it. The old owner may
not subsequently use or clean up the transferred value.

Flowanalyst establishes transfer facts. Flowcontracts validates the relations.
Binding, planning, optimization, preparation, and backends preserve those
facts and refuse unsupported projections. A backend cannot infer ownership
from a carrier or introduce its own transfer law.

The mechanism is type-independent. TextOutcome is an initial end-to-end
carrier test, not a separate ownership model. Other owned types and boundary
kinds use this same authority as their representation support matures.
Ordinary independent values remain governed by ADR 0052; this decision does
not silently turn every scalar or ordinary aggregate copy into a destructive
move. Owned obligations cannot be implicitly duplicated by such copying.

## First executable projection

One owned producer, one direct function return, and one caller destination,
followed by already-admitted local handling. The transfer contract records
source/destination ownership, function and operation identities, payload type,
and preserved obligation identity. Unsupported additional transfers, parameters,
recursion, aliasing, fan-out, and containers are explicitly refused until
admitted by extensions of this same mechanism.

## Unification constraint

No per-type return-transfer rule, stage-specific ownership state, or backend
ownership exception may substitute for the canonical contract. Supported
coverage may grow incrementally; the meaning of transfer remains uniform.
