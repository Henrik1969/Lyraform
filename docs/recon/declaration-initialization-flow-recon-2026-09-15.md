# Declaration, initialization, and directional-flow reconnaissance

Date: 2026-09-15  
Mission: Lyraform Language Maturation Mission 01, Phase 1 only  
Gate: 1 (fact check)  
Repository: `/home/henrik/Projekter/Udvikling/Flowcore`

## 1. Repository and revision baseline

The assessed checkout was the canonical line identified by `Lyraform/CURRENT.md`
and `AGENTS.md`: local `main` at
`680a79592b7db4f7aa5fd1d0afe52a211794ead1`, upstream `origin/main`, with
ahead/behind `0/0`. The commit date is `2026-09-15T21:22:35+02:00` and its
subject is `docs: publish scheduling inventory status`.

The worktree was clean before reconnaissance. `git worktree list --porcelain`
reported one worktree, on `main`. The repository remained free of tracked
changes until this report was created. No source file was edited, no commit or
push occurred, and neither FlowLFS nor Flowselection was touched.

Environment:

| Item | Observation |
|---|---|
| Igor | `./igor doctor`: PASS |
| Canonical verification | `./igor check`: PASS, 156/156 registered CTests |
| C++ (GCC) | 13.3.0 |
| Clang | 18.1.3 |
| CMake | 3.28.3 |
| Ninja | 1.11.1 |
| Build used for probes | `/tmp/lyraform-build` |

Focused evidence:

| Gate | Observed result |
|---|---|
| AST goldens | PASS, 28 |
| Symbol projection goldens | PASS, 14 |
| Frontend bundle | PASS, 8 goldens + 1 isolated positive + 19 negative |
| Flowanalyst, Flowbind, Flowparallel, Flowoptimize, Flowlower pipeline CTests | PASS, 5/5 |
| Profile-free lowering, TinyVM scalar parity, support boundary, SymbolTable CTests | PASS, 4/4 |
| Categorized direct-execution suite | FAIL, 83/138 pass and 55 `UNEXPECTED_FAIL` |

The categorized failure is a taxonomy/path discrepancy, not 55 newly diagnosed
declaration defects: many files under `examples/pass/` are compiler-chain or
provider-boundary fixtures that the legacy direct interpreter cannot execute.
The full generated report was preserved during analysis and then removed from
the source tree. The repository's green 156-test CTest graph does not run that
categorized suite as a CTest.

## 2. Current documentation authority

Current identity and branch authority come from `AGENTS.md` and
`Lyraform/CURRENT.md`: Lyraform, Igor, `main`, experimental/unstable. Language
surface descriptions currently come primarily from
`docs/language/flowmini-programmers-manual.md`, the Flowmini book, and the
frozen v0.24 frontend contracts. The latter remain authoritative for their
versioned structural artifact, but are historical language-border evidence,
not complete current executable semantics.

The mission's accepted aggregate-member decision (`x : int` canonical,
`field x : int` compatibility) is newer than the checked-in language manual
and executable compatibility parser. This report treats that decision as
governing design context, without changing implementation.

## 3. Implementation ownership map

