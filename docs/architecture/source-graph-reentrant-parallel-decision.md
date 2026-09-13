# Reentrant and parallel graph schedules

Status: schedule-contract implementation phase opened after the bounded
aggregate payload checkpoint (2026-09-13).

## v0.33 bounded schedule contract

`flowcore.graph_provider_map` v3 may select
`schedule_policy: "parallel_independent_v1"`. The policy is explicit input;
it is never inferred from an implementation name, a node name, or a provider
effect. It currently applies only to executable, acyclic, fresh-receiver
graphs. Streams and persistent receivers remain separate schedule families.

Flowparallel publishes the ordinary topological activation list together with
`parallel_waves`. A wave contains activation identities whose input dependency
belongs to an earlier wave, so no activation in a wave consumes another
activation in that same wave. Wave order and activation identities remain
deterministic even though a future worker runtime may dispatch a wave
concurrently. Fan-out retains one source output signal and distinct delivery
identities.

## v0.34 bounded worker runtime

Version 4 schedules now have a deliberately narrow native execution contract.
The runtime receives one immutable native `i32`/`i64` value per activation,
starts one worker for each activation in the current wave, and joins the
complete wave before publishing any result to the next wave. Lowering requires
one startup provider and return-only scalar or verified aggregate receiver
bodies; this is the current proof that worker-local evaluation has no shared
mutable state or borrowed payload lifetime. Joined results are recorded as
`flowcore.graph_parallel` evidence when graph tracing is enabled. Verified
aggregate payloads up to eight bytes use the same native carrier lane as their
serial graph counterpart.

`flow_graph_fail` remains process-fatal. A worker failure therefore cannot
publish a partial result or advance a later wave, and no state commit occurs.
There is not yet a recoverable cancellation API, a worker pool, or
persistent-state parallel contract; those are explicit future slices. A
requested version-4 schedule still cannot be silently
downgraded to serial execution.

Reentrant receiver pipelines continue to use the existing topological
activation identity law.

The finite-stream family now has a separate version-5 linear pipeline shape:
one root delivery may feed an acyclic chain of fresh receivers when each edge
preserves the declared carrier type. Each item is evaluated through the chain
in FIFO source order. Branching or merging stream pipelines remain outside
that contract; direct root fan-out remains the version-2 stream shape.

The next gate is a recoverable cancellation policy and persistent-state
interaction boundary; persistent state cannot be shared by parallel
activations.
