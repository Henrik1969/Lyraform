# Mission 03 — canonical scalar source path

Date: 2026-09-18. Authority: Phase 3 only. Stop: Gate 3.

## Baseline and worktree

Initial and current `main` HEAD:
`c03079e46e938e98b04f3bad21c991cc4f58eee4`.
An initial `git fetch origin` and a final `git ls-remote` confirmed the same
public main. One worktree was present. Mission 02 was already implemented but
uncommitted: seven modified tracked files and seven untracked files. Those
changes were preserved as the invoked mission's prerequisite, not mistaken
for a clean published baseline. The recorded Mission 02 gate was 158/158;
that is prior evidence, not a fresh baseline test run in this mission.

Final worktree: ten modified tracked files and eleven untracked files,
including the inherited Mission 02 changes and this checkpoint; nothing staged.
Mission 03 is also uncommitted. No staging, commit, push, branch change,
history rewrite, or subsequent phase was performed. Only local `main` exists;
the historical branches are remote-tracking refs, not local branches:

| Public branch | Verified SHA |
| --- | --- |
| main | c03079e46e938e98b04f3bad21c991cc4f58eee4 |
| master | 95d1d592714c301fd954901a1bf208a6622f5d63 |
| flowlfs-v0.1-alive | 7115ef366098a04c7cd83c516dccec36b5bc1c0e |

`master` and FlowLFS were not modified, merged, or copied.

## Migrated source forms and ownership

The migrated **whole-source** unit is a `program` with exactly one
parameterless `main` and a nonempty straight-line body containing:

- explicitly initialized `int`/`Bool` declarations;
- literal or established scalar identifier initializers;
- scalar identifier destinations and `->` placement;
- expressions governed by Mission 02's scalar operator algebra;
- existing `print identifier` as the observation boundary.

One scalar statement per source line is admitted by this bounded direct path
(a single statement may be inline with a brace). This does not migrate scalar
statements embedded in arbitrary non-migrated programs.

Ownership is chosen from constructs **before** scalar analysis/lowering.
Functions, control flow, returns, assignment syntax, non-core types, calls,
field/index targets, aggregates, lists, arrays, graph constructs, units, and
non-identifier print expressions remain whole-source compatibility territory.
The `module` header identifies compatibility runtime IR. A `.flowir` filename
alone does not redirect scalar source into legacy parsing.

Within the canonical slice, missing initialization, undecided typed placement,
unknown/unrepresented syntax, missing facts, unresolved identifiers, invalid
types, and unsupported runtime lowering are refusals. There is no catch/retry
through `parseModule`. Mixed compatibility programs still have legacy scalar
interpretation; this checkpoint does **not** claim full parser unification.

## Architecture and changed components

`Lyraform/compiler/include/flowmini_scalar_source.h` contains explicit ownership,
source-coverage verification, a bounded canonical model projection, and the
runtime representation adapter. `src/main.cpp` builds the structural AST once,
then its existing symbol/frontend bundle projection. The bundle is read as
structured JSON in memory, not converted back into source. No subprocess or
legacy source parser participates in this canonical path.

The Mission 02 fact analyzer moved without changing its scalar rules into
`Flowcontracts/include/flowcontracts/scalar_analysis.hpp`. It now optionally
exposes its inferred expression types. `Flowanalyst/src/scalar_analysis.hpp`
is a forwarding header. Flowanalyst and the direct model use the same lexical
lookup and the same fact analyzer. The adapter neither resolves source names
nor re-infers operator types.

The adapter consumes AST expression structure, resolved symbol IDs, admitted
declaration/placement facts, and inferred expression types. It produces
ordinary `ModuleSpec` nodes, policies, and sequential wires using existing
`start.record`, `const.int`, `const.bool`, `record.copy`, arithmetic/comparison,
Bool, stdout, and `halt.record` atoms. Every initialized destination receives
an explicit copy, even when the initializer is an identifier. Runtime policy,
graph execution, and the legacy declaration-lowering bug are unchanged.

Other Phase 3 changes:

- `Lyraform/compiler/CMakeLists.txt`: test-only source-path/oracle executable.
- `Lyraform/compiler/src/flowmini_ast_builder.cpp`: test-build-only assertion
  against a second structural parse in one process; no production grammar change.
- `Flowanalyst/src/main.cpp`: shared lexical lookup delegation.
- `Flowanalyst/CMakeLists.txt`: source-path CTest and explicit legacy oracle input.
- `Flowanalyst/tests/run-scalar-authority-tests.sh`: use the test-only legacy
  oracle instead of mislabeling the now-canonical default runtime as legacy.
