# ADR 0066: Source declares disposition consumers; policy selects authorized routes

## Status

Accepted architectural and language decision on 2026-10-10.

## Context

ADRs 0054 through 0063 establish explicit failure flow, distinct refusal,
failure, and fault dispositions, complete accountable routing, typed failure
consumers, and bounded evidence ownership. The compiler now has declarative
route, diagnostic, and reference execution contracts, but source does not yet
establish the producer-to-consumer association or the response function's
failure-envelope and transition meaning.

Leaving that association entirely to an external artifact would allow
configuration to change program meaning. Inferring it from function names,
return statements, dynamic scope, or graph proximity would create hidden
dispatch. A policy nevertheless needs to choose between several responses
that the program deliberately authorizes.

## Decision

Program source owns disposition meaning and legal route topology. Versioned
policy selects only among routes already authorized by source semantics.
Runtime executes the selected route without adding meaning.

The canonical source model contains four related declarations.

### Producer disposition clauses

A function may declare closed failure and fault sets:

```lyraform
fn read_sensor(): Reading fails { ReadFailure } faults { IntegrityFault } {
    ...
}
```

Omitting `fails` or `faults` means the corresponding set is empty. The sets
are finite, closed, and resolved by semantic type identity. Repetition,
unknown types, open sets, wildcards, and contradictory derived carrier facts
are admission refusals.

An existing canonical tagged outcome may derive the same closed disposition
facts without immediately requiring a rewritten producer declaration. If an
explicit clause and a derived carrier fact both exist, they must agree exactly.

### Failure-envelope parameters and response transitions

An ordinary function may receive one typed, read-only failure-envelope
projection and declare exactly what successful completion of that function
means:

```lyraform
fn use_cache(problem : failure ReadFailure): recover Reading {
    ...
}

fn report_outage(
    problem : failure ReadFailure
): transform SensorUnavailable {
    ...
}
```

`failure E` is a dedicated envelope type form, not generic-type syntax. It
projects the payload together with the immutable producer, disposition,
attempt, obligation, commit, and provenance evidence authorized for ordinary
inspection.

`recover T` closes the incoming obligation only by producing a valid `T`.
`transform E2` closes the incoming transition by producing a linked successor
`Failure<E2>` with a distinct obligation. Neither meaning is inferred from the
function name or body. A response function's own declared failures and faults
create separate explicit dispositions under the ordinary producer rules.

The first admitted response surface is `recover` and `transform`. Retry,
propagation, policy sinks, and general failure-producing response composition
retain their existing architectural meaning but require separately bounded
source and execution stages.

### Closed consumer declarations

A consumer declaration names exact ordinary response functions:

```lyraform
consumer read_failures {
    use_cache
    report_outage
}
```

Each member resolves to one semantic function identity. The accepted failure
set and possible outgoing dispositions are derived from those resolved
signatures. The declaration is closed: it performs no dynamic nearest-handler
lookup, textual-name dispatch, wildcard matching, implicit capture, ambient
global registration, or runtime discovery.

### Explicit graph association and typed rejoin

A consumer instance is an explicit disposition junction:

```lyraform
node reader  : fn read_sensor
node handled : consumer read_failures

wire reader.out     => handled.success
wire reader.failure => handled.failure
wire handled.out    => display.in
```

The `success` and `failure` inputs are mutually exclusive projections of the
same producer attempt. The success input passes the producer's `T` through
unchanged. A selected recovery may emit `T` on `handled.out` only when its
declared `recover T` identity equals the producer success type and the next
wire accepts that same type. This junction is the explicit graph rejoin; there
is no hidden jump to a continuation.

A selected transformation emits its declared successor failure on the
consumer's failure output. That output must itself have one accountable route:

```lyraform
wire handled.failure => outer_failures.failure
```

The compiler refuses missing producer disposition inputs, success/failure
inputs from different producer attempts, incompatible rejoin types, ambiguous
routes, unaccounted transformed failures, output co-emission, or cycles that
lack a separately admitted bound.

Faults never enter a consumer junction. They use the containment association
defined by ADR 0067.

## Policy selection

The source consumer declaration defines the authorized response set. A
versioned policy artifact may select one exact route for each admitted failure
type and consumer instance. The resolved artifact records at least:

```text
module and graph revision
consumer-instance identity
incoming failure type identity
selected response-function identity
response-transition identity
bounded parameters where separately authorized
policy profile, revision, authority, and provenance
```

A consumer with one compatible route has a deterministic fixed selection. The
compiler still emits a `fixed_single_route` selection record with source
provenance. A consumer with several compatible routes requires an explicit
policy selection. Missing, stale, ambiguous, incompatible, or unauthorized
policy is an admission refusal; there is no default route.

The same canonical policy artifact may be supplied as deployment
configuration or deliberately embedded into the executable. Embedding changes
storage and deployment, not semantic authority. A policy cannot add a
function, change a transition, widen a type set, erase an obligation, convert
failure to success, or route a fault into ordinary recovery.

## Consequences

- Source remains sufficient to inspect every legal failure route.
- Policy remains changeable without recompilation only within source-declared
  freedom.
- A compiled-in policy and an external policy obey one schema and validation
  law.
- Response functions remain ordinary named Lyraform functions with explicit
  envelope and transition contracts.
- Graph views can display normal flow, failure flow, the selected route, and
  the exact typed rejoin without reconstructing hidden control flow.
- Existing `TextOutcome` programs can migrate incrementally through their
  already-derived closed disposition facts.

## Explicit non-decisions

This ADR does not admit generic type syntax, open failure hierarchies, dynamic
handlers, implicit propagation, retry, cancellation, timeout, backpressure,
concurrent disposition composition, or new aggregate semantics. Those cannot
be inferred from this source model.
