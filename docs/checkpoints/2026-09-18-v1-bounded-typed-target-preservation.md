# Mission 07 — bounded typed assignable-target preservation

Date: 2026-09-18. Phase 7 only. Stop at Gate 7.

## Baseline and scope

Initial and current `main` HEAD and locally recorded `origin/main`:
`c03079e46e938e98b04f3bad21c991cc4f58eee4`.
Inherited work: 12 modified tracked files and 20 untracked files from Missions
02–06. All preserved; no staging, commit, push or branch changes.

Phase 7 covers identifier, ordered field-path and one-group indexed targets
already parsed by Mission 06. Mixed/chained forms remain refused by that parser.
No target mutation, member lookup, index typing, bounds or new syntax was added.

## Semantic model and single producer

`Flowanalyst/src/target_analysis.hpp` is the sole target-fact producer. It uses
Flowanalyst's existing shared lexical lookup and established symbol type map,
not a second target resolver in each backend. Every projected operation whose
origin is a placement receives an additive `target_fact`:

```text
format: lyraform.target_fact
version: 1
kind: identifier | field_path | indexed
operation_id, statement_id, base_symbol_id
base_type: known declared type spelling or null
destination_type: proven scalar int/Bool or null
resolution: resolved | partially_resolved | unsupported_semantics
execution: existing_policy | unsupported
members: ordered {ordinal, name, ast_path, location}
indices: ordered {ordinal, expression_id}
index_count
provenance: {source, line, column, ast_path}
```

All identities are local to the captured frontend/plan, not permanent global
IDs across compilations. `base_symbol_id` denotes the resolved declaration's
symbol identity. An unresolved base is explicitly -1/unsupported_semantics;
it is not invented. Statement and operation IDs link the target to its placement.

For identifiers, known destination type is taken from the existing shared
scalar fact; that compatibility evidence remains alongside the target fact.
There is no implicit Bool/int conversion or change to scalar admission.
Identifier facts outside that proven scalar case do not acquire a guessed
destination type; their existing admission policy remains authoritative.

For fields, the base declaration and its declared type survive alongside every
member segment and its AST/source location. Member semantic identities and
result types are deliberately unresolved: no member lookup has migrated.
Repeated member names may be legitimate (`a.a`); ordinal identity, not name
uniqueness, distinguishes segments. Duplicate/conflicting ordinals are refused.

For indices, the base declaration/container type survives with every ordered
index expression identity. Index count is syntax rank only. For example,
`array<int>[2,2]` is preserved as the already established declared base type,
without declaring rank compatibility, index type validity, bounds or resulting
element type. No expression is reduced to its textual spelling.

`resolved` identifies the retained scalar destination fact, not blanket
assignability or execution authorization. Member/index facts are
`partially_resolved` with `destination_type=null` and `execution=unsupported`.
No separate parse-validity taxonomy was introduced.

## Provenance

Target provenance retains the source-map-adjusted source file, line, target
column and `/statement_pool/N/payload/target` identity. Member segments retain
their frontend AST paths and expanded-source locations. Index expression IDs
refer to the captured frontend expression pool in original order. Operation
identity is explicit. Original full end spans and universal cross-artifact
identity catalogs are not invented in this phase.

## Artifact policy and stage preservation

The target fact is independently versioned and optional, following the existing
additive lowering-plan v1/v2 policy. Existing schema identifiers and envelope
versions remain unchanged. Historical artifacts without target facts remain
compatible but supply no Mission 07 target-preservation proof. Evidence is not
authenticated: complete removal or consistent rewriting is not prevented by
this optional compatibility contract.

| Boundary | Behavior |
| --- | --- |
| Flowanalyst | Produces structured target facts on placement-derived operations |
| Flowbind | Validates incoming facts; retains exact facts in additive root `target_facts` array |
| Flowparallel | Preserves the lowering plan and target facts |
| Flowoptimize | Preserves the lowering plan and target facts |
| flowprepare | Captures the same facts in the backend artifact's lowering plan |
| Independent flowvalidate | Validates shape, type/provenance consistency and operation linkage |
| LLVM / TinyVM | Invoke the same target execution guard; refuse member/index facts before output publication |

