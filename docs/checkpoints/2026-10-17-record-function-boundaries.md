# Checkpoint: finite record function boundaries

The finite scalar and nested ordinary-record bound now crosses pure source
function boundaries. Callable metadata carries the record identity for return
values and parameters. The native LLVM and TinyVM backends copy record values
at parameter and return boundaries, keep layout private, and preserve
independent values. Nested calls are ordered so a call such as
`read(make())` materializes the inner record before the enclosing call.

The public admission test exercises `make(): Pair`, `read(Pair): int`, and
`read(make())`; both backends produce exit/result `5`. Existing hostile contract
mutations remain rejected. This checkpoint does not admit member writes,
resource-bearing records, recursive records, or effectful/unsupported callable
forms.

Evidence is retained in
`experiments/configresolve/evidence/record-function-boundaries/`.

Validation: canonical suite `193/193`; scalar ConfigResolve probe `96` cases
with renamed-source and hostile-oracle checks; policy audit `41/41` contexts,
zero errors, zero evidence drift, zero open decisions; `git diff --check` clean.
