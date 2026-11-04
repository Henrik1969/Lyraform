# Dynamic failure routing — native callable-ABI slice — 2026-11-04

## Scope

This checkpoint proves the first executable runtime-routing slice for an
already accepted source-declared failure carrier. It uses the existing
`TextOutcome` producer, `failure TextFailure` response contract, source-derived
consumer route, and activation frame. It does not invent a numeric failure
type, implicit handler lookup, or new public syntax.

The tested path is:

```text
real provider failure
    -> generated producer callable
    -> source-derived immutable failure envelope
    -> exact selected recovery callable
    -> published success
    -> original obligation closed once
```

The source graph remains intentionally blocked, and TinyVM disposition
execution is not claimed. Those are the next integration boundaries.

## Implemented evidence

`Flowlower/tests/run-dynamic-failure-routing.py` builds a source specimen from
the canonical `text_outcome_consumer` declaration and makes the provider fail
through its real bounded input rule: a 4096-byte left operand plus one byte.
The test then:

- obtains the source-derived producer, consumer, route, response, policy,
  envelope, containment, and provenance identities;
- runs the normal frontend, semantic, binding, parallel, optimization, and
  preparation stages;
- emits the generated producer and recovery callable through the existing
  callable-ABI unit boundary;
- executes the generated producer and observes `FLOW_TEXT_EXHAUSTED`;
- enters that failure into `FlowActivationFrame` with `no_commit` evidence;
- invokes the generated response function using the immutable envelope;
- recovers the declared `Text` value and verifies one published result with
  the original envelope and obligation closure.

The semantic source graph still reports `execution: unsupported`; the unit
boundary is explicitly a bounded backend/runtime probe, not whole-graph
execution authority.

## Verification

Clean build tree:

```text
/tmp/lyraform-refinement-clean
```

Focused command:

```text
ctest --test-dir /tmp/lyraform-refinement-clean --output-on-failure -R '^(flowlower_dynamic_failure_routing|flowlower_source_callable_abi|flowactivation_source_runtime)$'
```

Result:

```text
3/3 passed
```

The new test's direct result is:

```text
dynamic failure routing: PASS (real provider failure, exact envelope, resolved recovery, closed obligation)
```

## Remaining boundary

This slice does not make the public source graph executable. The next work is
to carry the same source-derived route through the executable graph projection
and then provide equivalent TinyVM behavior. Dynamic integer division and
remainder remain refused because no canonical numeric failure type and source
route have been selected. A backend trap or process exit remains outside the
language contract.