Flowbind is an authorization side branch, not a replacement semantic-plan
producer: its historical `lowering_plan` field is a summary. The additive
`target_facts` array prevents that summary from erasing the inspected target
identity. Scheduling continues from the semantic plan, and backend preparation
preserves that plan; no target meaning is reconstructed from binding output.

The existing plan's `ready` status alone is not execution authorization for
newly preserved targets. The explicit target execution policy and backend
guard prevent an unsupported member/index target from becoming assignment to
its base. A prepared artifact may therefore be structurally valid while its
target semantics remain unsupported for execution.

## Validation and refusal

`Flowcontracts/include/flowcontracts/target_facts.hpp` is the shared validator
and backend refusal boundary. Existing shared scalar/plan validators invoke
it, so binding, planning, optimization, preparation and independent validation
check retained facts. Checks include known format/version/kind, required base
identity, positive source coordinates, explicit AST provenance, sequential
member/index ordinals, index count, kind-specific fields, resolution/type
consistency, operation/statement linkage and scalar-fact consistency.

A non-identifier fact cannot also claim a collapsed `result_symbol_id`.
LLVM and TinyVM call `require_executable_targets` and report
`unsupported target kind: member/index execution is not admitted`. The new
guard adds no backend mutation semantics.

The direct Mission 03 runtime adapter is unchanged and does not consume these
staged facts or gain member/index execution. Mission 06 mixed targets still
fail before adapter or legacy fallback. Structurally supported non-scalar
source retains its existing explicit compatibility ownership, not newly
canonical runtime semantics.

## Regression and hostile-artifact evidence

`lyraform_target_preservation` uses a source fixture with actual declared record,
list and array bases and the five required shapes: `x`, `p.x`, `p.a.b`,
`items[i]`, `matrix[r,c]`. For each lowering-plan version 1 and 2 it asserts:

- exactly five structured facts with the expected distinct kinds;
- scalar destination type and resolution unchanged;
- both ordered member segments retained;
- indexed expression IDs exactly match the frontend target's ordered IDs;
- field/index bases resolve, result types remain null, execution unsupported;
- repeated analysis is byte-identical;
- binding, scheduling, optimization and backend capture preserve facts exactly;
- independent stage artifact validation succeeds;
- both backend lowerers explicitly refuse and publish no executable output.

Ten hostile mutations exercise unknown kind, missing base identity, duplicate
member ordinal, missing ordered index identity, invalid index count, invented
destination type, malformed provenance, kind/field contradiction, unknown
version and conflicting operation ID. Independent validation and both backend
lowerers refuse all of them. Existing scalar suites cover compatibility facts,
direct no-fallback behavior, LLVM/TinyVM parity and historical imports. The
Mission 06 suite retains all 27 target refusals, including mixed/chained forms.

## Verification and final worktree

Normal GCC 13.3 build: `/tmp/lyraform-phase2-build`; CMake/Ninja configure and
build succeeded. Focused scalar/target tests: **7/7 PASS**, 6.50 seconds.
The target-semantic suite covers five shapes across two plan versions and ten
hostile mutations. Final canonical suite: **163/163 PASS**, 73.59 seconds,
including all seven focused suites and both updated pass-corpus registrations.

Commands use `LYRAFORM_BUILD_DIR=/tmp/lyraform-phase2-build` and
`CMAKE_BUILD_PARALLEL_LEVEL=3`: `./igor doctor`, `./igor build`, `./igor test`.
Doctor PASS; build exit 0. Logs:
`/tmp/lyraform-phase7-doctor.log`, `/tmp/lyraform-phase7-igor-build.log`,
`/tmp/lyraform-phase7-igor-test-final.log`.

The first full run reported 161/163: both registrations of
`flowcore_pass_corpus` expected every report-only lowering to be ready, including
previously structure-losing member/index placements. The corpus now checks the
new explicit unsupported-target refusal when the retained target fact requires
it, preserving semantic/binding checks and readiness assertions for all other
inputs. It uses generic fact classification, not fixture-name dispatch. The
final corpus checks 97 programs: 9 explicitly refused member/index targets and
88 ready lowering reports, in each of its two CTest registrations. This is an
intentional admission tightening, not new member/index execution.
The
initial result is retained in `/tmp/lyraform-phase7-igor-test.log`.

