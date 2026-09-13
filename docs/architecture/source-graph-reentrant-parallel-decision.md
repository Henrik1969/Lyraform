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

This checkpoint is intentionally schedule-only. Flowlower refuses version 4
parallel schedules until the worker runtime contract exists; it cannot silently
turn a requested parallel schedule into serial execution. Reentrant receiver
pipelines continue to use the existing topological activation identity law.

The next gate is a worker runtime with explicit join, failure, cancellation,
and side-effect/lifetime proof rules. Aggregate payloads may participate only
after those rules preserve immutable value delivery; persistent state cannot be
shared by parallel activations.
