# Lyraform language maturation mission

ID: `lyraform.language-maturation`; revision: `3`; owner: Henrik Sørensen.
Prepared: 2026-10-17. Status: selected for the ConfigResolve branch experiment.
Canonical definition: this Markdown file. The companion JSON records its SHA-256
and measurable criteria; any divergence must be reconciled before execution.

## Intent, goal and authorization

Intent: mature Lyraform into the contracted, graph-visible language already
intended, so real software can be developed in it.

Goal: establish an owner-ratified language profile and deliver a reproducible
ConfigResolve library port and consumer using that profile through the canonical compiler, with
explicit failure handling, honest support boundaries and measured backend
agreement. Profile completion and the application milestone must be measured
separately; neither alone warrants a claim of the full intended language.

Henrik explicitly authorized branch-local implementation and evidence-driven
successor missions on 2026-10-17, continuing until an actual binary built from
Lyraform source exists. Branch: `experiment/configresolve-lyraform`, forked from
current `codex/disposition-policy-selection-gate3` HEAD and preserving its dirty
working tree. Comparison and merge are deferred. No push or merge is authorized.
ConfigResolve-specific code, identifiers or fixture dispatch must not enter the
compiler/base runtime. Host providers must expose generic capabilities only.
Existing semantic decision gates remain binding; implementation choices within
accepted meaning can proceed autonomously. The owner selected the current
working tree as the experiment baseline; this does not claim main integration.


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
| T0 — Baseline and scope | none | Record branch/HEAD, main and available remote refs, dirty paths, build/tool versions, current support inventory and pinned ConfigResolve source/contract/test inventory. Reconcile Gate 3 policy wording. Preserve existing work. Confirm integrated prerequisite or explicitly authorized reviewed worktree baseline. |
| T1 — Graph preservation (Mission 07 Gate 4) | T0 | Project existing typed consumer, selected route, rejoin, transformed-failure and containment facts into Graph IR and preserve them through binding, scheduling, optimization, preparation and backend-neutral lowering. Serial scheduling only. Hostile mutations rejected at every boundary. No runtime maturity claim. |
| T2 — Executable recovery (Gate 5) | T1 | Existing TextOutcome success bypasses responses; failure executes exactly selected ordinary recovery; obligation closes once; native and TinyVM outputs and receipts agree. Transformation executes only if already-accountable successor routing exists. |
| T3 — Quarantine and closure (Gates 6–7) | T2 | One serial activation without sibling/prior-publication faults returns structured containment, publishes nothing and cannot continue. CLI report_and_fail and embedded host consume the same result. Diagnostics/evidence survive; sanitizers and hostile tests pass. |
| T4 — Ratify profiles and family decisions | T0 | Publish intended-language inventory and a separately named first practical profile. Every family has accepted authority, observable semantics, explicit exclusions and evidence obligations. Resolve needed aggregate/write authority, collection/bounds/ownership, module and I/O lifecycle decisions before their dependent implementation. |
| T5 — Data and library vertical slices | T3, T4 | Implement each required missing family separately through both backends. Prove independent aggregate behavior, admitted collection bounds/failure behavior, reusable library calls and resource cleanup. Unsupported cases remain rejected. Refresh inventory at each gate. |
| T6 — ConfigResolve port | T5 | Build the ConfigResolve engine and a consumer from clean Lyraform source using Igor. Resolution, validation, provenance, store/view and transaction behavior belong in Lyraform. Publish differential fixtures and explicit compatibility decisions; a wrapper around the C++ engine does not satisfy the port. |
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

## Selected acceptance target: ConfigResolve in Lyraform

Henrik selected a port of the existing ConfigResolve codebase on 2026-10-17.
This replaces revision 1's provisional file-processing CLI. Target selection
authorizes this mission revision; implementation is now selected by the subsequent branch-experiment instruction.

Source reference: [ConfigResolve](../../../ConfigResolve/README.md), inspected at
HEAD `35e5aae9974c796e26e797f144570bb9f18122d0`, with dirty documentation,
CMake changes and experimental/untracked work. T0 must pin the exact reference
files and hashes, not treat HEAD alone as the observed working tree. Preserve
that repository and follow its [policy map](../../../ConfigResolve/Policies/README.md).
Stable v1.1.0 is the initial contract reference; experimental/v2 behavior is not
automatically promoted. The port remains Lyraform-owned maturation work;
changes to the source project need their own scope and authority.

