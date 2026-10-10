# Lyraform mission: disposition foundation and source authority through Gate 2

Date: 2026-10-10

Intended repository path: `docs/tasks/disposition-foundation-and-source-authority-gate2.md`

## Selection, precedence, and finish line

This document is an explicitly selected Codex mission. Read `AGENTS.md`,
`docs/development/stage-execution-protocol.md`, and
`docs/development/autonomous-next-stage-selection.md` in full. Read the accepted
ADRs 0054–0067, the 2026-10-09 disposition seam inventory and checkpoint, the
decision brief as amended on 2026-10-10, and Mission 07. The 2026-10-10 decision
and ADRs 0066–0067 supersede the earlier recommendation for external semantic
declarations. External policy may select only source-authorized routes.

This mission has two outcomes, in order:

1. Independently review and verify the existing four-commit disposition branch;
   integrate its verified tree into a **dedicated working branch** based on
   current `origin/main`, preserving the pre-existing tracked run-state file's
   content from that base. Do not publish an unverified foundation as complete.
2. Implement and prove **only** the structural and semantic source authority
   corresponding to Gates 0, 1, and 2 of Mission 07 on that working branch.
   Stop at Gate 2 with a reviewable checkpoint. Gates 3–7 of Mission 07 are not
   authorized by this mission.

Mission 07 says its full campaign begins after the foundation is integrated on
`main`. This narrower mission deliberately prepares and tests an integrated
tree on a dedicated branch while leaving `main` untouched. It does not mark
Mission 07 complete and does not amend Mission 07's later gates. After this
mission, Henrik can review the integrated Gate 2 branch and decide when to
integrate it into `main` and select the remaining campaign.

On successful completion of every gate below, write `DONE` to this mission's
**local runner state file** and report `WAITING FOR HUMAN REVIEW` as the
architectural handoff. `DONE` means only that this bounded mission passed. On
a genuine semantic block, preserve passed gates, write a decision brief, set
the local runner state to `BLOCKED`, and report the exact unanswered question.
Use `CONTINUE` between turns and passed interim gates. Never use the tracked
`.codex-run-state` as this mission's runner state or change it to unblock an
old mission. A transient CLI/network/usage failure is not a semantic BLOCKED
decision.

## Published starting facts; verify before acting

At drafting time `origin/main` was
`86a5c69d58bacb09f393cef2a3d4e798ac9fe3ad`. The prior mission branch
`mission/canonical-disposition-lanes-wire-closure` ended at
`375e36fcf545c36241f0cb046bf2d458daa571d7`, four commits ahead of that
base and zero behind. Its implementation commit is
`e077f6745e1974ec4d50c94af70297c166d85bf5`. Its checkpoint reports
184/184 canonical tests, focused Clang ASan/UBSan, and a clean Valgrind probe.
These are **recorded claims** until independently reproduced in this mission.
There were no GitHub status checks or workflow runs for the branch head.

The branch contains a declarative route bridge, diagnostic contract, reference
evidence epoch, inventory, checkpoint, and accepted ADRs 0066–0067. Its
`.codex-run-state` is `BLOCKED` for the old mission; the base `main` version is
`CONTINUE`. Neither word is source semantics. The new branch must not inherit
the old `BLOCKED` file as its current tree merely through the merge.

`Lyraform/compiler/examples/text/text_outcome.flow` is a program with local
`outcome.code` branches. Flowanalyst derives an operation, tagged owner,
`Failure<TextFailure>` accounting, and `atomic_tagged_result`, but this source
does not yet declare an ordinary failure-envelope response function, consumer
junction, source-level failure wire, or policy choice. It is a candidate source
producer, not evidence that function-level failure routing already works.

If any branch or SHA has changed, inspect the ancestry and affected files.
Reconcile harmless, unambiguous changes with independent verification. Stop
with a precise report if a changed baseline alters the accepted meaning, the
integration tree, or the gate evidence. Never reset, discard, rebase published
history, or overwrite another worker's work to force these starting facts.

## Authority and protected scope

This selected mission authorizes reading, implementation, focused and complete
tests, documentation, stage-owned commits, and **ordinary pushes of one
dedicated `codex/` working branch only**. Create that branch from the verified
current `origin/main` in the checkout where this runner observes Git progress.
Integrate the prior mission branch into it with ancestry recorded. A merge
commit or another non-rewriting integration method is an engineering choice;
preserve the four published commits' provenance and the accepted ADR text.

