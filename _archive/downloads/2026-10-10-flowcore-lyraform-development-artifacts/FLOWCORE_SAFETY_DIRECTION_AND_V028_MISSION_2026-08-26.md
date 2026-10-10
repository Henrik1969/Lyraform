# Flowcore Safety Direction and v0.28 Execution Mission

**Status:** Codex-ready mission and architectural checkpoint  
**Date:** 2026-08-26  
**Repository:** `Henrik1969/Flowcore`  
**Development branch:** `v25-symboltable-projection`  
**Observed pushed head:** `fdbded38bc23cbd2f333df02b1dc0a9192203ac9` — `Mark reusable chain mission done`  
**Proposed next milestone:** Flowcore/Flowmini v0.28 — Typed Artifact Contract Boundary  

## 1. Mission summary

Flowcore has reached a meaningful narrow compiler-chain milestone. A previously
built toolchain can compile newly named Flow programs and newly selected native
bindings without adding application-specific C++ dispatch, handwritten LLVM
emitters, or rebuilding the compiler tools.

The next mission is not to add more surface-language features. The next mission
is to make the existing inter-stage contracts rigorously typed, versioned,
validated, deterministic, and independently checkable. This is the smallest
necessary step before implementing Canonical Graph IR and executable safety
profiles.

The central architectural conclusion is:

> Safety is not a universal list of language features to remove. Safety is the
> mechanically verified fact that a particular execution graph remains inside
> its declared capability, resource, failure, and timing envelope.

Flowcore should eventually support several governed execution profiles without
splitting into several languages:

```text
general
bounded
hard_realtime
safety_critical
```

The language may contain powerful capabilities. A graph admitted under a
restricted profile must be proven unable to reach capabilities that profile
forbids.

## 2. Origin of the safety direction

The immediate discussion was inspired by the Joint Strike Fighter C++ coding
rules and the broader “remove before flight” discipline:

- no C++ exceptions in the safety-critical subset;
- no direct or indirect recursion;
- no heap allocation/deallocation after initialization;
- bounded control-flow complexity;
- predictable execution and resource consumption.

The useful lesson is not that every program should use a permanently amputated
language. The useful lesson is that a deployment domain must be able to reject
unbounded, unpredictable, or unauthorised behavior before execution.

Flowcore can express this more generally than a fixed restricted-C++ subset:

```text
The language contains the capability.
The selected graph declares whether it uses the capability.
The active law decides whether that use is admissible.
The compiler or independent validator proves or rejects the graph.
```

This applies not only to aircraft software. The same mechanism can govern:

- normal desktop applications;
- hard-real-time control loops;
- safety-critical processes;
- kernel-adjacent services;
- untrusted containers;
- Android, Windows, Amiga, web, GTK, Qt, TUI, and other application substrates;
- CPU, CUDA, remote, JIT, and interpreted execution providers.

Each domain receives a different law set while retaining one language, one
canonical semantic model, and explicit provider boundaries.

## 3. System-level lesson from Ariane 5

The Ariane 5 Flight 501 failure must not be reduced to “one bad conversion” or
“an unhandled exception.” The complete failure involved:

- an out-of-range floating-point-to-integer conversion;
- reused alignment software whose assumptions belonged to Ariane 4;
- a function that remained active after it had become operationally useless;
- two redundant inertial systems containing the same fault;
- diagnostic output being consumed in a way that contributed to destructive
  control behavior;
- incomplete system-level testing under the real Ariane 5 trajectory.

Therefore Flowcore safety laws must govern more than syntax:

- input and environmental assumptions;
- lifecycle reachability;
- common-mode failure;
- diagnostic versus operational data identity;
- provider replacement and assumption invalidation;
- whole-graph behavior under the deployment envelope;
- explicit safe-state and failure disposition.

Return codes alone are not a safety mechanism. An ignored return code is merely
a quiet failure. A fallible Flow operation must expose an explicit outcome edge,
and a restrictive profile must reject any graph whose failure outcome is
unconsumed or lacks an admitted final disposition.

## 4. Binding architectural laws

The following laws govern this mission and later safety work.

### 4.1 Meaning before representation

