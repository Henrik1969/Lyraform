# Gate 9 — bounded member assignability reconnaissance

Date: 2026-09-19. Gate 9 scout and candidate-model evaluation only.

This reconnaissance stop remains the historical record of the original
blocker. Henrik subsequently selected independent aggregate values with
reconstruction in ADR 0052; implementation and final Gate 9 evidence are in
[the completion checkpoint](2026-09-19-v1-bounded-local-member-assignability.md).

## Baseline

Branch `main`; initial/current HEAD and locally recorded `origin/main`:
`c03079e46e938e98b04f3bad21c991cc4f58eee4`.

Gate 8 starts this mission with completely resolved bounded field paths,
`lyraform.target_fact` v2, `assignability: unresolved`, and
`execution: unsupported`. The inherited worktree had 15 modified tracked paths
and 27 untracked paths. No inherited path was discarded or staged. No branch,
commit, remote, `master`, or FlowLFS operation was performed.

## Reconnaissance checkpoint

The scout phase found insufficient canonical language semantics to admit member
assignment safely. Implementation therefore stops at this checkpoint, as
required by Candidate D. The result separates language facts from compatibility
behavior rather than promoting the legacy runtime's mutation mechanism into a
language rule.

### Proven language/structural facts

- Canonical source placement is `value -> target`; structural AST-only
  `name = value` exists as a separate assignment source form and does not prove
  accepted executable member syntax.
- `AssignableTarget` distinguishes identifier, field-path and indexed syntax.
- Gate 8 resolves a field path's base declaration/type, every member
  declaration/type and final destination type.
- The canonical scalar slice admits initialized `int` and `Bool` identifier
  destinations when source and destination types are identical.
- Local binding, parameter and field AST nodes contain names, type references
  and origins, but no mutability/read-only property.
- Structural symbol projection emits `Variable`, `Parameter` and `Field`
  symbols with declaration/type facts. It emits no writability fact for these
  declarations.
- The generic SymbolTable library defines a `Constant` symbol kind, but the
  active structural frontend does not project any source declaration to it.
- Named types can retain qualified name segments structurally. That is type-name
  syntax, not a mutability qualifier.
- ABI type declarations can carry ABI-specific ownership/access/lifetime
  clauses. Those govern native-carrier contracts, not local aggregate member
  assignment, and cannot be reused as general language mutability evidence.

### Implemented compatibility behavior, not canonical law

- The legacy parser's `lowerAssignmentToTarget` resolves target types, checks
  its compatibility policy and lowers a field placement to
  `record.field.set` using the root payload path plus textual field names.
- The compatibility runtime mutates a nested `RecordPayload` value in place.
- Legacy assignment to a scalar identifier updates its payload path.
- Existing legacy probes provide evidence of value-owned records and record
  copy behavior, but the compiler-unification reconnaissance explicitly marks
  general record/list/array ownership as `UNDECIDED` and deep-copy behavior as
  a test oracle rather than normative v1 semantics.

These mechanisms prove that an old runtime can perform a write. They do not
prove whether Lyraform member placement means object mutation, root rebinding,
aggregate reconstruction, or another governed state transition.

### Missing semantics

- no admitted `const`, immutable, mutable, readonly or writable qualifier;
- no declaration-level mutability default stated as canonical language law;
- no field-level writability property;
- no parameter mutation/rebinding rule;
- no distinction between initialization and later aggregate mutation in the
  structural declaration model;
- no canonical aggregate value/reference, identity, aliasing, ownership, copy
  or reconstruction rule;
- no rule explaining what happens to intermediate values in `p.a.b`;
- no mutation provenance/revision rule for a member write;
- no canonical member-source compatibility admission yet.

### Source probes

Three temporary probes were exported through the current compiler and analyzed.

1. A local `p.x` and parameter-root `p.a.b` both resolve completely to `int`,
   but remain `assignability: unresolved` and `execution: unsupported`.
2. `true -> p.x` where `Point.x : int` is resolved by canonical analysis but is
   not admitted or refused: Flowanalyst returns `status: ok` with unresolved
   assignability. The legacy oracle refuses it with `type mismatch assigning
   Bool to int`. This confirms the missing source/destination compatibility
   authority for member targets.
3. `const p : Point(...)` is not parsed as a constant declaration. `const` is
   lexed as an ordinary identifier, the structural shell later recognizes
   `p : Point(...)`, and symbol projection emits an ordinary `Variable` with no
   mutability fact. Treating that spelling as immutable would therefore invent
   semantics and would also conceal a current parser-completeness gap.

Probe artifacts are under `/tmp/lyraform-gate9-*`; they are evidence only and
are not repository fixtures because Gate 9 did not authorize a new language
decision.

## Current assignment authority

For the bounded canonical scalar slice, shared scalar analysis owns exact
`int`/`Bool` source/destination compatibility for initialized identifier
destinations. Parser structure owns target shape. Gate 8 Flowanalyst owns field
identity and result type. No canonical component currently owns member
assignability.

The legacy parser/runtime owns compatibility execution behavior only. LLVM and
TinyVM own neither source-language writability nor member semantics and continue
to refuse field-target execution.

## Candidate models

### Candidate A — root-binding mutability

Not selectable from current evidence. No root mutability fact exists, and the
language has no admitted `const` distinction. Declaring every `Variable` or
`Parameter` mutable would be a new default rule. More importantly, permitting
`p.a.b` solely from root mutability would silently choose either reference-like
nested mutation or reconstruction/rebinding behavior, neither of which is
defined.