Before integration, inspect the exact diff and commit sequence. The final
integrated tree must contain the bridge, inventory, decision, ADRs, and
Mission 07, and must retain the base branch's tracked `.codex-run-state`
content byte-for-byte. Resolve that **one known run-state difference** as
administrative state, with an explicit diff and test; do not casually resolve
other conflicts. Do not remove the historical `BLOCKED` record in the old
checkpoint or pretend the original mission reached DONE. Stage the selected
mission document as a task-owned file on the working branch.

Do not commit or push to `main` or `master`; merge a pull request; create a
pull request; force-push; delete branches or tags; change workflows, settings,
or protection rules; touch FlowLFS, Flowselection, the historical runtime, or
unrelated dirty/untracked files. Do not make the working branch appear to be
`main` in current-status documentation. Do not run two editing agents against
the same checkout. If safe work on a dedicated branch is impossible because
another process owns the worktree or unrelated work would be disturbed,
preserve it and report the concrete obstruction.

The runner's default local state is `git rev-parse --git-path codex/run-state`.
Use a fresh mission-specific `--state-file` under local Git state. It is not a
tracked repository file. The selected mission is the only task authority;
do not continue a previously blocked mission by flipping its state.

## Fixed meaning; no redesign during this mission

- ADR 0066: source owns closed producer dispositions, exact ordinary response
  functions, read-only typed failure-envelope input, declared `recover` or
  `transform` result meaning, closed consumer membership, explicit graph
  association, and typed rejoin. Versioned policy later chooses only among
  legal source routes. No external declaration may invent these facts.
- ADR 0067: a fault goes to exact activation-scope containment, producing an
  explicit quarantined activation result and receipt. Host policy later owns
  command status. No low-level exit is admitted for the future executable path.
- Refusal before execution creates no runtime attempt. An admitted attempt's
  `Success<T>`, `Failure<E>`, and `Fault<F>` are exclusive. A failure cannot be
  logged away; a fault cannot enter ordinary recovery. All possible
  unsuccessful paths require accountable typed destinations.
- Parser owns syntax and source spans; semantic analysis owns identity, types,
  legality, closed sets, same-attempt pairing, and route completeness; shared
  contracts validate; later stages preserve; policy selects; runtime executes.
- Graph wire IDs, plan wire IDs, operation, disposition, obligation, signal,
  delivery, activation, attempt, function, policy, and source provenance are
  separate concepts. Equal numbers or strings do not establish a mapping.
- A source failure lane constrains scheduling; this mission establishes
  legality only and admits no scheduler or executable disposition extension.

If a detail is genuinely missing from ADRs 0066–0067, choose routine internal
representations autonomously when they preserve the fixed meaning. If two
plausible choices change public syntax or semantic meaning, write a compact
decision brief and stop before dependent implementation. Do not turn an
implementation convenience into an additional accepted language decision.

## Gate F — independent foundation review and branch integration

**Question:** Does the prior branch add a sound, explicitly declarative seam
without widening canonical execution or weakening current refusal?

1. Record current branch, local/remote SHAs, merge base, `git status`, tracked
   and untracked paths, any active worker, and the selected mission path.
   Verify the prior branch's four commits and its diff against the current
   base. Record how the run-state file is resolved.
2. Review the bridge against independently parsed graph/plan inputs. Check
   exact endpoint direction, lane, type, producer/disposition/route/consumer,
   policy selection, scope, provenance, graph-to-plan wire mapping, duplicate
   and dangling refusals, and rejection of executable/`ready` claims. A
   hand-authored fixture is not a source producer.
3. Review diagnostic machine/human consistency, unverified artifact locations,
   evidence retention/closure, bounds, observer retention, and forged receipt
   refusal. Check that no existing compiler/backend path accidentally treats
   this bundle as executable.
4. Reproduce a clean configure/build/test from the integrated tree and focused
   bridge, CLI, sanitizer and Valgrind probes where supported. Record exact
   versions/counts and environmental limits; the recorded 184/184 is a
   baseline to check, not a count to fabricate. Run `git diff --check`.
5. If an actual defect is found, repair the smallest root cause on the
   dedicated branch, extend a discriminating regression, repeat affected and
   complete gates, and record the deviation. Do not widen syntax or runtime.
6. Correct diagnostic repair suggestions that point to **currently
   unimplemented** propagation or policy sinks. Repair advice must be
   version-aware and describe an available admitted repair or explicitly say
   that the form is unsupported. Do not add those features to justify a hint.