Development also encountered and corrected JSON-helper signature errors and a
C++ initializer delimiter error. A preliminary pipeline probe ran against the
previous analyzer while compilation was unfinished and is not preservation
evidence. The new explicit five-fact assertions passed only after rebuilding
the producer; the final test results supersede that preliminary probe.

Clang 18.1.3 Debug ASan/UBSan build `/tmp/lyraform-phase2-asan`, flags
`-fsanitize=address,undefined -fno-omit-frame-pointer`, linker
`-fsanitize=address,undefined`. Rebuilt the affected analyzer, binding,
validation, planning, optimization, preparation and backend consumers, with
the existing instrumented compiler/runtime test tools. Ran:

```sh
ASAN_OPTIONS=detect_leaks=0:verify_asan_link_order=0 \
UBSAN_OPTIONS=halt_on_error=1 \
ctest --test-dir /tmp/lyraform-phase2-asan -R '^lyraform_(scalar|target)_' --output-on-failure
```

**7/7 PASS**, 15.78 seconds; target preservation 0.89 seconds. Logs:
`/tmp/lyraform-phase7-asan-build.log`,
`/tmp/lyraform-phase7-asan-test-final.log`. Focused coverage only, not the full
canonical suite under sanitizers. LeakSanitizer was disabled.

Valgrind 3.22.0 with full leak checking, definite/indirect leak errors and
`--error-exitcode=99`: analyzer export and binding each exit 0; LLVM and TinyVM
each exit 1 as expected for explicit unsupported-target refusal. All four:
**0 errors; all heap blocks freed**. Neither refused executable output exists.
Logs: `/tmp/lyraform-phase7-valgrind-analyst.log`,
`/tmp/lyraform-phase7-valgrind-bind.log`,
`/tmp/lyraform-phase7-valgrind-llvm.log`,
`/tmp/lyraform-phase7-valgrind-tiny.log`.

Final worktree: 15 modified tracked paths and 25 untracked paths, including
inherited changes and this checkpoint. HEAD unchanged; no staged changes,
commits or pushes. `git diff --check` passed. External build directories were
rebuilt; this was not a fresh-clone or cross-platform qualification.

## Authority before and after

| Question | Before Phase 7 | After Phase 7 |
| --- | --- | --- |
| Which target syntax was written? | Structural parser | Same parser |
| Was it completely parsed? | Mission 05/06 validity | Same validity boundary |
| Which target identity survives analysis? | Scalar identifier mostly; member/index structure lost | One canonical target-fact producer with explicit partial resolution |
| Is it assignable? | Scalar authority plus legacy/backend limits | Same scalar authority; member/index assignability deferred |
| Can it execute? | Canonical scalar identifiers | Same scalar behavior; preserved member/index targets explicitly unsupported |

## Remaining gaps and next options

Member existence and result typing, index carrier typing, rank semantics,
bounds, assignability, ownership, aliasing and mutation/copy semantics remain
unimplemented here. Mixed targets remain refused. Compatibility spelling is
unchanged. Imported facts do not provide cryptographic authenticity or a
complete independent semantic reconstruction from arbitrary rewritten evidence.

Readiness: supported simple targets now retain enough structured identity to
begin separately authorized semantic resolution. They are not thereby ready
for member/index writes.

1. Member-target semantic resolution: next bounded candidate, beginning with
   explicit declaration/member identity and result typing, without mutation.
2. Indexed-target semantic resolution: needs a separate index/container/rank
   policy before any result-type or bounds claim.
3. Target assignability/admission: follows identity/type resolution, not merely
   successful serialization.
4. Aggregate ownership/copy reconnaissance: needed before mutation semantics.
5. Mixed-target grammar reconnaissance: remains prerequisite to generalized
   target syntax, not something this fact representation authorizes.
6. Defer: review this preservation contract before extending semantics.

No subsequent phase, self-hosting, new syntax, Igor redesign or target mutation
was undertaken. `master` and FlowLFS were untouched.

GATE 7: WAITING FOR HUMAN REVIEW