| Concern | Current owner(s) | Finding |
|---|---|---|
| Lexing `->` / `=>` | `Lyraform/compiler/src/flowmini_lexer.cpp`; token declarations in `include/flowmini_lexer.h` | Distinct `PlaceArrow` (`->`) and `Arrow` (`=>`) tokens. |
| Structural declaration parsing | `flowmini_ast_builder.cpp:2216-2257` | Builds `LetStatement`; initializer is optional in the AST. |
| Structural placement parsing | `flowmini_ast_builder.cpp:2268-2400` | Builds `PlacementStatement` or arrow-form `ReturnStatement`. |
| AST representation | `include/flowmini_ast.h:38-296` | Arena IDs; `LetStatement`; `PlacementStatement`; closed `AssignableTarget` variant. |
| Structural record members | `flowmini_ast_builder.cpp:328-380` | Accepts canonical bare members and optional compatibility `field`; the source-form distinction is not retained. |
| Frontend bundle/projection | `flowmini_frontend_bundle.cpp`, `flowmini_symbol_projection.cpp`, `flowmini_ast.cpp` | Exports AST and symbols without running executable semantic parsing. |
| Legacy executable declaration parsing | `flowmini_parser.cpp:1390-1700` | Separate parser requires initializers for scalar, record, list, and array values. |
| Legacy executable placement/type resolution | `flowmini_parser.cpp:709-731, 749-825, 1703-1780` | Resolves identifier/member/index target, checks type compatibility, then emits ModuleSpec operations. |
| Legacy local/value storage | `flowmini_payload.{h,cpp}`, `flowmini_runtime.cpp` | A mutable `RecordPayload` maps compiler-generated paths to value-owned variants. |
| Legacy aggregate/index mutation | `flowmini_runtime.cpp:400-465, 768-935` | Dedicated list/array/record get/set nodes mutate the envelope value; checked runtime bounds. |
| Semantic analysis | `Flowanalyst/src/main.cpp` | Resolves names and declared type names, but does not generally check declaration initializer or placement destination type compatibility. |
| Lowering-plan construction | `Flowanalyst/src/main.cpp:987-1099, 1155-1395` | Initializers become `value_definition`; identifier placements become `assignment`; target shape is not fully carried. |
| Binding | `Flowbind/src/main.cpp` | Provider authorization only; provider-free declaration probes produce `ready` with zero requirements. |
| Planning/pass-through | `Flowparallel/src/main.cpp`, `Flowoptimize/src/main.cpp`, `Flowlower/src/prepare.cpp` | Preserve the lowering operations for these provider-free scalar probes. No local-value Graph IR distinct from the lowering plan was found. |
| LLVM | `Flowlower/src/structured_plan.hpp:205-285, 346-348, 819-903` | Symbol slots are LLVM allocas; definitions and assignments store into them. `int` maps to `i32`. |
| TinyVM | `subprojects/TinyVM/tools/lower.cpp:105-155, 434-448, 690-708` | Symbol identities map to typed VM slots; assignment currently adopts the source slot's carrier. |
| Diagnostics | `flowmini` main wrapper, each stage's main wrapper, runtime atoms | Structured diagnostics exist, but responsibility/stage differs sharply between paths. |

### Multiple active semantic paths

There are two materially different current source consumers:

1. `--dump-frontend-bundle` uses the structural AST builder and does not call
   `parseModule()`.
2. Direct execution and `--emit-flowir` call the separate legacy
   `parseModule()` parser/lowerer and execute ModuleSpec atoms.

The frozen `Lyraform/flowmini_v24_explicit_ast` tree is explicitly a historical
checkpoint and is not included by the root CMake build. It is also no longer
byte-identical to the active parser or AST builder. It is not a third current
compiler authority.

## 4. Scout probes

All scout sources and outputs were disposable files under
`/tmp/lyraform-declaration-recon-20260915`. Results are embedded here so that
the temporary directory can be removed.

Legend: `A` accepted; `R` rejected; `P` preserved; `L` lowered; `BL`
backend-limited; `C` compatibility; `AP` accepted but not preserved faithfully.