```text
Source syntax is not the authority.
Canonical semantic meaning is the authority.
JSON, matrices, LLVM, ELF, GUI, TUI, CUDA, and native calls are projections or providers.
```

### 4.2 Explicit capability authority

- Provider discovery is evidence, not authorization.
- Symbol existence is not authorization.
- Authorization covers the exact provider, symbol, calling convention,
  parameter/result carriers, effects, resources, contract identity, and
  evidence identity.
- Adding an unused authorized capability must not change compiler behavior.
- A program or provider name must never select application semantics.

### 4.3 Explicit failure flow

- Fallible operations expose explicit outcome semantics.
- Failure may not disappear between compiler stages.
- A restrictive profile requires every reachable failure path to reach an
  admitted recovery, degradation, or safe terminal state.
- Diagnostics are not operational payloads unless an explicit adapter says so.

### 4.4 Explicit lifecycle

The future execution model must be able to represent lifecycle phases such as:

```text
initialize -> seal_resources -> operate -> degrade -> shutdown
```

After `seal_resources`, a hard-real-time graph should be structurally unable to
reach allocation capabilities forbidden during operation.

### 4.5 Stable identity and provenance

- Published structural entities retain identity.
- Mutation produces a new observable revision rather than silently destroying
  the previous published state.
- Diagnostics and proof evidence refer to exact entity revisions.
- Splitting, fusion, pruning, lowering, and provider selection retain derivation
  lineage.
- Exact revision storage machinery remains provisional until deliberately
  implemented.

### 4.6 Independent contracts

- Every compiler stage consumes a public, versioned contract.
- A stage must understand and validate authoritative fields; copying an
  unparsed JSON substring is not contract consumption.
- Unsupported versions and malformed authority fail closed.
- Independent consumers must not need private frontend or backend internals.

### 4.7 Graph authority

- `->` value placement and `=>` graph connection remain distinct.
- Nodes do work.
- Ports expose typed inputs and outputs.
- Wires connect ports and represent contracts, not moving values.
- Signals/envelopes move through wires.
- Scheduling policy remains separate from receiver activation semantics.
- Graph IR will be canonical; matrix representations remain derived views.

### 4.8 No premature safety claim

Flowcore remains experimental. Passing the current mission does not authorize a
production, security-critical, or safety-critical claim. Such claims require
the later Graph IR, law, boundedness, provider-evidence, and toolchain-evidence
gates described below.

## 5. Confirmed pushed baseline

The following facts were observed on the pushed development branch at
`fdbded38`. They are the starting baseline, not tasks to rediscover or
reimplement.

### 5.1 Frontend and structural model

- Recursive, precedence-aware expression AST exists.
- Typed declarations, statements, blocks, expressions, and source ownership
  exist in arena-backed structures.
- AST and structural SymbolTable roles are separated.
- SymbolTable projection includes scopes, typed origins, source locations,
  source maps, and stable-within-one-export identities.
- `flowmini.frontend_bundle` version 2 is independently consumable and has
  malformed-input coverage.

### 5.2 Semantic and provider chain

- Flowanalyst consumes the frontend bundle without reparsing source.
- It reports names, calls, types, refined declarations, ABI requirements,
  targets, regions, dependencies, diagnostics, and initial effect facts.
- Flowbind performs policy-gated provider and ABI verification.
- Exact effect and resource facts are retained for external operations.
- Imported short-name ambiguity remains permanent when several providers
  collide; qualified calls remain stable.

### 5.3 Generic lowering

- The chain is:

```text
Flow source
  -> Flowmini frontend bundle
  -> Flowanalyst semantic report and lowering plan
  -> Flowbind exact capability authorization
  -> Flowparallel execution plan
  -> Flowoptimize optimization report
  -> Flowlower LLVM IR
  -> native linker
  -> ELF artifact
```

- Required stages no longer select application behavior by source-unit,
  fixture, filename, profile name, or capability-set recognizer.
- Legacy handwritten Flowlower application emitters were removed.
- Values, expressions, results, branches, loops, assignments, checked argv
  access, external calls, cleanup, and returns can follow the generic structured
  path for the admitted subset.
