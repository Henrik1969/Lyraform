# ADR 0057: Diagnostics explain from canonical evidence

## Status

Accepted on 2026-09-19.

## Context

A refusal that merely says “invalid program” is safe but unnecessarily hostile
to the developer. Conversely, a friendly message that guesses, hides relevant
facts, or differs from debugger evidence makes safety analysis harder.

Lyraform should learn from Rust's emphasis on localized, explanatory compiler
diagnostics while preserving Lyraform's own semantic vocabulary and
machine-inspectable architecture.

Diagnostics remain observations. They explain a refusal, failure, or fault;
they do not consume, recover, or terminate its semantic disposition.

## Decision

Developer-facing diagnostics and debugger/machine views are projections of the
same canonical diagnostic and semantic evidence.

The human projection should make a refusal or runtime observation a learning
experience. Where the facts exist, it identifies:

```text
what happened
where it happened
which semantic entity produced it
which types, contracts, effects, guards, ports, or routes are involved
why the program is unsafe or inadmissible
which surrounding declaration or boundary owns the obligation
which classes of repair are legal
where related declarations and prior causes originate
```

The presentation may use source excerpts, primary and secondary spans, labels,
notes, and concise examples. It should lead with the root cause and suppress or
clearly mark cascading consequences.

## Repair suggestions

A diagnostic may offer a concrete edit only when that edit is mechanically
valid and does not require guessing the developer's intended semantics.

When several meanings are possible, the diagnostic explains the alternatives
instead of selecting one. For a dangling failure/fault wire, legal repair
classes may include:

```text
connect a compatible recovery or transformation
declare propagation through the enclosing boundary
connect an explicit top-level failure-policy sink
connect a fault to an authorized containment boundary
```

The compiler must not suggest dropping the disposition, converting it to
success, or adding an implicit termination path.

## Machine and debugger projections

Structured output is deterministic, versioned where public, and free from
personality text. It preserves at least the stable diagnostic identity,
severity/disposition, source origins, semantic identities, relevant type or
contract facts, route/provenance facts, related locations, and admissible repair
classes when known.

A debugger should be able to inspect an admitted runtime disposition without
reverse-engineering a human string. For a failure or fault it should expose,
subject to data-disclosure policy:

```text
producer and operation identity
disposition and payload type identity
current route and destination
origin and propagation provenance
commit/no-commit state
related guard, contract, provider, wire, and activation identities
causal observations
```

Human text and structured evidence may differ in presentation but must not
disagree in meaning.

## Sensitive evidence

Candidate values, provider details, paths, and provenance may contain secrets.
Diagnostics and debugger views apply declared disclosure and redaction policy.
Redaction must be visible; it must not fabricate a harmless replacement or
change the semantic classification.

## Dangling-wire example

A useful human diagnostic is conceptually:

```text
error: failure route has no destination
  operation: guarded placement into `x`
  produced: Failure<GuardViolation>
  guard: `positive`
  route ends at: function `sample`

  this failure cannot be discarded: callers would have no accountable result

  close the wire by declaring propagation, connecting compatible recovery,
  or routing it to an explicit top-level failure policy
```

Exact wording is not frozen by this ADR. The semantic facts and stable
machine-readable classification are the authority.

## Consequences

- Compiler refusals teach the relevant Lyraform rule instead of requiring
  folklore.
- Debuggers and automation consume structured evidence rather than parsing
  prose.
- Human and machine paths cannot drift into different explanations.
- Suggestions remain conservative and cannot silently choose program meaning.
- Diagnostics can improve over time without changing the underlying semantic
  contract.

## Deferred

This ADR does not define:

- exact diagnostic codes or JSON schema versions;
- terminal coloring and rendering conventions;
- IDE protocol integration;
- debugger transport;
- localization;
- diagnostic retention policy;
- the canonical failure/fault carrier selected by the pending projection
  decision.
