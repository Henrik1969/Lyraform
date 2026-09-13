---
title: Flowcore pipeline matrix
status: active-development-gate
---

# Flowcore pipeline matrix

The matrix exercises the complete process boundary:

```text
FlowMini → Flowanalyst → Flowoptimize → Flowlower
```

It distinguishes a successful semantic path from a deliberate semantic block.
This prevents unfinished language areas from being mistaken for regressions.

The companion `tools/run-flowcore-pass-corpus.sh` breadth gate runs every
program in `Lyraform/.../examples/pass` through the same four boundaries. The
current corpus contains 95 programs, all of which pass semantic analysis,
optimization, and the lowering report boundary. This does not claim that
Flowlower emits target code for every program.

## Current cases

| Fixture | Purpose | Expected result |
|---|---|---|
| `call_expression_probe` | names, calls, arguments, return types | accepted through Flowlower boundary |
| `control_flow_unary_probe` | control flow and unary expressions | accepted through Flowlower boundary |
| `expression_pool_probe` | expression ownership and local resolution | accepted through Flowlower boundary |
| `refined_contract_probe` | refined bases and invariant bindings | accepted through Flowlower boundary |
| `target_projection_probe` | named targets and entrypoints | accepted through Flowlower boundary |
| `abi_contract_probe` | ABI type identities and declarations | accepted through Flowlower boundary |
| `index_field_probe` | list/array types, indexing, records, field paths | accepted through semantic boundary; lowering is not emitted |
| `statement_initializer_probe` | function calls, collection literals, intrinsic `stdin.text` capability | accepted through semantic boundary; lowering is not emitted |
| `literal_expression_probe` | typed literals and collection values | accepted through Flowlower boundary |
| `type_reference_probe` | generic, qualified, and shaped types | blocked by incomplete type-family resolution |

The `empty_program_main`, `abi_abs_main`, `abi_strlen_main`, and
`flowcat_argv_main` profiles currently emit LLVM IR and produce executables.
The other accepted matrix
cases prove semantic and boundary continuation; they do not claim that
general lowering is implemented.

The named-target probe proves that `cli` and `daemon` each have exactly one
semantic `main`. Target-aware selection and separate artifact emission are not
yet part of Flowlower.

## Exploratory AST-family scan

The broader v0.25 AST fixture family was also scanned without promoting every
result to a gate. The scan exposed these current gaps:

```text
generic/container/shape types:
  list<int>, array<int>, optional<...>, result<...> are covered in accepted
  probes; advanced qualified and symbolic extents remain unresolved:
  collection.list<int>, math.scalar, Rows

General ABI lowering remains future work even though the canonical ABI spellings now
resolve:
  c_int, c_long, c_ulong, c_size_t

qualified/domain types:
  Point is now structurally projected from standalone `record` declarations;
  vendor.Domain and other domain-qualified names remain unresolved

cross-unit and recursive call visibility:
  some imported or recursively described call names are not yet projected
  into the resolver's visible scope chain
```

These are semantic implementation gaps, not failures of the structural
FlowMini export. They are the next candidates for expansion of the accepted
matrix. The matrix should gain a case only when the corresponding semantic
contract is implemented and tested.

Run it with:

```sh
tools/run-flowcore-pipeline-matrix.sh
```
