# Lyraform v1 compiler-unification reconnaissance

**Mission:** Lyraform v1 Compiler Unification — Mission 01  
**Authority:** Phase 1 reconnaissance and merger planning only  
**Gate:** Gate 1 — waiting for human review  
**Date:** 2026-09-18  
**Repository:** `Henrik1969/Lyraform` (`/home/henrik/Projekter/Udvikling/Flowcore`)

## 1. Exact baseline and scope

At inspection, `HEAD` and `origin/main` were both
`680a79592b7db4f7aa5fd1d0afe52a211794ead1` (`docs: publish scheduling
inventory status`). The current branch is `main`; `git worktree list` reports
only the canonical checkout. `origin` is the Lyraform GitHub SSH remote. The
tracked worktree is clean. One pre-existing untracked file,
`docs/recon/declaration-initialization-flow-recon-2026-09-15.md`, is present and
was not modified. No source, build, test, artifact, branch, or remote state was
changed for this report. No build gates were rerun; the earlier reconnaissance
report at the same baseline records its own commands and results. FlowLFS and
`master` were not inspected beyond the branch/worktree baseline and were not
touched.

The previously supplied declaration/initialization/flow reconnaissance is at
this exact revision. Its observed probe results are treated as evidence and
cross-checked against the current source layout; they are not silently promoted
to new tests run during this Phase 1 task.

## 2. Current compiler topology

The active source executable is built from `Lyraform/compiler` as `flowmini`.
`main.cpp` expands imports and calls the common `lexSource` lexer. It then
selects one of two materially different consumers:

```text
source ──► import expansion ──► shared lexer ──┬─► structural AST builder
                                               │    └─► AST / symbol projection /
                                               │        flowmini.frontend_bundle v2
                                               │             └─► Flowanalyst
                                               │
                                               └─► legacy Parser::parseModule
                                                    └─► ModuleSpec / runtime atoms /
                                                        direct execution or FlowIR text
```

`--dump-ast`, `--dump-frontend-bundle`, and AST-symbol export use
`ast::build_source_header_ast`; they return before `parseModule`. Direct
execution, legacy symbol dump, and `--emit-flowir` call `parseModule`. The
structural and executable paths share tokenization but not a common parsed
program or semantic result. Flowanalyst consumes the serialized AST and
SymbolTable projection, but derives semantic facts and a lowering plan in its
own implementation. The remaining stages mostly consume, validate, copy, or
project versioned JSON artifacts.

The legacy `ModuleSpec` is graph/runtime-oriented (nodes, wires, policies,
receiver frames), and its FlowIR text writer serializes graph declarations;
it is not a serialization of the structural AST or the complete language
program. Current source receiver frames are explicitly refused by that legacy
FlowIR writer in favor of a versioned graph artifact.

The checked-in `Lyraform/flowmini_v24_explicit_ast` and `_archive` trees are
historical snapshots, not active root-build compiler authorities. The current
structural implementation is under `Lyraform/compiler`; downstream tools live
in top-level Flowanalyst, Flowbind, Flowparallel, Flowoptimize, Flowlower, and
Flowcontracts directories.

## 3. Semantic-authority matrix

“Structural frontend” means the active AST builder and its projected bundle;
“legacy” means `flowmini_parser.cpp`, `ModuleSpec`, and the direct runtime.
“Later” covers analysis/binding/planning/optimization and the canonical
lowering artifact boundary. A `—` means no general authority was found, not
that the concern is irrelevant.

