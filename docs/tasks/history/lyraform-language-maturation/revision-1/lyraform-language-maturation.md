# Lyraform language maturation mission

ID: `lyraform.language-maturation`; revision: `1`; owner: Henrik Sørensen.
Prepared: 2026-10-17. Status: planned, execution not selected.
Canonical definition: this Markdown file. The companion JSON records its SHA-256
and measurable criteria; any divergence must be reconciled before execution.

## Intent, goal and authorization

Intent: mature Lyraform into the contracted, graph-visible language already
intended, so real software can be developed in it.

Goal: establish an owner-ratified language profile and deliver a reproducible
practical application using that profile through the canonical compiler, with
explicit failure handling, honest support boundaries and measured backend
agreement. Profile completion and the application milestone must be measured
separately; neither alone warrants a claim of the full intended language.

Henrik authorized examination and mission preparation. This document does not
start implementation, inherit Mission 07 publication permissions, authorize
commits/pushes/merges, or select historical missions. Execution needs explicit
selection. Semantic decisions still require the relevant accepted ADR or local
policy decision. No policy override is made.

Read [assessment](../development/language-maturation-assessment.md),
[project instructions](../../AGENTS.md), [local policies](../../Policies/README.md),
[stage protocol](../development/stage-execution-protocol.md),
[convergence rule](../development/mission-convergence-rule.md) and
[next-stage selection](../development/autonomous-next-stage-selection.md).
Shared GOV-001 v1.1, WORK-001/STATE-001/AGENT-001 v1.0 and the applicable
Development baseline, C++ supplement and component maps remain binding.

## Scope and staging

This is the requested maturation coordination mission. It provides a dependency
map, not blanket authorization for an unbounded compiler campaign. Select one
bounded implementation stage at a time with its own exact definition and fresh
state. Measure its evidence in this mission independently; never copy its status.
The first selectable implementation endpoint is Gate 4 below, a bounded
prerequisite for executable recovery. No separate language runtime is proposed.

| Task | Depends on | Endpoint and required evidence |
|---|---|---|
| T0 — Baseline and scope | none | Record branch/HEAD, main and available remote refs, dirty paths, build/tool versions, current support inventory and application decision. Reconcile Gate 3 policy wording. Preserve existing work. Confirm integrated prerequisite or explicitly authorized reviewed worktree baseline. |
| T1 — Graph preservation (Mission 07 Gate 4) | T0 | Project existing typed consumer, selected route, rejoin, transformed-failure and containment facts into Graph IR and preserve them through binding, scheduling, optimization, preparation and backend-neutral lowering. Serial scheduling only. Hostile mutations rejected at every boundary. No runtime maturity claim. |
| T2 — Executable recovery (Gate 5) | T1 | Existing TextOutcome success bypasses responses; failure executes exactly selected ordinary recovery; obligation closes once; native and TinyVM outputs and receipts agree. Transformation executes only if already-accountable successor routing exists. |
| T3 — Quarantine and closure (Gates 6–7) | T2 | One serial activation without sibling/prior-publication faults returns structured containment, publishes nothing and cannot continue. CLI report_and_fail and embedded host consume the same result. Diagnostics/evidence survive; sanitizers and hostile tests pass. |
| T4 — Ratify profiles and family decisions | T0 | Publish intended-language inventory and a separately named first practical profile. Every family has accepted authority, observable semantics, explicit exclusions and evidence obligations. Resolve needed aggregate/write authority, collection/bounds/ownership, module and I/O lifecycle decisions before their dependent implementation. |
| T5 — Data and library vertical slices | T3, T4 | Implement each required missing family separately through both backends. Prove independent aggregate behavior, admitted collection bounds/failure behavior, reusable library calls and resource cleanup. Unsupported cases remain rejected. Refresh inventory at each gate. |
| T6 — Practical application | T5 | Build the ratified application from clean source using Igor. Application logic is Lyraform; external providers are limited to declared capabilities, not a hidden implementation of the app. Publish fixtures, exact invocation and expected results. |
| T7 — Developer readiness and consolidation | T6 | Reproduce from a clean checkout, run profile conformance and app suites, document supported syntax/contracts and build/test/debug workflow; record exclusions and remaining intended-language families. No fixture-name compiler dispatch or silent legacy fallback. |

T4 can be investigated alongside T1–T3 but cannot silently expand their scope.
The longer horizon after T7 includes remaining ratified language families,
general effectful graph execution and eventual compiler/bootstrap workloads.
Create new bounded missions for those; this mission does not declare them done.

## Accepted law and single owners

