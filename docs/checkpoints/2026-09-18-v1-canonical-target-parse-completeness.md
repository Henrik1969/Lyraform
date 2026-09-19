# Mission 06 — canonical target parse completeness

Date: 2026-09-18. Phase 6 only; stop at Gate 6.

## Baseline and pre-repair classification

Initial `main` HEAD and recorded `origin/main`:
`c03079e46e938e98b04f3bad21c991cc4f58eee4`.
Inherited unstaged work: 12 modified tracked paths and 18 untracked paths from
Missions 02–05. Preserve it; no commit/push or branch changes are authorized.

Before repair, `parse_ast_assignable_target` receives its cursor by value.
It returns an IdentifierTarget, a vector of FieldPathSegment, or one vector
of index expression IDs. It neither returns consumed extent nor demands the
statement boundary. Placement subsequently skips the remaining source. Index
components use a permissive expression shell and separator-skipping loop.
The variants cannot express member/index alternation or repeated index groups.

Classification made before implementation:

| Forms | Current status | Phase 6 disposition |
| --- | --- | --- |
| `x` | CURRENTLY_CANONICAL | CANONICAL_EXISTING; retain scalar meaning |
| `p.x`, `p.a.b` | CURRENTLY_STRUCTURAL_ONLY | STRUCTURALLY_SUPPORTED_BUT_SEMANTICS_DEFERRED |
| `items[i]`, `matrix[r,c]`, `a[i,j]` | CURRENTLY_STRUCTURAL_ONLY | STRUCTURALLY_SUPPORTED_BUT_SEMANTICS_DEFERRED |
| `p.x[i]`, `items[i].field`, `items[i][j]` | UNDECIDED; shortened projection | AMBIGUOUS; refuse, no generalized target tree |
| `matrix[r,c].field`, `a.b[c].d`, `a.b[i].c`, `a[i].b[j]`, `a[i,j][k]` | UNDECIDED; not representable by current variants | AMBIGUOUS; refuse |
| `a.`, `a..b`, `a[`, `a[]`, `a[i`, `a[i].`, `a[i]junk` | UNSUPPORTED malformed syntax | MALFORMED; refuse |
| trailing identifiers or `: Type` after any complete target | UNSUPPORTED | OUTSIDE_CURRENT_GRAMMAR; refuse |

One bracket group's expression order/rank must be preserved. No equivalence
between multidimensional indexing and chained indexing is defined. Compound
index expressions may be preserved only when the expression shell's entire
token span is represented; otherwise conservatively refuse without deciding
future expression grammar. No index type or member lookup is performed.

## Implementation and representation

The existing target variants are unchanged. The target parser now returns an
optional complete shape and advances a caller-visible token cursor. A member
dot requires a following identifier; every segment is appended in source order.
An index group requires a nonempty expression for each comma-separated
component and a closing bracket. The closing bracket is consumed explicitly.

`target_index_complete` checks each index expression shell's ordered token
projection and grouping spans against its original component token range.
This prevents expressions such as `i junk`, empty comma gaps, missing operands
or malformed grouping from becoming a shorter retained index. Its bounded
proof covers identifier, integer, Bool, unary and binary AST forms, including
grouping. More complex index expressions (calls, field/index access, collection
or other literal forms) are conservatively refused rather than asserted
complete. This is a syntax limitation, not an index type restriction: the
positive corpus deliberately includes `a[true]` without claiming valid typing.

Placement requires the returned cursor to be at newline, enclosing right brace
or end of input. Any remaining dot, bracket group, annotation, identifier or
other meaningful suffix refuses the entire placement. The source expression
and operator retain their existing parser/Phase 5 checks; this mission does
not make whole non-scalar programs canonical merely because a target is complete.

On refusal, placement produces an `UnknownStatement` with a diagnostic reason
and cursor line/column, not a usable shortened PlacementStatement. Recovery
then advances to the existing statement boundary. Intermediate expression
nodes can remain inspectable, but they are not executable target facts.

The Mission 05 finalizer maps that recovery statement to `recovered`,
`recovery_used=true`, coverage `unassessed`. This intentionally uses one existing
taxonomy instead of distinguishing a second target-specific invalid enum.
All malformed, unsupported and undecided target refusals use this conservative
recovery result. Successfully parsed member/index targets remain whole-program
`outside_scope / compatibility / unassessed`, not canonical scalar programs.