| Domain | Structural AST/frontend | Legacy parser/runtime | Flowanalyst | Later stages | LLVM | TinyVM | Unique knowledge / divergence |
|---|---|---|---|---|---|---|---|
| 1. Lexical analysis | Shared lexer emits token kinds/locations; AST and legacy consumers reuse tokens | Same tokens; no independent lexer in active path | — | Artifacts retain source/origin projections | — | — | Lexing is shared; grammar and meaning diverge after tokenization. |
| 2. Declarations | `LetStatement`, declaration pools and symbol projection | Declaration grammar creates runtime path/type entries | Resolves/project symbols from bundle | Operations refer to symbol IDs where represented | Allocates supported scalar slots | Creates typed slots for supported operations | AST permits absent initializer; legacy parser requires it. |
| 3. Initialization | Optional initializer expression preserved | Initializes typed value/path; requires declaration initializer in observed legacy path | Emits value-definition/conversion operations, but not general compatibility proof | Operation carried in lowering plan | Stores admitted values | Moves admitted carriers into slots | Type-invalid initializer can be marked ready by analysis and only later rejected by LLVM, while TinyVM admitted a known Bool-to-int case. |
| 4. `->` placement | `PlacementStatement` with source form and closed target variant | Resolves target then emits typed runtime operations | Identifier-target operations; target semantics incomplete for non-identifiers | Plan preserves operations, with target identity gap | Scalar assignment subset | Scalar and other supported operation subset | AST retains more target shape than Flowanalyst plan. |
| 5. `=>` wiring | Structural graph/source forms are represented in AST | `WireDecl` in ModuleSpec and runtime graph checks | Builds source graph and checks endpoints/ports/types for admitted graph form | Flowcontracts validates source graph; Flowparallel plans execution | Graph execution subset | Graph execution subset | Graph facts exist in separate plan/artifact family; runtime path and source-graph artifact are not the same representation. |
| 6. Type-name resolution | Type references/spellings retained structurally | Resolves names against legacy type declarations during parsing/lowering | Resolves declared names and selected ABI/refined cases | Flowcontracts validates selected typed artifact fields | Carrier map for admitted operations | Carrier map for admitted operations | Type resolution and type compatibility are different questions; compatibility is not generally established by Flowanalyst. |
| 7. Type compatibility | Structure alone; no complete typed expression result | Checks initializer, destination, member/index and operation types in legacy path | Checks selected special cases, not general source-value-to-destination compatibility | Schema validation protects artifact shape, not full source-language typing | Rejects some mismatches late | Known mismatched carrier was accepted in TinyVM | Correctness-critical duplicated/incomplete authority. |
| 8. Aggregate declarations | Record declarations and field types represented | Parses compatibility `field` members; builds runtime type descriptions | Reads record fields and graph aggregate declarations | Flowbind/Flowcontracts carry ABI layout evidence separately | Direct aggregate path limited; graph/ABI slices have explicit subset | Aggregate subsets supported by artifact path | Structural parser accepts bare members; active legacy parser requires compatibility spelling. |
| 9. Aggregate values | Record-literal expression and fields represented | Parses/owns record values in legacy-supported forms | Current plan can classify unsupported expression forms without uniformly blocking readiness | Some graph/ABI artifacts carry aggregate layouts/payload contracts | Admitted graph/ABI subset only | Admitted graph/ABI subset only | Structural syntax exceeds the generic source-to-backend semantic path. |
| 10. Lists | List literal expression represented | Value-owned list construction and operations | Some list-like inputs are still classified unsupported rather than semantically typed | Artifact preserves only admitted operation projections | General list lowering not established | Bounded TinyVM surfaces exist, but not parity for arbitrary source list operations | Legacy has construction/mutation behavior not generalized through canonical plan. |
| 11. Arrays | Generic/array `TypeRef` and index structure | Array shape/value construction and checked operations | Limited shape/type checks; no complete runtime value semantics | Graph/artifact contracts may carry explicit shape/layout, not general source arrays | No general array source lowering | Bounded specific VM artifacts only | Legacy owns observed rank, shape and bounds behavior. |
| 12. Member access | `FieldAccessExpr` and field-path target | Resolves fields and checks runtime record shape | Resolves selected declared fields; typed lowering coverage incomplete | Plan does not carry every general member-access fact | Subset/unsupported refusal | Subset/unsupported refusal | Member identity may disappear before lowerers. |
| 13. Indexed access | `IndexExpr` with ordered index-expression IDs | Resolves list/array carrier, rank and bounds at runtime | Some expression identities retained; general index typing incomplete | No universal typed access operation | Unsupported in general scalar plan | Limited operation family; not general parity | Legacy has checked bounds and axis-specific runtime diagnostics. |
| 14. Member/index placement | `FieldPathTarget` / `IndexedTarget` are explicit variants | `lowerAssignmentToTarget` dispatches to member/list/array mutation | Existing lowering mostly asks for identifier target name; member/index identity is lost | Plan consumers cannot recover discarded target from source alone | Not admitted generally | Not admitted generally | Concrete AST-to-plan information loss. |
| 15. Functions | Function declarations, parameters, return and bodies represented in AST | Parses/executes functions through legacy runtime/receiver machinery | Builds function catalogs and operations; lowering plan v2 captures callable facts | v2 catalog passes through stage artifacts and is validated | Callable subset implemented from v2 | Callable subset implemented from v2 | Shared support improved, but legacy execution remains another implementation path. |
| 16. Parameters | Names/types and locations in AST | Bound to runtime activation paths | Stable parameter symbols and ordered callable parameter identities in v2 | IDs and types preserved by v2 plan | Binds parameter slots | Binds parameter-identity slots | v2 is the more explicit cross-stage authority; legacy has runtime binding behavior. |
| 17. Return | `ReturnStatement`; arrow return represented distinctly | Function return is runtime/control behavior; top-level arrow-return differs | Emits return operations and checks some callable constraints | Callable plan v2 carries function/return authority | Lowers supported return carriers | Lowers supported return carriers | Returns are explicit AST structure; backends consume plan operations, not AST. |
| 18. Scope | Nested blocks/declarations represented; symbol projection includes scopes | Legacy parser maintains name/type/path resolution scopes | Resolves names using projected scope/symbol relationships | Scope IDs are report-local and carried selectively | Uses symbol identity for local storage | Uses symbol identity for typed slots | AST/SymbolTable scope projection exists, but resolved semantic identity is rebuilt downstream. |
| 19. Control flow | If/while/break/continue/return forms and block pools | Runtime graph/control atoms execute legacy subset | Builds block/region/control relationships and diagnostics | Plan v2 operation ownership/control blocks; Flowparallel creates dependency/schedule projections | Structured control subset | Structured control subset | Control facts are separately represented and selectively lowered; do not infer full equivalence. |
| 20. Ports | AST may contain source graph/provider forms | Atom contracts define runtime input/output port names/types | Graph analysis validates endpoint direction/identity | Flowcontracts source-graph contract; binding artifacts | External/graph contracts consumed for supported ops | Same artifact boundary, backend-specific subset | Runtime atom registry and graph artifact validators both know port contracts in different domains. |
| 21. Graph construction | Structural graph syntax and identities retained | Builds ModuleSpec graph and checks it against runtime atom registry | Creates `flowcore.source_graph`, receiver/provider facts and graph analysis | Flowcontracts validates graph; Flowparallel plans; later preserve | Backend graph subset | Backend graph subset | Graph semantics are derived by both legacy graph runtime and Flowanalyst source-graph analysis. |
| 22. Contracts | Contract/ABI declarations and source locations represented | Atom/provider contracts constrain runtime graph/ABI operations | Projects external binding requirements and ABI facts | Flowcontracts validates artifact contracts; Flowbind verifies grants/provider symbols | Consumes admitted ABI contracts | Consumes admitted ABI contracts | Language declarations, runtime atom contracts and artifact schemas are distinct contract layers. |
| 23. Provider requirements | Contract/function source structure preserved | Runtime registry requirements and effects | Derives library/symbol/convention/effect/signature requirements | Flowbind authorizes exact grants; policy/evidence carried forward | Requires exact capability set for external operations | Same artifact capability boundary for supported calls | Requirement derivation and provider discovery/authorization have different owners. |
| 24. Binding | — | Runtime atom registry binding for interpreter; not the native grant artifact | Emits requirements only | Flowbind performs `dlopen`/`dlsym` discovery and exact policy authorization; Flowcontracts validates evidence | Consumes ready authorization | Consumes ready authorization | “Binding” means distinct operations on the two paths; Flowbind does discovery, not invocation. |
| 25. Scheduling/planning | Structural dependency/source facts | Runtime graph construction/order in interpreter | Builds dependency regions/matrices and schedule facts | Flowparallel validates/plans; Flowoptimize preserves identity/projection; runtime providers select only admitted policies | Consumes plan-derived graph/schedule contract | Consumes plan-derived graph/schedule contract | Static dependency evidence and execution order/provider choice are separate authorities. |
| 26. Optimization | — | Legacy ModuleSpec graph construction has no equivalent current optimizer artifact | Emits initial lowering/dependency facts | Flowoptimize currently identity-preserves canonical graph; COO dedup is a derived projection | Consumes optimization report | Consumes optimization report | Optimization is not a second semantic authority today; its transform is intentionally constrained. |
| 27. Lowering | AST is not directly consumed by backends | Legacy parser lowers source into ModuleSpec atoms | Flowanalyst creates lowering operations/plan | `flowprepare` captures backend-neutral artifact; Flowcontracts validates it | Structured-plan lowering | TinyVM artifact lowering | There are two source-to-execution lowering routes; backend plan input is the desired convergence point, not yet universal. |
| 28. LLVM | — | Legacy interpreter does not produce LLVM | Plan producer supplies operation facts | Shared `flowcore.backend_lowering_artifact` v1/v2 boundary | LLVM-specific carrier and code generation/refusal rules | — | LLVM may refuse unsupported plan operations; it must not decide source typing independently. |
| 29. TinyVM | — | Legacy interpreter is separate | Plan producer supplies operation facts | Same backend artifact family | — | TinyVM-specific carrier and bytecode mapping/refusal rules | Known type-invalid admission mismatch with LLVM makes plan semantic completeness critical. |
| 30. Diagnostics | Token/AST diagnostics and source provenance | Parser/runtime stage diagnostics | Stable semantic diagnostics and provenance | Each CLI/artifact boundary adds its own validation/diagnostic category | Backend-specific refusal and emission failures | Backend-specific refusal and execution failures | Same user error can arise at different phases/codes; source map/origin fields are the bridge. |
| 31. Provenance | AST origin roles, source locations, source map and symbol origins in bundle v2 | Legacy tokens/diagnostics have source positions; ModuleSpec is not the same origin graph | Projects AST path/source/symbol provenance into diagnostics and reports | Artifacts preserve source path and operation identity; Flowcontracts validates selected identities | Backend artifact/outputs carry target and source report metadata | Same captured plan provenance | Rich AST-origin detail is projected; downstream identity is often bundle/report-local. |
| 32. Runtime ownership/lifetime | Syntax/type shapes only; no complete canonical ownership facts for all source aggregates | `RecordPayload` owns values; record copy is value-copy; list/array/member mutation and bounds are implemented | ABI carrier ownership/lifetime facts for externs, but not general local aggregate ownership | Flowbind ABI evidence and artifact contract; runtime/provider contracts are separate | Explicit supported carrier/cleanup contracts | Explicit VM carrier/runtime subset | Legacy path contains concrete value and checked-bounds behavior not yet made canonical for general source aggregates. |

