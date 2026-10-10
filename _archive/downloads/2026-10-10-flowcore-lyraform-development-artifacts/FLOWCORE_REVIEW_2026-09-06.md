# Flowcore engineering review — 2026-09-06

## Verdict

Flowcore has made **substantial new progress** since the 2026-09-04 review, but
the project should pause its autonomous continuation at the current published
head and restore a bounded mission before adding more work.

The previously reviewed branch,
[`v25-symboltable-projection`](https://github.com/Henrik1969/Flowcore/tree/v25-symboltable-projection),
is still at `7182fbf2`. A new branch,
[`flowlfs-v0.1-alive`](https://github.com/Henrik1969/Flowcore/tree/flowlfs-v0.1-alive),
is **96 commits ahead** at
[`af769d40`](https://github.com/Henrik1969/Flowcore/commit/af769d403a2daf41c2f279a0bb40b7d06c09fa51),
dated 2026-09-06. It contains two largely independent bodies of work:

1. a reproducible, bootable FlowLFS v0.1 system and desktop/tooling foundation;
2. a Flowmini/Flowcore v0.29 language-maturation slice covering UTF-8 byte/text
   boundaries, constants, `guard`, `when`, enums, tagged variants, generic match
   facts, and LLVM/TinyVM parity for integer and enum matching.

The work is productive and several designs are strong. The immediate problem is
that the autonomous governance contract no longer describes the work being done:

- `AGENTS.md` still names the completed TinyVM bootstrap task as the active
  mission;
- `.codex-run-state` is now `CONTINUE`;
- a broad open-work inventory has effectively replaced a bounded definition of
  done;
- the branch mixes a distribution experiment with compiler/language evolution;
- the project version was bumped to `0.29.0`, although the latest checkpoint
  explicitly records the full canonical suite as the next required boundary;
- GitHub still provides no independent Actions or commit-status evidence.

This is not a reason to discard the work. It is a reason to **freeze the current
head as an evidence boundary, run the complete gates, split or clearly separate
the two change streams, and approve a new finite mission**.

The earlier recommendation—Canonical Graph IR and a resolved policy decision
contract—has not been implemented. The Flow Policy Envelope ambiguity,
diagnostic-envelope design, and scheduler contract remain open. The new language
work supplies useful prerequisites but must not silently become an unlimited
substitute mission.

GitHub was treated as read-only during this review.

## Evidence boundary and published delta

| Item | 2026-09-04 review | Current review | Assessment |
| --- | --- | --- | --- |
| Established v25 head | `7182fbf2` | `7182fbf2` | Unchanged |
| New development head | None observed | `af769d40` on `flowlfs-v0.1-alive` | 96 commits ahead of v25 |
| Project version | v0.28 state | [`VERSION` = `0.29.0`](https://github.com/Henrik1969/Flowcore/blob/flowlfs-v0.1-alive/VERSION) | Development identifier; not yet a verified release |
| Autonomous state | `DONE` | `CONTINUE` | Inconsistent with the declared completed mission |
| Recorded full suite | 75/75 on the earlier TinyVM closure | No later full-suite result recorded after the final v0.29 commits | Complete verification is required |
| GitHub CI/status | None | None | Repository claims are not independently reproduced |
| Pull request | Draft PR #4 from v25 | [PR #4](https://github.com/Henrik1969/Flowcore/pull/4) remains unchanged | Does not review or explain the new branch |
| Open status issue | Issue #5 | [Issue #5](https://github.com/Henrik1969/Flowcore/issues/5) remains unchanged | Covers only part of current documentation drift |

The branch comparison reports 96 commits and at least 300 changed files (the
compare response reaches GitHub's file-list cap). A recursive tree comparison
shows approximately **612 added blobs** and **29 modified files**, with roughly
**196 MB of newly tracked blob content**. Most of the byte volume is third-party
source archives, generated/reference HTML, PDFs, fonts, and other FlowLFS
materials—not handwritten Flowcore source.

This is important: line-count growth makes the branch look like an enormous
compiler rewrite, but most of the volume belongs to the FlowLFS evidence and
offline-source substrate. It is nevertheless permanent Git history unless the
storage policy changes before integration.

## Progress since the previous review

### 1. FlowLFS v0.1 is alive and evidence-oriented

The first part of the branch builds an LFS r13.0-201 systemd baseline, records
the source and mutation basis, isolates construction in a builder VM, produces a
verified bootable image, separates immutable control from a writable twin, and
then adds an explicitly layered terminal/desktop foundation.

Confirmed from the branch documentation and commits:

- exact upstream LFS basis and corrected source inputs;
- reproducible chapter 4–8 construction and a verified boot path;
- isolation of the builder environment;
- durable mutation and evidence records;
- public-key SSH callback rather than an ambient password path;
- zsh and modern CLI package construction from captured sources;
- separation of selection policy from `fzf` or native provider mechanics;
- package projection and an offline Cargo source factory;
- a minimal Wayland/Weston/Foot desktop foundation with explicit layer
  boundaries.

The
[`FlowLFS README`](https://github.com/Henrik1969/Flowcore/blob/flowlfs-v0.1-alive/subprojects/FlowLFS/README.md)
also makes useful nonclaims: this is not yet the Flowcore runtime, FrankenPOP, a
general distribution, or a frozen public API. That restraint is architecturally
healthy.

The accompanying
[`Flowcore reconciliation`](https://github.com/Henrik1969/Flowcore/blob/flowlfs-v0.1-alive/subprojects/FlowLFS/docs/FLOWCORE-RECONCILIATION.md)
correctly treats discovery as evidence rather than authority and keeps target
policy, provider mechanisms, and source operations separate. This aligns with
the compiler-chain decisions already ratified in v0.28.

### 2. Byte, text, outcome, and diagnostic foundations

The
[`byte-text-outcome foundation`](https://github.com/Henrik1969/Flowcore/blob/flowlfs-v0.1-alive/docs/language/byte-text-outcome-foundation-v1.md)
separates raw bytes, decoded scalar values, spans, diagnostics, and
`Outcome<T>`. It avoids assuming that hosted stdin, allocation, or UTF-8 input
is universally available, so the same semantics can later admit hosted and
bare-metal providers without confusing mechanism with language meaning.

The branch adds source-reader boundary artifacts, valid and malformed UTF-8
cases, replayable diagnostics, a hosted byte provider, and C++ parity checks.
This is good systems-language groundwork.

Important limit: these diagnostics and outcomes are language/runtime values and
focused test artifacts. They are **not yet** the versioned graph diagnostic or
signal envelope proposed in earlier reviews.

### 3. Constants and explicit control flow

The branch implements immutable semantic constants and a `guard` construct,
then introduces `when` as an ordered, single-selector control-flow form.

The
[`when/match contract`](https://github.com/Henrik1969/Flowcore/blob/flowlfs-v0.1-alive/docs/language/when-match-v1.md)
confirms:

- the selector is evaluated once;
- cases retain source order;
- integer literals and inclusive ranges are admitted;
- duplicate, overlapping, and descending ranges are rejected;
- a default is required for open integer domains;
- closed enums and variants may instead prove exhaustiveness;
- lowering uses explicit branch and join structure.

These are coherent decisions. The join here is a **compiler control-flow join**,
not a decision about runtime graph fan-in, multi-signal joins, buffering, or
scheduling. Those runtime questions remain open.

One documentation defect should be repaired: the older
[`constants-and-control-flow-v1.md`](https://github.com/Henrik1969/Flowcore/blob/flowlfs-v0.1-alive/docs/language/constants-and-control-flow-v1.md)
still says that `when`/`match` design is deferred, although the branch now
implements and documents it.

### 4. Nominal enums and tagged variants

The
[`enum and tagged-variant design`](https://github.com/Henrik1969/Flowcore/blob/flowlfs-v0.1-alive/docs/language/enums-and-tagged-variants-v1.md)
makes enum identity nominal rather than treating an enum as an integer alias.
Tagged variants preserve member identity, isolate member payloads, support
construction, and require exhaustive handling unless a default is present.

The frontend/runtime slice includes negative tests for cross-enum confusion,
non-exhaustiveness, duplicate cases, and payload isolation. These are exactly
the adversarial cases needed to demonstrate that the type boundary is real.

However, the
[`when lowering plan`](https://github.com/Henrik1969/Flowcore/blob/flowlfs-v0.1-alive/docs/architecture/flow-when-lowering-plan.md)
is explicit that generic tagged-variant payload layout and field bindings are
not yet carried through every backend. Integer and enum match parity have been
demonstrated in focused gates; full variant payload parity remains open.

### 5. Backend-neutral match facts

Match operations are promoted through Flowanalyst, Flowparallel, Flowoptimize,
and Flowlower rather than reconstructed from filenames or application profiles.
LLVM and TinyVM consume the captured ordered match facts for integer and enum
cases. This extends the successful v0.28 principle: a backend consumes governed
semantic artifacts; it does not rediscover source intent.

The focused parity tests are valuable, but they do not replace the complete
root suite, sanitizer gate, and clean-checkout build.

## Architecture status by requested concept

| Concept | Confirmed current state | New delta | Still proposal, inference, or unresolved |
| --- | --- | --- | --- |
| Nodes | Narrow Flowmini runtime nodes retain named kinds, typed inputs/outputs, effects, and terminal status. | `guard`, `when`, enum/variant construction, and match lowering add frontend/runtime node behavior. | Durable node identity, revision migration, state ownership, reentrancy, persistence, and distributed identity remain open. |
| Ports | Exact direction/type/connectivity checks exist for the admitted graph surface. | Variant payload isolation and nominal enum checks strengthen values crossing semantic boundaries. | Port cardinality, runtime fan-in, ownership, buffering, backpressure, control/outcome/diagnostic lanes, and revisioned identities remain unspecified. |
| Wires | `=>` graph connection remains distinct from `->` value placement; fan-out preserves signal identity while producing distinct deliveries. | No material wire-contract change found. | Durable wire IDs, delivery guarantees, ordering, loss, retry, timeout, transport, security, and placement remain open. |
| Signals | Process-local signal and failure provenance exist in the narrow runtime. | Outcome/diagnostic values are useful prerequisites, but not a new signal-envelope contract. | Parent/child causality, correlation, attempt, sequence, cancellation, replay, payload identity, policy binding, and cross-process identity remain open. |
| Contracts | Versioned compiler artifacts and exact authorization remain established. | Byte/text/outcome, `when`, enum/variant, and match-plan contracts have been documented and partly implemented. | Canonical Graph IR is still absent. Variant backend layout is incomplete. Storage/borrowing/lifetime contracts remain broad inventory, not decisions. |
| Policies | `flowcore.target_policy/v1` and provider authorization remain implemented examples of governed policy. | FlowLFS applies the discovery-versus-authority principle well. | The canonical Flow Policy Envelope remains undefined and its name still conflicts with an experimental mutable context-bag pattern. |
| Diagnostics | Compiler diagnostics have codes, paths, and provenance; graph failures retain local delivery context. | UTF-8 and language diagnostics gain typed values, spans, and replay-oriented artifacts. | No versioned runtime diagnostic event binds graph revision, signal causality, policy decision, retryability, and disposition. |
| Scheduling | Flowmini remains a synchronous FIFO reference runtime; Flowparallel admits only proved safe candidates with conservative fallback. | No scheduler-contract implementation found. | Joins, fairness, cycles/delay, cancellation, deadlines, backpressure, bounded queues, deadlock semantics, and trace contracts remain open. |
| Runtime | LLVM and TinyVM share governed lowering facts for the admitted surface. | Focused integer/enum match parity expands that surface; FlowLFS supplies a reproducible hosted environment foundation. | Arbitrary graph execution, variant payload parity, durable signals, recovery, parallel scheduling, distribution, and security isolation remain future work. |

## Decision register

### Confirmed decisions added or reinforced by this branch

1. Bytes and decoded text are distinct semantic domains.
2. UTF-8 decoding failures are structured outcomes with spans/diagnostics, not
   silent replacement or ad hoc strings.
3. Hosted byte acquisition is a provider mechanism, not the definition of the
   language operation.
4. Constants are immutable semantic symbols.
5. A `when` selector is evaluated once and case order is preserved.
6. Integer range cases are inclusive; duplicates, overlaps, and descending
   ranges are rejected.
7. Exhaustiveness may be proved for closed nominal enums and variants; an open
   integer domain requires a default.
8. Enum identity is nominal and cannot be substituted merely because the
   underlying integer values agree.
9. Tagged-variant payloads belong to their member arm and may not leak across
   members.
10. Backend match behavior comes from explicit captured facts, not source-name
    or capability-set profiles.
11. FlowLFS discovery evidence does not grant authority; provider selection and
    target policy remain separate.

### Confirmed limits and nonclaims

1. The FlowLFS work is not yet the general Flowcore runtime or a public stable
   distribution contract.
2. Tagged-variant payload layout/binding is not yet complete across LLVM and
   TinyVM.
3. The latest v0.29 ledger does not record a final complete canonical and
   sanitizer pass after all new commits.
4. GitHub has no independent CI result for the current head.
5. Canonical Graph IR, a resolved Flow Policy Envelope, a signal envelope, and
   a versioned scheduler contract are not implemented.

### Proposals or inference—not repository-confirmed decisions

1. The current `0.29.0` version should be treated as a development milestone
   until the complete gates pass and the documentation is reconciled.
2. FlowLFS and v0.29 language maturation should become separately reviewable
   histories or at least separately bounded integration units.
3. Large captured sources should live in a content-addressed artifact store or
   release/object store, referenced from Git by digest manifests.
4. FlowLFS may deserve a separate repository because its artifact lifecycle and
   storage profile differ sharply from the compiler/runtime repository.
5. The next bounded compiler mission should close v0.29 verification and
   documentation before returning to Graph IR and policy work.
6. Human testing should begin only from a clean, reproducible, narrowly
   explained preview checkpoint—not from the current mixed 96-commit branch.

## Contradictions and risks

### 1. Autonomous state says continue; declared mission is complete

[`AGENTS.md`](https://github.com/Henrik1969/Flowcore/blob/flowlfs-v0.1-alive/AGENTS.md)
still points at `docs/tasks/tinyvm-cross-target-bootstrap.md`, a completed
mission. The reusable-chain and TinyVM ledgers record their own completion, yet
`.codex-run-state` is `CONTINUE` and the v0.28 maturation ledger now contains an
open-ended list spanning language design, ownership, concurrency, package
management, self-hosting, bare metal, and office applications.

That inventory is useful as a backlog, but it is not a safe autonomous mission.
It lacks one coherent objective, a bounded proof surface, and a terminal
definition of done. Continuing under it defeats the control mechanism that was
specifically built to prevent unbounded scope expansion.

**Recommended resolution:** stop after the current coherent head; move the
inventory to backlog status; create a new task naming one finite objective and
its gates; set `CONTINUE` only for that task.

### 2. One branch contains two architectural programs

`flowlfs-v0.1-alive` contains both operating-system construction and compiler
language maturation. The two can inform each other, but they have different
reviewers, risks, artifacts, validation gates, and integration timelines.

Consequences of leaving them combined:

- a reviewer cannot assess v0.29 without downloading and mentally filtering
  FlowLFS artifacts;
- a FlowLFS change carries unrelated compiler risk and vice versa;
- bisecting and future merging become harder;
- the branch name no longer communicates its actual content;
- human review becomes unnecessarily expensive.

### 3. Version and status documents disagree

- `VERSION` says `0.29.0`.
- the root
  [`README`](https://github.com/Henrik1969/Flowcore/blob/flowlfs-v0.1-alive/README.md)
  describes a v0.29 slice but still names `v25-symboltable-projection` as the
  active branch and still reports 57/57 tests.
- [`current-status-v1.json`](https://github.com/Henrik1969/Flowcore/blob/flowlfs-v0.1-alive/docs/presentation/current-status-v1.json)
  remains dated 2026-08-26 and reports the earlier 75/75 TinyVM closure.
- `Flowmini/CURRENT.md` still describes substantially older v0.26/v0.27
  architecture and profile-era limitations.

The project currently has no single accurate public status authority.

### 4. Full validation trails focused development

The focused UTF-8, control-flow, enum, variant, and backend-parity tests appear
well chosen. The ledger nevertheless says the full suite remains the next
required boundary after the final enum checkpoint. A version bump is harmless
as a development marker, but it must not be interpreted as a verified release
or completed milestone.

### 5. FlowLFS artifact capture is costly in ordinary Git

Approximately 196 MB of new blobs include large source archives, books,
generated/reference HTML, and fonts. Keeping exact inputs is valuable for
offline reconstruction and audit. Keeping every large immutable object directly
in the main repository imposes that cost on every future clone and retains it
through history even if later deleted.

This is the clearest current example of the proportional-architecture rule:
durable evidence is justified, but its storage mechanism should not cost more
than the evidence problem it solves.

## Concise design note: restore control without losing momentum

### Findings

The new work demonstrates that autonomous execution can produce coherent
technical increments quickly. It also demonstrates why the mission boundary is
part of the architecture. Once a completed task remained named as active, a
broad ledger became de facto authority and two projects accumulated on one
branch.

The code direction does not need a rescue rewrite. The integration and control
plane need correction.

### Alternatives

#### A. Continue the current branch and broad inventory unchanged

Fastest in the immediate hour and preserves uninterrupted momentum.

Consequences: scope has no natural stop; complete gates may continue to lag;
review cost rises; FlowLFS and compiler history become harder to separate; the
autonomous state ceases to mean anything precise. Not recommended.

#### B. Keep one branch but replace the mission with two sequential phases

Less Git surgery. First close v0.29, then close FlowLFS, each with explicit
gates.

Consequences: the branch remains heavy and mixed; reviewers still receive both
change streams; future merge and bisection remain awkward. Acceptable only as a
temporary recovery path.

#### C. Freeze the head, separate integration units, and resume one bounded mission — recommended

Preserve `af769d40` as the immutable recovery/evidence point. Create separately
reviewable histories for:

1. FlowLFS v0.1 construction, evidence, and artifact manifests;
2. Flowcore/Flowmini v0.29 language and backend maturation.

Then give the autonomous worker only the v0.29 closure mission: complete gates,
fix documentation, close or explicitly defer variant backend payload support,
and stop.

Consequences: modest branch-management work now; much lower review, merge,
storage, and governance cost later. No implemented work is lost.

### Artifact-storage alternatives

| Alternative | Benefit | Cost | Recommendation |
| --- | --- | --- | --- |
| Keep all archives in ordinary Git | Simplest offline clone; exact inputs travel together | Permanent history bloat and expensive clones | Do not use for the full corpus |
| Git LFS | Familiar pointers and clone filtering | Adds hosting/tooling dependency; still needs retention governance | Viable, but not uniquely preferred |
| Digest manifests plus release/object storage | Small Git history, exact verification, content-addressed retrieval | Requires an availability/cache policy | Recommended baseline |
| Separate FlowLFS repository | Clean lifecycle and reviewer boundary | Cross-repository coordination | Strongly consider alongside digest manifests |

Recommended minimum contract for every externalized object:

- logical name and upstream URL;
- exact version;
- SHA-256 digest and byte length;
- license/source classification;
- required versus optional/offline-cache status;
- retrieval and verification command;
- evidence linking it to the image/build that consumed it.

### Flow Policy Envelope consequence

Nothing in this branch resolves the existing naming conflict between an
experimental mutable pipeline context and a durable resolved policy decision.
Do not use the new `Outcome` or variant machinery as an excuse to conflate them.
The earlier recommendation stands:

- `FlowExecutionContext`: local mutable services, caches, and collectors;
- `flowcore.policy_decision/v1`: immutable resolved authority—the Flow Policy
  Envelope if that name is retained;
- `flowcore.signal_envelope/v1`: activation facts referencing the graph revision
  and policy decision.

### Decisions Henrik still needs to make

1. **Branch separation:** split FlowLFS and v0.29 histories now, or accept one
   mixed recovery branch and separate only before integration?
2. **v0.29 completion boundary:** is tagged-variant payload parity required for
   v0.29, or explicitly deferred to v0.30 after integer/enum match closure?
3. **Artifact home:** ordinary Git, Git LFS, digest-addressed external storage,
   or a separate FlowLFS repository plus artifact storage?
4. **Policy naming:** reserve “Flow Policy Envelope” for durable resolved
   authority, or retain it for the mutable experimental context and choose a
   different public name?
5. **Constitutional rule:** ratify proportional architecture so future artifact,
   graph, and runtime work must justify its lifetime cost.

## Human review and testing readiness

Henrik has explicitly asked for extra human eyes rather than relying on AI-only
review. That is the right next quality step, but the current branch is a poor
review package because it mixes 96 commits and nearly 200 MB of largely
unrelated artifacts.

Prepare a small **Flowmini v0.29 preview** after the canonical gates pass. It
should contain:

- one exact commit and reproducible clean-checkout build command;
- a short “what changed / what is deliberately unsupported” page;
- three to five focused tester missions:
  1. malformed and boundary UTF-8 differential cases;
  2. `when` range ordering, overlap, default, and selector-once behavior;
  3. cross-enum identity and exhaustiveness failures;
  4. tagged-variant construction and payload isolation;
  5. LLVM versus TinyVM output/diagnostic parity;
- expected outcomes without prescribing the implementation;
- a compact bug-report template capturing source, command, backend, actual
  output, expected output, compiler revision, and environment;
- at least one tester asked to design hostile cases independently rather than
  merely repeat the repository tests.

FlowLFS should receive a different human test plan: clean-room reconstruction,
digest verification, boot evidence, writable-twin recovery, provider fallback,
and network-authority review. Combining both invitations would dilute the
feedback.

## Prioritized concrete implementation steps

1. **Pause autonomous continuation at `af769d40`.** Preserve the branch and
   working tree; do not add another open-ended iteration under the completed
   TinyVM task.
2. **Run the complete verification boundary.** From a clean checkout, run the
   canonical configure/build/CTest suite, the focused LLVM/TinyVM parity gates,
   and the documented ASan/UBSan configuration. Record exact commands, toolchain
   versions, counts, failures/exclusions, and head SHA.
3. **Define the v0.29 closure.** Decide whether tagged-variant payload backend
   parity is in or out. Write one bounded task with explicit positive, hostile,
   parity, documentation, and clean-tree gates. Remove the broad inventory from
   mission authority and retain it as backlog.
4. **Separate the change streams before review or merge.** Preserve an archival
   mixed head, then produce independently reviewable FlowLFS and compiler v0.29
   integration units. Do not rewrite or discard the only copy of any evidence.
5. **Repair autonomous authority.** Point `AGENTS.md` to the newly approved task;
   make `.codex-run-state` reflect only that task; require `DONE` at its finite
   completion boundary.
6. **Reconcile public status.** Update the root README, `VERSION` interpretation,
   `Flowmini/CURRENT.md`, current-status manifest, branch pointers, supported
   surface, nonclaims, and verified test counts from the same evidence record.
7. **Adopt an artifact-storage contract.** Keep small recipes, manifests,
   digests, licenses, and evidence summaries in Git; move large immutable
   third-party objects to the chosen digest-addressed store. Strongly consider a
   separate FlowLFS repository.
8. **Package the human v0.29 preview.** Recruit reviewers only after steps 2–6
   make the review surface small and reproducible. Seek at least one independent
   hostile-test author and one systems/compiler reviewer.
9. **Close the language slice before widening it.** Finish or explicitly defer
   variant payload layout; add the full regression result; correct stale
   `when`-is-deferred wording. Do not expand next into generics, ownership,
   closures, async I/O, packages, self-hosting, and office applications as one
   mission.
10. **Return to the previously approved architectural decision point.** Ratify
    proportional architecture, resolve the Flow Policy Envelope name/authority,
    and then build the deliberately small Graph IR → policy decision → signal
    execution slice. Keep advanced joins, retries, distribution, durable queues,
    cancellation, and parallel scheduling deferred until a concrete consumer
    demands them.

## Bottom line

This week produced real engineering, not churn. FlowLFS has a credible
evidence-oriented bootstrap, and the v0.29 language work advances Flowcore
toward typed, explicit, backend-neutral semantics. The project is not blocked by
bad code; it is at risk of losing the very governance discipline that made the
previous autonomous mission successful.

The correct move is therefore neither “carry on unchanged” nor “throw it away.”
It is: **freeze, verify, separate, bound, document, invite human scrutiny, then
resume one mission at a time.**
