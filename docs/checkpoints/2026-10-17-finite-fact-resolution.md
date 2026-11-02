# Checkpoint: finite ConfigResolve fact resolution

The next bounded ConfigResolve slice is executable from Lyraform source. Three
`Fact` records carry `present`, `value`, `precedence`, and `order` through a
function boundary. The generic compiler admits typed scalar identifiers and
field projections inside record construction, and preserves source order so
fact definitions precede the resolving call and its result definition.

`resolve_highest` implements highest precedence with deterministic last-order
ties, reject conflict, first wins, last wins, and missing input. Six generated
source cases agree between a real Clang-linked LLVM binary and TinyVM: selected
values `33`, `11`, `11`, `33`, missing code `1`, and rejected tie code `3`.

Evidence: `experiments/configresolve/evidence/finite-fact-resolution/` and
`experiments/configresolve/run-finite-resolution-probe.py`.

Validation after the implementation: canonical suite `193/193`; the existing
ConfigResolve scalar regression remains `96` cases; policy audit reports
`41/41` contexts, zero errors, zero evidence drift, and zero open decisions;
`git diff --check` is clean.

This checkpoint does not claim the full ConfigResolve value, fact-set, policy,
diagnostic, loader, store, view, binding, or C API families. Record-valued
result construction across branching return paths remains a separate bounded
runtime decision.
