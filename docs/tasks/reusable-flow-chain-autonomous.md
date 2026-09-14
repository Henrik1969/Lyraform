# Autonomous reusable Flow compiler-chain mission

> **Historical completed mission — superseded.** This task records the
> reusable-chain work completed before the Lyraform housekeeping migration. It
> is retained as evidence and is not ambient authority for future work.

## Authority

Execute this mission autonomously to completion. Normal commits and pushes to
the currently checked-out development branch are authorized at coherent green
checkpoints. The Git limits in the repository `AGENTS.md` remain binding.

Do not finish a Codex turn merely because one implementation slice is complete.
Continue to the next unfinished gate. If the outer supervisor starts a fresh
turn, recover from this task, the architecture plan, maturation ledger, run
state, Git history, and current worktree.

## Current reported checkpoint

The supplied 21 August 2026 transcript reports:

- Flowanalyst emits additive `flowcore.lowering_plan` version 1 operations.
- External operations contain source provenance, provider identity, convention,
  effect, carrier parameter types, return type, argument expression IDs, and a
  result symbol where applicable.
- Flowbind parses and structurally validates external plan operations against
  semantic binding requirements.
- Existing source-name profiles remain as transitional compatibility.
- The complete regression suite passed 52/52.
- The next intended slice is profile-free scalar external-call emission in
  Flowlower using a program with no recognized profile name.

These are reported facts, not permission to assume the checkout is healthy.
First inspect the current branch, worktree and recent history; run the relevant
focused gate and reconcile any newer work before editing.

## Final objective

A previously built Flowcore toolchain must compile new Flow programs into native
executables using only:

- the installed Flow compiler binaries;
- Flow source and imports;
- provider contracts and generated binding artifacts;
- explicit policies and provider selection;
- target/toolchain inputs such as LLVM and the native linker.

Adding an application or a supported native capability must not require:

- editing or rebuilding Flowcore's C++ compiler tools;
- adding an application/source-unit/profile name to any compiler stage;
- adding a handwritten LLVM emitter for that application;
- inferring authority from a symbol merely existing in a provider library.

The intended reusable chain is:

```text
Flow source
  -> Flowmini frontend bundle
  -> Flowanalyst semantic report and lowering plan
  -> Flowbind exact capability authorization
  -> Flowparallel execution plan
  -> Flowoptimize transformed plan
  -> Flowlower backend IR
  -> LLVM/native linker
  -> executable artifact
```

## Non-negotiable semantic boundaries

- Application behavior belongs in Flow source, not compiler fixture dispatch.
- Qualified source identity, provider identity, native symbol identity and
  contract identity remain distinct.
- `->` value placement and `=>` graph connection remain different semantics.
- Provider discovery is evidence, not authorization.
- Authorization covers the exact callable ABI contract, including provider,
  symbol, convention, parameter/result carrier types, effects and versioned
  contract/evidence identity.
- Unsupported operations fail explicitly with structured diagnostics and
  provenance. A stage must not guess from source-unit names.
- Published plans retain stable operation identity and provenance through
  downstream stages.
- Compatibility paths may remain temporarily only while covered migration tests
  exist and their removal gate is documented.

## Execution program

### Gate 1 — Verify and harden lowering-plan v1

- Inventory every remaining source-name, profile-name, substring, boolean flag
  and handwritten allowlist across Flowanalyst, Flowbind, Flowparallel,
  Flowoptimize and Flowlower.
- Verify the versioned lowering-plan schema and reject malformed, blocked,
  incompatible or unsupported plans.
- Make Flowbind authorize each external operation by exact provider and ABI
  contract facts. Bind authorization to the generated contract/evidence identity
  rather than symbol existence alone.
- Add positive, negative and adversarial contract tests.

### Gate 2 — Preserve the generic plan across the middle stages

- Make Flowparallel consume and publish the generic operation plan without
  application-profile allowlists.
- Make Flowoptimize preserve operation identity, ordering constraints,
  provenance, effects and authorization evidence.
- Reject silent operation loss, mutation without provenance, and incompatible
  plan versions.

### Gate 3 — Implement reusable lowering

- Implement generic lowering for the currently supported literals, variables,
  assignments, call arguments, result placement and return values.
- Implement generic external calls for the established integer, size, long,
  unsigned-long, string and opaque-pointer ABI carriers.