## Critical regressions, members and indices

`p.x[i]`, `items[i].field` and `items[i][j]` now produce explicit recovered
non-executable statements. No shorter field/index placement survives. The same
policy covers `matrix[r,c].field`, `a.b[c].d`, `a.b[i].c`, `a[i].b[j]` and
`a[i,j][k]`. Their future grammar remains undecided.

`p.a.b` retains the base `p` and ordered fields `[a,b]`. `matrix[r,c]` retains
the base `matrix` and two expression IDs resolving to `[r,c]` in that order.
Rank is the index vector's length, not inferred from discarded punctuation.
Grouped arithmetic indices preserve AST expression structure. Missing, leading,
trailing and doubled separators refuse the entire placement; they cannot
compress a rank-two target into rank one. Chained bracket groups are refused,
not normalized to a multidimensional group.

## Source extent and frontend boundary

Target nodes still retain their existing start locations and member locations.
No global source-map or AST serialization redesign was made. During parsing,
the caller has both targetStart and targetEnd and can distinguish consumed
extent from meaningful remaining suffix. The refusal diagnostic points at the
parser cursor; these are expanded-source token coordinates, not a newly added
original-file full-span mapping. Index component begin/end token ranges are
checked before the target is accepted. Full serialized end spans remain deferred.

The existing bundle v2 `lyraform.parse_validity` v1 field carries recovery
through export/import. Flowanalyst refuses it with a blocked lowering plan.
Downstream plan validators retain the same refusal even if outer status flags
are forged ready. Direct execution refuses before adaptation or legacy fallback.
No extra target schema, lowering operation or backend behavior was introduced.

Historical artifacts without evidence remain readable under Mission 05's
unproven compatibility policy. Even `outside_scope` evidence is not a universal
target-completeness certificate on an imported artifact. No authentication or
mandatory historical proof is claimed.

## Corpus and authority

`lyraform_target_parse_completeness` exercises eight positive structural cases,
explicit member/index order assertions, and 27 refused targets:

- 8 mixed/chained forms;
- 7 malformed forms from the mission's minimal corpus;
- 3 unexpected identifier suffixes and 1 typed-placement suffix;
- 8 additional separator, grouping, operand and suffix-loss probes.

Every refusal verifies exported recovery, non-ready staged admission, no direct
adapter/fallback, and failure of an outer-status-forged plan. Every export is
repeated and byte-compared. Scalar identifier placement additionally runs the
direct no-legacy path. Existing five scalar suites remain regression gates.

All 23 historical collision input pairs remain in the reconnaissance corpus.
The regression runner no longer requires the now-fixed target losses to have
identical ASTs: recovery statements deliberately differ from shortened targets.
It additionally asserts recovery and blocked analysis for member_dot,
member_index, index_missing, index_field and index_again. The Phase 4 report and
Phase 5 checkpoint retain their historical observations unchanged.

| Responsibility | Authority after Phase 6 |
| --- | --- |
| Target token extent and complete shape | Structural target parser and placement boundary |
| Index shell full representation | Parser-owned component token/AST check |
| Recovery and execution eligibility | Existing Mission 05 parse-validity finalizer and consumers |
| Scalar whole-source completeness | Existing Mission 05 bounded parser check |
| Scalar type compatibility | Unchanged shared scalar semantic authority |
| Member/index semantics | Not migrated by this mission |

There is no new Flowanalyst or runtime suffix checker. The existing scalar
whole-source check remains a broader defensive correspondence check for scalar
programs, not an independent parser of member/index targets.

## Verification

Normal GCC 13.3 external build: `/tmp/lyraform-phase2-build`.
`cmake -S . -B /tmp/lyraform-phase2-build -G Ninja`: exit 0.
With `LYRAFORM_BUILD_DIR=/tmp/lyraform-phase2-build` and
`CMAKE_BUILD_PARALLEL_LEVEL=4`:

- `./igor doctor`: PASS.
- `./igor build`: exit 0.
- `./igor test`: **162/162 PASS**, 66.23 seconds.
- Focused `ctest -R '^lyraform_(scalar|target)_'`: **6/6 PASS**, 10.27 seconds.
  The target suite itself passed in 0.56 seconds, with 8 positive and 27
  refused target cases. This includes the seven explicitly requested malformed
  forms plus separator/grouping/operand probes listed above.

