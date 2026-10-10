# Canonical disposition lanes and wire closure

Date: 2026-10-09.

## Invocation and authority

This is an explicitly selected Lyraform mission for `docs/tasks/canonical-disposition-lanes-and-wire-closure.md`. Read `AGENTS.md`, the [Stage Execution Protocol](../development/stage-execution-protocol.md), and [Autonomous Next-Stage Selection](../development/autonomous-next-stage-selection.md) completely. Follow the accepted ADRs, especially 0054–0065, and the dated checkpoints through 2026-10-08. Historical Flowmini behavior and the experimental Flow Policy Envelope pattern are evidence, not semantic authority.

This mission authorizes implementation, tests, contracts, documentation, checkpoint reports, and coherent commits **on a dedicated mission branch only**. If no dedicated branch is active, create one from the verified current `main` head without overwriting a dirty worktree. Push that branch after a gate passes and record its exact SHA. Do not merge, force-push, write to `main` or `master`, create a pull request, change settings or workflows, or modify FlowLFS or Flowselection. Preserve unrelated user files, including any untracked work. A repository state file is local execution state, not a source of semantic authority.

The runner must be invoked with this mission selected by `--task`; this document does not make an old task ambient. Each new Codex turn resumes from the latest checkpoint and the active gate. A single passing test or reference executor is not mission completion.

## Starting point

At drafting time, published `main` is `86a5c69d58bacb09f393cef2a3d4e798ac9fe3ad`. The most recent recorded canonical suite is 182/182, with focused Clang sanitizer and Valgrind probes; GitHub has no independent CI evidence. Recheck branch, HEAD, upstream, worktree, test graph, and relevant files at entry. If the repository has moved, reconcile the delta before editing; do not reset or overwrite newer work.

The current value graph has stable node IDs, typed `in`/`out` ports, string wire IDs, signal IDs, distinct fan-out delivery IDs, fresh receiver activations, bounded FIFO execution, and admitted independent waves. The failure and fault reference plans have typed `failure`/`fault` ports, numeric wire/route/obligation identities, exact function or containment authority, and validated envelopes and receipts. They are separate contract families. General failure routing is not emitted by Flowanalyst or executed from the canonical source graph through LLVM and TinyVM. The graph runtime's existing terminal failure path still emits a diagnostic and exits; it is not the new disposition route.

## Mission objective

Close one **bounded, canonical** gap between source-derived disposition authority, typed graph lanes, artifact validation, and backend execution. The first executable slice is one admitted `Failure<E>` produced by an operation, delivered through one explicit typed route to one already authorized ordinary response function with a declared `recover` transition, ending in one `Success<T>` and a verified obligation closure receipt. A companion `Fault<F>` route must project to the already accepted activation-scope `halt_and_quarantine` authority where it can be supported without inventing a new top-level policy; otherwise prove exact fail-closed refusal and provide a decision brief for the missing boundary.

The mission must improve error-line diagnostics for all three classes:

1. **Refusal before execution:** a syntax, semantic or admission error creates no runtime attempt.
2. **Expected runtime failure:** an admitted attempt produces `Failure<E>`, carries one live obligation through its typed lane, and closes it only under a validated recovery or transformation rule.
3. **Integrity fault:** `Fault<F>` has a distinct obligation and an exact containment route; it cannot become ordinary recovery, disappear into logging, publish a normal result, or continue within its quarantined scope.

The required result is one canonical producer of each semantic fact, independent validation at every consumer, executable LLVM/TinyVM parity for the selected expected-failure route, and precise evidence of everything still refused. New public source spelling is **outside this mission**. Do not claim general language support from an internal fixture or reference API.

## Governing invariants

- One admitted attempt completes exactly once as `Success<T> | Failure<E> | Fault<F>`; an admission refusal is outside execution.
- Every *possible* unsuccessful disposition has one statically inspectable accountable route at each admitted boundary. A selected run takes exactly one of those routes. Logging, tracing and diagnostics observe; they do not consume.
- The value graph and disposition route plan may have different serialized forms, but a route can be executable only when its graph, operation, producer disposition, endpoint, type, obligation, destination, scope, provenance, and policy selection agree with canonical authority.
- Graph IDs, operation IDs, wire IDs, signal IDs, delivery IDs, activation IDs, attempt IDs, disposition IDs, obligation IDs, and policy IDs are distinct concepts. Never equate them by coincidental numeric or string values. Define an explicit stable mapping where the same fact is projected into two artifacts.
- One signal fanning out creates distinct deliveries. A failure from one receiver activation must retain the originating delivery and signal correlation while owning its own disposition and obligation identity. It must not broadcast or duplicate a unique obligation merely because the value path fans out.
- Semantic analysis owns legality, types, closed failure sets and route completeness; shared contracts validate; later stages preserve; policy chooses only among authorized routes; runtimes execute. A backend may refuse an unsupported admitted semantic operation, but cannot repair a missing wire or choose a fallback meaning.
- No normal output may publish from a failed attempt before its declared commit law permits it. Preserve original commit/no-commit evidence through recovery, transformation and containment.
- Existing value-flow order and effect/resource conflicts remain authoritative. A disposition edge constrains scheduling and joins; policy or a provider cannot erase it. The first failure lane executes serially.
- Refusal and runtime diagnostics derive from the same canonical facts, with bounded, versioned machine fields and a human explanation. Diagnostic events are observations, never a failure destination.
- Live evidence is retained while an obligation depends on it. No implicit truncation, invented success, generic exception unwinding, dynamic nearest handler, source-name profile, or implicit termination is permitted.

