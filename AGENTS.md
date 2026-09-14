# Lyraform repository guidance

These are durable repository rules. Task-specific scope, authority, state,
commit/push permission, and definition of done belong in an explicitly invoked
mission document; no historical task or checkpoint is ambient authority for a
future task.

## Working safely

- Inspect the branch, worktree, relevant history, and build graph before
  editing.
- Preserve unrelated user changes exactly. Stage only paths owned by the
  current task.
- Prefer small, reversible changes and record assumptions when semantics are
  not yet confirmed.
- Keep historical records historically accurate. Distinguish confirmed
  behavior, compatibility behavior, experiments, and proposals.
- Do not perform blind global renames. Treat serialized identifiers, schemas,
  public APIs, source extensions, and executable names as separate compatibility
  decisions.

## Architecture and tests

- Keep application policy, source-unit names, and fixture names out of compiler
  dispatch; use generic language and contract machinery.
- Keep formal architecture terminology precise: contract, provider, adapter,
  graph, port, wire, artifact, lowering plan, projection, effect, policy, and
  provenance.
- Run focused tests while iterating and the canonical configure/build/test
  gates at meaningful boundaries. Record exact results rather than inferred
  counts.
- Keep generated output in build or temporary directories unless it is a
  deliberately curated artifact with documented provenance.

## Git and project boundaries

- Never rewrite published history or force-push.
- Do not delete branches, tags, releases, issues, or repository settings unless
  an explicitly invoked task authorizes that exact operation.
- Preserve `master` as historical seed and `flowlfs-v0.1-alive` as an
  independent experiment. Do not merge or copy FlowLFS implementation into
  Lyraform.
- The current project identity is Lyraform and the human-facing toolchain is
  Igor. Historical Flowcore/Flowmini names may remain where they describe
  lineage or compatibility contracts.
