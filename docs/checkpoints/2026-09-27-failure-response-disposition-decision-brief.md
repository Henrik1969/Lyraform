# Decision brief — failure-response function disposition

Date: 2026-09-27.

Status: **DECIDED — Candidate A accepted and refined on 2026-09-27.**

Durable authority:
[ADR 0062](../architecture/decisions/0062-typed-failure-consumers-and-policy-routing.md)
and [ADR 0063](../architecture/decisions/0063-bounded-disposition-evidence-epochs.md).

## Problem

The declarative failure-consumer contract can now prove:

```text
consumer C accepts Failure<E>
route R names ordinary function F(FailureEnvelope<E>) -> T
policy P selected R
```

It cannot yet prove what `T` means for the originating failure obligation.
An ordinary return value must not silently mean that the function recovered,
transformed, propagated, terminated, or requested a retry.

The bounded question is:

> How does one ordinary response function declare the exact canonical
> disposition transition produced by its successful completion?

## Why the decision is required

Without this authority, the compiler cannot safely:

- determine whether the original `Failure<E>` wire was discharged or
  transferred;
- type-check the value or failure produced by recovery or transformation;
- distinguish a retry request from retry execution;
- preserve commit/no-commit and provenance laws;
- account for failures or faults produced by the response function itself;
- admit producer-to-consumer wiring or runtime guard failure;
- give all backends the same already-decided transition.

The existing `FailureConsumerFunction.result_type` is intentionally
insufficient. Treating an arbitrary non-void result as recovery would recreate
implicit error handling under a different name.

## Current evidence

### Canonical facts

- Every admitted attempt completes with exactly one of `Success<T>`,
  `Failure<E>`, or `Fault<F>`.
- Every failure/fault wire needs one statically inspectable route.
- One canonical disposition fact has contextual value, function, graph,
  provider, and execution projections.
- A tagged outcome transfers a must-account obligation; storage is not
  discharge.
- A typed failure consumer exposes exact ordinary function identities.
- Policy selects only among semantically authorized routes.
- Retry is a new attempt and requires separate retry-safety and bounded-policy
  authority.

### Current implementation

- `lyraform.failure_consumer/v1` proves a closed accepted failure set and exact
  compatible ordinary response functions.
- `lyraform.failure_policy_selection/v1` proves a policy selected one of those
  routes.
- Both are declarative and cannot authorize execution.
- Existing bounded `Outcome<Text,TextFailure>` facts already prove explicit
  success/failure accounting, but are not a general response-function model.

### Undefined

- the response function's outgoing disposition kind and payload identity;
- whether that meaning resides in a separate semantic contract or a source
  carrier type;
- exact recovery-result, transformation, propagation, and retry-request
  spelling;
- general failure/fault sets for the response function's own attempt;
- policy-sink and top-level termination representation.

## Candidate A — explicit response-transition contract on an ordinary function

Keep the developer function ordinary. It receives the typed read-only
`FailureEnvelope<E>` projection carrying the producer's disposition,
obligation, attempt, commit, and provenance evidence. Bind the function's
semantic identity to a separate canonical response-transition contract. For
the bounded first form, the contract declares exactly one successful response
class:

```text
recover      Failure<E> -> Success<T>
transform    Failure<E> -> Failure<E2>
propagate    Failure<E> -> Failure<E or declared supertype>
retry_request Failure<E> -> RetryRequest<R>
```

The contract records the incoming obligation, route and function identities,
response class, exact outgoing payload type, commit law, and provenance law.
The ordinary result type must exactly match the payload/carrier required by the
declared response class. A response function's own `Failure<E2>` or `Fault<F>`
possibilities remain new explicit obligations; they are not confused with its
intended successful response.

`retry_request` only transfers authority to a later bounded policy/execution
stage. It does not make retry executable and is refused unless canonical retry
safety exists.

**Pros**

- reuses ordinary functions and existing canonical disposition facts;
- keeps semantic meaning independent of naming and concrete carrier layout;
- gives policy a closed set of typed transitions rather than arbitrary code;
- permits value, graph, provider, and backend projections without making one
  syntax form authoritative;
- lets recovery, transformation, propagation, and retry mature independently.

**Cons**

- introduces one additional semantic contract beside the function signature;
- source declarations will eventually need a concise way to express or derive
  that contract;
- requires exact compatibility checks between function result and transition.

**Migration impact**

The existing failure-consumer v1 contract can remain unchanged. A new
declarative response-transition fact can refine each route before any route is
marked executable.

## Candidate B — dedicated result carrier types are the sole authority

Require response functions to return distinguished types such as
`Recovered<T>`, `Transformed<E2>`, `Propagated<E>`, or `RetryRequest<R>`.
The return type itself establishes the route meaning.

**Pros**

- disposition intent is visible in an ordinary signature;
- straightforward local type checking;
- fewer separate metadata objects.

**Cons**

- couples canonical semantics to one value projection;
- risks a family of special-purpose carrier types and source verbosity;
- graph/provider projections would still need equivalence contracts;
- a type name alone does not prove commit law, provenance transfer, retry
  safety, or complete accounting.

**Migration impact**

Requires new canonical types and likely new generic/type-system surface before
the current consumer contract can become executable.

## Candidate C — infer the response from function body behavior

Analyze the function body and infer recovery, transformation, propagation, or
retry request from its operations and return paths.

**Pros**

- minimal declaration burden;
- may produce useful verification evidence later.

**Cons**

- makes meaning dependent on whole-body inference;
- small implementation edits could silently change the public disposition
  contract;
- separate compilation, imported declarations, policy inspection, and stable
  artifacts become difficult;
- inference still needs a canonical contract as its output.

**Migration impact**

Requires substantially more control-flow, effect, and commit analysis than the
current bounded language supports.

## Candidate D — dedicated response keywords or handler syntax

Introduce special declarations or statements for recovery, transformation,
propagation, and retry.

**Pros**

- highly visible source intent;
- direct syntax-to-contract mapping.

**Cons**

- expands the language surface before the underlying contract is mature;
- duplicates behavior already expressible by ordinary functions;
- encourages keyword growth and risks creating a second function system.

**Migration impact**

Requires parser, AST, source audit, diagnostics, and language-reference changes
before semantic convergence can continue.

## Decision

**Candidate A — an explicit response-transition contract attached to an
ordinary function identity — is accepted.**

It best preserves the established division of responsibility:

```text
ordinary function    implements developer behavior
semantic contract    states the exact legal disposition transition
policy               selects among authorized transitions
runtime              executes the selected transition
```

It is also the least verbose semantically complete core. Later source syntax
may derive or abbreviate the contract, but cannot replace its inspectable
authority. The first bounded implementation should support only `recover` and
`transform` if those can be proved using existing disposition facts;
`propagate`, `retry_request`, policy sinks, and runtime execution should remain
refused until their own prerequisites close.

The accepted refinement is that a response function receives the canonical
failure envelope rather than the naked payload. Forwarding transfers the
existing obligation, transformation creates a linked successor, recovery
closes it through a declared transition, and retry creates a separately linked
attempt only after retry authority exists. Evidence lifetime and policy limits
follow ADR 0063.

## Historical choices presented

```text
A. Explicit response-transition contract on an ordinary function
B. Dedicated result carrier types are the sole authority
C. Infer disposition meaning from function bodies
D. Introduce dedicated response keywords/handler syntax
E. Defer and define another prerequisite first
```

No source integration or executable failure routing is implied by accepting
the design. The response-transition contract and its bounded implementation
remain required before execution.
