# Flowanalyst

Flowanalyst is the semantic-analysis sibling of the Lyraform compiler. It
consumes the historical Flowmini-compatible frontend artifact.

```text
Lyraform source -> Flowmini-compatible frontend -> flowmini.frontend_bundle -> Flowanalyst
```

It consumes the versioned Flowmini frontend bundle. It does not reparse source,
link Flowmini internals, or rewrite the structural AST/SymbolTable projection.
Its output is a separate, versioned semantic report containing diagnostics and
facts established by analysis.

Every semantic report also carries `source.path`, copied from the frontend
bundle. This downstream provenance field is preserved by Flowoptimize and
Flowlower, so consumers can identify the source artifact without reopening
Flowmini internals.

## Try it

From the Lyraform repository:

```sh
./Lyraform/compiler/cmake-build-debug/flowmini \
  --dump-frontend-bundle \
  Lyraform/compiler/examples/ast/target_projection_probe.flow \
  | Flowanalyst/build/flowanalyst
```

The command also accepts a bundle file path instead of stdin:

```sh
Flowanalyst/build/flowanalyst bundle.json
```

Flowanalyst currently reports frontend diagnostics, duplicate declarations,
declared type resolution, identifier and call resolution, call arity, refined
invariants, record fields, external ABI requirements, and named-target
entrypoint shape. It emits analysis regions and a Boolean dependency matrix.
More checks will be added as explicit semantic contracts, without moving
semantic meaning backward into Flowmini.

The bounded `int`/`Bool` declaration and identifier-placement path now uses
shared canonical scalar rules. Incompatible flows are refused before a ready
lowering plan, and covered operations carry versioned type/identity/provenance
facts through the stage artifacts. See
[`canonical scalar semantics`](../docs/architecture/canonical-scalar-semantics-v1.md)
for the exact scope, compatibility boundary and Gate 2 status.

It also emits `effect_facts`. The first proven effect is `pure` for function
bodies consisting only of return expressions over literals, parameters, and
pure unary/binary operators. Calls, mutation, control-state constructs,
external effects, and unsupported forms remain `unknown`.

The independent consumer boundary is specified in
[`docs/flowanalyst/v0.1-consumer-contract.md`](../docs/flowanalyst/v0.1-consumer-contract.md).

With `--diagnostics json`, an invalid or unprocessable frontend bundle emits
`FLOWANALYST_INPUT_INVALID` at the `analysis` stage on stderr, keeps stdout
empty, and marks the attempt `no_artifact`. Allocation exhaustion and unknown
non-standard failures retain their separate machine-readable dispositions.
It defines version negotiation, provenance navigation, diagnostic identity,
partial-result handling, and exit semantics for IDEs, debuggers, AI tools, and
other consumers.
