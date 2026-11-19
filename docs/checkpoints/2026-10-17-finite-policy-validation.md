# Checkpoint: finite policy validation

The finite fact resolver now validates integer candidates against a source
`Policy` record before applying conflict resolution. Four parity cases agree in
native LLVM and TinyVM: valid selection `33`, invalid first candidate `4`,
invalid later candidate `4`, and missing optional input `1`.

This is a bounded integer range slice. Tagged values, string policies,
diagnostic records, loaders, stores, views, bindings, and the C API remain
outside the mission.

Evidence is retained in
`experiments/configresolve/evidence/finite-policy-validation/`.
