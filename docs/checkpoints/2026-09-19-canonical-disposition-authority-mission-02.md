# Canonical Disposition Authority — Mission 02 checkpoint

Date: 2026-09-19.

Gate 2: **PASS**.

## Baseline and protected scope

- branch: `main`;
- starting `HEAD` and `origin/main`:
  `6ba9db303db453af952327069814c3a607b26d03`;
- governing decision: ADR 0059, tagged Outcome storage transfers a
  must-account obligation;
- mission:
  [Canonical Disposition Authority — Mission 02](../tasks/canonical-disposition-authority-mission-02.md);
- `master`, `flowlfs-v0.1-alive`, and `meta-discusions.md`: untouched;
- no staging, commit, push, branch mutation, or repository-host mutation.

## Accepted bounded authority

The existing atomic `Outcome<Text,TextFailure>` producer now creates one
canonical disposition identity. Storing the tagged value transfers its
obligation to its semantic owner; storage alone does not account for failure.

The admitted source shape proves:

```text
one local tagged owner
one owner.code projection
one simple code == 0 success branch
one simple code != 0 failure branch
owner.value used only in the success branch
owned success Text disposed exactly once
nonempty explicit failure behavior
no copy, alias, return, container, loop, or nested control flow
```

The historical Text-returning compatibility operation retains its established
behavior and does not falsely claim tagged-owner authority. Only the existing
provider boundary returning `TextOutcome` enters this bounded stage.

## Canonical evidence

Flowanalyst emits one `lyraform.disposition_fact` version 1 for each admitted
bounded Text outcome. It records:

```text
producer operation, source, scope, and function-owner identities
exactly-one Success<Text> | Failure<TextFailure> completion
atomic tagged-result publication and tagged owner route
stable invalid_input, exhausted, and provider_unavailable failure codes
must-account identity and accounted status
code projection operation and symbol
success/failure branch operations and blocks
exact success-value uses and disposal operation
exact failure-recovery operations
complementary-zero-branch proof and source provenance
```

Flowcontracts independently validates every relation. Removing the fact,
emptying its set, duplicating it, changing its branch operand, or corrupting an
authority field is rejected before LLVM or TinyVM admission. Carrying stages
preserve the fact exactly.

## Refusal evidence

The focused source gate refuses eight incomplete or unsafe forms with the root
diagnostic `FLOWANALYST_DANGLING_OUTCOME_WIRE`:

1. uninspected outcome;
2. missing failure branch;
3. non-complementary branches;
4. empty failure branch;
5. failure-path `.value` access;
6. missing success disposal;
7. repeated success disposal;
8. copied tagged owner.

The diagnostic retains source provenance and explains the legal repair
classes: inspect both variants, propagate the outcome, or route failure to an
explicit policy sink. The latter two remain future semantics and are not
silently synthesized.

## Hostile evidence

The focused gate rejects **20 hostile artifacts** through Flowvalidate, LLVM
preparation, and TinyVM preparation. Coverage includes missing, empty, or
duplicate authority; forged branch operands; changed completion, payload, code,
route, obligation, owner, projection, branch, value-use, disposal, recovery,
and proof fields.

## Verification

- `./igor doctor`: **PASS**;
- `./igor build`: **PASS**;
- focused related Text/guard set: **5/5 PASS**, 3.82 seconds;
- canonical `./igor test`: **165/165 PASS**, 80.85 seconds;
- Clang ASan/UBSan focused Text outcome and guard set: **2/2 PASS**, 4.91
  seconds, with LeakSanitizer disabled because of the known host
  ptrace-wrapper limitation;
- Valgrind Flowanalyst producer: **PASS**, 0 errors, 0 bytes in use at exit;
- Valgrind Flowvalidate consumer: **PASS**, 0 errors, 0 bytes in use at exit;
- two executable Text outcome cases: **PASS** for success and failure;
- eight source refusals: **PASS**;
- twenty hostile artifacts across three admission consumers: **PASS**;
- exact disposition preservation across parallel, optimized, and lowering
  plans: **PASS**;
- changed shell script syntax: **PASS**;
- `git diff --check`: **PASS**.

The full build retained the pre-existing unused-function warning in
`Flowparallel/src/graph_cuda.cpp`; this mission did not change that file.

## Non-goals preserved

- No new source syntax or language-surface expansion.
- No generalized Outcome type or ownership system.
- No implicit propagation, generic discard, exception, or unwind behavior.
- No runtime guard route or policy syntax.
- No copy, alias, return, container, concurrency, loop, or nested-control
  accounting semantics.

## Gate result

```text
Candidate A must-account law                         PASS
bounded deterministic producer                      PASS
educational dangling-wire refusal                   PASS
independent relation validation                     PASS
exact current-stage preservation                    PASS
hostile LLVM/TinyVM refusal                         PASS
legacy Text compatibility preserved                 PASS
canonical suite                                     PASS
```

GATE 2: PASS

## Required stop

The mission requires a stop at Gate 2. Generalizing obligation transfer across
copy, alias, return, containers, propagation, policy sinks, loops, or nested
control flow would establish new ownership and failure-routing semantics and
therefore requires a later focused decision and mission.