- `flowcat`, `sel`, `flow_less`, libc calls, kernel calls, generated bindings,
  and ncurses examples provide native evidence.

### 5.4 Resource and graph evidence

- Ncurses windows use a distinct typed ABI carrier.
- Flowbind validates acquisition and cleanup path-sensitively across branches.
- Missing cleanup, double cleanup, cleanup-before-acquisition, and unsupported
  loop lifetime cases are rejected.
- Runtime graph routing preserves source/destination port identity.
- Wires and signals have distinct identities.
- Fan-out retains signal identity across distinct wires.
- Required/optional/terminal input laws and failure propagation have executable
  coverage.

### 5.5 Parallel/provider boundary

- Flowparallel publishes a conservative execution plan.
- CPU and CUDA provider experiments exist with mandatory CPU fallback.
- Runtime provider decisions can consume capabilities, calibration, and policy.
- Graph-to-matrix projections and CPU/CUDA differential experiments exist.
- These do not yet constitute general execution of arbitrary user regions.

### 5.6 Recorded test evidence

The repository records:

```text
canonical root CTest:        54/54 PASS
ASan/UBSan CTest:            54/54 PASS
AST goldens:                 28/28 PASS
Flowmini categorized suite:  78/78 PASS
```

The sanitizer record disables leak detection for the external-provider test
environment. Preserve that qualification. Reproduce the gates locally before
changing public contracts.

### 5.7 Repository state

- `v25-symboltable-projection` is 173 commits ahead of `main` at the observed
  checkpoint.
- PR #4 is open, mergeable, and draft.
- PR #4's description still describes the earlier v0.25 provenance scope and
  does not accurately summarize the complete v0.27 branch.
- `main` has an active protection ruleset.
- No GitHub Actions workflow was present at the observed tree.

## 6. Current limitations and architectural debt

### 6.1 Critical: raw JSON substring consumption

Flowparallel and Flowoptimize currently locate authoritative fields by searching
and slicing raw JSON text. Examples include format/status checks, target arrays,
lowering plans, matrix entries, provenance, and counters.

This is incompatible with the intended safety model because:

- nested fields may be mistaken for top-level authority;
- duplicate-key behavior is inconsistent;
- field ordering or formatting can change behavior;
- malformed structures may silently collapse into `{}` or `[]`;
- copied data may be preserved without semantic validation;
- a stage cannot prove that it understood the artifact it forwarded.

Flowbind and Flowlower contain stricter structured parsing, but parsing and
validation are not yet consistent across the entire chain.

### 6.2 Canonical Graph IR is absent

The current `flowcore.lowering_plan` is an important reusable bridge but remains
statement/lowering oriented. It is not yet the canonical graph containing typed
nodes, ports, wires, signals, outcomes, contracts, effects, revisions, and
provenance.

Executable runtime graph-routing evidence must not be misreported as completed
Canonical Graph IR.

### 6.3 Effect analysis is incomplete

User-defined functions are largely classified as `pure` or `unknown`. External
operations carry richer declarations, but the language does not yet infer and
compose a complete effect set covering:

```text
allocation
blocking
I/O domain
memory read/write regions
nondeterminism
failure
resource acquisition/release
stack demand
timing bounds
```

### 6.4 Boundedness is not proven

The current compiler does not generally prove:

- loop iteration bounds;
- complete call-graph recursion properties;
- maximum stack usage;
- maximum allocation;
- worst-case execution time;
- queue, signal, or buffer capacity.

### 6.5 Writable storage semantics are provisional

Positive `c_pointer(N)` currently represents call-lifetime writable storage as
a compatibility mechanism. It must not silently become permanent language
semantics.

Candidate permanent forms are:

1. dedicated bounded buffer type;
2. explicit lexical storage declaration;
3. effectful allocation operation.

The recommended smallest permanent direction is:

```text
bounded buffer value/type
  + lexical storage by default
  + explicit checked ABI borrow conversion
  + separate effectful allocation for dynamic/non-lexical storage
```

Do not implement this public-language choice as part of v0.28 unless Henrik
explicitly authorizes it. Preserve and test the compatibility boundary instead.