## 4. Duplicate semantic authority and information loss

| Question/fact | Current independent owners or derivations | Preferred eventual authority | Observed loss, contradiction, or boundary |
|---|---|---|---|
| What syntax forms exist? | Structural AST builder and legacy parser both parse the shared token stream | One canonical parser/grammar, with explicit canonical/compatibility classification | Bare record fields and missing initializers differ; one parser's accepted form does not imply executable support. |
| Is a declaration initialized and what type is its value? | Legacy parser checks typed initialization; Flowanalyst derives value definitions from AST; LLVM/TinyVM each map carriers | Canonical semantic analysis over the AST, with explicit type and initialization facts | Flowanalyst can mark invalid carrier combinations ready; later backend behavior diverges. |
| Does a placement destination exist and what is its type? | Legacy parser resolves/checks; Flowanalyst resolves some identifiers; lowerers validate what remains | Canonical typed target resolution before binding/planning | AST has a target sum type, but Flowanalyst plan retains identifier target more reliably than field/index target. |
| What does an aggregate/member/index operation mean? | Legacy parser/runtime, structural AST, partial Flowanalyst recognition and backend operation interpreters | Canonical expression/target typing and explicit ownership/bounds facts | Legacy value semantics are not all represented in the generic lowering plan; member/index identity may be lost. |
| Is a graph/wire valid? | Legacy ModuleSpec builder/runtime atom registry; Flowanalyst graph analyzer; Flowcontracts validator; runtime graph consumers | One canonical graph semantic pass, with artifact validators checking representation/invariants | Similar checks exist for different graph forms and at different trust boundaries; later checks should validate, not reinterpret source meaning. |
| What types/carriers may flow through an operation? | Legacy type system, Flowanalyst cases, artifact schema, LLVM carrier map, TinyVM carrier map | One admitted semantic/carrier fact in the canonical plan; backends only map/refuse | LLVM refuses some invalid carriers late; TinyVM accepted a proven invalid Bool-to-int case. |
| What counts as an external requirement and authorized call? | Flowanalyst derives requirements; Flowbind discovers/authorizes; lowerers match capabilities; runtime atoms have separate contracts | Semantic analyzer owns requirement derivation; Flowbind owns authorization evidence; backend only verifies exact match | Legitimate staged authority, but repeated parsing/projection risks identity/requirement drift; discovery is not invocation. |
| What is parallel-safe / executable order? | Flowanalyst derives dependency/effect facts; Flowparallel plans; Flowoptimize exposes derived matrix; providers select policy | Semantic facts in analysis; explicit planner/provider policy at later borders | A graph/matrix projection is not itself semantic proof of purity, mutation absence, or execution authority. |
| What error and source location should be reported? | Frontend, Flowanalyst, legacy parser/runtime, artifacts and backends | Origin-bearing canonical diagnostics, then stage-local projection | AST source map/origins are richer than many later reports; path/line provenance is projected rather than full source identity. |