- `Lyraform/compiler/tools/run-scalar-source-path-tests.sh`: new evidence corpus.
- This checkpoint. Other dirty files remain the inherited Mission 02 work.

## Authority before and after

| Question, within the migrated whole-source slice | Before | After |
| --- | --- | --- |
| Declaration/initializer grammar and expression structure | structural AST plus legacy source parser | structural AST once, with losslessness admission check |
| Scalar name identity | staged lookup plus legacy scope interpretation | canonical projection IDs and shared lexical lookup |
| Initializer/placement compatibility and operator-result types | shared rules used by two interpretations | shared fact analyzer and scalar algebra; facts consumed directly |
| Destination initialization | legacy declaration lowering | copy from admitted expression identity to canonical destination identity |
| Runtime execution | existing runtime | same runtime, fed by representation adapter |

## Source completeness finding

The structural frontend includes recovery/shell behavior: for example,
`20 -> x : int` can leave an identifier-placement AST while discarding the
suffix. A valid-looking AST alone therefore cannot safely authorize execution.

For the selected slice, a token-projection comparison verifies that the AST
accounts for the input. It checks delimiter balance, initializer delimiters,
expression-parenthesis spans, the main signature, statement boundaries, and
otherwise discarded tokens. It constructs no new AST, does not resolve types,
and never reparses projected text. Cases with trailing tokens, empty extra
parentheses, duplicated main signatures, and missing delimiters are refused.
This is a bounded admission safeguard, **not** a general structural-parser
completeness proof. Broader coverage belongs to another reviewed slice.

## Provenance

The adapter retains a node-to-origin side table with statement ID, declaration
statement ID, source expression ID, destination symbol ID, original mapped
file/line/column, and AST path. Runtime paths use canonical numeric identities,
not reconstructed source names. The side table remains alive through execution
and is inspectable with existing `--trace true`; normal output is unchanged.

Losses are explicit: `ModuleSpec` and legacy textual FlowIR have no full source
provenance field. Runtime IR export/reimport preserves behavior but not this
side table. Existing runtime diagnostics identify runtime nodes; they do not
automatically render the full canonical origin. Synthetic start/halt nodes
have no scalar statement origin. No public schema or extension was renamed.

## Regression, oracle, and parity evidence

The new CTest has 12 direct/oracle cases:

- nine `AGREES` cases, including literals, repeated placement, int/Bool
  identifier placement, arithmetic, grouping, comparisons, Bool equality, and `not`;
- three `KNOWN_LEGACY_DEFECT` cases: int identifier initialization, Bool
  identifier initialization, and preservation of the initialized copy when
  the original int destination changes afterward.

For `a : int(20); b : int(a)`, the canonical adapter prints `20`; the explicit
legacy oracle still reports a missing record path for `b`. The legacy defect
was bypassed, not patched or adopted as normative semantics.

Eleven scalar bodies agree across direct runtime, LLVM, and TinyVM, each
through lowering-plan versions 1 and 2: 22 LLVM executions and 22 TinyVM
executions. Direct execution observes `print identifier`; modern backends use
an existing integer-return observation, with an existing conditional for Bool
observations. This does not migrate conditionals into the direct adapter.

The twelfth direct case exercises existing Bool `not`. LLVM `not` was not
implemented, and this case makes no three-backend parity claim. Other admitted
operators with no existing runtime atom mapping (`<=`, `>=`, `!=`) remain
runtime-backend refusals. The test exercises `<=` explicitly. Runtime integer
literals also remain limited by the existing checked `int` policy carrier.

All 23 negative source cases refuse **before adapter entry** and produce no
runtime IR file or stdout. The legacy oracle also refuses all 23 (`AGREES`),
covering Bool/int mismatches in both directions, self/forward/missing names,
duplicate declarations, uninitialized/typed-placement exclusions, and malformed
source coverage. The separately admitted-but-unimplemented `<=` case refuses
in adaptation before runtime execution, without legacy retry.

`--test-forbid-legacy` exists only in the test executable and fails any attempt
to invoke the legacy parser. A separate test-build assertion fails a second
structural parse. Tests prove that the explicit compatibility control-flow
case does invoke the compatibility path and is rejected by the forbid hook.
The production executable exposes no oracle/bypass switch.

## Stage-border evidence

