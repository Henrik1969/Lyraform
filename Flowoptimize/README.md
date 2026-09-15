# Flowoptimize

Flowoptimize is the sibling boundary after Flowparallel:

```text
Lyraform source -> Flowmini-compatible frontend -> Flowanalyst -> Flowparallel -> Flowoptimize
```

It currently establishes the optimization input/output boundary. It accepts an
accepted `flowparallel.execution_plan` v1 and emits an inspectable
`flowoptimize.optimization_report` v1. Its first transform is the safe
`coo_deduplicate` pass over the derived Boolean matrix projection. It removes
duplicate `(row,column)` entries, records before/after counts, and carries the
Boolean-idempotence proof. With no duplicates it records `not-needed`; the
canonical graph is never rewritten.

When the input contains Flowanalyst's canonical `region_dependency` view,
Flowoptimize exposes it as a derived `graph_to_matrix` projection. The graph
remains authoritative; the Boolean COO matrix is an acceleration view.
`provider_policy` records CPU and CUDA as candidates, but selection is deferred
to runtime capability discovery, measured cost, and the provider contract. CUDA
is never assumed to exist, and CPU remains the fallback.

Flowoptimize can consume an explicit verified
`flowparallel.graph_provider_decision` artifact with
`--provider-decision FILE`. It records that decision in the optimization report
without turning it into a source-level transform. Invalid or unverified
decisions block the stage. This keeps provider selection inspectable and lets
later optimizer passes use the decision as policy input while the canonical
graph remains unchanged.

The current consumption is intentionally identity-preserving: the report marks
`state.canonical_graph` as `unchanged`, `state.transformation` as `identity`,
and `state.decision_effect` as `advisory_policy_only`. A later optimizer may
emit a real transform only after its own legality, provenance, and differential
gates are defined.

It also preserves the source path from the semantic report as inspectable
provenance.

With `--diagnostics json`, invalid or unprocessable upstream artifacts are
reported as `FLOWOPTIMIZE_INPUT_INVALID` at the `input` stage on stderr, with
empty stdout and `disposition: no_artifact`. Contract failures and allocation
exhaustion retain their distinct machine-readable categories.

Try the complete pipeline:

```sh
Lyraform frontend ... | Flowanalyst/build/flowanalyst | Flowoptimize/build/flowoptimize
```

The future optimizer will consume the accepted semantic bundle, preserve
provenance, and emit a distinct versioned transformed state rather than
overwriting its input.
