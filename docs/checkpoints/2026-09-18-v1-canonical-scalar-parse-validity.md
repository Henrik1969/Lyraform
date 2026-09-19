# Mission 05 — canonical scalar parse completeness and validity

Date: 2026-09-18. Authority: Phase 5 only. Gate 5 review checkpoint.
This is parser/admission hardening, not a language extension or safety certification.

## Baseline and worktree

Initial and final `main` HEAD and locally recorded `origin/main`:
`c03079e46e938e98b04f3bad21c991cc4f58eee4`.
The implementation was performed on the existing worktree, not a fresh clone.
The inherited Phase 2/3/4 work comprised 10 modified tracked paths and 13
untracked paths. It was preserved; no reset, staging, commit or push was done.
Final worktree: 12 modified tracked paths and 18 untracked paths, including this
checkpoint. `master` and `flowlfs-v0.1-alive` were not touched.

Phase 5 adds the parser-validity header, shared artifact-evidence validator,
two test scripts and this checkpoint. It updates the AST result, builder,
bundle exporter, direct admission, Flowanalyst, shared plan validation and
CTest registration. The inherited scalar adapter loses its independent
coverage implementation. The Phase 4 probe script gains only an opt-in
fixtures-only mode; its original inputs and historical observations remain.

## Implemented invariant and ownership

`Lyraform/compiler/include/flowmini_parse_validity.h` owns the former Mission
03 token/AST coverage comparison. `build_source_header_ast` finalizes validity
before returning its recognized program. Canonical scalar admission requires
`canonical_valid`, then the unchanged shared scalar semantic admission.

The bounded comparison checks ordered meaningful tokens against the AST,
balanced delimiters, initializer delimiter spans, AST expression grouping
spans, exactly one parameterless nonempty main, and complete statement
representation. It rejects valid prefixes with meaningful leftover source.
Whitespace, comments and represented grouping may disappear. It does not
reparse source into a second AST. No backend decides source validity.

Whole-source canonical ownership remains the Mission 03 straight-line
`int`/`Bool` initialized declarations, selected expressions, identifier
placements and `print identifier` subset. There is no new grammar or target
semantics. Unknown structural statements are explicit recovery, not success.

The return-observed staged regression requires coverage of the same scalar
body with the existing return spelling. That inspection envelope is separately
labelled `scalar_with_compatibility_return`; a successful result is
`outside_scope`, not `canonical_valid`. Direct canonical runtime ownership
does not gain return statements. It closes the staged suffix-loss witness
without claiming return semantics have migrated.

Empty main retains its existing staged compatibility lowering, with
`outside_scope / compatibility / unassessed`. Direct scalar execution still
refuses it, as Mission 03 did. Other compatibility constructs do not acquire
source completeness proof. Unknown/legacy source headers return unassessed
compatibility results, not canonical proof.

## Validity and recovery representation

The AST result contains `ParseValidity`. Its frontend projection is an additive
optional independently versioned object in the existing bundle v2:

```json
{"format":"lyraform.parse_validity","version":1,
 "state":"canonical_valid","scope":"canonical_scalar",
 "coverage":"complete","recovery_used":false,"message":""}
```

States:

- `canonical_valid`: complete owned syntax, no recovery. Type validity remains
  a separate semantic question; `int(true)` is not thereby semantically valid.
- `recovered`: an `UnknownStatement` was needed; it carries the recovery reason.
- `incomplete`: missing structure, such as unclosed delimiters or operands.
- `invalid`: unexpected or unrepresented meaningful syntax.
- `outside_scope`: compatibility only, not canonical proof.

Coverage is `complete`, `incomplete` or `unassessed`. The bounded classifier
currently distinguishes incomplete from invalid using the coverage check's
diagnostic reasons; neither can execute. Recovery outside this bounded policy
is not comprehensively redesigned. Export/inspection of refused ASTs remains
possible and deterministic; export success does not mean executable source.

## Staged admission and compatibility import policy

Flowanalyst checks explicit evidence through
`Flowcontracts/include/flowcontracts/parse_validity.hpp`. Negative evidence
produces `FLOWMINI_PARSE_NOT_EXECUTABLE`, semantic status `error`, exit 2 and
a blocked lowering plan. Malformed or contradictory evidence is an input
contract error. Evidence is copied unchanged into `lowering_plan.parse_validity`.

Existing shared plan validation, via `scalar_facts.hpp`, enforces retained
evidence at binding, planning, optimization, preparation, independent validation,
LLVM and TinyVM boundaries. Forging only outer `ok`/`ready` does not override
negative evidence. No executable output is published from the tested negative
prepared artifacts. The direct runtime checks the same parser result before
adaptation or legacy fallback.

