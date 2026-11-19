# Checkpoint: ConfigResolve text value record

The generic scalar model now recognizes `Text` as an admitted immutable scalar
for finite ordinary-record construction. Record catalogs, operand validation,
LLVM lowering, and TinyVM lowering preserve the field as an opaque text handle;
record copies remain independent.

`experiments/configresolve/text-record.flow` returns 7 from native LLVM and
TinyVM returns 7 with `trap:0`. Existing record catalog, shape, finiteness,
policy, resolution, branch, and canonical-suite checks remain the required
regression evidence.

This does not decide mutable Text ownership, dynamic concatenation, tagged
`Value` unions, loaders, or diagnostics.
