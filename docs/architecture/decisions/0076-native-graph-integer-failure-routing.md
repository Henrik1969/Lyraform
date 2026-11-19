# 0076 — Native graph integer failure routing uses the existing terminal graph route

**Status:** accepted bounded native-graph slice  
**Date:** 2026-11-04  
**Mission:** Lyraform numeric-semantics foundation

## Decision

The initial executable route for runtime-unknown integer `div` and `%` in a
validated native source graph is the existing terminal
`flowcore.graph_failure` channel. The graph lowering must guard signed integer
division and remainder before executing the LLVM operation and must report the
current operation identity, activation, and an operation-specific reason:

```text
div -> invalid_division
%   -> invalid_remainder
```

The route exits the current native graph invocation with status 70. It is a
terminal failure report, not a recovery route, a numeric failure type, or an
implicit source handler.

## Admission boundary

Only a receiver function in a ready `flowcore.source_graph` v2 plan may use
this runtime route for an otherwise runtime-unknown divisor or left operand.
The frontend and Flowcontracts still reject:

- statically known zero divisors;
- statically proven `INT_MIN / -1` and `INT_MIN % -1`;
- ordinary non-graph calls without an admitted route;
- any operation whose integer carrier is not supported by the native guard.

The graph route does not change the separate true-division (`/`) floating
contract. TinyVM integer execution and source-level recovery/disposition
execution remain outside this slice.

## Lowering law

For the admitted native carriers, LLVM lowering checks the divisor for zero
and checks signed minimum divided or remainderd by `-1` before `sdiv`/`srem`.
The checked branch calls the existing `flow_graph_fail` runtime entry point;
the valid branch executes the original signed operation. No unchecked LLVM
operation may be reached for a guarded invalid case.

The focused evidence is recorded in the
[native graph integer failure-routing checkpoint](../../checkpoints/2026-11-04-native-graph-integer-failure-routing-slice.md).

## Deliberate boundary

This decision does not define a language-level numeric failure value, recovery
or retry semantics, implicit handler selection, TinyVM graph parity, or a
general dynamic-failure policy for ordinary functions. Those require separate
contracts and must not be inferred from process termination or a backend trap.
