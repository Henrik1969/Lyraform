# Canonical failure response transition — Gate 2

Date: 2026-10-07.

## Baseline and protected scope

This stage began on synchronized `main` at
`16a1cfff6e560ed343286af0f9f604edbb3d02e5`, equal to `origin/main`. The only
pre-existing worktree path was the unrelated untracked `meta-discusions.md`;
it was preserved untouched. `master`, `flowlfs-v0.1-alive`, source syntax,
Flowanalyst, runtime behavior, and backends were not modified.

## Bounded question and selected authority

The stage asked how successful completion of an already-authorized ordinary
failure-response function acquires one exact canonical meaning without making
the route executable. Candidate A from the accepted response-disposition
decision remains authoritative: meaning belongs to a separate shared semantic
contract, not inference from a function body, return type alone, naming, or a
new keyword family.

`flowcontracts/failure_response_transition.hpp` now defines:

```text
lyraform.failure_response_transition/v1
    exact transition, consumer, route, and function identities
    exact incoming failure type and failure-envelope projection
    recover   -> Success<T>  + close original obligation
    transform -> Failure<E2> + linked successor obligation
    exact function-result/outgoing-payload type agreement
    immutable origin commit evidence
    response-to-origin provenance link
    separate obligations for response-attempt failure or fault
    declarative status only
```

A validated transition set covers every authorized route exactly once. It
cannot add a route, change its failure/function identity, erase an obligation,
reinterpret the response result, or claim executable status.

## Evidence

Positive evidence proves bounded recovery and transformation, complete route
coverage, deterministic serialization, and exact round-trip reconstruction.
Negative and hostile evidence refuses empty/incomplete sets; invalid,
duplicate, missing, and contradictory identities; wrong input projections and
types; incompatible results; unsupported response classes; inverted
dispositions and obligation actions; changed commit/provenance/failure laws;
declaration-only and void functions; executable status; missing required
fields; and future versions.

The canonical-language drift guard now pins the new format, both admitted
response classes, origin-commit preservation, and separate response-attempt
obligations.

## Verification

- Focused GCC consumer plus response-transition tests: **2/2 PASS**.
- Focused Clang 18 ASan/UBSan response-transition test: **1/1 PASS**.
- Focused Valgrind response-transition probe: **0 errors**, **0 bytes in use at
  exit**, 1,335 allocations and 1,335 frees.
- Canonical-language authority freeze: **PASS**.
- Canonical current-documentation drift guard: **PASS**.
- `./igor doctor`: **PASS**.
- `./igor build`: **PASS**, including the new response-transition target.
- `./igor test`: **175/175 PASS**, 58.75 seconds.

## Deliberate boundary

This contract does not establish source spelling, a source function-carrier
projection, producer-to-consumer association, graph ports or wires, runtime
dispatch, propagation, retry, sinks, guard-failure execution, evidence storage,
or backend support. No executable plan consumes the new fact.

The next activity is a semantic-to-syntax-to-graph behavior map. It must show
how existing function and graph structures can project this authority without
making syntax or graph layout a second semantic owner.
That review artifact now exists as
[`failure-flow-semantic-syntax-graph-map.tex`](../architecture/failure-flow-semantic-syntax-graph-map.tex);
its syntax and executable graph examples are explicitly provisional.

## Gate 2

**PASS.** The bounded shared semantic authority is complete and remains
declarative. The required next work is the separately reviewed
semantic-to-syntax-to-graph behavior map.