Tests independently invoke frontend export, Flowanalyst, Flowbind,
Flowparallel, Flowoptimize, flowprepare, flowvalidate, LLVM, and TinyVM. They
save/reimport artifacts and compare scalar facts after canonicalization of
semantic, execution, optimization, and lowering artifacts. The Phase 2 hostile
artifact/source-map corpus remains active. Ordinary runtime FlowIR export and
reimport of the identifier-initializer regression also yields `20`.

No stages were collapsed into the adapter. Optional compatibility-artifact
proof policy, artifact versions, and backend admission contracts are unchanged.

## Verification

Final gates used the external build directory `/tmp/lyraform-phase2-build`,
reconfigured for the updated graph, with GCC 13.3.0 on Linux x86-64. This was
an incremental external build, not a fresh-clone reproducibility claim.

| Gate | Final result |
| --- | --- |
| `LYRAFORM_BUILD_DIR=/tmp/lyraform-phase2-build ./igor doctor` | PASS |
| `LYRAFORM_BUILD_DIR=/tmp/lyraform-phase2-build CMAKE_BUILD_PARALLEL_LEVEL=4 ./igor build` | PASS |
| `LYRAFORM_BUILD_DIR=/tmp/lyraform-phase2-build ./igor test` | **159/159 PASS**, 67.22 seconds |
| Focused scalar-source CTest, verbose | **1/1 PASS**, 4.20 seconds; 12 direct cases, 11 parity bodies, 23 pre-adapter refusals |
| Clang 18.1.3 ASan/UBSan scalar CTests | **3/3 PASS**, 8.86 seconds |
| Valgrind 3.22.0, int initializer/copy path | output `20`; 0 errors, 0 bytes live at exit |
| Valgrind 3.22.0, Bool initializer and `not` path | output `false`; 0 errors, 0 bytes live at exit |
| Valgrind 3.22.0, Bool-to-int refusal | expected exit 1; 0 errors, 0 bytes live at exit |
| `git diff --check` | PASS |

Sanitizer build: `/tmp/lyraform-phase2-asan`, Debug, Clang 18.1.3,
`-fsanitize=address,undefined -fno-omit-frame-pointer`, matching executable/shared
linker sanitizer flags. Changed compiler/test-oracle/analyzer targets were
rebuilt. The focused corpus invokes the instrumented stage tools from the
Mission 02 build as well. Environment:
`ASAN_OPTIONS=detect_leaks=0:verify_asan_link_order=0`,
`UBSAN_OPTIONS=halt_on_error=1`. LeakSanitizer was disabled; the full 159-test
suite was **not** run under sanitizers. Valgrind separately used
`--error-exitcode=99 --leak-check=full --errors-for-leak-kinds=definite,indirect`
on the final test-only direct-path executable.

Local logs (ephemeral, not committed artifacts):
`/tmp/lyraform-phase3-build.log`, `/tmp/lyraform-phase3-final-tests.log`,
`/tmp/lyraform-phase3-asan-build.log`, `/tmp/lyraform-phase3-asan-tests.log`,
and `/tmp/lyraform-phase3-valgrind-{int,bool,invalid}.log`.

An intermediate integration run failed the newly added empty-main test against
the pre-rebuild executable. The final rebuilt code and complete corpus passed
the gates above. No failing canonical test is being waived at Gate 3.

## Remaining authority and next options (not implemented)

The compatibility parser still owns the non-migrated whole-source territory
listed above, including its known identifier-initializer defect in mixed
programs. Structural source completeness outside this slice, aggregate target
identity, general activation, and legacy artifact proof policy remain open.

1. **Preserve typed assignable-target identity:** a useful next structural
   step before member/index semantics; no automatic admission of new targets.
2. **Member-target semantic resolution:** defer until target identity and
   aggregate contracts can be carried and tested without loss.
3. **Another scalar/function subset:** possible, but control-flow/function
   ownership needs explicit scope, activation, and parity evidence first.
4. **Compatibility-artifact proof policy:** a separate versioned compatibility
   decision, not a side effect of source-path convergence.
5. **More reconnaissance:** recommended focused audit of structural source
   completeness before broadening direct execution.
6. **Defer:** safe; keep this small whole-source boundary explicit.

No new syntax, aggregate semantics, LLVM `not`, runtime redesign, Igor redesign,
mandatory artifact proofs, documentation consolidation, or self-hosting work
was undertaken. No subsequent phase is authorized.

## Gate status

GATE 3: WAITING FOR HUMAN REVIEW