## Stage 0 — inventory the seam

**Question:** Which exact facts in the admitted source/semantic/graph path can produce a runtime expected failure today, and which current failure plans are merely reference contracts?

Produce a compact machine-readable map of the current producer, route, selection, graph, artifact, backend and runtime paths. Name the earliest authoritative producer of every identity. Classify each possible error as refusal, expected failure, integrity fault, terminal legacy behavior, or undefined. Record the first source-derived operation whose existing semantics and provider/guard contract can support the selected `Failure<E>` without new public syntax. If none exists, continue with independent bridge and diagnostic work in Stages 1–2, then issue a decision brief before pretending the end-to-end gate is possible.

**Gate 0:** a reviewer can tell exactly why the current reference executor is not yet canonical source-graph execution. Do not count a hand-authored plan as a source-derived producer.

## Stage 1 — typed route projection and wire closure

**Question:** How does one semantic unsuccessful route project into both the source graph and an existing failure/fault plan without creating a second semantic authority?

Add the smallest versioned shared bridge contract, provisionally `lyraform.disposition_route_projection/v1`. Its exact filename and internal API are engineering choices. It should bind:

- graph identity and source provenance;
- producing operation and disposition IDs;
- lane (`failure` or `fault`) and exact payload type;
- stable route identity and explicit typed producer/destination endpoints;
- existing wire ID in each serialized representation where applicable;
- consumer/response function or containment authority and scope;
- obligation-transfer/closure law and commit/no-commit contract;
- selected policy identity/revision and the authorized route set;
- activation, signal and delivery correlation rules.

Keep `SourceGraphWire` and the current failure/fault wire types readable for compatibility. Validate a bridge against both independently parsed sides; do not bless a plan because it claims `ready`. Reject absent, duplicate, ambiguous, wrong-lane, wrong-type, wrong-port, wrong-scope, reordered, renamed, dangling, foreign or forged routes. A normal optional output may remain unconnected under its own contract; unsuccessful dispositions cannot.

**Gate 1:** one positive expected-failure bridge and one positive activation-scope fault bridge round-trip deterministically; every mutation above is rejected by all relevant consuming validators. No runtime claim yet.

## Stage 2 — source-derived disposition and clear error lines

**Question:** Can an existing admitted source operation establish the producer's complete exclusive disposition set and precise route requirements before scheduling?

Flowanalyst/shared semantic authority must derive the producer, types, operation identity, possible outcomes, commit law and source span once. Connect the bridge only for a bounded current source form whose semantics are already settled; refuse other forms early. If no form provides enough information, keep Stage 1 declarative and produce a focused decision brief on the *missing source/producer fact*, comparing existing syntax or external declarations; do not sneak a new spelling into the parser.

Implement a bounded structured diagnostic projection for: missing failure route, wrong destination type, ambiguous route, attempted fault-to-recovery, unsupported source form and forged artifact. Every diagnostic must include stable code, phase/classification, primary source location, producer operation and disposition/type, route or missing endpoint, enclosing boundary, related cause, and legal repair classes where known. Human and machine renderings must come from the same fact. Never suggest logging/dropping as a repair. Redaction must be explicit when details are sensitive.

**Gate 2:** each invalid program/plan is refused at the earliest authoritative phase with no execution attempt; line and column refer to original source through imports; machine and human views agree on identity and cause. One bounded admitted form has complete source-derived routing facts or the gate is reported BLOCKED with a decision brief.

## Stage 3 — preserve and execute one serial expected-failure lane

**Question:** Does the source-derived route survive the complete compiler chain and execute identically on LLVM and TinyVM?