The desired division is: parse structure once; resolve and prove source
semantics once; serialize stage projections; have each consumer validate its
own artifact contract and refuse unsupported work. Binding authorization,
scheduling policy, backend representation, and runtime resource checks remain
legitimate later-stage responsibilities; unification does not collapse those
distinct decisions into source semantic analysis.

## 5. Legacy knowledge inventory

Classifications below distinguish desired language semantics from observed
implementation behavior. `V1_REQUIRED_SEMANTIC` means a behavior/fact that
appears necessary for a coherent v1 definition, not that its current legacy
implementation is automatically normative.

| Legacy behavior/knowledge | Classification | Evidence and migration note |
|---|---|---|
| Destination existence checking for writes | `V1_REQUIRED_SEMANTIC` | Legacy parser resolves destination paths; Flowanalyst's current ready-plan gaps show this must be proved or rejected before lowering. |
| Member existence/type checking | `V1_REQUIRED_SEMANTIC` | Legacy record field lookup/checking plus runtime field access; migrate as resolved typed member identity, not string-path re-derivation in each backend. |
| Index carrier, rank, and bounds diagnostics | `V1_REQUIRED_SEMANTIC` | Legacy list/array runtime checks; exact static-vs-runtime failure contract remains `UNDECIDED`. Preserve tests as an oracle while choosing v1 behavior. |
| Initializer compatibility | `V1_REQUIRED_SEMANTIC` | Legacy type check is useful; define one rule and diagnose before a ready plan. Do not assume every legacy restriction is correct. |
| Placement compatibility | `V1_REQUIRED_SEMANTIC` | Legacy checks typed target writes; canonical analysis currently incomplete. |
| Refined/literal restrictions | `USEFUL_TEST_ORACLE` | Legacy restrictions are valuable test cases, but refinement/admission policy requires the current language contract to confirm exact normative behavior. |
| Record/list/array value ownership | `UNDECIDED` | `RecordPayload` and `record.copy` demonstrate value-owned behavior; no general canonical ownership law was found for all source aggregates. Preserve as evidence, not assumed v1 semantics. |
| Record deep/value copy on placement | `USEFUL_TEST_ORACLE` | Existing probe observed destination independence after source mutation. Decide whether this is v1-required and represent it explicitly before lowering. |
| List and array mutation operations | `USEFUL_TEST_ORACLE` | Runtime supports set/get and observed bounds behavior; generic backend parity is not established. |
| Function execution and activation-local values | `USEFUL_TEST_ORACLE` | Legacy runtime has receiver/function activation machinery; callable lowering v2 independently captures function identities and parameters. Compare behavior before retiring legacy execution. |
| Mandatory declaration initializer | `UNDECIDED` | Legacy parser requires it; structural AST permits absence; Flowanalyst/backends have inconsistent behavior. Syntax acceptance is not enough to make either choice canonical. |
| Compatibility `field` aggregate-member spelling | `COMPATIBILITY_ONLY` | Structural syntax accepts bare members and compatibility `field`; executable parser accepts only `field` at the observed baseline. Decide retirement/migration policy before v1. |
| Legacy parser/runtime implementation shape (`ModuleSpec`, path strings, runtime atoms) | `HISTORICAL` | Implementation architecture is not language semantics; preserve only the behavior/tests that survive semantic review. |
| Legacy source diagnostics and exact stage labels | `USEFUL_TEST_ORACLE` | Useful migration evidence; canonical diagnostic codes/messages and phase ownership may differ. |