Historical bundles with no evidence remain admissible under existing policy.
Flowanalyst does not synthesize proof for them. This is explicitly unproven
compatibility, not authenticated source provenance. Removing all evidence or
consistently replacing an artifact remains outside this assurance claim;
mandatory evidence and cryptographic provenance require another policy gate.
The validator checks detectable evidence/AST-shape contradictions, not every
possible forgery or a reconstruction of original source.

## Mission 03 guard status

Moved, not stacked: its essential comparison exists only in the parser-owned
header. The scalar adapter consumes validity and shared semantic facts; it no
longer independently reconstructs completeness. Flowanalyst validates the
projected result rather than reverse-engineering source completeness.

## Critical regression and focused evidence

Both direct and return-observed staged versions of:

```lyraform
x : int(0)
20 -> x : Bool
```

now export `invalid / incomplete` coverage with the explanation that source
tokens are not completely represented. Flowanalyst blocks the plan; direct
execution refuses before adapter/fallback. No typed-placement meaning is
assigned to the suffix. Backend hostile-artifact tests additionally verify
that retained negative evidence cannot publish LLVM or TinyVM output.

`lyraform_scalar_parse_validity` covers 12 refusals: typed suffix (two
observation forms), trailing declaration tokens, extra initializer literal,
extra placement identifier, unknown tokens, stray nested block, missing and
extra delimiters, malformed print tail, incomplete expression, and recovered
incomplete `if`. Each tests no ready plan and no runtime adapter/fallback.

Five positive cases explicitly check canonical validity and execution:
integer initialization, Bool initialization, identifier initialization,
repeated placement and grouped arithmetic. The inherited source-path suite
continues direct/oracle, LLVM/TinyVM, semantic refusal, provenance and
compatibility tests. Repeated exports and analysis are byte-compared.
Empty-main staged compatibility and direct refusal are separately asserted.

Hostile cases: nine frontend inputs (unknown version/state, contradictory
coverage/recovery/message, extra field, recovery AST marker and duplicate
JSON evidence key); three prepared artifacts (unsupported version, recovered,
invalid), each refused by independent validation and both backend lowerers.
Historical missing-evidence import is tested as successful but unproven.

## Preserved projection-collision corpus

The 68 original Phase 4 inputs remain in the reconnaissance script. The new
`lyraform_scalar_parse_collisions` test invokes its fixtures-only mode, verifies
all 23 original normalized AST/graph pairs still match, and checks their
separate validity/admission dispositions. Normalization intentionally excludes
validity; full bundles now distinguish the relevant refusals.

`UNDECIDED` means Phase 5 does not adjudicate legality/normalization in that
out-of-scope grammar. It is not an endorsement of silent loss. Distinct
projection candidates remain deferred, not implemented.

| Original pair | Classification |
| --- | --- |
| scalar / decl_tail | CANONICAL_INVALID |
| scalar / literal_tail | CANONICAL_INVALID |
| scalar / unknown | CANONICAL_INVALID |
| scalar / nested_block | CANONICAL_INVALID |
| place / place_type | CANONICAL_INVALID |
| place / place_junk | CANONICAL_INVALID |
| return_keyword / return_tail | CANONICAL_INVALID |
| member / member_dot | UNDECIDED |
| member / member_index | REQUIRES_DISTINCT_PROJECTION |
| index / index_missing | UNDECIDED |
| index / index_field | REQUIRES_DISTINCT_PROJECTION |
| index / index_again | REQUIRES_DISTINCT_PROJECTION |
| type_bare / type_field | EXPECTED_NORMALIZATION |
| type_bare / type_tail | UNDECIDED |
| if_ok / if_tail | UNDECIDED |
| while_ok / while_tail | UNDECIDED |
| function / parameter_gap | UNDECIDED |
| function / call_gap | UNDECIDED |
| list / list_gap | UNDECIDED |
| record_literal / record_literal_bad | UNDECIDED |
| import_ok / import_tail | UNDECIDED |
| target / target_extra | REQUIRES_DISTINCT_PROJECTION |
| abi / abi_unknown | UNDECIDED |

Totals: 7 CANONICAL_INVALID, 4 REQUIRES_DISTINCT_PROJECTION,
1 EXPECTED_NORMALIZATION, 11 UNDECIDED, 0 SEMANTICALLY_IRRELEVANT.
All seven loss variants are blocked; all remaining variants assert that
they do not claim canonical scalar validity. Historical `field` normalization
is retained without settling compatibility-spelling provenance.

## Verification

Normal GCC 13.3 build directory: `/tmp/lyraform-phase2-build`. Configure:
`cmake -S . -B /tmp/lyraform-phase2-build -G Ninja`, exit 0. Final commands:

```sh
LYRAFORM_BUILD_DIR=/tmp/lyraform-phase2-build CMAKE_BUILD_PARALLEL_LEVEL=4 ./igor doctor
LYRAFORM_BUILD_DIR=/tmp/lyraform-phase2-build CMAKE_BUILD_PARALLEL_LEVEL=4 ./igor build
LYRAFORM_BUILD_DIR=/tmp/lyraform-phase2-build CMAKE_BUILD_PARALLEL_LEVEL=4 ./igor test
```

Doctor PASS, build exit 0, canonical CTest **161/161 PASS**, 65.18 seconds.
This includes all five scalar suites (semantics, authority, source path,
parse validity and parse collisions). Logs:
`/tmp/lyraform-phase5-configure.log`, `/tmp/lyraform-phase5-doctor.log`,
`/tmp/lyraform-phase5-igor-build.log`,
`/tmp/lyraform-phase5-igor-test-final.log`.

The first full run was 160/161: `flowlower_pipeline` exposed the empty-main
compatibility regression. Its previous behavior was restored without granting
canonical proof, a focused assertion was added, and the full suite rerun.
The initial failure is retained in `/tmp/lyraform-phase5-igor-test.log`.

Clang 18.1.3 Debug ASan/UBSan, `/tmp/lyraform-phase2-asan`:
`-fsanitize=address,undefined -fno-omit-frame-pointer`, linker
`-fsanitize=address,undefined`. Rebuilt changed compiler and stage consumers:
`flowmini`, `flowmini_scalar_path_test`, `flowanalyst`,
`lyraform_scalar_semantics_test`, `flowbind`, `flowparallel`, `flowoptimize`,
`flowprepare`, `flowlower`, `flowtinylower`, `flowtinyrun`, `flowvalidate`.
Ran:

```sh
ASAN_OPTIONS=detect_leaks=0:verify_asan_link_order=0 \
UBSAN_OPTIONS=halt_on_error=1 \
ctest --test-dir /tmp/lyraform-phase2-asan -R '^lyraform_scalar_' --output-on-failure
```

**5/5 PASS**, 12.16 seconds. Logs:
`/tmp/lyraform-phase5-asan-build-final.log`,
`/tmp/lyraform-phase5-asan-test-final.log`.
This is focused instrumented coverage, not a full sanitizer run of 161 tests;
LeakSanitizer was disabled. Valgrind supplies the following limited leak checks.

Valgrind 3.22.0, final normal direct-path test binary, `--test-forbid-legacy`,
`--error-exitcode=99 --leak-check=full --errors-for-leak-kinds=definite,indirect`:

- Integer identifier initialization: output `20`, exit 0.
- Bool identifier initialization and `not` placement: output `false`, exit 0.
- Typed suffix with return observation: parser refusal, expected exit 1;
  no runtime adaptation or fallback.

All three: **0 errors, all heap blocks freed**. Logs:
`/tmp/lyraform-phase5-valgrind-int.log`,
`/tmp/lyraform-phase5-valgrind-bool.log`,
`/tmp/lyraform-phase5-valgrind-refusal.log`.

Verification uses rebuilt existing external build directories, not a fresh
clone or cross-platform qualification. No safety certification is claimed.

## Authority accounting

| Question | Before Mission 05 | After Mission 05 |
| --- | --- | --- |
| Structural recognition | Structural parser | Structural parser |
| Entire owned source consumed | Direct scalar guard; staged gap | Parser-owned validity result |
| Recovery used | No shared execution boundary | Explicit recovery state and admission refusal |
| Semantic validity | Shared scalar semantic authority | Same shared scalar semantic authority |
| Execution admission | Multiple defensive conditions | Canonical parse validity plus semantic admission; backend support may still refuse |

## Remaining gaps and next options (not authorized)

This is a bounded token/AST correspondence proof, not universal parser
completeness. It does not add original byte-range provenance, change source
mapping, or make every compatibility parse trustworthy. Arbitrary historical
artifacts still lack source-completeness proof. Existing backend operator
support limitations, including LLVM `not`, remain.

1. Target-specific parser completeness hardening is the strongest next bounded
   safety candidate: refuse incomplete target syntax before execution.
2. Typed assignable-target preservation could retain member/index chains;
   preserve structure first, without inventing mutation semantics.
3. Compatibility-spelling provenance could distinguish historical `field`
   from bare members without retiring either spelling.
4. Broader recovery/validity rollout needs separately bounded ownership and
   compatibility decisions for functions, control flow and aggregates.
5. More reconnaissance is appropriate before selecting any of those grammars.
6. Defer is valid: review this patch and its compatibility boundary first.

No subsequent phase, target hardening, new syntax, Igor redesign, self-hosting,
backend semantics changes, commit or push was undertaken.

GATE 5: WAITING FOR HUMAN REVIEW