| Probe | Structural frontend / AST | Flowanalyst and plans | LLVM | TinyVM | Legacy direct path / observable result |
|---|---|---|---|---|---|
| `x : int(20)` | A/P: `let`, type `int`, literal initializer ID | A; `value_definition`; binding ready; plan ready | L; process result 20 | L; result 20 | A/L; prints 20 |
| `x : int` then `20 -> x` | A/P: initializer `null`, then identifier placement | A; assignment to symbol; plans ready | R late: `unsupported structured assignment` | L; result 20 | R parser: declaration requires initializer |
| `20 -> x : int` | AP: recorded only as placement to `x`; `: int` disappears | With later read: R unresolved `x`; alone: incorrectly `ok`, assignment lacks result symbol | R late: unsupported assignment | R contract: missing `result_symbol_id` | R: undeclared target `x` |
| initialized `x`; `20 -> x`; `30 -> x` | A/P: one `let`, two indistinguishable placement payloads | A; one `value_definition`, two `assignment` operations | L; stores 0, 20, 30; result 30 | L; result 30 | A/L; prints 30 |
| canonical `type Point { x : int; y : int }`, initialized `p` | A/P; bare members represented as fields | A; record literal is emitted as `type:"unsupported"`, but report/plan still `ok/ready` | BL: unsupported structured value definition | BL: record literal not admitted | R parser: only `field` spelling accepted |
| canonical member `30 -> p.x` | A/P: `FieldPathTarget(base=p, fields=[x])` | A, but assignment loses target/result identity | BL before member lowering | BL before member lowering | R at canonical type syntax |
| compatibility record plus `{x:10,y:20} -> p` | AP: placement exists but source expression is `unknown` | Incorrectly `ok/ready`; unsupported operand | R | R | R parser at leading `{` |
| compatibility records, `source -> destination` | A/P identifier placement | Aggregate initialization already blocks newer backends | BL | BL | C/A: emits `record.copy`; after mutating `source.x`, `destination.x` remains 10 |
| `30 -> items[index]` | A/P: indexed target and index expression ID | A, but target base/result identity is lost; list literal marked unsupported | BL: unsupported list definition | BL: list literal not admitted | A/L; prints `1,30,3`; OOB is `list.set` failure |
| `30 -> matrix[row,column]` | A/P: indexed target with two index IDs | A, but target base/result identity is lost; array initializer appears as unsupported list literal | BL | BL | A/L; prints rows `1 2` / `30 4`; OOB identifies the failing axis |
| `true -> x` where `x : int(0)` | A/P | Incorrectly `ok/ready`; binding ready | R late: carrier mismatch, `unsupported structured assignment` | **Incorrectly L; returns 1** | R semantic lowerer: Bool-to-int mismatch |
| `x : int(true)` | A/P | Incorrectly `ok/ready`; bool `value_definition` associated with int symbol | R late: `unsupported structured return` | **Incorrectly L; returns 1** | R semantic lowerer: initializer is not int |

### Stage-by-stage classification

| Form | source | parser/AST | semantic analysis | binding/planning | lowering representation | LLVM | TinyVM | result/diagnostic |
|---|---|---|---|---|---|---|---|---|
| initialized scalar | accepted | accepted | accepted | accepted | accepted | accepted | accepted | parity: 20 |
| repeated scalar placement | accepted | accepted | accepted | accepted | accepted | accepted | accepted | parity: final value 30 |
| declaration then first placement | accepted structurally | accepted | accepted | accepted | accepted | backend-limited | accepted | LLVM/TinyVM disagreement |
| flow-first declaration candidate | rejected semantically | accepted-but-not-preserved | ambiguous/incomplete | invalid plan possible | target identity absent | rejected | rejected | no implementation support |
| canonical aggregate declaration/member | accepted structurally | accepted | accepted despite unsupported operands | accepted | preserved incompletely | backend-limited | backend-limited | no executable canonical path |
| compatibility aggregate member | compatibility | accepted | accepted | accepted | preserved incompletely | backend-limited | backend-limited | executable only in legacy path |
| record literal placement | rejected semantically | accepted-but-not-preserved | incorrectly accepted | incorrectly ready | source operand becomes unknown | rejected | rejected | no supported form |
| indexed placement | accepted | accepted | accepted | accepted | target not preserved | backend-limited | backend-limited | executable only in legacy path |
| type-invalid init/placement | rejected in legacy semantics | accepted | incorrectly accepted | incorrectly ready | invalid carrier relation retained | rejected late | incorrectly accepted | backend disagreement |

## 5. Observed semantics

### OBSERVED

1. A declaration is represented structurally as a `LetStatement` and projected
   as a symbol. In the legacy executable path it also creates a typed symbol-to-
   path association. In LLVM it leads to an `alloca`; in TinyVM it leads to a
   symbol slot when the symbol is used. It is therefore currently a combination
   of binding/symbol introduction and backend storage destination, not merely
   one of those concepts.