## 6. Candidate canonical model

The strongest existing foundation is the active structural AST in
`Lyraform/compiler/include/flowmini_ast.h`, augmented by its AST arena pools,
the stable-within-bundle SymbolTable projection, `flowmini.frontend_bundle` v2
origin/source-map information, and the typed/versioned operation model in
`flowcore.lowering_plan` (especially callable v2). Reasons:

- The AST is the only current active representation with explicit expression,
  statement, block, declaration, type-reference, source-form, and closed
  identifier/member/index target structure.
- The frontend bundle already carries the AST and symbol/scope projection,
  AST paths, locations, import-expanded source mapping, and origin roles across
  a process boundary.
- Lowering-plan v2 already demonstrates stable function/parameter/callee
  identity and operation ownership without asking a backend to rediscover
  source names.
- Flowcontracts supplies reusable validation boundaries for artifacts,
  lowering plans, source graphs, and backend artifacts without linking the
  semantic analyzer to frontend internals.

No current single object combines these as a resolved, typed, provenance-bound
semantic model. The SymbolTable is a projection, not a complete typed semantic
authority; the AST is syntax, not proof; the lowering plan is a backend-facing
projection and currently omits or under-specifies source facts. The strongest
candidate is therefore “AST + resolved semantic facts + origins,” with the
current AST as its structural core—not the raw AST alone, SymbolTable alone,
ModuleSpec, nor any current artifact treated as complete.

Minimum eventual extensions to carry known legacy semantics (not authorized
for implementation here):

1. Resolved declaration/type/function/contract identities and an explicit
   type identity/carrier relation, including expression result types.
2. Typed assignable targets for identifier, complete field path, and indexed
   path, each with resolved base/member/index type and source origin.
3. Explicit declaration initialization state and the proof needed for any
   later first-placement/definite-initialization policy.
4. Canonical type-compatibility/refinement facts for initializers, placements,
   calls, returns, and aggregate members; diagnostics tied to source origins.
5. Aggregate/list/array shape, value ownership/copy, mutation, bounds, and
   failure facts only after v1 policy is decided.
6. Graph node/port/wire identities and contract/provider requirements linked
   to declaration identities; explicit effect and provenance facts.
7. A typed semantic-model contract or equivalent shared module API so
   Flowanalyst and adapters share rules while versioned stage artifacts remain
   exportable and independently validated.

## 7. Stage artifact and import/export map

