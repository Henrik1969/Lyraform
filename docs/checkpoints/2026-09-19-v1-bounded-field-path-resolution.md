# Mission 08 — bounded field-path semantic resolution

Date: 2026-09-19. Phase 8 only. Stop at Gate 8.

## Baseline

Initial and current `main` HEAD and locally recorded `origin/main`:
`c03079e46e938e98b04f3bad21c991cc4f58eee4`.

The worktree began with 15 modified tracked paths and 25 untracked paths from
Missions 02–07. Those changes were preserved. No files were staged, no commit
or push was made, and no branch was changed. `master` and FlowLFS were not
touched.

Phase 8 covers only structurally complete plain field paths already admitted by
the Mission 06 parser and preserved by the Mission 07 target fact. It adds no
syntax, member write, index, bounds, ownership, aliasing, initialization,
mutation or aggregate copy rule.

## Aggregate/member semantic source

`Flowanalyst/src/target_analysis.hpp` builds one bounded aggregate catalog from
the canonical structural frontend bundle:

- each `Struct` symbol supplies the aggregate name, symbol identity and
  introduced scope;
- that scope's `Field` symbols supply member declaration identity;
- `declared_type_spelling` supplies the current member type spelling;
- symbol origins supply the declaration-pool AST path;
- symbol declaration locations supply declaration line and column;
- a member whose type names exactly one structural aggregate also records that
  aggregate's symbol identity.

This is structural declaration evidence, not a lookup through legacy runtime
path strings. Type aliases, qualified/imported type lookup and generic type
domains are not generalized in this bounded catalog.

The structural frontend currently permits duplicate member names. Resolution
therefore retains every candidate and refuses ambiguity; it never selects the
first declaration. Duplicate aggregate names are handled by the same refusal
principle.

## Resolved field-path representation

Mission 08 enriches, rather than competes with, `lyraform.target_fact`.
Newly produced facts use version 2. Version 1 remains readable compatibility
evidence. A version-2 field target carries:

```text
operation_id, statement_id
base_symbol_id
base_type, base_type_symbol_id
ordered members[]:
    ordinal
    name
    occurrence ast_path and location
    owner_type and owner_type_symbol_id
    member_symbol_id
    member_declaration_name
    member_declaration_path
    member_declaration_location
    member_type and member_type_symbol_id
destination_type and destination_type_symbol_id
resolution: resolved | refused
assignability: unresolved
execution: unsupported
failure: null | {code, message}
target provenance
```

Built-in scalar types have stable spelling identity (`int` or `Bool`) and no
structural type-symbol identity, so their type-symbol field is null. Structural
aggregate types carry both spelling and symbol identity. IDs are stable within
the captured frontend/plan, not promised as global IDs across compilations.

`member_declaration_location.file` is the frontend's expanded-source location
field and can be empty; the target-level source-map-adjusted provenance records
the source path. The declaration AST path, symbol ID, line and column still
provide explicit declaration identity and origin.

## Base resolution

Flowanalyst uses the established lexical symbol lookup to resolve the path base
in the placement operation's scope. The base symbol's declared type comes from
the shared symbol type map. The aggregate catalog then establishes the unique
structural type symbol for that spelling.

An unknown base is refused with `FLOWANALYST_TARGET_BASE_UNRESOLVED`. A base or
intermediate type without aggregate member structure is refused with
`FLOWANALYST_FIELD_OWNER_NOT_AGGREGATE`. A non-unique structural owner type is
refused with `FLOWANALYST_FIELD_OWNER_AMBIGUOUS`.

Target facts are keyed and emitted by operation ID, not only statement ID. This
matters because one statement can project more than one lowering operation;
operation and statement linkage are validated downstream.

## Segment-by-segment resolution

Resolution starts with the proven base type. For every member occurrence in
ordinal order it:

1. requires exactly one aggregate declaration for the current owner type;
2. records that owner's name and symbol identity;
3. requires exactly one field declaration with the requested member name;
4. records the field symbol, declaration name/path/location and member type;
5. advances the current owner type to that member's result type.