**Gate F PASS:** the integrated branch preserves all four commits' provenance,
the final tree retains base `.codex-run-state`, independent tests pass, hostile
mutations are meaningful, and the bridge is still declarative. Commit and
push a coherent checkpoint on the dedicated branch, then continue to Gate 0.
If an actual contract defect cannot be repaired without new meaning, report
the failing mutation and stop `BLOCKED`.

## Gate 0 — establish the source boundary

**Question:** Which canonical source fact can make one `TextOutcome` operation
an explicit function-level `Failure<TextFailure>` producer without silently
reinterpreting its existing local tagged branch?

Produce a machine-readable owner/identity map from source/imported span through
AST, symbol, operation, disposition, tagged owner and obligation to the
future producer function, consumer instance and typed endpoints. Separately
name the reference plan IDs that presently come from caller-supplied fixtures.
Classify each fact as existing source authority, accepted but unimplemented
ADR meaning, declarative consistency, or executable behavior.

Show an admitted **new source specimen** using ADR 0066's declaration forms
and existing `TextOutcome` production. Preserve the old example's local
meaning. Identify exactly where function-level unsuccessful completion is
declared or derived, where the response function obtains its immutable
failure-envelope input, and how the producer's `atomic_tagged_result`/owned
payload law prevents normal publication on failure. Do not implement a new
failure mechanism merely to make the specimen easy. Do not interpret
`outcome.code != 0` as an implicit function failure or turn its local `puts`
branch into a response consumer by name.

The full source spelling examples in ADR 0066/0067 and Mission 07 are fixed
for this mission. If exact punctuation is impossible in the current lexer,
record the lexer conflict and one mechanically equivalent punctuation choice
**before** parser edits, preserving every keyword, role, and association.
Do not create multiple spellings. Stop for a decision if the adjustment would
change meaning or public compatibility.

**Gate 0 PASS:** a reviewer can trace an actual existing `TextOutcome` operation
to the intended explicit source associations, and can see precisely which
steps are not yet executable. Record the inventory, source specimen and gate
checkpoint. Continue to Gate 1 without awaiting routine review.

## Gate 1 — structural syntax only

**Question:** Can the accepted declarations and typed endpoints be captured
completely with original source spans and stable structural identities?

Implement only the bounded canonical lexer/parser/AST/frontend forms listed
by Mission 07 Gate 1:

- closed `fails { ... }` and `faults { ... }` function clauses;
- `failure E` parameter type and `recover T`/`transform E2` result forms;
- closed `consumer` declarations containing ordinary function references;
- explicit producer-function, consumer-instance, and activation-containment
  node declarations;
- explicit `success`, `failure`, and `fault` graph endpoint spellings and wires.

Preserve imported origin, line/column, statement/function scope, all tokens
needed to reconstruct the association, and stable structural node/edge IDs.
Parser recognition grants **no semantic admission, policy selection, backend
execution, or runtime route**. Existing programs and existing recognized
`in`/`out` graphs retain their prior meaning. Do not let new spelling fall
back to the deprecated parser or legacy runtime.

Positive tests: deterministic structural round-trip and imported declaration
spans for one producer, one response, one consumer and junction, and one fault
containment association. Negative tests: incomplete or duplicate sets,
duplicate response members, malformed envelope/result forms, wrong declaration
position, malformed endpoints, and ambiguous or missing wire syntax. The
parser refuses malformed forms before semantic analysis.

**Gate 1 PASS:** the structure is complete and deterministic, with source
spans preserved, no syntax alias or executable claim, and the canonical suite
green. Commit and push the stage-owned checkpoint; continue to Gate 2.

## Gate 2 — one semantic authority and early refusal

**Question:** Does one canonical semantic producer resolve a complete, closed,
typed and accountable disposition topology for the bounded source specimen?

Resolve exact semantic identities for producer function and operation,
`TextFailure` and success type, declared failure/fault sets, response functions,
immutable envelope projection, `recover` or `transform` transition and result
type, closed consumer membership, consumer instance, same-attempt mutually
exclusive success/failure inputs, typed rejoin, and activation containment
scope. An existing tagged outcome may supply the producer set **only when**
the call and declared function boundary establish the same facts and commit
law; an explicit clause and derived carrier facts must agree exactly.

Connect the semantic topology to the independently validated declarative
bridge by an explicit identity mapping. Keep the route non-executable and
unselected where policy is required. Do not let a supplied JSON plan become a
second source of semantics. If the existing source graph serializer cannot
represent these facts without altering its executable v1 meaning, add the
smallest separate versioned **declarative** projection and refuse `ready`.
There must be one owner of legality; consumer validators verify exact facts
and may not independently reconstruct them from text names or graph layout.