| Boundary/artifact | Version and producer | Consumer(s) | Facts represented / omitted | Validation, provenance, and authority note |
|---|---|---|---|---|
| `flowmini.frontend_bundle` | v2; Flowmini structural route | Flowanalyst; independent Python consumer | Expanded source path/map, AST pools, symbols/scopes, origins and projection diagnostics; no resolved complete expression typing | Flowanalyst requires v2 and reads AST + projected symbols. Bundle/source/origin are the best current semantic input boundary. |
| Legacy FlowIR text / `ModuleSpec` | Text writer has no versioned AST artifact envelope; emitted from legacy route | Compatibility tooling/runtime paths; graph import is distinct | ModuleSpec node/wire/policy graph; not full AST; receiver frames cannot be exported by legacy writer | `writeFlowIr` rejects receiver frames requiring versioned graph artifact. Do not confuse this with a canonical source-model round trip. |
| `flowanalyst.semantic_report` | v1; Flowanalyst | Flowbind, Flowparallel, Flowoptimize, validator | Diagnostics, targets, binding/ABI/effect facts, analysis graph/matrix, lowering plan and source path; omitted/under-proved source type and target facts remain gaps | Flowcontracts has typed readers/validators for parts; Flowanalyst performs analysis itself. Plan is a projection, not a complete semantic proof today. |
| `flowcore.lowering_plan` | v1 and v2; embedded by Flowanalyst | Flowbind, Flowparallel, Flowoptimize, Flowlower/TinyVM preparation | Ordered operations/blocks, operands/results, callable catalog in v2, provider/effect/resource/provenance subsets; general typed field/index target facts are incomplete | Flowcontracts validates plan shape/identities; v2 adds callable identities. Stage consumers should not infer missing source meaning. |
| `flowcore.source_graph` | v1/v2; emitted by Flowanalyst | Flowcontracts, binding/planning/lowering consumers | Graph nodes, wires, endpoints, receivers/providers, policies and provenance; v2 includes bounded schedule policy | Independently validated by Flowcontracts; status/validation does not itself authorize provider execution. |
| `flowbind.binding_report` | v1; Flowbind | Flowlower and artifact inspection | Exact discovered/authorized provider capability, ABI/policy evidence; execution is not performed | Flowbind owns external discovery/authorization, not language type semantics or foreign invocation. |
| `flowparallel.execution_plan` | v1; Flowparallel | Flowoptimize and provider/planner tools | Preserved semantic plan/source provenance, graph-to-matrix projection, fallback and runtime capability/schedule constraints | The original contract inventory documents historical text-search weaknesses; current implementation has since received hardening. Current report treats schemas/producers as present and does not re-audit every parser defect. |
| `flowoptimize.optimization_report` | v1; Flowoptimize | Flowlower | Carries lowering plan, derived projections, identity-preserving transform evidence, provider decision and source path | Current transform is identity-preserving/derived projection. Flowlower consumes report and later `flowprepare` captures backend artifact. |
| `flowcore.backend_lowering_artifact` | v1/v2; `flowprepare` | LLVM and TinyVM lowerers | Selected target, plan, ABI/external ops, exact authorization, optimization/binding provenance; does not restore semantic facts already omitted upstream | Flowcontracts validates artifact; both backends consume this shared artifact family but apply backend-specific support/carrier mapping. |
| `flowcore.target_policy` | v1; policy input to `flowprepare` | `flowprepare`, validator | Backend/architecture/ABI/capability/resource/lifecycle/evidence/fallback policy | Versioned validation; selection is explicit policy, not a source semantic interpretation. |

Stage artifacts can remain useful and largely intact as projections if a
canonical model becomes their producer and import validates the projection
before reconstructing or continuing a model. Likely version pressure is first
at the semantic/lowering-plan boundary (resolved types and complete targets),
then frontend/model serialization only if existing v2 AST/origin data cannot
represent needed source facts. This mission authorizes no schema changes.

### Current information flow

```text
AST + symbol/origin bundle v2
       │
       ├── Flowanalyst resolves selected names/types/graph facts
       │       └── semantic report v1 + lowering plan v1/v2
       │               ├── Flowbind adds authorization evidence
       │               └── Flowparallel adds planning projection
       │                       └── Flowoptimize adds optimization report
       │                               └── flowprepare captures backend artifact
       │                                       ├── LLVM
       │                                       └── TinyVM
       │
       └── separate legacy Parser -> ModuleSpec -> runtime atoms / FlowIR text
```

Each JSON artifact is independently parseable/validatable at intended borders,
but “round-trip to equivalent canonical semantic model” is not currently a
general guarantee: there is not yet one such complete model, and the legacy
FlowIR text is not an AST/model serialization.

## 8. Igor orchestration audit

`./igor` currently implements:

| Command | Actual behavior |
|---|---|
| `doctor` | Checks `cmake`, `ctest`, `git`, Ninja when selected, and required repository paths. |
| `check` | Configures CMake and lists registered CTests (`ctest -N`); it is discovery/configuration, not semantic source checking. |
| `build` | Configures only if no cache exists, then builds the root CMake graph. |
| `test` | Builds and runs the canonical CTest suite. |
| `run` | Builds, then invokes compatibility executable `$BUILD_DIR/flowmini/flowmini` on the supplied path/arguments. |

Igor has no current `parse`, `analyze`, `bind`, `graph`, `optimize`, `lower`,
or `compile` command. Stage binaries remain directly invocable. The smallest
orchestration path is to let Igor delegate to existing stage entry points and
artifact handoffs without absorbing their logic; keep stop/export/import as
explicit stage-level operations. Exact CLI syntax is outside this mission.

Yes, stage executables can gradually become thin adapters over shared
canonical compiler components while keeping their process, artifact, testing,
and diagnostic boundaries. Likely shared components/modules: structural parser
and AST construction; semantic resolution/type/target analysis; canonical
diagnostic and origin projection; typed artifact readers/writers and shared
schema definitions; graph/contracts facts where they are source semantics.
Flowbind provider discovery, Flowparallel policy/planning, optimization,
target policy, LLVM/TinyVM emission, and runtime capability checks should
remain separate components because their authority is intentionally staged.
This is an architectural finding, not approval to create libraries or alter
Igor.

## 9. Correctness-critical divergences and information loss

The following are observed in the earlier same-revision focused probes and
confirmed against current representations:

1. **Two grammars after one lexer.** AST inspection and executable parsing can
   disagree on canonical bare aggregate members and missing initializers.
2. **Accepted structure is not semantic validity.** Structural parsing can
   retain incomplete or unsupported forms; Flowanalyst may produce an `ok` /
   `ready` projection without proving general initializer or placement type
   compatibility.
3. **Target identity loss.** AST represents identifier, field-path, and
   indexed targets; Flowanalyst's general operation path does not preserve all
   member/index target identity and type facts.