T0/T4 must map the complete public behavior surface to port requirements:

- scalar values, key paths, ordered facts, source precedence and conflict policies;
- resolved values, provenance, candidate evidence and explanation;
- schema validation and diagnostics;
- defaults and argument/environment/file loaders;
- governed stores, transactions, mutation/reset and failure-state preservation;
- scoped views, getters/fallbacks, export and secret redaction;
- typed consumer projections, errors, lifecycle and public interoperability.

Map every reference test family: core, loaders, files, store, views, bindings,
schema, API consistency, fire/hostile tests and C API. Classify each requirement
as implemented, explicitly deferred, intentionally different with approved
rationale, or host-specific (with its Lyraform equivalent). Partial slices are
milestones, not completion of the port. C++ template syntax, binary ABI and
packaging are separate compatibility decisions; semantic parity does not claim
binary compatibility. No public behavior may disappear through an unrecorded
exclusion.

Use [API stability](../../../ConfigResolve/docs/API_STABILITY.md),
[API map](../../../ConfigResolve/docs/PUBLIC_API_MAP.md) and the
[scalar behavior record](../../../ConfigResolve/docs/V1_SCALAR_EQUIVALENCE_AND_POLICY_AXES.md).
The latter identifies finite-double equivalence defects and unresolved signed-zero,
NaN and other policy choices. Differential mismatch is evidence to classify,
not authority to preserve a defect or silently fix semantics. Freeze the chosen
compatibility ruling and expected outcomes before implementation of that case.

Run shared language-neutral fixtures through the pinned C++ reference and the
Lyraform port, comparing values/types, diagnostics, provenance, state transitions,
exports/redaction and scoped visibility. Normalize only declared incidental
representation differences. Test positive, negative and hostile inputs, including
ambiguous precedence, invalid keys/types, rejected transactions, secret leakage,
scope escape and scalar edge cases. Add held-out inputs to detect hardcoding.
Native and TinyVM must agree on admitted port behavior. Reference runs are test
oracles; the executing port must not delegate the engine to C++, the C ABI or
Capabilities/ConfigResolveAdapter. Host I/O may use declared providers.

Deliver a reusable Lyraform library plus a separate Lyraform consumer exercising
layered configuration, validation, scoped access and explicit failure handling.
Exact source layout, invocations and expected artifacts are T0/T4 outputs.

## Measurable completion criteria

- C1: zero required profile families without accepted authority and passing
  source-to-execution conformance evidence; complete inventory also lists all
  deferred intended-language families. Freeze its revision before measuring.
- C2: all Gate 4–7 requirements above pass on an identified reproducible baseline,
  including native/TinyVM receipt equivalence and containment non-continuation.
- C3: zero unclassified public behaviors, zero unapproved semantic divergences,
  zero deferred required port behaviors and zero failures in the frozen differential
  and hostile corpus. A reusable Lyraform library and separate Lyraform consumer
  execute through the canonical workflow; held-out cases need no compiler changes.
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
The initial Gate 3 review/integration question and profile and compatibility decisions
are explicit dependencies, not permission requests made by this document.

At every gate update support claims, falsifying tests and evidence; remove proven
redundancy; document what became executable and what remains declarative. Only
then select the next bounded stage within authorized scope. At T7 preserve a
separate list of remaining full-language work instead of claiming full maturity.

Portable state lives at `.mission-state/lyraform.language-maturation/3/`.
Verify this document's hash against the JSON companion and validate exact state
identity/revision before any executor resumes. Handoff includes baseline,
permissions, measured criteria, evidence hashes, unresolved decisions and next
bounded stage. Prior mission state supplies no completion or authorization.

## Revision provenance

Revision 2 records Henrik’s explicit ConfigResolve target selection.
[Revision 1](history/lyraform-language-maturation/revision-1/lyraform-language-maturation.md)
and its JSON definition are preserved. Revision 1 state remains unchanged;
revision 2 starts planned with all criteria unmeasured.

Revision 3 records authorization to execute and chain bounded missions on the
experiment branch; revision 2 definition and state are preserved. T0 is selected
first: [ConfigResolve contract inventory](configresolve-port-contract-inventory.md).
