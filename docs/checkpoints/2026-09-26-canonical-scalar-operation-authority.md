# Canonical scalar operation authority — Stage 2A

Date: 2026-09-26.

## Baseline and scope

- branch and upstream: `main`;
- baseline HEAD and `origin/main`:
  `ecdac7184236dab24423ca7495e882f771cdf287`;
- retained uncommitted Gate 0, Gate 1, ownership, current-truth, and legacy
  deprecation stages preserved;
- unrelated `meta-discusions.md` preserved;
- no syntax, scalar type, conversion, target, ownership, failure, scheduling,
  ABI, or runtime meaning added.

This is the first bounded bridge in semantic-producer convergence. It covers
only the already-admitted initialized `int`/`Bool` declaration and scalar
identifier-placement slice.

## Scout finding

Flowanalyst produced canonical `lyraform.scalar_fact` evidence, but lowering
operation construction independently looked up declaration and placement
destination symbols. Identifier target projection also repeated base identity
and type lookup, and operand projection recovered the destination type from the
symbol table.

Those lookups usually agreed, and artifact validation rejected contradictions,
but the construction path still contained two decisions for one bounded fact.
The deprecated direct Flowmini runtime adapter is outside Igor's canonical path
and remains oracle evidence only under ADR-0061.

## Question and authority

Question:

> Once Flowanalyst's bounded scalar analysis has established destination
> identity, source/destination types, compatibility, and provenance, may a
> canonical lowering operation independently resolve those facts again?

Answer: **no**. Within the bounded scalar slice, the scalar fact is the one
canonical producer. Lowering operations, identifier-target projection, and
operand type projection consume its destination identity and type. Existing
fallback lookup remains only for source families outside this scalar slice;
this stage does not assign those families new meaning.

## Implementation

- scalar facts are produced once before lowering-operation construction;
- scalar declaration and identifier-placement operations take
  `result_symbol_id` from `scalar_fact.destination_symbol_id`;
- identifier target projection takes base identity and type from that same
  fact;
- scalar operand projection takes its declared destination type from the fact;
- the existing Flowcontracts validator continues to require operation/fact
  identity and type agreement at every consumer;
- the authority-map drift guard checks the connected bridge and source-level
  consumption points.

## Evidence

- valid declaration and placement operations assert
  `result_symbol_id == scalar_fact.destination_symbol_id`;
- twelve invalid scalar flows remain refused at Flowanalyst;
- four parity programs execute through LLVM and TinyVM under lowering-plan v1
  and v2;
- the retained legacy oracle agrees in three cases and retains one documented
  historical identifier-initializer defect;
- seven hostile artifact mutations are rejected by `flowvalidate`, LLVM
  lowering, and TinyVM lowering, including both directions of an
  operation/fact destination-identity contradiction;
- mapped provenance and conflicting frontend type refusal remain covered;
- scalar, target, field-path, guard, documentation, and authority focused
  gates: **7/7 PASS**, 4.19 seconds;
- `igor doctor`: PASS;
- `igor build`: PASS;
- accumulated-tree canonical suite: **170/170 PASS**, 81.60 seconds;
- Clang 18.1.3 ASan/UBSan scalar gates: **2/2 PASS**, 4.93 seconds,
  with leak detection disabled under the host's ptrace supervision;
- Valgrind 3.22.0 positive and refusal probes: **2/2 clean**, zero errors
  and zero bytes live at exit;
- repeated semantic reports: byte-identical PASS;
- documentation, authority, and legacy-deprecation drift guards: PASS;
- `git diff --check`: PASS.

## Gate

Stage 2A: **PASS**. The complete canonical suite and applicable pressure gates
are green. This checkpoint does not claim completion of Stage 2; target, guard,
disposition, ownership, effect, and provider producer convergence remain
separate bounded work.

Recommended next stage: audit target/operation identity production first,
because it is immediately downstream of the scalar bridge. If reconnaissance
finds no duplicated decision, record that fact and advance to the next producer
family rather than refactoring without an authority defect.
