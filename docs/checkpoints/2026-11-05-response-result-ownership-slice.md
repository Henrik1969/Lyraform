# Response-result ownership slice — 2026-11-05

## Result

Source-disposition response functions now publish an explicit
`result_resource` contract. The compiler proves the current fallback fixture's
static `Text` literal as:

```text
ownership: borrowed
lifetime: call
cleanup_capability: none
origin: static_literal
proof: literal_has_no_escape
```

The default for a response without that proof is explicit `owned` with
`lifetime: owned`, `origin: disposition_boundary_default`, and an unresolved
cleanup capability. The semantic artifact remains inspectable, but the native
bridge refuses it until the cleanup and response body are both admitted.

The selected policy carries the same resource contract, so policy selection
cannot change the response's ownership or cleanup law. The bridge forwards
producer and response cleanup independently; the borrowed fixture passes a
null response disposer. The runtime test also proves an owned response is
disposed once after rejoin and that a borrowed response is not disposed.

## Verification

Build directory:

```text
.tmp/build
```

Focused command:

```text
ctest --test-dir .tmp/build \
  -R 'lyraform_source_disposition_gate2|lyraform_source_disposition_policy_gate3|lyraform_disposition_graph_preservation|flowactivation_source_runtime|flowlower_source_callable_abi|flowlower_dynamic_failure_routing' \
  --output-on-failure
```

Result:

```text
6/6 passed
```

Full clean verification:

```text
cmake --build .tmp/build -j2
ctest --test-dir .tmp/build --output-on-failure
```

Result:

```text
207/207 passed
Total Test time = 124.89 sec
```

The build emitted only the repository's existing `__int128` pedantic
warnings.

## Honest boundary

This is a lifecycle contract and native runtime slice, not general source
recovery execution. Source graph execution, TinyVM parity, arbitrary response
bodies, multiple-route execution, and a complete language-wide owned-resource
cleanup catalog remain open.