### 6.6 Revisioned graph identity is not implemented

Arena IDs and origins exist within one frontend export. Full cross-revision
identity, structural sharing, diff layers, branchable roots, proof invalidation,
and transformation lineage remain architectural direction.

### 6.7 Provider verification is narrow

`dlopen` and `dlsym` prove presence, not truthful behavior. Aggregate ABI layout
evidence exists, but aggregate call lowering remains deliberately blocked.
Future provider evidence must bind artifact identity, version, platform, ABI,
effects, qualification, and revocation/expiry as appropriate.

### 6.8 Documentation drift

Current documents disagree about:

- v0.25/v0.26/v0.27 naming;
- canonical root versus sibling build scope;
- 12/12 versus 14/14 SymbolTable tests;
- whether explicit lowering profiles or handwritten emitters remain;
- named-target and separate-artifact maturity;
- current versus historical conformance status.

Documentation must be reconciled from executable evidence rather than by
copying the newest wording blindly.

## 7. v0.28 objective

Create one strict typed artifact-contract boundary across all required compiler
stages.

After v0.28, every required stage must either:

1. parse and validate the complete authoritative artifact it consumes; or
2. reject it with a structured diagnostic identifying the exact contract,
   version, field, source provenance, and failure reason.

No required compiler stage may determine authority through substring searches,
formatting assumptions, application names, source names, profile names, or
capability-set recognition.

The v0.28 mission prepares Canonical Graph IR. It does not implement the full
Graph IR, safety profiles, WCET analysis, or self-hosting.

## 8. Non-goals for v0.28

Do not widen the mission into:

- new user-facing syntax unrelated to contract hardening;
- full Canonical Graph IR;
- final revisioned persistent storage machinery;
- hard-real-time certification;
- general aggregate ABI lowering;
- arbitrary native library support;
- general CUDA lowering or arbitrary parallel user-region execution;
- self-hosting;
- a new GUI, TUI, IDE, package manager, or runtime;
- merging PR #4 or changing GitHub settings.

## 9. Execution gates

### Gate 0 — establish the real baseline

1. Read repository `AGENTS.md` and all task-specific instructions.
2. Inspect branch, upstream, worktree, recent commits, and `.codex-run-state`.
3. Preserve unrelated and pre-existing user changes.
4. Confirm that the starting branch contains `fdbded38` or reconcile newer work.
5. Run the canonical clean build and CTest suite.
6. Run focused Flowmini, Flowanalyst, Flowbind, Flowparallel, Flowoptimize, and
   Flowlower gates.
7. Record exact commands, toolchain versions, counts, durations, and failures.
8. Do not assume the recorded 54/54 result still applies to a newer checkout.

Exit condition:

- starting state is understood;
- existing failures are distinguished from mission changes;
- the worktree is safe to modify;
- baseline evidence is recorded.

### Gate 1 — inventory every artifact contract

Inventory the complete current pipeline:

```text
flowmini.frontend_bundle
flowanalyst.semantic_report
flowcore.lowering_plan
flowbind.binding_report
flowparallel.execution_plan
flowoptimize.optimization_report
flowlower.lowering_report
provider manifests
runtime capability snapshots
provider-decision artifacts
```

For every artifact record:

- producer;
- consumers;
- format identity;
- version;
- required and optional fields;
- authority-bearing fields;
- provenance fields;
- stable identities;
- compatibility behavior;
- rejection behavior;
- current parser implementation;
- current tests and uncovered attacks.

Search for all uses of:

```text
find(
substr(
string_view searches
format/status/version text matching
JSON object/array slicing
default {} or [] substitution
application/source/profile/capability-set dispatch
```

Do not treat every string operation as a bug. Classify whether it processes
ordinary data or decides artifact authority.

Exit condition:

- one checked-in artifact inventory identifies every authoritative boundary and
  every remaining text-based authority decision.

### Gate 2 — establish public typed contract ADTs

Create or extract a small public contract component with explicit ownership and
version boundaries. Prefer the smallest reusable design consistent with the
existing independent-stage law.

Required properties:

- strict JSON value parsing;
- complete-input consumption;
- duplicate-key rejection;
- integer range validation;
- required versus optional fields;
- exact top-level format and version validation;
- explicit unknown-field policy;
- typed enums or tagged variants for authoritative categories;
- stable operation/target/provider/resource/provenance identities;
- deterministic serialization;
- structured validation diagnostics;
- no dependency on private Flowmini AST implementation;
- no backend or application-specific behavior.

Do not build a grand generic serialization framework. Build the narrow public
contracts needed by the current chain.

Candidate ownership:

```text
Flowcontracts/
    public artifact ADTs
    strict JSON adapter
    validators
    deterministic serializers
    contract-focused tests
```

The exact directory name is not binding. The ownership boundary is.

Exit condition:

- the public component can parse, validate, and deterministically serialize the
  existing accepted artifacts without depending on compiler-stage internals.

### Gate 3 — migrate Flowparallel

Replace raw JSON searching and substring extraction in Flowparallel with typed
contract consumption.

Flowparallel must:

- validate top-level format, version, and status structurally;
- consume typed targets, lowering operations, ABI contracts, effects,
  dependencies, matrices, provenance, and identities;
- distinguish missing, malformed, unsupported, unresolved, and blocked states;
- preserve the lowering plan and provenance through typed data, not copied text;
- reject duplicate or conflicting identities;
- preserve CPU fallback law;
- emit deterministic `flowparallel.execution_plan` output.

Adversarial tests must include:

- nested fake `format`, `version`, and `status` fields;
- duplicate top-level keys;
- reordered fields and arbitrary legal whitespace;
- escaped strings containing field-like text;
- malformed/truncated arrays and objects;
- absent and incompatible lowering plans;
- duplicate operation IDs;
- lost target or provenance identity;
- malformed matrix entries;
- input whose pretty-printed text differs but semantic content is equal.

Exit condition:

- no raw text search decides Flowparallel artifact authority;
- all previous valid behavior remains green;
- hostile inputs fail closed with structured reasons.

### Gate 4 — migrate Flowoptimize

Replace raw JSON searching and substring extraction in Flowoptimize with typed
contract consumption.

Flowoptimize must:

- accept only valid semantic reports or execution plans of supported versions;
- consume provider decisions structurally;
- preserve canonical graph authority while treating matrices as derived views;
- validate matrix dimensions and entries before deduplication;
- preserve operation, target, provider, effect, resource, and provenance
  identities;
- emit a new artifact rather than overwriting its input;
- describe every transform, including identity transforms;
- deterministically serialize the optimization report.

Adversarial tests must include:

- all Gate 3 JSON attacks;
- fake nested provider decisions;
- unsupported provider/representation pairs;
- out-of-range matrix coordinates;
- conflicting duplicates;
- transform count/provenance mismatches;
- silent operation loss or reordering;
- mutation without derivation evidence.

Exit condition:

- no raw text search decides Flowoptimize artifact authority;
- accepted transformations are typed, attributable, and independently
  reproducible.

### Gate 5 — reconcile Flowbind and Flowlower

Flowbind and Flowlower already contain stricter parsing work. Do not rewrite
them gratuitously. Move them onto the same public contracts where this removes
duplicate authority logic and strengthens consistency.

Required checks:

- exact format/version/status handling;
- duplicate-key rejection;
- operation identity uniqueness;
- exact provider/ABI/effect/resource authorization;
- complete binding-report validation;
- target selection identity;
- structured refusal for unsupported carriers or operations;
- complete operation reachability and controlling-block validation;
- cleanup law preservation;
- no silent default to empty plans or empty authorization.

Preserve the existing profile-free and application-independent lowerer.

Exit condition:

- all required stages agree on the same contract semantics;
- stage-local compatibility parsing no longer defines competing authority.

### Gate 6 — add an independent validator

Add an independently invocable validator, provisionally named `flowvalidate`.

It must:

- identify an artifact by format and version;
- validate structure and cross-field invariants;
- optionally emit canonical deterministic JSON;
- emit human-readable and machine-readable diagnostics;
- avoid linking private stage implementations;
- return stable exit statuses for valid, invalid, unsupported, and blocked
  artifacts;
