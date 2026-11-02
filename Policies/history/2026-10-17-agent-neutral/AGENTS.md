# Lyraform repository guidance

These are durable repository rules. Task-specific scope, authority, state,
commit/push permission, and definition of done belong in an explicitly invoked
mission document; no historical task or checkpoint is ambient authority for a
future task.

Compiler and architecture maturation stages follow the standing
[`Stage Execution Protocol`](docs/development/stage-execution-protocol.md).
Every explicitly selected maturation mission follows the [Mission
Convergence Rule](docs/development/mission-convergence-rule.md): prioritize
vertical slices, check accepted law, identify missing canon, simplify and
reuse, and consolidate at each gate. A prerequisite stage must name its
executable dependency and state its narrower maturity claim.
An explicitly invoked stage mission supplies its bounded objective and may
override the protocol only where it says so directly.
After a gate, use
[`Autonomous Next-Stage Selection`](docs/development/autonomous-next-stage-selection.md)
to continue obvious engineering work or stop with a decision brief when the
next step would establish language, architecture, or policy meaning.

## Authority and placement

Use this direction chain, in order:

1. the applicable `AGENTS.md` files;
2. the standing development rules linked above;
3. one explicitly selected tracked mission directly under `docs/tasks/`;
4. only the ADRs, protocols, and evidence named by that mission;
5. Git-local run state and a new dated checkpoint as administrative state and
   result evidence, never as semantic authority.

Chat history, historical tasks and checkpoints, `.agents/`, `.codex/`, local
`output/`, and stale runner-state files do not select or authorize work.
Mission run state belongs under `.git/codex/runs/<mission-id>/`; ephemeral
transcripts belong under `/tmp/lyraform-codex-$UID/<mission-id>/`; persistent
local scratch may use ignored `.local/codex/<mission-id>/`. Curated artifacts
must be promoted deliberately to the tracked directory that owns their
meaning and provenance.

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
- Preserve the explicit-failure boundary: canonical semantics establish
  dispositions and legal routes, typed failure consumers expose statically
  resolved ordinary functions, policy selects only among those authorized
  routes, and runtimes execute without inventing meaning. Do not introduce
  exception-style unwinding, dynamic nearest-handler lookup, implicit
  propagation or termination, diagnostic-and-drop, policy-created success, or
  ordinary recovery of faults.
- Preserve disposition-evidence liveness: evidence remains attached while an
  obligation depends on it, may compact only at a proven closure boundary, and
  must never be silently truncated to satisfy a resource or policy limit.
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
