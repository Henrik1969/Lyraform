# Lyraform mission: Source disposition policy selection — Gate 3

Date: 2026-10-13

## Mission type

Short-loop compiler convergence / versioned disposition policy selection.

## Selected branch and baseline

```text
branch:       codex/disposition-policy-selection-gate3
origin/main:  b50534c380879a5a70a77ed5c1dd442bfc4142bd
local start:  b50534c380879a5a70a77ed5c1dd442bfc4142bd
required stop: Gate 3
```

The branch begins after the Gate 2 and hygiene convergence loop was merged.
Re-fetch and verify the exact baseline before implementation. Stop if `main`
changes in a way that alters this mission's semantic or operational basis.

## Objective and exact endpoint

Implement only Mission 07 Stage 3 for the bounded source-derived disposition
topology:

```text
source establishes every legal response route
    -> one versioned policy artifact selects exactly one existing route
    -> independent contract validation binds the selection to the exact
       module revision, consumer instance, failure type, response function,
       response transition, policy authority, and provenance
    -> the same canonical artifact is accepted as external input and retained
       byte-for-meaning as embedded build input
    -> execution remains unsupported
```

Support exactly:

1. deterministic `fixed_single_route` selection for the existing one-route
   `TextOutcome` specimen;
2. one explicitly supplied selection for a bounded source specimen with two
   legal response functions for `TextFailure`; and
3. one versioned host-policy selection admitting only ADR 0067's
   `report_and_fail` command-host action.

Stop after Gate 3 evidence and documentation. Do not begin executable Graph IR
or any later Mission 07 stage.

## Authority and accepted law

Read and follow:

1. the applicable `AGENTS.md` chain;
2. `docs/development/stage-execution-protocol.md`;
3. `docs/development/mission-convergence-rule.md`;
4. ADR 0066 and ADR 0067;
5. this mission; and
6. the Gate 2 mission, authority map, and checkpoint as factual baseline
   evidence only.

The canonical semantic analyzer remains the sole owner of legal source route
topology. Policy owns only the selection of one route from that closed set.
Flowcontracts independently validates the selection/topology agreement. No
runtime or backend may create or reinterpret the selection.

## Convergence contract

1. **Usable endpoint:** one deterministic versioned source-disposition policy
   artifact supports fixed and explicit selection, one separate versioned host
   selection admits only `report_and_fail`, both are independently validated,
   and the semantic report remains blocked for execution.
2. **Accepted law and fact owners:** source semantics owns legal functions,
   routes, transitions, types, junctions, and containment; policy owns only one
   exact choice and its profile/revision/authority/provenance; the host-policy
   artifact owns only the command-host action already admitted by ADR 0067.
3. **Unresolved decisions and stops:** stop on any need for new syntax,
   executable Graph IR, a default multi-route choice, policy-created success,
   widened failure or fault sets, host actions beyond `report_and_fail`, or a
   changed `main` that invalidates the baseline. Artifact field organization is
   routine internal engineering when it preserves all required identities.
4. **Reuse and retirement:** reuse the Gate 2
   `lyraform.source_disposition_topology/v1`, its source identities, the JSON
   contract machinery, and the historical declarative policy tests as
   evidence. Do not treat the older carrier-independent
   `lyraform.failure_policy_selection/v1` as source authority; retain it as a
   compatibility/declarative contract and add no competing semantic topology.
5. **Positive, negative, hostile evidence:** positive—fixed one-route and
   supplied two-route selection; negative—multi-route source without supplied
   policy; hostile—missing, stale, foreign-module, wrong-revision,
   incompatible, unauthorized, ambiguous, extra, forged-function,
   forged-transition, and unsupported-host-action selections.
6. **Reproducibility and maturity:** configure/build/test in a fresh external
   build directory and run a focused Gate 3 test from source through policy
   embedding and independent validation. Claim declarative policy-selected
   maturity only; execution remains unsupported.