2. `x : int(20)` is not parsed as a call to an `int` function or constructor.
   The structural AST owns type `int` plus a literal initializer expression.
   Flowanalyst emits `value_definition`; LLVM emits allocation plus store;
   TinyVM emits a literal plus symbol-slot movement.
3. In the initialized case, the declaration initializer and later placements
   are distinguishable: the former is `value_definition`, the latter
   `assignment`. Two later placements are not semantically distinguished from
   each other.
4. The legacy execution path implements placement as writing to a compiler-
   generated path in one mutable record envelope. Repeated scalar writes
   overwrite the same path.
5. The canonical AST permits identifier, field-path, and indexed placement
   destinations. `-> return` is a separate `ReturnStatement`, not an
   `AssignableTarget`.
6. Member and index destinations share `PlacementStatement` and the closed
   target variant structurally. The legacy parser shares one
   `lowerAssignmentToTarget` entry, then selects distinct runtime operations.
   The current Flowanalyst lowering-plan construction only resolves the
   identifier target spelling and loses field/index destination identity.
7. Legacy compound values are value-owned C++ variants. `record.copy` performs
   a value copy; the probe proves the destination is unchanged after subsequent
   source-member mutation. List, array, and member setters mutate the selected
   value inside the envelope. No reference semantics are exposed by these
   forms.
8. The legacy parser enforces destination existence, member existence, index
   carrier/rank, declaration/placement type compatibility, and literal/refined
   restrictions. List/array bounds are checked by runtime atoms.
9. Flowanalyst checks declared type-name resolution and some special Text,
   field-access, call, and refined-invariant facts, but not general initializer-
   to-declaration or value-to-placement-target type compatibility.
10. Valid initialized scalar definitions and repeated identifier placements
    agree between LLVM and TinyVM. Declaration-without-initializer and invalid
    Bool-to-int cases do not.

### INFERRED

- The deliberate current semantic model for executable legacy locals appears
  to be mutable, value-owned slots carried in an execution record. The newer
  backend model appears intended to preserve mutable symbol slots, but its
  semantic authority is incomplete.
- There is no implemented distinction between “first placement” and later
  mutation once a declaration exists. Such a distinction cannot be inferred
  from current placement operations.
- Parenthesized declaration syntax currently compresses symbol introduction
  and initial value definition; the implementation offers no evidence that
  `int(20)` denotes a general constructor protocol.

### OPEN QUESTION

- Must every declaration be definitely initialized at declaration, or may a
  declaration establish typed storage completed by one later placement?
- If later first placement is admitted, what flow-sensitive rule proves use-
  before-initialization across branches, loops, calls, and graph activations?
- Is `int` permanently a 32-bit backend carrier? The legacy payload stores
  signed 64-bit values, while LLVM and TinyVM map `int` to 32 bits; the current
  documents do not define the full range/overflow contract here.
- Should aggregate values remain deep-copy values for ordinary placement, and
  if so where is that ownership/copy law made canonical beyond the legacy
  runtime implementation?
- Should bounds failures be language-level checked failures, traps, outcomes,
  or backend refusal? Only the legacy interpreter currently demonstrates them.
- Should compatibility `field` spelling retain explicit source-form provenance
  for diagnostics/migration, or is normalization at AST ingress sufficient?

### CANDIDATE

Candidates are recorded for Gate 1 only; none is selected here.

- Retain mandatory initializer syntax and document `Type(expr)` as declaration
  initialization, not general construction.
- Admit declaration without initializer plus a statically proven first
  placement, with explicit definite-initialization semantics.
- Keep one placement operation but add complete typed target identities for
  identifier/member/index targets to the semantic/lowering artifact.
- Split initialization and mutation into distinct semantic operations while
  retaining `->` as the directional source spelling.
- Keep aggregate/index execution as an explicit backend refusal until a
  versioned ownership, bounds, and target-lowering contract is complete.
- Retain the dual structural/legacy paths and document their boundaries, or
  consolidate syntax and semantic authority behind the versioned AST.

## 6. Implementation divergences

1. **Canonical record syntax:** the structural AST accepts bare members; the
   direct executable parser accepts only `field`; current pass examples and
   standard-library record declarations predominantly still use `field`.