- Implement the language control flow required by the existing examples and
  `flow_less`, driven by structured AST/semantic facts.
- Validate argument count/types, result type, convention, effects and
  authorization before emitting LLVM.
- Produce structured failure reports for unsupported language or ABI features.
- Do not identify applications by filenames, source-unit names or profile names.

### Gate 4 — Prove profile independence

- Migrate the current native examples onto the generic path while keeping their
  observable behavior and gates green.
- Add one new external scalar capability and one newly named Flow program as an
  acceptance proof.
- Build the new program using already-built Flowcore binaries.
- Prove that the acceptance program and binding require zero C++ compiler source
  changes and no toolchain rebuild.
- Remove corresponding legacy profile branches only after equivalent positive,
  negative, adversarial and native-execution coverage passes.

### Gate 5 — Repair known correctness gaps

- Represent imported short-name resolution with a candidate set or permanent
  ambiguity state. Test one, two, three and more providers exporting the same
  name. Qualified calls must remain stable.
- Retain unqualified imported calls only as clearly diagnosed compatibility
  behavior, with a documented removal path.
- Make `sel` behavior source-derived rather than profile-derived.
- Repair its read buffer handling: reserve terminator space; never use a negative
  result as an offset; distinguish error, EOF and positive reads; cover the paths
  with tests.
- Model ncurses resource ownership/lifetime and cleanup sufficiently to prevent
  successful paths from concealing invalid generic pointer behavior.

### Gate 6 — Implement `flow_less` and real `=>` semantics

- Implement `flow_less` as Flow-owned pager behavior with injectable input,
  terminal and output providers.
- Preserve complete source and destination port identity through validation,
  planning, lowering and runtime delivery.
- Assign wire and signal identities and carry their provenance into diagnostics.
- Exercise `=>` as graph connection, not sequential-call syntax.
- Test ordering, fan-out, unconnected-output laws, failure propagation, input
  selection and observable results.
- Keep scheduling delivery policy separate from the receiving node/plug
  activation contract.

### Gate 7 — Final hardening and reconciliation

- Run focused tests during each slice.
- Run the complete canonical build and CTest suite at major boundaries.
- Run available sanitizer, malformed-input, native-link and native-execution
  gates.
- Reconcile README, architecture notes, checkpoint ledger, test counts and PR
  metadata descriptions that live in the repository. Do not merge the PR.
- Document remaining platform limitations honestly; do not describe a
  handwritten or fixture-specific path as generic.
- Ensure generated files are reproducible and no transient build/log artifacts
  are committed.

## Checkpoint protocol

At each coherent boundary:

1. Run the focused gate and `git diff --check`.
2. Run the complete suite when a public contract or stage boundary changes.
3. Update the maturation ledger with confirmed behavior, evidence, temporary
   compatibility, remaining work and the exact next action.
4. Inspect the diff for unrelated changes and accidental generated output.
5. Commit with a descriptive message and push the current development branch.
6. Leave `.codex-run-state` as `CONTINUE` and proceed immediately.

Do not create empty checkpoint commits. Do not commit failing states unless a
preserved diagnostic fixture is itself the intended, documented checkpoint.

## Definition of done

Set `.codex-run-state` to `DONE` only when all of the following are true:

- Every required compiler stage consumes the versioned generic plan without
  application/source-name profile selection.
- No migrated example depends on a handwritten application-specific LLVM
  emitter.
- Exact capability/ABI authorization is verified end to end.
- A newly named acceptance program using a newly selected binding compiles with
  the already-built binaries and without C++ changes or rebuilding Flowcore.
- `sel` is source-faithful and its read/error paths are safe.
- Namespace ambiguity tests cover at least one, two and three colliding
  providers.
- `flow_less` owns its application semantics in Flow source.
- The required `=>` port, wire, signal, fan-out, failure and scheduling behaviors
  are executable and covered by tests.
- The complete build, tests, sanitizer gates and native execution demonstrations
  pass.
- Documentation and recorded test counts match the verified checkout.
- Transitional profile machinery covered by the mission is removed or an
  explicitly approved semantic blocker is recorded. Ordinary remaining work is
  not a deferral.
- The final ledger records exact commands and results, the final commit is
  pushed, and the worktree is clean.

The final report must include the commit range, gates and counts, the profile-free
acceptance demonstration, native artifact evidence, remaining explicitly scoped
platform limitations, and confirmation that the branch is pushed and clean.