Carry the exact bridge and existing envelope/consumer/selection/transition facts through binding, planning, optimization and prepared backend artifacts. Each consumer independently validates its inputs and relationships. The backend must dispatch by resolved response-function identity through existing call lowering; no name/capability-set profile or separate ABI guess. Execute one deterministic input that succeeds and one that produces the selected `Failure<E>`; the failed attempt takes the typed route exactly once, invokes the ordinary response exactly once, emits a declared `Success<T>`, and closes the original obligation with one validated receipt. Preserve the original historical failure, source origin, signal/delivery/activation/attempt correlation and commit state.

Mutate each stage artifact at the format it actually consumes. Include forged success after failure, missing or extra route, wrong consumer/function, duplicate dispatch, lost obligation, wrong signal/delivery correlation, changed commit fact, altered policy selection, and optimizer removing the failure edge. The two backends must reject the same violations before side effects; normalize nondeterministic host details for parity comparison.

**Gate 3:** one source-derived program, two outcome paths and one recovery route traverse the actual canonical chain on both backends. A reference executor alone cannot pass this gate. Record exact execution, receipt, refusal and preservation evidence.

## Stage 4 — fault projection and no-continuation proof

**Question:** Can the same graph identity bridge project one declared `Fault<F>` to the accepted activation-scope halt-and-quarantine contract without weakening its boundary?

Use ADR 0065 and the Gate 5/6 reference contracts. Prove one exact typed fault edge, selected authority, distinct obligation when a response attempt faults, `unresolved_contained` state for any original expected failure, suppressed normal publication, no local continuation or rejoin, and a validated containment receipt. If the current native runtime cannot represent containment as an observable activation state without changing process-level termination meaning, retain an explicit refusal at that boundary and provide a decision brief. Do not silently replace `flow_graph_fail`'s process exit with a generic recovery path or claim whole-graph/process quarantine.

**Gate 4:** either an end-to-end admitted activation-scope fault path with matching LLVM/TinyVM and hostile evidence, or an honest boundary stating which accepted fact is not yet representable. Expected-failure Gate 3 can pass independently.

## Stage 5 — evidence lifetime and closure audit

**Question:** What evidence is required to prove a single route closed, and when can details be released without losing proof?

For the selected serial route, implement only the bounded evidence epoch needed to retain origin, transition, live obligation and policy evidence until the response receipt is validated. Emit an independently validatable closure receipt; then release details only if no live obligation or required observer depends on them. Make the byte/count/depth bound deterministic for this slice and refuse over-budget admission or execution according to the already accepted ADR 0063 classes. Do not invent public source names for those classes or claim general recursive chain compaction.

**Gate 5:** positive closure, open-obligation retention, forged receipt, early erasure, truncated provenance and limit-hit cases demonstrate that no failure becomes success or disappears under budget pressure.

## Final gate and truthful status

Mission `DONE` requires:

1. Gate 0–3 and Gate 5 PASS with exact canonical source-to-LLVM/TinyVM parity for one expected-failure route.
2. Gate 4 PASS if activation-scope containment is representable within existing accepted meaning; otherwise mark the mission `BLOCKED`, retain all completed gates, and publish the precise decision brief. Never claim universal closure from a partial fault gate.
3. Positive, negative and format-correct hostile tests at every artifact consumer, with no fixture/source-name selectors.
4. `./igor doctor`, `./igor build`, `./igor test`, applicable focused Clang ASan/UBSan and Valgrind probes, `git diff --check`, exact counts and environmental limits.
5. Current-status, authority inventory and documentation updated to distinguish declarative facts, reference execution, integrated source/backend execution, and refused features.
6. A durable checkpoint per stage with base SHA, changed files, gate evidence, remaining boundaries, untouched scope, and a final trace from source location and operation through graph endpoint, wire, signal/delivery, failure obligation, response, policy, receipt and diagnostic.
7. Clean, coherent, pushed commits on the dedicated branch only; no unrelated user work staged.

If no existing source form can establish a complete failure disposition, or if fault containment requires a new top-level termination/restart policy, finish all independent stages, write the exact alternatives and recommended choice, then set the local runner state to `BLOCKED`. Do not mark `DONE` or keep iterating on speculative meaning. A tool outage or usage limit is a transient runner failure, not a semantic decision.

## Explicit non-goals

No new public failure/consumer/port syntax, runtime guard overwatch, open failure sets, arbitrary propagation, retry, async, cancellation, queue/backpressure, parallel fallible effects, shared owned values, distributed execution, durable quarantine, global `PolicyDecision v1` rollout, general `DiagnosticEvent v1` rollout, or self-hosting expansion. The mission may document separate proposals for those topics, but it cannot implement them as a side effect.

The historical mutable Flow Policy Envelope stays experimental. This mission preserves exact existing policy selection and its provenance; choosing a universal immutable policy envelope is a separate architecture decision.