- validate artifacts captured between every current pipeline stage.

Required proof:

```text
producer output
  -> independent validator
  -> consumer
```

The consumer must never be the only validator of the producer's claims.

Exit condition:

- every public current artifact has independent positive, negative, mutation,
  and round-trip validation.

### Gate 7 — prove end-to-end identity preservation

Create an acceptance program containing enough structure to exercise:

- local values;
- external call and exact result placement;
- a conditional;
- a loop or repeated operation;
- a fallible/resource-bearing provider operation where currently admitted;
- cleanup;
- explicit return;
- source provenance;
- named target when the current artifact boundary supports it.

At every stage, assert preservation or documented derivation of:

```text
source identity
target identity
operation identity
block/control identity
provider identity
ABI contract identity
effect facts
resource facts
authorization evidence
provenance
```

Mutation attacks must independently alter each category and prove that the
appropriate next consumer rejects it.

Exit condition:

- the already-built compiler chain produces and executes the native artifact;
- no stage silently loses or reinvents authoritative identity;
- no compiler C++ application dispatch is added.

### Gate 8 — documentation and repository truth

Reconcile present-tense documentation against the verified checkout.

At minimum inspect and update where necessary:

```text
README.md
Flowmini/README.md
Flowmini/CURRENT.md
Flowmini/VERSION_INDEX.md
Flowmini/CHANGELOG.md
docs/architecture/README.md
docs/architecture/FLOWCORE-FULL-STACK-IMPLEMENTATION-PLAN.md
docs/architecture/frankencore-current-conformance.md
docs/language/flowmini-programmers-manual.md
Flowlower/README.md
PR #4 handoff notes, without modifying GitHub unless separately authorized
```

Documentation must distinguish:

- implemented and executable;
- implemented but narrow;
- compatibility behavior;
- binding architectural direction;
- provisional mechanism;
- future work;
- historical evidence.

Do not merely replace all old counts with one number. Different suites may
legitimately have different counts; name the exact gate that each count belongs
to.

Exit condition:

- no active document claims that removed profile dispatch still exists;
- no active document presents Graph IR or safety profiles as implemented;
- build/test instructions identify one canonical scope;
- version naming and test counts are internally consistent.

### Gate 9 — complete hardening evidence

Run, at minimum:

- focused unit and contract tests;
- complete canonical CMake/Ninja build;
- complete CTest suite;
- ASan and UBSan suite;
- malformed-input and duplicate-key attacks;
- deterministic serialization checks;
- round-trip checks;
- native LLVM linking and execution examples;
- `sel`, `flowcat`, `flow_less`, ncurses, graph-routing, and generated-binding
  gates;
- `git diff --check`;
- repository artifact/hygiene inspection.

Leak sanitizer exclusions or environmental compensation must be documented
precisely. A skipped hardware/provider gate is not a pass; record it as skipped
with the missing capability and retain the required fallback evidence.

Exit condition:

- all required gates pass;
- every exception or skip is explicitly scoped;
- evidence is tied to the exact commit and environment.

## 10. Definition of done for v0.28

The mission is complete only when all of the following are true:

- Flowparallel no longer uses raw JSON searches to decide artifact authority.
- Flowoptimize no longer uses raw JSON searches to decide artifact authority.
- Flowbind and Flowlower use compatible public contract semantics.
- Every required current artifact has a typed ADT and strict validator.
- Duplicate keys, malformed authority, incompatible versions, missing required
  fields, and conflicting identities fail closed.
- Serialization is deterministic.
- Operation, target, provider, effect, resource, authorization, and provenance
  identity are preserved or explicitly derived across every stage.
- An independent validator accepts every valid captured stage artifact and
  rejects every required adversarial mutation.
- The reusable profile-free native chain still works without application-
  specific C++ changes.
- Existing cleanup and graph-routing laws remain green.
- Canonical and sanitizer suites pass with exact recorded counts.
- Active documentation matches executable reality.
- A checkpoint document records commands, results, limitations, commit range,
  and exact next action.
- Final changes are committed and pushed to the authorized development branch.
- The worktree is clean and synchronized with its upstream.

