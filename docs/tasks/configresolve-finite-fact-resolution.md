# Finite ConfigResolve fact resolution

ID: `lyraform.configresolve-finite-fact-resolution`; revision 1. This is the
next bounded mission on `experiment/configresolve-lyraform`; preserve unrelated
working-tree changes and defer comparison or merge.

## Measurable goal

Lyraform source must carry three scalar configuration facts as ordinary records
through a function boundary and execute all four conflict policies: highest
precedence with deterministic last-order ties, reject conflict, first wins,
and last wins. Missing input is also explicit. Native LLVM and TinyVM must
agree on the result for positive and hostile cases.

The accepted slice returns a scalar result code/value. It does not claim the
full ConfigResolve `Value`, `FactSet`, `PolicySet`, diagnostics, loader, store,
view, binding, or C API families.

## Generic implementation boundary

Record construction may consume typed scalar identifiers and field projections.
The compiler validates the declared field type and keeps record layout private;
it does not recognize ConfigResolve names or policies. Source operation order
must preserve earlier fact definitions before a call and its result definition.

## Evidence

The source probe is `experiments/configresolve/finite-resolution.flow`; the
four-case parity runner is
`experiments/configresolve/run-finite-resolution-probe.py`. The checkpoint and
retained logs record the exact cases and limitations.