No suffix is opaque and no segment is skipped. A missing member is refused as
`FLOWANALYST_FIELD_NOT_FOUND`; a duplicate visible field is refused as
`FLOWANALYST_FIELD_AMBIGUOUS`. On any failed hop, the complete target fact is
retained with `resolution=refused`, null destination type,
`assignability=unresolved`, `execution=unsupported` and a deterministic failure
object. Flowanalyst blocks the lowering plan before assignability or lowering.

## Resulting type facts and positive cases

The focused source declares `Inner.b : int`, `Point.x : int`,
`Point.y : Bool` and `Point.a : Inner`. It proves:

| Target | Ordered semantic path | Result type |
| --- | --- | --- |
| `p.x` | `Point` symbol 4 → field symbol 5 | `int` |
| `p.y` | `Point` symbol 4 → field symbol 6 | `Bool` |
| `p.a.b` | `Point` symbol 4 → field symbol 7 (`Inner` symbol 2) → field symbol 3 | `int` |

Repeated analysis is byte-identical. Successful resolution explicitly remains
`assignability=unresolved` and `execution=unsupported`; member existence is not
treated as write authority.

## Negative cases

The focused suite proves early semantic refusal for:

- `p.z`: missing member;
- `p.x.foo`: scalar intermediate owner;
- `missing.x`: unresolved base;
- `x.foo` where `x : int`: scalar base;
- duplicate `Point.x` declarations with `int` and `Bool`: ambiguous member.

Each case exits analysis with a blocked plan, a specific diagnostic and a
refused target fact. A forged ready envelope containing a refused target is
rejected by the shared validator/planner.

## Artifact preservation

The resolved version-2 fact is preserved exactly through the applicable
boundaries:

| Boundary | Phase 8 behavior |
| --- | --- |
| Flowanalyst | Sole producer and member resolver |
| Flowbind | Validates input and exports exact additive `target_facts` |
| Flowparallel | Validates and preserves the lowering plan |
| Flowoptimize | Validates and preserves the lowering plan |
| flowprepare | Captures the same plan in the backend artifact |
| flowvalidate | Independently validates target structure and linkage |
| LLVM / TinyVM | Validate the resolved fact, then refuse unsupported execution |

No later stage reopens source spelling or performs independent member lookup.
Version 1 stays readable for historical Mission 07 artifacts. Version 2 adds
base/destination type-symbol identity, semantic member identities,
assignability state and explicit failure evidence without changing the outer
lowering-plan schema versions.

As before, this optional compatibility evidence is not cryptographically
authenticated. Complete removal or a fully self-consistent malicious rewrite
is outside this schema validator's guarantee.

## Backend and runtime behavior

LLVM and TinyVM both call the shared executable-target guard. They accept the
resolved artifact as structurally valid and then refuse field-path execution
with `unsupported target kind: member/index execution is not admitted` before
publishing output. The direct canonical runtime adapter is unchanged and gains
no member-write path or legacy-parser fallback.

This is the intended state: resolved-but-unsupported. Neither backend may
reinterpret `p.x` as assignment to `p`.

## Hostile artifact evidence

Eight version-2 contradictions are independently rejected by `flowvalidate`,
LLVM and TinyVM:

- wrong owner type symbol;
- occurrence name inconsistent with declaration name;
- member type inconsistent with the chain;
- duplicate/out-of-order segment ordinal;
- wrong intermediate owner type;
- final destination type contradicting the final member;
- missing member semantic identity;
- field evidence carried under indexed target kind.

The shared validator additionally checks declaration locations, declaration AST
paths, first-owner/base identity, every intermediate owner/result identity,
final result identity, operation/statement linkage, refusal/failure consistency
and ready-plan exclusion for refused facts.

## Mixed-target status

`p.x[i]`, `items[i].field` and `a.b[i].c` remain parser-recovered/non-executable
forms. The focused suite confirms all three remain outside the target corpus.
No mixed member/index grammar or semantics were introduced.

## Verification

Normal GCC 13.3 build directories: `/tmp/lyraform-phase2-build` for focused
development and `/tmp/lyraform-build` for the final Igor run. A clean rebuild
of all 302 targets succeeded. Focused scalar/target coverage: **8/8 PASS** in
8.59 seconds. The new field suite covers 3 resolved paths, 5 semantic refusals,
8 hostile facts and 3 mixed-parser refusals.