### Candidate B — per-segment mutability

Rejected for the current model. Structural fields carry identity, type and
origin only. Adding implicit field mutability, or interpreting every field as
mutable because no qualifier exists, would invent a field-level language law.
There is no evidence object for intermediate segment write authority.

### Candidate C — value reconstruction

Architecturally plausible and compatible with a value-oriented language, but
not currently proven. It needs canonical aggregate construction, copy/update,
root rebinding, evaluation-order and failure/provenance semantics. Legacy
in-place `RecordPayload` mutation cannot establish reconstruction semantics.
No reconstruction was implemented.

### Candidate D — admission deferred

Selected. A completely resolved path remains:

```text
exists = proven
type = proven
assignability = unresolved
execution = unsupported
```

This preserves Gate 8 truth without claiming that member existence implies
writability.

## Minimum required language decision

Before member assignability can be implemented, Lyraform needs one explicit
aggregate update law answering:

1. Are aggregate bindings values or identities/references?
2. Does `value -> p.a.b` mutate shared state, reconstruct nested values and
   rebind `p`, or denote another explicit state transition?
3. What declaration fact grants write/rebind authority to locals and
   parameters?
4. Are fields independently writable, and where is that property declared?
5. How are initialization and later update distinguished?
6. What ownership/copy/alias and mutation-provenance guarantees apply?

The smallest safe next mission is an aggregate update/ownership decision,
supported by legacy behavior probes but not dictated by them. Once chosen, the
frontend must represent the relevant declaration facts before Flowanalyst can
derive assignability.

## Semantic authority and schema decision

The intended future architecture remains:

```text
parser → target/declaration structure
Flowanalyst/shared semantics → resolution + source compatibility + assignability
Flowcontracts → validate evidence
later stages → preserve/validate
backends → refuse until execution is separately implemented
```

No `target_fact` version or shape changed in Gate 9. Version 2 continues to say
`assignability: unresolved`. Inventing `assignable` or `refused` evidence in the
absence of a declaration/update law would be false authority. Old artifacts are
therefore neither reinterpreted nor invalidated.

## Positive and negative evidence

No positive member-assignability case is claimed. `p.x`, `p.y` and `p.a.b` are
positive resolution/type cases only.

Gate 8 semantic refusals remain authoritative for missing members, scalar
intermediates, unknown/scalar bases and ambiguous fields. The Bool-to-int probe
demonstrates a required future negative member-admission case; it is not
papered over as a Gate 9 implementation result.

A const-root negative cannot honestly be specified because `const` has no
admitted declaration semantics. A read-only field case likewise cannot be
constructed from the current language model.

## Hostile artifacts and pipeline preservation

No new assignability schema exists to mutate. Gate 8's hostile target-fact
coverage continues to validate member identities, chain/result types,
operation linkage and unsupported execution. A hostile artifact changing
version-2 `assignability` from `unresolved` is already rejected by the shared
validator.

The current resolved facts remain preserved through Flowbind, Flowparallel,
Flowoptimize and flowprepare. No downstream stage repeats member lookup or
promotes unresolved assignability.

## Backend boundary

LLVM and TinyVM remain unchanged. They validate target evidence and refuse
member/index execution before publishing output. They do not calculate offsets,
infer layout, reconstruct aggregates, emit partial stores, collapse a member to
its root or decide language-level writability.

## Verification

This mission intentionally changes documentation only. The existing focused
Gate 8 suites are the applicable no-regression tests; no fake positive Gate 9
fixture was introduced.

- `./igor doctor`: PASS;
- `./igor build`: PASS, no work required against the verified 302-target build;
- `./igor test`: **164/164 PASS**, 69.97 seconds;
- normal focused scalar/target suites: **8/8 PASS**, 6.62 seconds;
- Clang 18.1.3 ASan/UBSan focused suites: **8/8 PASS**, 18.46 seconds;
- `git diff --check`: PASS;
- changed shell scripts: none.

The sanitizer run used `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and
`UBSAN_OPTIONS=halt_on_error=1`. LeakSanitizer remains incompatible with the
host ptrace wrapper. No process/runtime code changed in this mission, so a new
Valgrind run was not meaningful; Gate 8's immediately preceding analyzer,
semantic-refusal, LLVM-refusal and TinyVM-refusal probes remain **0 errors with
all heap blocks freed** on the unchanged binaries.

Final worktree: 15 modified tracked paths and 28 untracked paths, including the
inherited Missions 02–08 work and this checkpoint. HEAD and `origin/main`
remain unchanged; the index is empty.

## Untouched scope

No backend storage, aggregate layout, ownership implementation, alias analysis,
borrowing, copy/move rule, reconstruction, indexed resolution, mixed target,
generic, alias or global-identity work was performed. No language syntax was
added. `master`, FlowLFS and remotes were untouched.

## Remaining gaps and recommended next mission

The blocking gaps are aggregate value/update semantics and declaration write
authority. Source/destination member compatibility can be implemented alongside
assignability once the admission model exists; it must use the resolved final
member type and the shared type policy rather than legacy lookup.

Recommended next mission: **Canonical aggregate value and update semantics
decision**. It should compare immutable reconstruction, uniquely owned mutation
and explicitly referenced mutation; define local/parameter/field authority;
specify initialization versus update and provenance; and end at a design gate
before backend work.

GATE 9: BLOCKED — LANGUAGE SEMANTICS INSUFFICIENT