[Mission 07](canonical-source-disposition-and-quarantine-execution-mission-07.md)
provides the exact Gate 4–7 scope and laws under ADR 0066/0067. Reuse its staged
requirements, not its old baseline or permission grant.
Flowanalyst owns resolved disposition/type/operation identities; Flowcontracts
validates those artifacts; policy chooses only source-authorized routes;
Graph IR carries their identity; downstream stages preserve rather than re-derive
meaning; backends execute; activation authority quarantines; host policy reports.
ADR 0052 governs independent aggregates and ADR 0064 graph-derived parallelism.
New semantic facts need an explicit owner and accepted decision before code.

Reuse canonical structural frontend, TextOutcome, disposition evidence,
provider authorization, stage validators and backend pipelines. Retire duplicate
fact derivation and transitional projections when their replacements are proven.
Remove flow_graph_fail reachability from the newly admitted quarantine path;
do not delete compatibility behavior for unrelated paths. Historical language
manuals and FlowLFS do not supply missing canon.

## Practical application acceptance proposal

Until Henrik selects another target, propose a file-processing CLI: read an
explicit input and configuration, parse records, compute a deterministic summary,
and write a result through declared I/O capabilities. The ratified specification
must fix input/config formats, resource limits, output bytes, exit statuses and
recoverable-error behavior before implementation. It must include at least two
Lyraform source units exercising reusable application/library composition.

Required cases: valid input; empty input; malformed record; invalid configuration;
missing input; unavailable output; provider/resource failure. Explicitly declare
which failures may recover and which remain unsuccessful. Test deterministic
outputs and cleanup on every admitted exit. Supply changed input/configuration
cases not used during compiler development to detect fixture-specific behavior.
This is a proposed acceptance target, not newly accepted language semantics.

## Measurable completion criteria

- C1: zero required profile families without accepted authority and passing
  source-to-execution conformance evidence; complete inventory also lists all
  deferred intended-language families. Freeze its revision before measuring.
- C2: all Gate 4–7 requirements above pass on an identified reproducible baseline,
  including native/TinyVM receipt equivalence and containment non-continuation.
- C3: all seven application case classes pass with exact expected output/status,
  and altered-input cases pass without compiler changes; two or more Lyraform
  source units compile and run through the documented canonical workflow.
- C4: zero failing canonical, focused negative/hostile and required sanitizer
  checks; tool/platform exclusions explicitly reported and not counted as passes.
- C5: independent clean-checkout reproduction passes; support inventory, examples,
  diagnostics and developer guide agree; no unaccounted transitional semantic owner.

These criteria remain unmeasured. A passing pre-existing test count, parsed syntax,
a task checklist or reference runtime alone cannot satisfy them.

## Falsification and gate verification

Positive: real TextOutcome failure selects the declared recovery exactly once,
rejoining the same typed output as success on both backends.
Negative: fault wired into ordinary recovery or missing multi-route selection is
rejected before execution. Hostile: mutate producer/attempt/obligation identity,
route revision or rejoin type at each artifact boundary; refusal is mandatory.
After quarantine, instrument publication and continuation counters: either being
nonzero falsifies containment. Aggregate fan-out followed by member placement
must leave the earlier value unchanged; alias mutation falsifies ADR 0052.

For each selected stage record exact commands, inputs, artifact schemas/hashes,
expected diagnostics and results in its checkpoint. From a clean checkout with
that stage's changes and documented dependencies, use a fresh build directory:

```sh
./igor --build-dir /tmp/lyraform-maturation-check doctor
cmake -S . -B /tmp/lyraform-maturation-check -G Ninja
cmake --build /tmp/lyraform-maturation-check -j 4
ctest --test-dir /tmp/lyraform-maturation-check --output-on-failure -j 4
```

Use a unique fresh directory if that path already exists. Add the exact new
focused tests and sanitizer configurations at the relevant stage; the above
existing suite alone is insufficient. T6 must provide a checked-in application
fixture/runner with one documented reproduction command, not an imagined Igor
subcommand. Report hardware-dependent tests separately.

## Stops, consolidation and handoff

Stop the affected stage for unresolved semantic authority, red baseline,
unreviewed prerequisite, foreign/stale artifacts, unexpected baseline change,
or a required policy exception. Present a decision brief at the owning policy
or ADR layer. No silent workaround, widened scope or automatic main integration.
The initial Gate 3 review/integration question and application/profile decisions
are explicit dependencies, not permission requests made by this document.

At every gate update support claims, falsifying tests and evidence; remove proven
redundancy; document what became executable and what remains declarative. Only
then select the next bounded stage within authorized scope. At T7 preserve a
separate list of remaining full-language work instead of claiming full maturity.

Portable state lives at `.mission-state/lyraform.language-maturation/1/`.
Verify this document's hash against the JSON companion and validate exact state
identity/revision before any executor resumes. Handoff includes baseline,
permissions, measured criteria, evidence hashes, unresolved decisions and next
bounded stage. Prior mission state supplies no completion or authorization.