4. **Backend contradiction.** A proven Bool-to-int mismatch was rejected by
   legacy semantics/LLVM but accepted by TinyVM at this baseline. This is the
   strongest immediate correctness concern: shared admitted semantics must be
   established before either backend executes.
5. **Initialization disagreement.** An absent initializer is structurally
   admitted, rejected by legacy parsing, and handled differently by Flowanalyst
   and backends. Whether it is legal remains a language decision.
6. **Aggregate/runtime semantics stranded in legacy path.** Legacy runtime
   knows value ownership/copy, nested member mutation, list/array index and
   bounds behavior; the generic plan/backends do not represent all of it.
7. **Graph construction overlaps.** ModuleSpec/runtime atom validation and
   Flowanalyst/Flowcontracts source-graph analysis each own graph-related
   checks. Some differences are appropriate boundary validation; any duplicate
   source-level well-formedness rule needs one designated semantic owner.
8. **Provenance compression.** Frontend v2 carries AST origins and expanded
   source map; later reports preserve source and operation identities but do
   not universally preserve all AST-origin detail.
9. **Artifacts are projections, not yet round-trip semantic state.** Plan and
   backend artifact validation can prove schema/operation consistency, but
   cannot recreate source typing or target facts discarded upstream.

## 10. Candidate first merger slices

Estimates are planning estimates only. “Semantic change: none” means preserve
the currently chosen admitted behavior for the bounded slice; it does not
declare the contradictory forms acceptable. All candidates retain process
stage execution, export/import artifacts, validation, stop/resume, focused and
hostile-artifact tests, and provenance unless explicitly stated. No candidate
is authorized by this report.

| Option | Bounded slice and components | Expected semantic change | Artifact/schema and compatibility impact | Test surface / rollback / scope |
|---|---|---|---|---|
| **A — Canonical scalar declaration/type/identifier-placement semantic facts** | Extract a small shared semantic component over existing AST + symbol projection for scalar declarations, initializers, identifier targets and scalar operations; Flowanalyst calls it first, then staged adapters consume its result. Keep all other constructs refused/outside slice. | Preferably none for currently well-typed initialized scalar programs; invalid or unproved scalar inputs become early refusal, including backend-disputed cases. Must explicitly choose treatment of uninitialized declarations before scope is finalized. | Likely additive/versioned semantic-plan fields or a new typed semantic projection; existing stage envelopes remain. Legacy direct source path remains temporarily compatible, so this only partially removes authority until it consumes the same facts. | AST/frontend goldens, positive/negative semantic cases, artifact hostile tests, LLVM/TinyVM parity and stop/resume; few constructs but crosses several stages. Reversible by removing shared component/adapter wiring; **medium** scope. |
| **B — Structural-AST-to-legacy-runtime adapter for a tiny supported scalar subset** | Have the executable path build the structural AST and translate only a declared scalar subset into existing ModuleSpec/runtime operations; retain legacy parser as fallback only for explicitly labeled compatibility forms during transition. Candidate files: `main.cpp`, AST builder/model, adapter, tests. | None for the declared subset if differential tests match legacy and existing backend plan; unsupported syntax must refuse rather than silently fall back into another meaning. | No public artifact change initially; frontend bundle/stage exports survive. Internal ModuleSpec adapter is not yet a serialized canonical model. Compatibility handling and path selection become visible behavior. | Differential legacy-vs-adapter corpus, AST goldens, runtime outcomes, error/provenance, import expansion. Strongly exercises shared syntax authority but risks mixing AST and runtime lowering. Rollback easy behind an explicit internal switch; **medium** scope. |
| **C — Preserve typed assignable-target identity in lowering plan** | Resolve existing AST identifier/field/index target variants into explicit typed target facts in Flowanalyst; add Flowcontracts schema validation and update consumers to preserve/refuse unknown variants. No aggregate execution implementation required in this slice. | No new executable semantics; member/index plans remain blocked until target/type proof and backend support exist. Improves early refusal and prevents loss. | Requires a new lowering-plan version or clearly additive contract under an approved policy; current v1/v2 artifacts remain readable. Compatibility risk concentrated in plan consumers. | AST target goldens, Flowanalyst positive/negative target cases, artifact mutation/fuzz tests, Flowbind/Flowparallel/Flowoptimize/Flowlower round-trip preservation, LLVM/TinyVM refusal parity. Easy-to-review but schema fan-out; **small-to-medium** scope. |
| **D — Uniform early refusal for unproved scalar mismatches** | Make Flowanalyst block known invalid initializer/placement carrier cases before ready plan; validate same condition in artifact imports if source proof is absent. No parser merge or new accepted semantics. | Narrows execution of currently mis-admitted programs; valid subset unchanged. Semantically safest short step but only closes a contradiction, not parser authority. | Could fit semantic report diagnostics/status; avoid schema change if existing diagnostic contract suffices. Captured artifacts still require downstream defense. | Strong negative tests and backend parity; few components; rollback straightforward. **Small** scope. |
| **E — DO NOTHING / DEFER** | Preserve both paths; do no production changes until language decisions and model design are reviewed. | None. | None. | No regression risk; current divergence and TinyVM mismatch remain, so explicitly not a resolution. **No implementation scope.** |