An intermediate green build, one migrated stage, a documentation update, a
commit, or a context boundary is not completion.

## 11. Git and repository constraints

When Henrik supplies this document as an execution instruction:

- normal commits and pushes to the currently checked-out development branch are
  authorized at coherent green checkpoints;
- inspect and preserve unrelated user changes;
- use small reversible architectural slices;
- update a maturation ledger before major checkpoints;
- do not force-push;
- do not rewrite published history;
- do not merge PR #4;
- do not mark PR #4 ready;
- do not delete branches;
- do not change repository settings or rulesets;
- do not commit build trees, IDE indexes, transient logs, downloaded platforms,
  or generated scratch artifacts;
- do not edit unrelated issues or pull requests.

If the checkout contains newer work than the observed baseline, reconcile it
instead of resetting or discarding it.

## 12. Legitimate stopping conditions

Do not stop merely because:

- the work is large;
- one stage is finished;
- a test fails and can be diagnosed;
- several ordinary implementation alternatives exist;
- a checkpoint was committed;
- context was compacted;
- a safer reversible choice is available.

Stop only when:

1. the complete definition of done passes; or
2. all safe progress is blocked by a genuine public-language decision, missing
   credential, unavailable external authority, or incompatible dependency.

A blocked report must state:

- the exact blocker;
- evidence;
- attempted reversible alternatives;
- preserved passing state;
- the smallest decision or authority required from Henrik.

Do not implement the permanent writable-storage syntax without Henrik's ruling.
That decision is outside the required v0.28 contract-hardening mission.

## 13. Required final report

The final Codex report must include:

- starting and ending commits;
- complete commit range;
- changed contract boundaries;
- eliminated raw-authority searches;
- new public ADTs and validator surfaces;
- adversarial tests and what each proves;
- exact build and test commands;
- exact counts and durations;
- sanitizer configuration and exclusions;
- native execution evidence;
- remaining compatibility behavior;
- remaining platform limitations;
- documentation reconciled;
- pushed/clean worktree confirmation;
- exact recommended next milestone.

## 14. Roadmap after v0.28

### v0.29 — Canonical Graph IR

Define a versioned typed Graph IR containing, at minimum:

```text
GraphId and GraphRevision
NodeId and NodeRevision
PortId and contract
WireId and endpoints
Signal/outcome identity
typed payload/envelope contract
effect set
resource set
failure edges
lifecycle phase
source and derivation provenance
target membership
```

Required laws include connection compatibility, required-input satisfaction,
unconnected-output policy, fan-out, failure propagation, control/data
separation, and explicit scheduling-policy boundaries.

### v0.30 — Executable safety laws

Add a law/admission layer over Canonical Graph IR.

Initial profiles:

| Profile | Initial intent |
| --- | --- |
| `general` | Broad language and provider use with explicit contracts |
| `bounded` | Explicit memory, queue, loop, and resource ceilings |
| `hard_realtime` | Bounded execution, no forbidden post-seal allocation, deterministic admitted providers |
| `safety_critical` | Hard-real-time constraints plus complete failure disposition, certified providers, and auditable evidence |

The profile system must be additive policy over one canonical language, not a
collection of disconnected dialects.

### Later safety work

- compositional effect inference;
- call-graph recursion proof;
- bounded-loop and stack analysis;
- allocation and queue ceilings;
- WCET provider interfaces and target-specific evidence;
- lifecycle capability revocation;
- complete outcome/failure topology;
- revisioned proof invalidation;
- provider artifact identity and qualification;
- diverse redundancy/common-mode analysis;
- toolchain qualification and reproducible build evidence.

## 15. Final architectural statement

The reusable v0.27 compiler chain proves that Flowcore can carry source-derived
meaning through explicit capability authorization into native execution without
application-specific compiler dispatch.

The v0.28 mission must now prove that each stage understands the authoritative
artifact it consumes.

After that, Canonical Graph IR can make whole-system structure explicit, and
executable laws can admit or reject graphs for particular deployment domains.

The governing principle is:

> Grant only before execution what the graph can prove it needs, preserve every
> relevant identity and failure path, and revoke everything its deployment law
> forbids.