7. **Falsifying evidence:** PASS is disproved if policy changes a semantic
   fact, a multi-route graph receives a default, external and embedded
   selections disagree, a hostile selection validates, any execution claim
   beyond policy selection becomes true, or canonical tests regress.
8. **Consolidation before expansion:** update current status, the disposition
   authority map, task lifecycle index, and a dated Gate 3 checkpoint; leave
   one small review branch ready for human review before Gate 4 is selected.

## Artifact requirements

The source-disposition selection must carry and validate at least:

```text
format and version
status and execution boundary
module identity and revision
consumer declaration and instance identities
incoming failure type identity
selected route identity
selected response-function identity
response class, outgoing type, and obligation transition
selection kind: fixed_single_route or supplied
policy profile, revision, authority, and provenance
```

The host-policy selection must carry and validate at least:

```text
format and version
module identity and revision
containment node and activation scope
action: report_and_fail
policy profile, revision, authority, and provenance
execution: unsupported
```

The artifact schema may use nested records when this improves identity
clarity. It must not rely on names, graph order, numeric coincidence, or
policy prose to reconstruct semantic meaning.

## Implementation stages

### Stage A — baseline and seam proof

Record branch/worktree state, exact SHAs, protected untracked paths, Gate 2
artifact ownership, historical policy-contract boundaries, and current test
count. Run a fresh canonical baseline before semantic edits.

### Stage B — shared contract

Implement deterministic serialization/read/validation for the two Gate 3
artifacts. Validation consumes the complete Gate 2 topology and checks every
selected identity against it. The fixed selector may derive its one choice
only when exactly one compatible route exists. The supplied selector must be
present when more than one compatible route exists.

### Stage C — compiler integration

Add one explicit Flowanalyst input for supplied source-disposition policy and
one for host policy. Emit a fixed selection when the topology has exactly one
route. Embed the canonical validated artifacts in the blocked semantic report.
Do not alter source legality or make the lowering plan executable.

### Stage D — evidence and consolidation

Add focused source-through-policy tests, independent Flowvalidate coverage,
hostile mutations, deterministic repeatability, external/embedded equality,
and proof that execution remains unsupported. Update current-truth documents
and add a dated Gate 3 checkpoint.

## Protected boundaries

Do not:

- add or change Lyraform syntax;
- begin Gate 4 Graph IR work or backend/runtime routing;
- introduce exceptions, hidden handlers, implicit propagation, defaults for
  ambiguous choice, diagnostic-and-drop, or policy-created success;
- add retry, timeout, cancellation, backpressure, parallel disposition
  composition, restart, or fault recovery;
- alter the meaning of `TextOutcome` or existing response transitions;
- modify `master`, FlowLFS, Flowselection, `meta-discusions.md`, or `output/`;
- rewrite history, force-push, merge to `main`, create/merge a pull request, or
  delete branches without separate explicit authority.

## Gate 3

PASS only when:

- source still establishes every legal choice and no policy field can add or
  change one;
- fixed one-route selection is deterministic and source-bound;
- two-route source is refused without an explicit policy and accepts exactly
  one authorized supplied selection;
- external and embedded canonical artifacts are equivalent;
- missing, stale, foreign, incompatible, unauthorized, ambiguous, extra, and
  forged selections are refused before any executable plan exists;
- only `report_and_fail` is admitted as the bounded host action;
- topology and report validation reject hostile mutations;
- all execution claims except policy selection remain false, with the lowering
  plan blocked and execution explicitly unsupported;
- focused and canonical clean verification pass; and
- the branch stops ready for human review without beginning Gate 4.

## Publication authority

Local implementation and verification are authorized by the selected work.
Do not commit, push, merge, or create a pull request until Henrik explicitly
requests that publication step.

## Gate result

**PASS — WAITING FOR HUMAN REVIEW (2026-10-13).**

The bounded implementation, hostile evidence, fresh clean build, complete
canonical suite, and sanitizer result are recorded in
[`2026-10-13-source-disposition-policy-gate-3.md`](../checkpoints/2026-10-13-source-disposition-policy-gate-3.md).
Gate 4 has not started. No commit or push has been performed.
