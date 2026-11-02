# Lyraform executor guidance

Follow [the local policy map](Policies/README.md) and the shared Development
policies. [EXECUTION-001](Policies/EXECUTION-001.md) defines this project’s
explicit executor-neutral adaptation. Policies govern these instructions;
actual platform/system instructions and tool permissions still constrain execution.

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
specialize the protocol only through a linked, explicit local policy decision. Belonging to a mission does not itself authorize an override.
After a gate, use
[`Autonomous Next-Stage Selection`](docs/development/autonomous-next-stage-selection.md)
to continue obvious engineering work or stop with a decision brief when the
next step would establish language, architecture, or policy meaning.

## Authority and placement

Within project governance, use this direction chain:

1. shared policies and applicable local policy records;
2. applicable AGENTS.md instructions and standing development procedures;
3. one explicitly selected mission directly under `docs/tasks/`, with measurable
   goal criteria, one or more subtasks and any explicit policy decisions linked;
4. accepted ADRs and evidence relevant to that mission;
5. exact-mission state and checkpoints as evidence, never semantic authority.

History, old task files, private agent caches and generated output do not select
or authorize a mission. New state lives under
`.mission-state/<mission-id>/<revision>/`, bound to the exact definition.
Essential state must remain exportable and understandable by any executor.
Legacy `.git/codex/runs/` records are not automatically migrated or adopted.
Follow EXECUTION-001 for explicit identity-checked handoff. Curated evidence
retains its existing owning documentation location. No state from another
mission may supply completion, blocked status or authorization.

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

## Resolved governance context

[Policies/context.json](Policies/context.json) resolves the local policy and
executor map. [Policies/AUDIT.md](Policies/AUDIT.md) records inspected sources,
validation surfaces and open decisions. These are navigation/evidence records,
not replacements for existing semantic law or an approval of open gaps.

The [legacy runner boundary](Policies/RUNNER-001.md) applies to
`tools/run-autonomous-codex`: its loop flags are not portable goal-confirmation
state. Use explicit evidence and the portable model before a completion claim
or executor handoff.

## FrankenCore and FrankenPOP identity

[IDENTITY-001 v1.1](../FrankenCore/Policies/IDENTITY-001.md) governs the terminology: FrankenCore defines
the architecture and policies; FrankenPOP is a possible future system based
on them. They are distinct. Existing prototype/component paths do not establish
a different authority or imply that the future system is implemented.

## Canonical placement

Follow [PLACEMENT-001](Policies/PLACEMENT-001.md): current name Lyraform,
canonical directory LyraForm. Use worktrees/ for linked development branches
and history/ for preserved external evidence. Earlier names and version labels
are provenance, not current project identity or authority. Compatibility
identifiers remain unchanged until a separate migration is authorized.
