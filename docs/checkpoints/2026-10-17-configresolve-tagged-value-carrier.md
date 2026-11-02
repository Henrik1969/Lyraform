# ConfigResolve tagged carrier checkpoint

The semantics decision for `lyraform.configresolve-tagged-value-boundary` is
accepted. Lyraform will use a closed `variant` declaration for the exclusive
ConfigResolve scalar family, with `Null`, `Bool`, `Int`, `Text`, and a
declared-but-refused `Double` case.

The compiler does not yet execute this construct. That is deliberate: the
existing ordinary-record lowering cannot prove exclusivity and must not be
reused as a disguised union. The next implementation task is to add the
source declaration and typed construction/projection path, then prove native
LLVM/TinyVM parity for the four admitted cases.

Evidence already establishes the need for this boundary:

- `../ConfigResolve/include/configresolve/value.hpp` uses an exclusive
  `std::variant` over the five alternatives.
- `docs/architecture/text-value-v0.1-proposal.md` establishes owned Text
  semantics and rejects pointer identity.
- `experiments/configresolve/contract-matrix.json` keeps the carrier boundary
  separate from loaders, stores, schemas, views, and diagnostics.