Final required gates:

- `./igor doctor`: PASS;
- `./igor build`: PASS, 302 targets;
- `./igor test`: **164/164 PASS**, 59.07 seconds.

The first full run was 154/164. It exposed two integration defects rather than
accepted failures: the version-2 validator required a plan status in historical
plan fragments where it had previously been optional, and target facts were
initially associated by statement ID even when a statement projected multiple
operations. Status compatibility was restored and facts were keyed by operation
ID. Targeted historical/target tests then passed, both pass-corpus registrations
passed, and the final clean Igor suite above supersedes the preliminary run.

Clang 18.1.3 Debug ASan/UBSan build:
`/tmp/lyraform-phase2-asan`, with
`-fsanitize=address,undefined -fno-omit-frame-pointer` and matching linker
flags. Focused coverage: **8/8 PASS**, 16.83 seconds, with
`ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and
`UBSAN_OPTIONS=halt_on_error=1`. LeakSanitizer cannot run under the host's
ptrace wrapper and was disabled; ASan and UBSan remained active.

Valgrind 3.22.0 used full leak checking and `--error-exitcode=99` on successful
field analysis, missing-member semantic refusal, LLVM refusal and TinyVM
refusal. All four reported **0 errors and all heap blocks freed**. Analyzer
success exited 0; semantic refusal exited 2; LLVM and TinyVM each exited 1 as
expected. Neither forbidden backend output was published. Logs:

- `/tmp/lyraform-phase8-valgrind-analyst-final.log`;
- `/tmp/lyraform-phase8-valgrind-refusal-final.log`;
- `/tmp/lyraform-phase8-valgrind-llvm-final.log`;
- `/tmp/lyraform-phase8-valgrind-tiny-final.log`.

`git diff --check` and shell syntax checks pass. Verification is local Linux
coverage, not fresh-clone, cross-platform or safety certification.

## Authority before and after

| Question | Before Mission 08 | After Mission 08 |
| --- | --- | --- |
| What field path was written? | Parser + Mission 07 target fact | Same |
| Is path syntax complete? | Parser | Parser |
| What base declaration/type is referenced? | Base symbol/type spelling partially retained | Canonical Flowanalyst resolution with structural type identity |
| Does each member exist? | Legacy/partial behavior only | Canonical member-by-member resolution |
| What declaration does each segment denote? | Not preserved canonically | Field symbol, declaration path/location and owner/type identity preserved |
| What is the final target type? | Unresolved | Proven for completely resolved paths |
| Is the target writable? | Deferred | Still deferred; explicitly unresolved |
| Can a backend execute it? | Refused | Still refused after semantic preservation |

Syntax completeness remains parser authority. Base/member/type resolution is
Flowanalyst/shared-semantic authority. Assignability is deliberately unowned in
this phase. Backend execution policy remains the shared target guard plus the
individual lowerer; neither backend owns member meaning.

## Remaining member/aggregate gaps

Member assignability, declared mutability, initialization state, ownership,
aliasing, mutation versus reconstruction/copy semantics, aggregate construction
semantics, index carrier/result typing, rank, bounds and mixed targets remain
unresolved. Qualified/imported member-type lookup, aliases, generics and
cross-compilation stable global identities also remain future work.

Readiness: the compiler can now prove what bounded `p.a.b` denotes and its type.
That is sufficient evidence for a separately authorized assignability study or
for separately bounded indexed-target resolution. It is not authorization to
write members or execute field placements.

## Next options — evaluation only

1. Indexed-target semantic resolution: establish container, index/rank and
   result-type authority without bounds or writes.
2. Member assignability/admission: define explicit write authority after this
   resolution gate; do not infer it from existence.
3. Aggregate ownership/copy reconnaissance: collect current behavior before any
   mutation rule.
4. Aggregate construction/value semantics: define canonical aggregate values
   independently of member writes.
5. Mixed-target reconnaissance: study grammar and semantic prerequisites while
   preserving current refusal.
6. Defer and review the version-2 target contract.

No subsequent phase is authorized or started.

GATE 8: WAITING FOR HUMAN REVIEW
