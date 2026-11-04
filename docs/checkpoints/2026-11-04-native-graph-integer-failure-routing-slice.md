# Native graph integer failure-routing vertical slice — 2026-11-04

## Scope

This slice carries runtime-unknown integer `div` and `%` through the existing
native source-graph path. It does not add source syntax, a numeric failure
type, implicit handler semantics, or a recovery disposition. The only admitted
runtime route is the existing terminal `flowcore.graph_failure` report.

The tested path is:

```text
ready source graph v2
    -> runtime-unknown integer divisor/left operand admitted
    -> guarded LLVM sdiv/srem
    -> flow_graph_fail(operation, reason)
    -> structured graph_failure with activation provenance
```

## Implemented evidence

The native source-graph provider fixture now supplies zero and signed-minimum
values. Its end-to-end pipeline exercises:

- dynamic `div` with a provider-produced zero divisor;
- dynamic `%` with a provider-produced zero divisor;
- `INT_MIN / -1` with an unknown graph input;
- `INT_MIN % -1` with an unknown graph input;
- static `div 0` refusal inside the graph boundary;
- operation-specific reasons `invalid_division` and `invalid_remainder`;
- exit status 70, current receiver activation, wire provenance, and no normal
  receiver output after failure.

The semantic validator authorizes the exception only for function symbols
listed as receivers in a ready source-graph v2 plan. LLVM lowering emits the
zero and signed-minimum guards for both `sdiv` and `srem` and routes invalid
cases through the existing `flow_graph_fail` ABI.

## Verification

Focused command:

```text
ctest --test-dir /tmp/lyraform-refinement-clean --output-on-failure -R '^native_source_graph$'
```

Result:

```text
1/1 passed
```

The focused regression covers successful graph execution, dynamic division and
remainder failure, signed-minimum overflow failure, static-zero refusal, and
the pre-existing source-failure path.

The complete current CTest result after the implementation, test-harness, and
documentation changes is:

```text
207/207 passed
100% tests passed, 0 tests failed
```

`git diff --check` passed. The clean build used for this verification was
`/tmp/lyraform-refinement-clean`; its configure and build were green before
the focused and full CTest runs.

## Boundary

This is a terminal backend/runtime route, not a language-level recovery route.
Ordinary functions remain subject to the existing refusal rules when integer
safety is unproved. TinyVM has not been extended with this graph activation
route, and source-derived recovery/disposition execution remains a separate
future slice.