## Blocked state

Set `.codex-run-state` to `BLOCKED` only if all safe work is prevented. Before
doing so:

- attempt reversible alternatives;
- preserve passing work in a coherent checkpoint where possible;
- record the exact blocker and evidence in the maturation ledger;
- identify the smallest decision, credential or authority required from Henrik;
- do not disguise a large workload, failed test, ordinary design choice, or
  context limit as a blocker.


## Immediate steering override — 2026-08-22

The checkpoint through `ee61794` is accepted as a green transitional
checkpoint. Apply the following constraints before continuing the wider mission.

### Remove disguised terminal profiles

The current `interactive_terminal_plan` capability-set recognition is
transitional. Do not add further capability-set, source-name, filename,
fixture-name or literal-pattern profile selectors.

Replace the hardcoded terminal-selection emitter with ordered, structured,
source-derived operations, argument expressions, result placement, branches and
control flow.

Add adversarial proof tests demonstrating:

- two programs using the same capability set but expressing different behavior
  lower differently;
- changing source operation order changes generated behavior;
- adding an unused authorized capability does not select or alter a backend path;
- renamed source units and programs continue to work;
- positive input, EOF and input failure behavior is derived from Flow source.

A renamed `sel` program alone is not sufficient proof of source-derived
lowering.

### Keep writable storage semantics explicit

Treat interpreting `c_pointer(N)` as writable allocation as temporary
compatibility behavior. Do not generalize it into permanent language semantics.

Prepare a concise design note comparing explicit buffer/storage representations,
for example a dedicated buffer carrier, storage declaration or allocation
operation. Record the consequences and the semantic choice Henrik must make.
Continue using the compatibility representation only where required by the
current migration.

### Consume typed plans

Move Flowlower toward parsed, typed lowering-plan and binding-report structures.
Do not introduce new authorization, profile selection or semantic decisions
based on JSON substring searches.

### Preserve repository hygiene

At the next clean checkpoint:

- add appropriate ignore rules for generated `.intellijPlatform` state;
- commit the autonomous task, runner and repository agent instructions as a
  dedicated coherent checkpoint;
- never commit generated IDE indexes, locks, downloaded platform files or
  transient logs.

### Immediate priority

Complete removal of the transitional terminal capability-set emitter before
expanding the same technique elsewhere. Keep the focused and complete gates
green, update the maturation ledger honestly, commit and push the coherent
checkpoint, then continue the remaining mission autonomously.

Stop only for a genuine public-language or architectural choice that materially
changes the design. Do not treat ordinary implementation work as a blocker.

## Owner decision — source graph activation contract

Henrik approves the proposed receiver contract in
`docs/architecture/source-graph-activation-decision.md` for the bounded v0.29
language-chain surface, with the following normative interpretation.

One delivered input creates exactly one fresh activation frame and invokes the
receiving Flow function exactly once.

The activation frame owns fresh function-local state. Local variables and
temporary control-flow state MUST NOT leak between activations.

One successful function return creates one logical output activation carrying
the returned value. Runtime fan-out MAY create one delivery per connected wire,
but MUST NOT re-execute the Flow function. All fan-out deliveries MUST preserve
the originating output activation's signal identity while retaining distinct
wire and delivery identities.

A failed activation MUST NOT emit a normal result. It produces the structured
failure or diagnostic behavior already admitted by the current runtime
contract.

For the v0.29 admitted surface:

- input delivery is the activation trigger;
- the function body executes once per delivered input;
- `guard` and `when` operate inside that activation;
- the selector and other source expressions obey their existing evaluation-count
  contracts;
- successful return emits one logical result;
- fan-out belongs to runtime delivery rather than function evaluation;
- activation-local storage is fresh for every invocation.

This decision is deliberately narrow. It does not define:

- multi-input joins or readiness rules;
- persistent or shared node state;
- streams or repeated output from one activation;
- asynchronous suspension or continuation;
- reentrant or parallel scheduling;
- cancellation, retries or backpressure;
- zero-output sinks or generalized multi-result functions;
- distributed execution or durable signal identity.

Those remain outside the v0.29 mission and require separate contracts if a
concrete use case demands them.

Implement this bounded receiver contract, add positive and hostile tests, update
the decision note and recovery ledger, keep all complete gates green, and
continue the remaining mission without requesting this decision again.