Logs: `/tmp/lyraform-phase6-configure.log`,
`/tmp/lyraform-phase6-doctor.log`, `/tmp/lyraform-phase6-igor-build.log`,
`/tmp/lyraform-phase6-igor-test.log`, `/tmp/lyraform-phase6-focused.log`.

Clang 18.1.3 Debug instrumented external build:
`/tmp/lyraform-phase2-asan`; C/C++ flags
`-fsanitize=address,undefined -fno-omit-frame-pointer`, executable/shared
linker flags `-fsanitize=address,undefined`. Rebuilt parser/frontend binaries
and applicable stage tool targets before running:

```sh
ASAN_OPTIONS=detect_leaks=0:verify_asan_link_order=0 \
UBSAN_OPTIONS=halt_on_error=1 \
ctest --test-dir /tmp/lyraform-phase2-asan -R '^lyraform_(scalar|target)_' --output-on-failure
```

**6/6 PASS**, 13.82 seconds; instrumented target suite 1.53 seconds.
Logs: `/tmp/lyraform-phase6-asan-build.log`,
`/tmp/lyraform-phase6-asan-test.log`. This is focused coverage, not a full
162-test sanitizer run. LeakSanitizer was disabled.

Valgrind 3.22.0, final normal binaries, full leak checking and error exit 99:

- Direct scalar identifier initialization: output `20`, exit 0; legacy forbidden.
- Frontend export of `matrix[(r + 1),c]`: exit 0, preserved indexed structure.
- Direct `items[i].field`: expected parser recovery refusal, exit 1, before
  runtime adapter or compatibility fallback.

All three: **0 errors, all heap blocks freed**. Logs:
`/tmp/lyraform-phase6-valgrind-scalar.log`,
`/tmp/lyraform-phase6-valgrind-index.log`,
`/tmp/lyraform-phase6-valgrind-refusal.log`.

Compatibility impact is deliberate: newly parsed malformed/mixed targets and
index expression shells outside the bounded completeness proof are now refused
early instead of potentially executing a shorter projection. Existing canonical
scalar behavior and the full existing suite remain green. Historical imports
retain their previous unproven compatibility policy.

Final HEAD remains `c03079e46e938e98b04f3bad21c991cc4f58eee4` on `main`.
Final worktree: 12 modified tracked paths and 20 untracked paths; no staged
changes. Phase 6 changed the structural builder, CTest registration and the
inherited collision runner, and added the target test runner and this checkpoint.
No commits, pushes, branch changes, FlowLFS changes or master changes occurred.
`git diff --check` passed. Verification rebuilt existing external build trees;
it was not a fresh-clone or cross-platform assurance exercise.

## Remaining gaps and typed-target readiness

The three existing target variants now provide a trustworthy bounded producer
boundary: supported simple shapes are retained completely, and unsupported
combinations cannot silently become shorter placement targets. That is enough
to consider typed-target preservation for these supported shapes at another
gate, not enough to promise a general mixed-target model or begin its semantics.

Target semantic identity, member existence, target/index types, bounds,
assignability, ownership and mutation remain unresolved here. Complex index
expressions need additional structural completeness work before admission.
Compatibility `field` spelling remains untouched. Whole-program completeness
outside canonical scalar ownership and authenticated historical imports remain
outside this proof.

Next options, evaluated but not implemented:

1. Typed assignable-target preservation: feasible for the bounded existing
   shapes after agreeing on semantic identities and projection contracts.
2. Member-target semantic resolution: separate authority for member existence,
   type and assignability; not implied by successful parsing.
3. Index-target semantic resolution: needs explicit type/rank/bounds policy;
   no chained/multidimensional equivalence should be assumed.
4. Further mixed-target grammar reconnaissance: recommended before designing
   any generalized target tree or admitting currently refused combinations.
5. Compatibility-spelling provenance: independent parked work.
6. Defer: review the conservative refusals and compatibility impact first.

No language syntax, aggregate execution, backend semantics, Igor commands,
compatibility-spelling rules, or subsequent phase were implemented.

GATE 6: WAITING FOR HUMAN REVIEW
