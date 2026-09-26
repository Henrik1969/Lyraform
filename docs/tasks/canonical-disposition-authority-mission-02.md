# Canonical Disposition Authority — Mission 02

## Objective

Project the existing bounded `Outcome<Text,TextFailure>` operation into
canonical disposition authority and enforce its must-account obligation for the
already-established complementary-code-branch recovery shape.

## Baseline

- synchronized `main` baseline remains
  `6ba9db303db453af952327069814c3a607b26d03`;
- Mission 01 Gate 1: PASS;
- ADR 0059: tagged storage transfers, but does not discharge, the obligation;
- continue in the existing isolated unstaged working tree;
- no staging, commit, push, `master`, or FlowLFS authority.

## Admitted bounded shape

```text
produce Outcome<Text,TextFailure> into one local owner
project owner.code into one local code value
prove complementary code == 0 and code != 0 sibling branches
success branch may access owner.value and disposes it exactly once
failure branch never accesses owner.value and performs explicit behavior
no copy, return, alias, container storage, loop, or nested control flow
```

## Required facts

The disposition fact must retain:

```text
producer operation and owner symbol identity
exclusive Success<Text> | Failure<TextFailure> possibilities
the exact stable failure codes
atomic tagged-result publication
must-account obligation identity
code-projection symbol and operation identity
success/failure branch operation and block identities
success value-use and dispose operation identities
failure recovery operation identities
accounting proof classification and provenance
```

## Refusals

At minimum refuse:

- produced outcome never inspected;
- code inspected by only one branch;
- non-complementary branch conditions;
- empty failure branch;
- `.value` access on failure;
- missing or repeated disposal on success;
- copied/returned/aliased outcome;
- unsupported nested/loop control flow;
- hostile mutation of any authority or accounting linkage.

Use an explanatory `FLOWANALYST_DANGLING_OUTCOME_WIRE` root diagnostic for an
unaccounted owner. More specific safety violations may use specific codes when
the evidence distinguishes them.

## Non-goals

- new source syntax;
- generalized Outcome types;
- runtime guards;
- implicit propagation;
- generic discard;
- exception/unwind behavior;
- copy/move/borrow semantics;
- containers or concurrency.

## Verification

Run focused positive, negative, preservation, hostile-validator, LLVM, TinyVM,
ASan/UBSan, Valgrind, Igor build/test, shell syntax, and diff checks.

## Gate 2

PASS only when the existing bounded Text outcome is admitted by canonical
must-account evidence, all incomplete/unsafe variants fail before executable
admission, every carrying stage preserves the fact exactly, hostile projections
are rejected by both backends, and the canonical suite remains green.
