# Native source-disposition callable bridge — 2026-11-04

## Result

The existing source-derived `TextOutcome` specimen now has a bounded generated
native bridge in the callable lowering unit. The bridge is emitted only after
the source topology, selected policy, host policy, callable identities,
carrier types, response body, and provenance have been checked by the existing
artifact validators and the emitter's shape gate.

The generated function is:

```text
flow.source.disposition()
```

It invokes the compiler-emitted producer, the selected `recover` response, the
typed `Text` rejoin, and the existing activation runtime. It does not use a
handwritten route table or select a policy at runtime.

## Evidence

Success path:

```text
TextOutcome success
  -> generated source-disposition bridge
  -> typed display rejoin
  -> provider-owned Text disposed once
```

Failure path:

```text
real provider exhaustion
  -> generated TextOutcome producer
  -> exact immutable failure envelope
  -> selected response
  -> exact response identity/provenance recovery
  -> typed display rejoin
  -> original obligation published and closed
```

The failure fixture forces the existing text provider's bounded exhaustion
rule with a 4096-byte operand plus one byte and observes `text unavailable` at
the generated native endpoint. The success fixture observes `Lyraform` at the
same endpoint.

The runtime bridge returns the rejoin's host status. The test wrapper treats
the normal non-negative `puts` status as success and reserves status 70 for a
runtime/ABI refusal.

## Verification

Clean external build tree:

```text
/tmp/lyraform-source-recovery-build
```

Focused command:

```text
ctest --test-dir /tmp/lyraform-source-recovery-build \
  -R 'flowlower_(source_callable_abi|dynamic_failure_routing)' \
  --output-on-failure
```

Result:

```text
2/2 passed
```

The existing activation runtime and callable ABI tests remain in the same
verification set. The implementation build emitted only the repository's
pre-existing `__int128` pedantic warnings.

## Honest boundary

The source disposition and lowering plan still report `execution: unsupported`;
the public source graph is not executable. TinyVM parity is not implemented.
Only the one-route TextOutcome shape with a statically borrowed response
fallback is admitted by this private native bridge. General response-result
ownership, fault execution, multiple-route selection, and graph-level source
execution remain open.