### Border preservation conditions for any selected candidate

- Keep structural parsing/export and each stage executable independently
  invocable, with an explicit artifact at each durable boundary.
- Each consumer validates the exact supported artifact version and refuses
  unsupported fields/operations; no consumer guesses semantics from source
  names or reconstructs a lost target from a diagnostic string.
- Preserve `flowmini.frontend_bundle` v2, semantic report, lowering plan,
  execution plan, optimization report, binding evidence, backend artifact,
  target policy, and source-graph validators until a separately reviewed
  migration gives each an explicit successor/compatibility policy.
- Add stage-stop/export/import tests and canonical round-trip equivalence tests
  for whatever new semantic facts are selected; retain provenance and source
  maps through every projection.
- Keep focused positive/negative, malformed artifact, duplicate identity,
  bounds/carrier, and cross-backend parity tests at the relevant border.

## 11. Cost/risk surface

The smallest scope is a bounded refusal correction (D); it materially improves
the known TinyVM type-admission defect but does not unify the parsers. The
smallest information-preservation slice is C, but it creates schema fan-out
before deciding full semantic ownership. A and B can establish a real shared
source/semantic path over a tiny subset, but require an explicit compatibility
story and cross several implementation layers. Extending aggregates,
collections, initialization flow, or graph runtime before the scalar authority
is settled would multiply the test and compatibility surface and is not
recommended as the first move.

Primary risks for subsequent phases:

- accidentally treating AST syntax as a semantic proof;
- accepting a form in the canonical parser while later stages cannot represent
  its target, type, effects, provenance, or failure behavior;
- fixing LLVM/TinyVM disagreement in only one backend;
- retiring useful legacy runtime behavior without a parity oracle;
- changing serialized protocol identifiers or captured artifacts without
  compatibility/version review;
- conflating language semantics with provider authorization, graph scheduling,
  target policy, or runtime capability selection;
- making Igor orchestration less inspectable by hiding existing stage tools.

## 12. Gate 1 decision inputs

### FACTS

- Active `flowmini` shares tokenization, then branches into structural-AST and
  legacy `ModuleSpec` source-consumer paths.
- The AST/frontend bundle is the richest current structural/provenance input;
  Flowanalyst adds semantic facts but does not generally prove scalar
  initializer/placement compatibility.
- Later stages preserve and validate useful versioned projections; the
  backend artifact does not recover source facts absent from its lowering
  plan.
- LLVM and TinyVM consume the same backend-artifact family, yet their carrier
  support differs; earlier same-revision probes expose at least one
  correctness-relevant invalid-program disagreement.
- Igor currently orchestrates build/test/run and tool readiness, not the
  individual semantic stage commands named in the target architecture.

### DUPLICATED AUTHORITY

- Grammar/acceptance: AST builder versus legacy parser.
- Type and target compatibility: legacy parser, Flowanalyst, artifact
  validators, LLVM, TinyVM at different completeness/timing.
- Graph source well-formedness and runtime atom/port checks: legacy path,
  Flowanalyst, Flowcontracts, and runtime consumers, with some legitimate
  artifact-boundary duplication to classify further.
- Operation/carrier interpretation: lowering-plan producer and each backend.

### UNIQUE LEGACY KNOWLEDGE

- Destination/member existence, index rank/carrier/bounds diagnostics,
  initializer/placement checks, and refined/literal restrictions.
- Concrete value-owned record/list/array runtime behavior, deep-copy evidence,
  nested mutation, and function activation behavior.
- These are implementation evidence and test oracles, not automatically
  normative v1 rules.

### INFORMATION LOSS

- AST member/index target variants are not completely represented by current
  Flowanalyst lowering operations.
- General expression and destination type facts are absent or incomplete
  before lowerers.
- AST origin/source-map detail is projected to slimmer downstream provenance.
- Legacy FlowIR text is a ModuleSpec graph dump, not a round-trip AST or
  canonical semantic-model format.

### CANONICAL MODEL CANDIDATE

The AST plus symbol/origin projection is the strongest existing structural
foundation, joined to a new resolved semantic-facts layer. The current AST
alone, SymbolTable alone, ModuleSpec, semantic report, or lowering plan is not
the complete common model.

### RECOMMENDATION

Recommended next review focus: first decide whether to take D as a very small
correctness containment step, or select A/B for actual source-path convergence.
If the human goal for Mission 01 is specifically to start unification rather
than only close a known admission defect, prefer **A** as the first merger
design candidate, with an explicit decision that declarations require
initializers (or a separately specified definite-initialization rule) and
with unsupported forms refused. Keep **C** as a following information-preserving
step if target facts are required by the chosen scalar slice. Do not begin
aggregate/collection generalization or self-hosting in this campaign step.
This is a recommendation only; no course is selected or authorized.

### GATE STATUS

```text
GATE 1: WAITING FOR HUMAN REVIEW
```

Stop here. Await `GO`, `MODIFY`, `MORE RECON`, `HOLD`, `PARK`, `ROLL BACK`, or
`ABORT` from the human reviewer before doing compiler-unification Phase 2 work.
