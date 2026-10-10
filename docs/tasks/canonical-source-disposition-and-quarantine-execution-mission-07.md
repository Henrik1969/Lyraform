# Lyraform Canonical Disposition Execution — Mission 07

## Source-declared consumers, policy-selected routes, and quarantined activations

**Mission type:** Compiler convergence / canonical disposition execution
**Authority:** ADR 0066 and ADR 0067 only, together with their prerequisite
accepted disposition laws
**Required stop:** Gate 7
**Canonical branch:** `main` after the disposition-lane bridge is integrated
**Baseline:** Begin from synchronized `main` containing commit
`e077f6745e1974ec4d50c94af70297c166d85bf5` or its merged descendant. Verify
the exact HEAD and stop if the bridge, diagnostics, evidence epoch, or accepted
ADRs are absent.

---

# 1. Objective

Implement the smallest end-to-end executable projection of the decisions now
frozen by ADR 0066 and ADR 0067:

```text
source-declared producer dispositions
    -> typed ordinary response functions
    -> closed consumer declaration
    -> explicit success/failure consumer junction
    -> versioned selection among authorized routes
    -> canonical Graph IR
    -> LLVM and TinyVM parity
    -> explicit quarantined activation result for one bounded fault
```

The first expected-failure proof uses the existing `TextOutcome` producer
because its success/failure set, commit law, ownership, and local accounting
already exist. Do not invent another producer merely to simplify tests.

The first fault proof is one serial activation satisfying ADR 0067's no-sibling
and no-prior-publication bound.

---

# 2. Frozen language decisions

The following meanings are selected and are not design questions in this
mission:

```lyraform
fn producer(): T fails { E } faults { F }

fn recover_response(problem : failure E): recover T
fn transform_response(problem : failure E): transform E2

consumer responses {
    recover_response
    transform_response
}

node producer_node : fn producer
node handled       : consumer responses

wire producer_node.out     => handled.success
wire producer_node.failure => handled.failure
wire handled.out           => next.in

node quarantine : containment activation
wire producer_node.fault => quarantine.fault
```

The implementation may adjust punctuation only where the existing lexer makes
the shown spelling impossible. It must not change the selected semantic
categories, hide the association, replace semantic identities with strings, or
introduce a second equivalent spelling. Any necessary punctuation adjustment
must be documented before parser implementation and preserve these words and
roles.

---

# 3. Campaign restrictions

Do not implement or infer:

- exceptions, stack unwinding, dynamic nearest handlers, or `try`/`catch`;
- implicit propagation, default consumers, diagnostic-and-drop, or silent
  command termination;
- generic type syntax from `failure E`;
- open failure sets, wildcard matching, subtype dispatch, or textual runtime
  function lookup;
- retry, timeout, cancellation, backpressure, or automatic suppliers;
- fault recovery, restart, process quarantine, or parallel fault composition;
- new aggregate, mutation, guard, or context-scope semantics;
- FlowLFS or legacy-runtime implementation reuse as canonical authority.

Legacy behavior may remain an oracle only where it agrees with accepted
canonical laws.

---

# 4. Stage 0 — Baseline and identity inventory

Record:

- exact local and remote `main` SHA;
- current worktrees and dirty/untracked paths;
- the merged bridge commit ancestry;
- parser, AST, symbol, semantic, Graph IR, policy, LLVM, TinyVM, and runtime
  ownership points for every new fact;
- current `TextOutcome` operation, owner, obligation, commit, and provenance
  identities;
- current native and TinyVM terminal-failure paths.

Run the canonical build and tests before editing. Stop on a red baseline.

## Gate 0

PASS only with a clean reproducible baseline and a machine-readable identity
map. Preserve unrelated work exactly.

---

# 5. Stage 1 — Structural source completeness

Extend the canonical lexer/parser/AST/frontend bundle for exactly:

- closed `fails { ... }` and `faults { ... }` function clauses;
- `failure E` parameter type form;
- `recover T` and `transform E2` response result forms;
- closed `consumer` declarations containing ordinary function references;
- `node ... : consumer ...`;
- `node ... : containment activation`;
- `success`, `failure`, and `fault` endpoint spellings used by the frozen graph
  association.

Preserve complete source spans and stable structural identities. Parsing does
not establish semantic legality.

Negative evidence must cover malformed and incomplete sets, duplicate clauses,
wrong declaration positions, malformed envelope/result forms, duplicate
consumer members, and malformed endpoint declarations.

## Gate 1

PASS only when complete syntax round-trips deterministically and malformed
forms fail before semantic analysis without contaminating legacy parsing.

---

# 6. Stage 2 — Canonical semantic authority

Semantic analysis must resolve:

- every failure and fault type identity;
- every response function identity;
- exact envelope payload and immutable evidence projection;
- response class and outgoing type;
- consumer closed accepted set and possible outgoing set;
- producer node, consumer instance, and exact paired success/failure origin;
- containment authority and exact activation scope;
- all source provenance.

For the first executable expected-failure slice, derive the producer set from
the existing `TextOutcome` authority and require the response recovery type to
equal its success type exactly.

Refuse different producer attempts feeding one junction, missing disposition
inputs, incompatible envelope or recovery types, faults entering ordinary
consumers, unaccounted transformed failures, ambiguous routes, and output
co-emission.

## Gate 2

PASS only when one canonical semantic component owns these facts and every
later consumer validates rather than re-derives them.

---

# 7. Stage 3 — Versioned policy selection

Define one versioned route-selection artifact carrying the identities required
by ADR 0066. Support:

- deterministic `fixed_single_route` selection for a consumer with one legal
  route;
- one explicit supplied policy selection where two legal response functions
  exist;
- the same canonical artifact as external input and embedded build input.

Policy must never create a function, route, transition, type, or success.
Refuse missing, stale, foreign-module, wrong-revision, incompatible,
unauthorized, ambiguous, and extra policy selections.

Define one versioned host-policy selection for ADR 0067's initial
`report_and_fail` action. No other host action is admitted in this mission.

## Gate 3

PASS only when source establishes all legal choices and policy selects exactly
one without changing semantic facts.

---

# 8. Stage 4 — Graph IR and preservation chain

Project the producer, consumer junction, selected response, typed rejoin,
transformed-failure output, containment authority, and all route/evidence
identities into canonical Graph IR.

Carry and validate the facts through binding, scheduling, optimization,
preparation, and backend-neutral lowering. The optimizer may not erase,
duplicate, reorder across, or fuse a disposition boundary without an explicit
equivalence proof. Initial scheduling is serial.

The successful producer path and recovered path must converge only at the
consumer junction's typed `out` projection. A transformed failure must remain
unsuccessful and separately wired.

## Gate 4

PASS only when hostile artifact mutations at every boundary are refused and no
stage reconstructs route meaning from names, numeric coincidence, or graph
layout.

---

# 9. Stage 5 — LLVM and TinyVM expected-failure parity

Lower one existing `TextOutcome` failure through one source-declared consumer
and one selected recovery function. Prove:

- success bypasses response execution and reaches the same typed junction;
- expected failure invokes exactly the selected ordinary function;
- unselected functions do not execute;
- the envelope preserves producer, attempt, obligation, commit, and provenance;
- recovery closes the obligation and emits exactly one success value;
- no exception, hidden unwind, dynamic lookup, or implicit termination exists;
- LLVM and TinyVM produce equivalent observable results and receipts.

Add one bounded transformation test only if its successor failure already has
an accountable declared route within this mission's admitted source surface.
Otherwise retain transformation as validated non-executable semantic authority
and document that exact boundary; do not fabricate a sink.

## Gate 5

PASS only with backend parity, deterministic artifacts, positive, negative,
hostile, and sanitizer evidence.

---

# 10. Stage 6 — Explicit activation quarantine

Replace low-level termination only on the newly admitted bounded path with the
explicit activation disposition protocol from ADR 0067.

Native lowering must use explicit status/result branches. TinyVM must return
the same disposition to its interpreter loop. Neither may use exceptions,
non-local jumps, dynamic handlers, or low-level process exit.

Prove for one serial activation:

- the exact activation is halted and quarantined;
- no normal value is published;
- no local continuation executes;
- the containment receipt preserves all required identities and evidence;
- the graph executor returns structured `ContainedFault`;
- command-host `report_and_fail` renders the structured evidence and returns
  the documented non-zero status;
- an embedded test host receives the structured result without continuation
  authority;
- LLVM and TinyVM agree.

Compatibility `flow_graph_fail` may remain for non-admitted paths but must not
be reachable from or claimed by the new executable projection.

## Gate 6

PASS only when process exit is absent below the host-policy boundary and the
quarantined activation cannot publish or resume.

---

# 11. Stage 7 — Evidence, diagnostics, and closure audit

Integrate the existing structured disposition diagnostics and bounded evidence
epoch with real source spans and runtime receipts. Prove human/machine agreement
for:

- missing or ambiguous consumer association;
- wrong envelope, recovery, rejoin, or transformed-failure type;
- missing/stale/unauthorized policy;
- fault wired to ordinary recovery;
- missing containment authority;
- attempted continuation or publication after quarantine;
- evidence budget refusal without silent truncation.

Run focused tests, canonical configure/build/test gates, ASan/UBSan where
supported, Valgrind for the bounded reference/runtime path, and clean fresh
artifact reconstruction.

## Gate 7

PASS only if Gates 0 through 6 pass, all diagnostics derive from canonical
evidence, no unsuccessful wire dangles, and both backends preserve identical
meaning. Otherwise record the exact failed gate and set `BLOCKED`; do not weaken
the model.

---

# 12. Git and publication authority

When explicitly invoked, this mission authorizes:

- one dedicated `codex/` mission branch;
- stage-scoped commits containing only mission-owned paths;
- pushes of that dedicated branch after each passed gate;
- durable checkpoints and exact verification records.

It does not authorize:

- writes, merges, or direct pushes to `main` or `master`;
- pull-request creation or merging;
- force push or history rewriting;
- FlowLFS or Flowselection changes;
- repository settings, branch protection, workflow, issue, or release changes.

Preserve unrelated dirty and untracked work. Stop if the canonical baseline
changes unexpectedly or any gate would require weakening an accepted law.

---

# 13. Required final report

Return:

```text
MISSION 07: PASS / BLOCKED

Starting SHA:
Final branch SHA:
Branch:

Source disposition forms:
Consumer and junction forms:
Policy artifacts:
Graph IR projection:
LLVM expected-failure result:
TinyVM expected-failure result:
Native quarantine result:
TinyVM quarantine result:
Evidence/diagnostic result:

Canonical configure/build/tests:
Focused hostile tests:
ASan/UBSan:
Valgrind:

Exceptions introduced: NO
Dynamic handler lookup introduced: NO
Implicit termination introduced: NO
FlowLFS touched: NO
master touched: NO
main written directly: NO
force used: NO

Remaining known boundaries:
```