Reject at the earliest authoritative phase: unknown or duplicate types and
functions; contradictory declared/derived sets; a response with a mutable or
incorrect envelope; wrong recovery/rejoin type; faults entering an ordinary
consumer; mixed producer attempts at a junction; co-emission of success and
failure; a missing, extra, duplicate, ambiguous or dangling unsuccessful
route; an unaccounted transformed successor; foreign module/revision facts;
and unsupported recursive or cycle composition. No execution attempt may be
created on refusal. A normal optional output follows its established contract.

For this gate, source-derived diagnostics must use original import-aware
locations and stable machine codes; human rendering comes from the same fact.
Artifact-only locations remain visibly unverified. Make repair hints match the
admitted language subset. Apply deterministic size bounds without truncating
live evidence. Mutation tests at the actual semantic artifact consumer must
reject forged IDs, types, endpoints, scope, origin, set membership and false
`ready`/execution status.

New semantically valid forms must remain **execution unsupported** at the
first later boundary that lacks policy/Graph IR/backend support; do not
redirect to the old local `TextOutcome` branch, a C++ callback, `flow_graph_fail`,
LLVM, TinyVM, or the legacy runtime. Keep current admitted source behavior
working and the canonical test graph green.

**Gate 2 PASS:** one source specimen derives the complete closed semantic
topology and declarative bridge from actual source facts; invalid specimens
are refused early with verified locations; hostile artifacts fail validation;
later execution refuses explicitly; no policy selection or runtime semantics
have been invented. Commit and push the checkpoint. Set this mission's local
state to DONE and return `WAITING FOR HUMAN REVIEW`.

If the accepted ADRs still leave a material function-level `TextOutcome`
escape/ownership question unanswered, stop before inventing an answer. Keep
the verified foundation and any independent structural work, write a decision
brief that names the exact missing law and alternatives, set the local state
to BLOCKED, and do not claim Gate 2 PASS.

## Verification, claims, and stopping rules

At each coherent stage run focused positive, negative and format-correct
hostile tests. At Gate F and after Gates 1 and 2 run `./igor doctor`,
`./igor build`, `./igor test`, `git diff --check`, and applicable focused
Clang ASan/UBSan and Valgrind probes. Record commands, compiler versions,
exact counts, environmental limitations, branch and commit SHA. A test that
only calls the reference C++ API cannot prove source admission or execution.
Do not lower a gate's criteria merely because the current compiler is narrow.

Stop at once and preserve the branch if a proposed step would require a new
public meaning outside ADRs 0066–0067, a conflicting module/import identity
model, a general policy envelope, a changed disposition/ownership law, or
destructive treatment of unrelated work. Produce the standing protocol's
decision brief. Ordinary parser implementation, representation choice,
validator hardening and regression repair are engineering work: continue
autonomously within these bounds.

No policy artifact implementation, executable Graph IR, scheduler change,
LLVM/TinyVM failure dispatch, fault-result runtime conversion, host exit
policy, general diagnostics rollout, cancellation, retry, propagation, policy
sink, exception unwinding, dynamic handler lookup, source-name profile,
parallel fallible effects, distributed behavior, or FlowLFS work. Do not claim
canonical error-line/wire **execution closure** at this stop point.

The final checkpoint must show a factual trace and maturity table:

`source/import span -> producer operation/disposition/owner/obligation ->
typed endpoint and wire -> consumer/function/transition -> declarative bridge`.

For each link mark `source-derived`, `validated declarative`, `reference only`,
`unsupported for execution`, or `not implemented`. Record original and final
SHAs, changed files, test evidence, every preserved unresolved question,
and untouched protected paths. The final report must explicitly state:

```text
MISSION: FOUNDATION + SOURCE GATE 2
Gate F: PASS / BLOCKED
Gate 0: PASS / BLOCKED
Gate 1: PASS / BLOCKED
Gate 2: PASS / BLOCKED
Runner state: DONE / BLOCKED
Handoff: WAITING FOR HUMAN REVIEW / DECISION REQUIRED
Foundation base and integrated branch SHA:
Canonical and focused test counts:
Source form and first producer fact:
Diagnostic origin verified at source: YES / NO
Source-derived route executable: NO
LLVM/TinyVM new failure routing implemented: NO
ADR 0067 runtime quarantine implemented: NO
Main/master/FlowLFS/Flowselection touched: NO
Remaining Mission 07 gates: 3–7
```
