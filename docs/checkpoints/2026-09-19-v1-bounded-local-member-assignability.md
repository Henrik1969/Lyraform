# Gate 9 completion — bounded local member assignability

Date: 2026-09-19.

## Result

Lyraform now has one bounded canonical authority for member-placement
admission. Given a fully resolved field path rooted at an explicitly
initialized local `Variable`, Flowanalyst proves exact `int`/`Bool` source and
destination compatibility under ordinary aggregate reconstruction semantics.

```text
member placement
    = reconstruct the aggregate path
    + preserve every untouched subvalue
    + rebind the initialized local root
```

Other bindings that hold the prior aggregate value remain unchanged. Physical
in-place update is permitted only as an observationally equivalent future
optimization. This language decision is recorded in
[ADR 0052](../architecture/decisions/0052-independent-aggregate-values.md).

## Baseline and safety boundary

Branch `main`; initial/current HEAD and locally recorded `origin/main`:
`c03079e46e938e98b04f3bad21c991cc4f58eee4`.

The inherited worktree contained Missions 02–08 and the Gate 9 reconnaissance.
No inherited path was discarded or staged. No branch, commit, remote,
`master`, or FlowLFS operation was performed.

This stage does not execute member placement. `execution` remains
`unsupported`, and LLVM/TinyVM continue to refuse all member targets before
publishing output.

## Reconnaissance and selected model

The initial Gate 9 reconnaissance found no canonical aggregate update law and
correctly stopped with unresolved member assignability. It evaluated root
mutability, per-segment mutability, reconstruction, and deferral. The follow-up
decision selected immutable ordinary aggregate values with recursive
reconstruction.

The smallest existing write authority that can support that model is the
canonical scalar slice's established-local rebinding rule:

- the root symbol is a structural `Variable`;
- its local declaration has an explicit initializer;
- the declaration precedes the placement;
- the complete member path resolves;
- the final member and source expression are both in the admitted `int`/`Bool`
  scalar slice;
- their types are identical under the shared scalar compatibility rule.

Parameters and uninitialized locals do not receive this authority. The source
language still has no admitted `const` or read-only declaration/field syntax,
so no fictitious negative fixture was created for either spelling.

## Semantic authority

Authority remains deliberately separated:

```text
structural frontend
    target shape, declarations, scopes, symbol/type origins

Flowanalyst + shared scalar semantics
    member identity/type resolution
    initialized-local root authority
    source-expression type
    exact source/destination compatibility
    reconstruction/rebinding admission or refusal

Flowcontracts
    target-fact schema and cross-operation validation

later stages
    byte-preserving propagation and validation

LLVM / TinyVM
    validated refusal because execution is unsupported
```

No downstream stage repeats member lookup or makes a language-level
assignability decision.

## Contract evolution

Field paths now use `lyraform.target_fact` version 3. Versions 1 and 2 remain
readable and retain their original meaning; v2 `assignability: unresolved` is
not reinterpreted.

Version 3 carries an assignability object with:

- `state`: `assignable`, `refused`, or `unresolved`;
- authority `lyraform.aggregate_reconstruction/v1`;
- root authority `established_local_rebinding/v1`;
- update operation `reconstruct_and_rebind`;
- source expression and source type;
- final destination type;
- root declaration statement identity.

The authority fields are populated only for a decision inside this bounded
slice. Structurally refused paths and roots outside local rebinding authority
remain explicitly unresolved. A type-incompatible source produces a resolved
target, `assignability.state: refused`, deterministic failure evidence, a
diagnostic, and a blocked plan.

Identifier and indexed target facts remain version 2. This avoids unrelated
schema churn.

## Positive, negative, and hostile evidence

Positive cases prove `20 -> p.x`, `true -> p.y`, and nested `20 -> p.a.b`.
They verify final member identity/type, reconstruction authority, deterministic
repeated output, and byte-identical preservation through Flowbind,
Flowparallel, Flowoptimize, and flowprepare.

Negative cases prove early refusal of both `Bool -> int` and `int -> Bool`.
An uninitialized local aggregate and a parameter root remain unresolved rather
than acquiring invented authority. All Gate 8 missing-member, scalar
intermediate, missing/scalar base, duplicate-field, and mixed-target refusals
remain intact.

Sixteen hostile member facts cover member-chain contradictions plus altered
assignability state, authority, root authority, update mode, source expression,
source type, destination type, and root declaration identity. Flowvalidate,
LLVM, and TinyVM reject the contradictions. A forged ready plan containing a
refused member assignment is also rejected.

## Backend boundary

The backends validate the evidence and then refuse member/index execution.
They do not collapse a member target to its root, infer offsets or layout,
reconstruct values, emit stores, or reinterpret compatibility. Semantic
admission is therefore known while executable support remains honestly absent.

## Verification

- `./igor doctor`: PASS;
- `./igor build`: PASS, coherent 302-target Ninja graph;
- focused scalar/target suites: **8/8 PASS**, 11.25 seconds;
- focused member evidence: **3 admitted paths, 2 type refusals, 3 unresolved
  roots, 5 target-resolution refusals, 16 hostile facts, 3 mixed parser
  refusals**;
- Clang 18.1.3 ASan/UBSan focused suites: **8/8 PASS**, 16.54
  seconds;
- canonical `./igor test`: **164/164 PASS**, 67.90 seconds;
- Valgrind admitted/refused analyzer probes: **0 errors**, **0 bytes in 0
  blocks at exit** for both;
- `git diff --check`: PASS;
- changed shell scripts pass `sh -n`.

LeakSanitizer uses `detect_leaks=0` because the host ptrace wrapper is
incompatible with LeakSanitizer; Valgrind provides the leak-accounting probe.

## Untouched scope and remaining gaps

No backend field storage, aggregate layout, reconstruction implementation,
ownership system, alias analysis, borrow/copy/move syntax, field mutability,
parameter rebinding, index resolution, mixed target, generic, alias, reference,
cell, or stable-global-identity behavior was added.

The next bounded stage should specify and lower an explicit aggregate
reconstruction operation without weakening the current backend refusal until
both LLVM and TinyVM can consume the same validated operation contract.

GATE 9: PASS — BOUNDED MEMBER ASSIGNABILITY PROVEN