2. **Initializer requirement:** the AST permits no initializer; the legacy
   parser rejects it; the newer chain admits it, LLVM cannot lower it, and
   TinyVM can execute it after placement.
3. **Target preservation:** field/index targets are explicit in AST but are
   not represented in lowering operations. Flowanalyst asks only for
   `target.name`, which exists only on identifier targets.
4. **Type authority:** legacy parsing rejects type-invalid placement early;
   Flowanalyst and the plans mark it ready; LLVM detects an incompatible
   carrier only while emitting; TinyVM overwrites the destination slot carrier
   and executes.
5. **Aggregate/index coverage:** direct ModuleSpec/runtime supports records,
   lists, arrays, member/index reads/writes, and bounds diagnostics; generic
   native/TinyVM lowering does not.
6. **Flow-first candidate:** structural parsing silently discards `: int` after
   the target. This is not legitimate candidate support.
7. **Return:** the AST correctly models `value -> return` as control transfer;
   the legacy direct parser only has a synthetic return symbol while lowering
   function bodies, so a main-level arrow return is rejected by that path.

## 7. Documentation contradictions

| Documented claim | Checkout evidence |
|---|---|
| Manual teaches `type Point { field ... }` as the record form. | Mission authority says bare member is canonical and `field` is compatibility; structural AST agrees, executable parser does not. |
| Manual says current assignable targets are identifier, field, and indexed. | True for AST and legacy interpreter, but not end-to-end through the canonical LLVM/TinyVM chain. |
| Manual says declaration gives a name, type, and initializer. | True for legacy execution; structural AST permits missing initializer and the newer chain partially admits it. |
| Historical v0.24 matrix says the placement probe demonstrates executable/parser agreement. | That statement is historical and uses compatibility `field`; it does not cover current bare-member syntax or end-to-end lowering. |
| `Lyraform/CURRENT.md` cites a root 81-test result. | Current Igor/CTest count is 156. |
| `Lyraform/compiler/README.md` opening baseline cites AST 26, projection 11, frontend 5/1/11, suite 78, CTest 2. | Current focused counts are AST 28, projection 14, frontend 8/1/19; categorized suite is 83/138; root CTest is 156. |

## 8. Proven gaps and estimated affected components

| Proven gap | Likely affected components | Surface |
|---|---|---|
| Bare record members rejected by executable parser | legacy parser, direct-runtime fixtures/tests, compatibility diagnostics, docs | Medium |
| General initializer/placement type compatibility absent after frontend artifact | Flowanalyst semantic facts/diagnostics, lowering-plan schema/validation, negative tests | High |
| TinyVM executes Bool-to-int invalid programs | lowering artifact validation, TinyVM symbol-slot carrier rules, parity/refusal tests | High and correctness-critical |
| Field/index target identity lost after AST | Flowanalyst target resolution, lowering-plan schema, Flowcontracts validation, both lowerers | High |
| Record/list/array definitions and member/index operations not lowered | expression/type model, artifact schema, LLVM, TinyVM, runtime/bounds contract, parity tests | High |
| Missing initializer has contradictory acceptance | grammar policy, semantic representation, definite-initialization analysis, both backends, tests/docs | High if admitted; Low if uniformly refused |
| Flow-first candidate suffix silently disappears structurally | structural parser validation/recovery and negative tests | Low/Medium |
| Aggregate copy/ownership law exists only in legacy runtime behavior | language contract, semantic facts, lowering/backends, tests/docs | Medium/High |
| Test taxonomy and counts disagree with docs | suite classification, current-status docs, verification policy | Medium |

## 9. Candidate directions and cost surface

| Candidate direction | Grammar/parser | Semantics/artifacts/graph | LLVM/TinyVM/runtime | Tests/compatibility/docs |
|---|---|---|---|---|
| **Do nothing / retain behavior** | None | None | None | High documentation burden; preserves known backend mis-admission and is not legitimate for the type-safety defect unless invalid programs are explicitly outside the accepted language before TinyVM. |
| Require initialization at declaration everywhere | Add structural rejection or semantic diagnostic; keep syntax | Definite-initialization remains simple; plan requires every local definition | Small backend change; reject absent definitions before lowering | Update negative cross-chain tests; compatibility impact on structural-only uninitialized probes |
| Admit declaration then first `->` | Grammar already represents it | Add storage declaration and definite-initialization facts; version plan target/initialization state | Allocate typed slots before first write; consistent uninitialized-use refusal | New branch/loop/use-before-init suites; docs and compatibility decision |
| Consolidate behind AST + Flowanalyst | Retire duplicate syntax authority gradually; preserve `field` compatibility | Flowanalyst becomes semantic authority; versioned diagnostics and target facts | Direct interpreter must consume canonical semantic/lowering representation or become compatibility-only | Broad regression/migration surface; clearest long-term authority but not a Phase 1 decision |
| Extend target-aware placement plan | No syntax change | Add typed identifier/field/index target variants and ownership/bounds facts; graph involvement only if values cross graph ports | Implement record/list/array storage and writes in LLVM/TinyVM; runtime failure contract | Extensive positive, negative, parity, malformed-artifact, and compatibility tests |
| Split initialization from mutation semantically | Source syntax may remain unchanged | Distinct plan operations/state transitions; explicit first-write law | Separate initialization/store checks in both backends | More artifacts and tests; improves diagnostics; compatibility can preserve source spelling |
| Refuse unsupported target/type forms earlier | No grammar change | Flowanalyst blocks unsupported or unproved cases; plans cannot be `ready` | Backends simplify and agree by refusal | Moderate negative/refusal tests; narrows claimed language temporarily |
| Preserve `field` source provenance | Parser records optional compatibility marker | Small AST/schema revision if externally visible | None | Enables warnings/migration tooling; versioning and golden churn |

## 10. Gate 1 decision inputs

### FACTS

- Declaration currently introduces AST structure and a symbol; executable
  paths additionally materialize mutable storage/slots.
- `Type(expr)` is represented as declaration plus initializer, not as a
  general constructor call.
- Initialized scalar placement and repeated scalar mutation execute with
  LLVM/TinyVM parity; final value is the last placement.
- Member/index placement is structurally first-class and executes in the
  legacy interpreter, but is not carried through generic lowering.
- Legacy aggregate/list/array values are owned by value and bounds are checked
  at runtime.

### GAPS

- Canonical bare record members do not execute through the legacy parser.
- General type compatibility is not enforced by Flowanalyst.
- TinyVM admits and executes proven Bool-to-int invalid programs.
- Missing-initializer behavior and aggregate/index lowering disagree by path.
- Field/index target identity is lost between AST and lowering plan.

### SURPRISES

- A structurally accepted `20 -> x : int` loses the apparent declaration
  suffix rather than being rejected at the frontend boundary.
- Flowanalyst can publish `ok` and a `ready` plan for an undeclared write-only
  target and for type-invalid initialization/placement.
- LLVM refuses those type-invalid programs, while TinyVM emits and returns 1.
- The current categorized direct suite is 83/138 despite the 156/156 CTest
  graph and older published counts.

### CANDIDATE DIRECTIONS

- Do nothing and document exact path limitations.
- Require declaration-time initialization consistently.
- Admit declaration plus statically proven first placement.
- Consolidate syntax/semantic authority behind AST + Flowanalyst.
- Add typed target-aware placement operations for member/index destinations.
- Split semantic initialization from mutation while retaining directional `->`.
- Refuse unproved forms in Flowanalyst until lowerers support them.
- Preserve compatibility `field` source provenance if migration diagnostics
  are desired.

### COST SURFACE

The detailed candidate table above records effects on grammar/parser,
semantic representation, artifacts, graph representation, lowering, LLVM,
TinyVM, runtime, tests, compatibility, and documentation. The smallest surface
is early uniform refusal; the largest is admitting uninitialized storage plus
aggregate/index mutation end-to-end. Fixing TinyVM's type-invalid admission is
a correctness boundary regardless of the later syntax decision.

### GATE STATUS

```text
GATE 1: WAITING FOR HUMAN REVIEW
```
