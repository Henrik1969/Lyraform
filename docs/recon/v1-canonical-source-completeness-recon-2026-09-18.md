# Mission 04 — canonical source completeness reconnaissance

Date: 2026-09-18. Phase 4 reconnaissance only. Stop: Gate 4.

## 1. Conclusion and baseline

**Successful structural export does not prove complete source consumption or
compilation validity. Do not begin member/index semantic migration yet.**
Meaningful suffixes and whole source fragments can disappear. The staged path
can admit the resulting projection even when the direct scalar path refuses
the original source. This is an upstream source-completeness problem, not a
reason to weaken the shared scalar type rules.

Repository: `/home/henrik/Projekter/Udvikling/Flowcore`, branch `main`.
Initial/current HEAD and local `origin/main`:
`c03079e46e938e98b04f3bad21c991cc4f58eee4`.
Baseline commands: `git status`, `git branch --show-current`,
`git rev-parse HEAD`, `git rev-parse origin/main`, `git log -3 --oneline`.
Initial worktree: ten modified tracked files and eleven untracked files,
all inherited Mission 02/03 work; nothing staged. No new fetch was needed to
observe that baseline, and this report does not claim a new remote-tip check.

Only this report and the adjacent observational probe script were added.
No production source, existing tests, build graph, language policy, runtime,
backend, or Mission 03 ownership changed. No commits, pushes, branch operations,
FlowLFS work, or `master` changes occurred. Final worktree therefore retains
the ten inherited tracked modifications and has thirteen untracked files.

## 2. Reproducible evidence and verification

The companion [probe script](v1-source-completeness-probes-2026-09-18.sh)
contains every fixture and comparison. It is an observational harness, not a
new parser conformance specification or production test gate.

Commands run:

```sh
cmake --build /tmp/lyraform-phase2-build \
  --target flowmini flowmini_scalar_path_test flowanalyst -j 4
sh docs/recon/v1-source-completeness-probes-2026-09-18.sh
ctest --test-dir /tmp/lyraform-phase2-build \
  -R '^lyraform_scalar_source_path$' --output-on-failure
git diff --check
```

Build result: exit 0, `ninja: no work to do`. The existing Mission 03 binaries
were current. The final probe run exited 0 and produced
`/tmp/lyraform-phase4-recon.IwT9Zk/` (ephemeral evidence). An earlier run used
`/tmp/lyraform-phase4-recon.bGAfgB/` before adding the direct/staged comparison.

Final corpus: **68 probes**, **65 structural exports successful**, **3
structural refusals**, **51 analyst `ok/ready`**, **14 analyst `error/blocked`**.
No timeout occurred. **23 paired normalized AST/graph comparisons were equal**.
Normalization removes only `location`/`provenance`, not expression text, names,
types, child IDs, source-form fields, or graph payloads. Equality is semantic
projection equality, not equality of all source coordinates or bundle bytes.

Nine test-only direct-path observations: two ordinary scalar controls succeed;
seven malformed/lossy forms refuse. The guard remains effective for these
probes. The existing scalar-source-path CTest passed **1/1 in 3.88 seconds**.
`sh -n` on the companion script and `git diff --check` also passed.
No full build/test/sanitizer campaign was necessary for documentation-only
reconnaissance; Mission 03's 159/159 result is prior evidence, not a new run.

Most fixtures use `--dump-frontend-bundle` on a `.flowir` filename to bypass
import expansion and observe the structural parser directly. This flag still
invokes the structural frontend, not the runtime IR parser. Missing imports in
these raw probes are not claims that normal import resolution succeeds.
The decisive staged-loss case was additionally run as a normal `.flow` input.

### Executable staged-loss witness

```text
program probe
main {
x : int(0)
20 -> x : Bool
x -> return
}
```

The `: Bool` suffix is absent from the AST. Flowanalyst emits `ok/ready`;
Flowparallel, Flowoptimize, flowprepare, flowvalidate and both lowerers accept
the shortened projection. The generated LLVM executable exits **20** and
TinyVM reports **result 20**. Exact commands and generated artifacts are in the
probe script/output directory (`staged-loss.*`). No external provider is run.

This does **not** assign semantics to typed placement. That form is explicitly
outside the admitted slice. The defect is accepting it as the shorter form
without representing or refusing the extra source. Shared facts cannot recover
syntax that disappeared before analysis. The corresponding direct scalar
program ending in `print x` refuses in `require_source_coverage`.

## 3. Active parser topology and ownership

Primary sources:
[builder](../../Lyraform/compiler/src/flowmini_ast_builder.cpp),
[lexer](../../Lyraform/compiler/src/flowmini_lexer.cpp),
[AST model](../../Lyraform/compiler/include/flowmini_ast.h),
[AST export](../../Lyraform/compiler/src/flowmini_ast.cpp),
[frontend bundle](../../Lyraform/compiler/src/flowmini_frontend_bundle.cpp),
[source entry](../../Lyraform/compiler/src/main.cpp),
[scalar guard](../../Lyraform/compiler/include/flowmini_scalar_source.h).
Line references below are to this exact worktree; `B` means the builder.

1. `main.cpp` reads/expands source and builds a line-origin mapping. Import
   scanning/expansion is a separate source-processing boundary.
2. `lexSource` produces tokens with kind, decoded text, start line and column.
3. `build_source_header_ast` (B2884) first calls the independent
   `capture_graph_syntax` scan (B2790), records unsupported graph locations,
   then walks top-level tokens and appends declaration/statement/expression pools.
4. Declaration helpers own an advancing `i` or return the next index. Body
   helpers recognize statement **shells**; expression helpers receive token
   slices and populate child nodes rather than return a complete-consumption
   result. Target parsing takes `i` **by value** and returns only a target.
5. Symbol projection attaches scopes/origins. Bundle export does not apply
   the direct scalar coverage guard. Its general AST has no compilation-valid
   flag or consumed-token ledger. Existing graph diagnostics are a narrow,
   explicit exception.
6. Direct execution classifies the AST first. Only its selected whole-source
   scalar slice invokes the guard; other recognized territory uses explicit
   legacy compatibility parsing. Staged Flowanalyst consumes the exported
   projection, not the original source token stream.

### Token-advancement / skip and recovery audit

| Owner/family | Position/termination behavior | Classification and risk |
| --- | --- | --- |
| B31 `parse_qualified_type_name` | consumes identifier/dot-name prefix; malformed dot stops | PARTIALLY REPRESENTED; no complete-name result |
| B60 `consume_type_ref_spelling`, B92 `parse_type_ref` | consumes generic/array groups; optional closing tokens; skips unexpected generic tokens; depth 64 produces UnknownTypeRef | PARTIALLY REPRESENTED / INTENTIONALLY DEFERRED depth fallback; not a validity proof |
| B184 `parse_function_signature` | optional `(`; skips commas/newlines and unexpected non-identifiers; colon/type and closing `)` optional | partial parameter/header nodes possible; no canonical/recovery distinction |
| B275 `skip_until_line_end` | advances through all remaining non-newline tokens | safe only if already classified; import, ABI, refinement, target callers can discard meaningful suffixes |
| B302 imports | keeps module string/alias; missing alias after `as` explicitly throws; remaining line skipped | selective validation, unsafe suffix loss |
| B328 record fields | skips separator tokens; optional `field`; ignores unrecognized tokens; colon/type and closing brace optional | compatibility normalization plus accidental loss/recovery |
| B383 refined/type declarations | optional/member shells; unknown invariant lines skipped; recognized invariant builds shallow comparison leaves | possible loss within meaning-bearing invariant; no broad invariant-parser assurance |
| B517/585/637/696 ABI helpers | recognize known property/clause prefixes; skip tails/unknown lines; missing bodies/closers may still append declarations | known source properties represented, unknown obligations not classified |
| B802 `skip_group` | counts one delimiter kind, returns on close or EOF without an error result | swallowed bare/body/top-level groups can contain meaningful source |
| B834 `skip_nonsemantic_separators` | consumes commas/newlines | structural separator normalization; repeated separators' legality is not established by this helper |
| B1011 operator split / B1125 postfix / B1235 expression slice | balance-aware scanning, postfix end-position check; expression boundaries stop slicing | useful local structure, not exhaustive ownership of slice contents |
| B1291 shallow expression / B1336–2153 population | defaults to first leaf; empty argument/list segments may disappear; record fields without usable colon/value return; recursion stops at 64 | accidentally discarded tails or partially populated expressions |
| B2224 typed binding shell | creates declaration and initializer placeholder; returns at initializer opener rather than its validated end | later body scanner advances over leftovers without a consumption proof |
| B2296 index skip / B2317 target | target local index never returned; dot branch or index branch returns a prefix | compound suffixes/missing delimiters not validated |
| B2365 placement | parses source/target then advances to newline/right brace regardless of target parse endpoint | semantically meaningful suffix loss |
| B2403 equals assignment | records value placeholder; returns near expression start | same shell/outer-scan weakness |
| B2443 header skip; B2462 if / B2522 while | scans to brace/newline; creates UnknownStatement if incomplete; optional else attachment | header junk may vanish; recovered node may be exported as ordinary input |
| B2560 body scanner | closes at right brace; skips bare groups; recognizes return/break/continue then advances; otherwise `++i`; EOF accepted | unknown tokens, nested bare blocks, trailing control tokens may disappear |
| B2667 body container | creates block only when opener is present | missing body is represented as absence, not necessarily an error |
| B2690 target | recognizes owned functions/mains; unknown lines skipped; missing opener still appends target | other target contents can vanish |
| B2759 top-level resynchronization | skips grouped material and arbitrary tokens until a recognized boundary | recovery-like behavior without explicit recovery status |
| B2790 graph capture | depth-zero declarations use strict `require`; demands newline/End after recognized node/wire/policy/state | narrow positive full-line check; nested/bare/unrecognized graph forms outside it |
| B2884 top-level walk | program/unit header; optional name; recognized declarations; final fallback `++i` | global EOF reached does not prove ownership of traversed tokens |

Advancing to EOF is not equivalent to consuming every token into a recognized
role. Local graph/import errors do not turn the overall shell parser into a
strict parser. No production hook was necessary to observe this behavior.

## 4. Source spans and provenance

- `Token` (`flowmini_lexer.h:62`) contains kind/text/start line/column, **no
  byte offset, end offset, token identity, or original lexeme range**. Lexer
  local byte index is not retained. Columns advance through bytes; tabs are
  incremented rather than expanded into display columns.
- Spaces/tabs/CR and comments disappear. Newlines outside block comments
  produce tokens and can delimit constructs. Strings are decoded, so token
  text is not a complete record of original spelling.
- AST `SourceLocation` is only line/column. C++ expressions, blocks,
  declarations, fields, statements and target segments have start locations,
  not complete spans or consumed-token intervals. Locations sometimes identify
  an operator or first token after normalization, not the original construct start.
- Exported expression-pool entries omit even their C++ start location;
  exported blocks contain IDs/statements without block locations
  (`flowmini_ast.cpp:725,743`). Selected child/type/member locations survive.
- AST paths and symbol/scope origin descriptors identify **represented**
  entities and roles. They cannot account for tokens for which no node exists.
- `ExpandedSource` (`main.cpp:260`) associates rendered lines with original
  file/line; generated blanks may have no origin. Bundle `source_map` maps
  expanded lines to file IDs and original lines. This supports diagnostics,
  not within-line token completeness or a source-integrity attestation.

**Current metadata cannot prove that every meaningful token in an owned
construct has a recognized role.** Small candidate additions are original
token intervals plus a complete/recovered/refused parse outcome and explicit
compatibility source-form evidence. These are representation/validation
candidates only; no schema or AST change is made here.

## 5. Mission 03 source-coverage guard

`require_source_coverage` is scalar-specific. It compares ordered
`Token.text` values with a projection of the AST, drops newlines and parentheses
from that comparison, checks brace/parenthesis balance, matches parenthesis
spans against projected expression intervals, requires initializer delimiters,
one main, and one scalar statement per start line. It also bounds expression
traversal. Those intervals are **indices in a normalized token-text vector**,
not persistent original-source ranges. Token kinds/lexeme offsets do not
survive the equality check.

It proves bounded agreement with that projection algorithm, not the general
language grammar or every possible token-role assignment. In nine direct
probes it admitted the two ordinary controls and refused declaration/literal
tails, missing/extra parentheses, unknown lines, a discarded nested bare block,
and a placement type suffix. No bypass of the selected direct slice was found
in this corpus; this is not an exhaustive proof.

Nested constructs have two distinct outcomes: represented non-migrated
constructs classify the whole program into compatibility before the guard;
discarded nested source in an otherwise scalar AST can be caught by the token
comparison (`nested_block`). The guard is not a blanket protection for mixed
programs or staged exports. Extending its reconstructed-token comparison into
all domains would duplicate increasingly large amounts of syntax knowledge.
A parser-owned consumption/outcome invariant would be cleaner.

## 6. AST completeness matrix

`S` = start-only locations, no full span; `E` = expression start exists in C++
but root expression export omits it; `O` = represented origin/role available.
“No” means no full-consumption guarantee for arbitrary successful input, not
that the ordinary form has no useful representation. Owner numbers refer to B.

| # | Construct | Parser owner | Full consumption proven? | Meaningful loss / probe evidence | Recovery | Span | Canonical-safety status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | Lexical tokens | `lexSource` | recognized tokens only, not syntactic ownership | trivia removed; text decoded; no raw ranges | malformed UTF-8/comments and unknown characters have error boundaries | S | not source-completeness proof |
| 2 | Declarations | 2884 dispatch | No | unknown prefixes/lines skipped | append partial / advance | S,O | domain-specific validation required |
| 3 | Initialized scalars | 2224 | No | `decl_tail`, `literal_tail` equal ordinary AST | shell | S,E,O | direct guarded; staged gap |
| 4 | Placement | 2365 | No | `place_type`, `place_junk` lose suffix | scan to boundary | S,E | critical staged gap |
| 5 | Return forms | 2560 / 2365 | No | source-form enum preserves keyword vs arrow; `return_tail` loses extra literal | scan/advance | S,E | source-form good, consumption incomplete |
| 6 | Unary | 1291/1556 | No | operator/operand retained in `unary`; child tail/depth risk | partial child | E | direct guarded where selected |
| 7 | Binary | 1011/1526 | No | precedence tree retained in `binary`; leaf tails can disappear | partial leaves | E | no general full-slice proof |
| 8 | Parenthesized | 1185/1281 | No | grouping changes tree; missing/extra parens still export | optional matching/leaf fallback | E | direct guard stronger than staged path |
| 9 | Calls | 1125/1403 | No | base/ordered arguments retained; `call_gap` identical to ordinary call | empty argument segment omitted | E | not ready for sole grammar authority |
| 10 | Parameters | 184 | No | ordered names/types retained; `parameter_gap` identical | unexpected tokens skipped; optional type | S,O | needs header consumption contract |
| 11 | Functions | 184/2667/2884 | No | `function_header_tail` loses body, leaves function declaration | resynchronizes past group | S,O | no function migration yet |
| 12 | Blocks | 2560 | No | `nested_block` entirely vanishes | bare group skip; EOF accepted | S; export lacks S | high risk |
| 13 | If | 2462 | No | `if_tail` equals normal if; `if_missing` UnknownStatement | explicit unknown but no validity flag | S,E | high/critical admission gap |
| 14 | Else if | recursive 2462 | No | ElseIf ID structure present in `else_chain` | continuation recovery | S,E | structure useful, no full header proof |
| 15 | Else | 2462/2560 | No | ordinary ElseBlock retained; `else_dangling` block lost | unsupported standalone else advanced/skipped | S | unsafe to migrate now |
| 16 | While | 2522 | No | `while_tail` equals normal loop | header skip / unknown | S,E | high risk |
| 17 | Break | 2560 | No | `break_tail` retains break but loses following tokens | advance | S | ownership is not encoded by the token parser |
| 18 | Continue | 2560 | No | same trailing-token loss | advance | S | enclosing-loop validity is later work |
| 19 | Aggregate/type declarations | 383 / record branch | No | `type_missing` exports without closing brace; refinement comparison is shallow | optional body/close, skipped invariants | S,O | not ready |
| 20 | Aggregate members | 328 | No | `type_tail`; `field` spelling erased | optional syntax and token skipping | S,O | order retained; completeness/policy lost |
| 21 | Record literals | 2031/1976 | No | field order/value IDs retained; `record_literal_bad` equals ordinary form | malformed field can return without adding node | E, field S | unsupported semantics and incomplete parsing are separate |
| 22 | Member access expressions | 1125/1748 | No | recursive base/field supports `p.a.b`, `items[i].field` in `access` | postfix failure can fall back to leaf | E | richer than targets, not proof of completeness |
| 23 | Member placement targets | 2317 | No | `p.x[i]` and `p.x.` collapse to `p.x` | returns dot prefix | S, segment S | unsafe for typed-target migration |
| 24 | List literals | 1799 | No | ordered elements retained; repeated comma gap disappears | empty segment omitted | E | separator policy not explicitly recorded |
| 25 | Array declarations/forms | 92 | No | array element/extents retained in `array`; missing delimiters can leave partial type | optional closer, text extents | S | partial type differs from semantic support |
| 26 | Indexed expressions | 1125/1654 | No | recursive base and ordered indices represented | missing/empty children possible | E | not a completeness guarantee |
| 27 | Indexed targets | 2317/2296 | No | `items[i].field`, `items[i][j]`, missing `]` collapse to `items[i]` | returns first bracket prefix | S,E | unsafe; mixed targets not representable |
| 28 | Imports | 302 + ImportExpander | No at AST boundary | module/alias retained, `import_tail` suffix disappears | selective alias error, then line skip | S,O + line map | raw parse proof does not establish import resolution |
| 29 | Target blocks | 2690 | No | main/function IDs retained, `target_extra` port line lost | unknown line skip | S,O | incomplete ownership |
| 30 | Ports | endpoint in 2790; no general port declaration node | No | endpoint port name retained; standalone port declaration in target discarded | skip | endpoint S only | standalone form unsupported, not silently classifiable as canonical |
| 31 | Graph nodes | 2790 | Yes for recognized depth-zero line | node/role/implementation flags retained; `node_tail` refuses | explicit errors | S,O | strong local boundary, not whole-language proof |
| 32 | `=>` wiring | 2790 + graph-marker scan | Yes for recognized `wire` line | `wire_bad`/`wire_tail` refuse; bare wiring has marker but no wire record | explicit error or unsupported marker | S,O | distinguish parsed wire from intentionally non-executable syntax |
| 33 | Providers/contracts | ABI 517–696; graph syntax; external provider maps | No general source guarantee | ABI known clauses retained; `abi_unknown` unknown line erased; `provider_unknown` declarations disappear | line/token skip | S,O for known clauses | external contract validation cannot prove omitted source |
| 34 | Compatibility syntax | 328,184,637, header dispatch | No policy-wide accounting | optional `field`/`fn`, alternate spellings normalize without source-form tags | historical normalization | S/O inconsistently | inventory below; not automatically canonical |

## 7. Compatibility / retired syntax inventory

These are evidence classifications, not new retirement decisions.

| Form | Classification for this review | Evidence/policy consequence |
| --- | --- | --- |
| Bare `type Point { x : int }` member | CANONICAL structural spelling (per prior reconnaissance) | ordered field/name/type retained; completeness still unproved |
| `field x : int` member | TEMPORARY_COMPATIBILITY | B328 consumes keyword without storing a source-form flag; normalized AST equals bare member |
| `record Point { ... }` vs `type Point { ... }` | UNDECIDED | both accepted spellings produce RecordDecl; introducer not an enum field; no new v1 retirement decision |
| Function return `: Type` vs `-> Type` | UNDECIDED v1 spelling policy | B184 accepts both without recording which separator occurred |
| `extern fn f` vs `extern f` | TEMPORARY_COMPATIBILITY, final policy UNDECIDED | B637 makes `fn` optional, no source-form marker |
| Equals assignment vs arrow placement | distinct forms retained; v1 retirement policy UNDECIDED here | StatementSourceForm and distinct variants are a positive example |
| Keyword return vs arrow-to-return | distinct forms retained; policy not changed | source_form distinguishes them |
| Legacy `module` / runtime graph artifact | TEMPORARY_COMPATIBILITY | direct path explicitly recognizes header; structural source unit is unknown, graph syntax may still be captured |
| Unknown `provider`, `contract`, standalone `port` declarations | unsupported/unclassified, NOT granted compatibility authority | recognized fragments elsewhere do not establish a grammar for these forms |
| Any proposed RETIRE_BEFORE_V1 decision | not selected by this mission | retained spelling evidence is needed before an enforceable retirement gate |

For `field`, member order is preserved. A location may point to the keyword
rather than the bare name, and the original source could be reopened to inspect
it. **The AST alone cannot reliably tell that `field` occurred**: there is no
keyword role, source-form enum, or token range. Coordinate differences caused
by added text are not a trustworthy compatibility discriminator. A future
deprecation diagnostic needs explicit syntax evidence, not guessed columns.

## 8. Safe discard versus unsafe/ambiguous discard

| Required category | Observed examples | Could changing it change meaning/policy? |
| --- | --- | --- |
| SEMANTICALLY REPRESENTED | scalar values, operator trees, declaration types, retained member order, ordinary index order, return source-form | yes; represented nodes retain the relevant distinction in ordinary forms |
| STRUCTURAL PUNCTUATION | balanced grouping whose operator tree is retained; recognized block ownership; valid element separators | yes if structural role changes; safe to omit glyphs only when their role is fully encoded |
| LEXICALLY IRRELEVANT | ordinary spacing between already separated tokens, comments | generally no after lexing; deleting separators can merge tokens, and newline boundaries are not universally irrelevant |
| EXPLICITLY RETIRED / COMPATIBILITY | legacy module path; optional historical member `field` | runtime meaning may be redundant, but canonical-source policy can change; `field` lacks explicit AST classification |
| INTENTIONALLY DEFERRED | unsupported graph markers; uninitialized AST declarations; depth-limit UnknownTypeRef | representation or refusal must remain distinguishable from executable admission |
| ACCIDENTALLY DISCARDED | placement type suffix, literal tail, unknown body line, standalone nested block, ABI unknown clause | yes or possibly; successful silent discard is unsafe |
| PARTIALLY REPRESENTED | mixed member/index target, malformed header with retained prefix, missing-close type | yes; prefix is not the full construct |
| UNKNOWN | intended legality of duplicate separators, new standalone contract/port spelling, undecided typed-placement meaning | unresolved; this report does not supply language semantics |

## 9. Recovery versus compilation validity

The code has recovery-like behavior (functions explicitly named `shell`,
placeholders, UnknownExpr/UnknownStatement, skipping and resynchronization),
but no separate `RECOVERED_FOR_DIAGNOSTICS` versus `VALID_FOR_COMPILATION`
result or documented caller-selected strict mode was found. Tooling/IDE intent
is not inferred solely from tolerant implementation names.

`if_missing` produces UnknownStatement with text `incomplete if statement`,
an empty frontend diagnostic list, and Flowanalyst `ok/ready`. Missing
aggregate closers and dropped function bodies similarly export successfully.
Not all of these plans necessarily execute: a later stage can refuse an
unsupported operand/target. Such later refusal does not make the source
projection complete. Member placement with unresolved base `p` can even emit
an assignment operation without a resolved destination while still `ready`.

Design requirement for a later mission: recovery results must not silently
share execution authority with complete parses. A status label alone is
insufficient unless every admission path enforces it and accounts for skipped
tokens. Compatibility artifacts require an explicit policy rather than a
retroactive blanket assertion of source completeness.

## 10. Highest-risk findings, ordered by correctness impact

1. **CRITICAL — staged execution bypasses source completeness.** Extra typed
   placement syntax produces the same useful AST and runs on both backends.
   Also observed equal ready projections for initializer/return/header tails
   and discarded unknown/nested source. The scalar guard is not consulted on
   export; downstream type proofs describe the shortened AST only.
2. **CRITICAL — target meaning can collapse to a prefix.** Member/index
   combinations and suffixes are lost while AST payload stays equivalent.
   No target endpoint is returned to placement parsing. Backend execution of
   these specific target probes is NOT claimed; current semantic/backend
   refusal is distinct from the demonstrated structural loss.
3. **HIGH — partial/recovery nodes can be `ready`.** Incomplete `if`, missing
   closes, lost function bodies, dangling else, and ignored ABI obligations
   lack a general compilation-invalid boundary.
4. **HIGH — expression/member subparsers can omit meaningful content.**
   Extra literal tokens and record-value tails disappear; refined invariants
   use a separate shallow leaf construction. Generalizing ownership now would
   inherit these omissions.
5. **MEDIUM — source provenance is insufficient for coverage/retirement.**
   Start locations, incomplete export locations and absent compatibility
   source-form evidence cannot prove token ownership.
6. **LOW when correctly represented — ordinary punctuation/trivia removal.**
   Do not mistake missing comma/brace AST nodes for semantic loss when ordered
   children and ownership really retain the distinction. Duplicated or missing
   punctuation remains a grammar-policy/validation question, not automatically LOW.

## 11. Readiness of the next domains

**Typed targets: not ready for semantic migration.** `x`, `p.x`, `p.a.b` and
ordinary `matrix[r,c]` have useful structure; indices preserve order. But
AssignableTarget is a closed choice of IdentifierTarget, FieldPathTarget, or
IndexedTarget. It cannot compose arbitrary nested target segments. `p.x[i]`
loses `[i]`; `items[i].field` loses `.field`; `items[i][j]` loses the second
index; missing `]` is not proved absent by the node. `matrix[r,,c]` preserves
an UnknownExpr comma between r/c rather than recording a valid index grammar.
The richer recursive expression model must not be confused with target support.

**Aggregates: not ready.** Bare/field members normalize, member order survives,
but introducer policy and trailing-member syntax do not. Record literal value
tails can vanish. A fully represented ordinary record can still be semantically
unsupported; that is separate from the malformed cases.

**Control flow: not ready.** If/else-if/else/while relationships exist for
ordinary inputs, but header skipping, bare-block loss, dangling-else behavior,
and recovered UnknownStatement admission prevent sole authority. Break/continue
tokens are represented without a parser-owned proof of valid loop ownership.

**Functions: not ready.** Names, parameter order, types, return types and body
IDs are useful, but optional/missing delimiters and resynchronization can
turn a body-bearing source into a bodyless declaration. Call argument gaps
normalize, and return tails disappear. No activation or capture work was done.

**Graph syntax: stronger local boundary, not globally ready.** Recognized
depth-zero node/wire lines enforce required tokens and trailing-line termination;
the three graph-negative probes refuse structurally. Bare wiring receives
unsupported markers rather than a complete wire node. Depth-scoped graph
coverage, target contents, unknown standalone ports/contracts, and external
provider admission remain different boundaries. Do not infer a complete source
grammar from successful contract/artifact validation.

## 12. Candidate invariants and bounded corrective options

Candidate invariant: a compilation-valid owned construct has a complete
original-token interval, a recognized role for each meaningful token in it,
explicitly classified compatibility/deferred material, required delimiters,
and no recovery debt. Successful execution admission must require this outcome
at both direct and staged borders. These are review candidates, not implemented
schemas, syntax, or mandatory artifact requirements.

| Option | Affected components | Semantic impact | Artifact impact | Required tests | Rollback | Benefit |
| --- | --- | --- | --- | --- | --- | --- |
| A: bounded full-consumption invariant | builder statement/expression helpers, export/admission callers | refuse unconsumed owned syntax; no new language | internal parse result first; stage evidence policy needs review | paired mutations, exact end positions, ordinary corpus, both admission paths | medium | fixes source authority at its owner |
| B: source-coverage map | lexer token ranges, AST origins/export, stage validator | no new semantics; makes omissions measurable | likely versioned coverage evidence; old artifacts need explicit policy | gaps/overlaps, nested intervals, imports, source mapping, roundtrip | medium/high | scalable inspectable completeness proof |
| C: canonical/recovery distinction | builder result, diagnostics, frontend exporter, analyst admission | reject recovered input for execution; keep tooling inspection | validity/recovery field or separate API; compatibility policy required | UnknownStatement, missing delimiter, skipped-tail and error-mode corpus | medium | prevents recovery nodes impersonating valid source; insufficient alone if skips remain unrecorded |
| D: target-specific completeness first | target parser + placement cursor/end check | only refuse incomplete targets; no typed target semantics | can start internal; compositional target redesign explicitly deferred | x/p.x/p.a.b/indices, mixed suffix refusal, malformed brackets/dots, stage boundaries | low/medium | small preparatory slice; does not close staged scalar/header losses |
| E: compatibility spelling evidence | RecordField/declaration syntax origins and exporter | no immediate retirement; retain spelling distinctions | optional source-form field or versioned syntax evidence | field/bare pairs, alternate introducers, preserved source maps | low/medium | enables honest later deprecation/refusal |
| F: more recon / defer | probe corpus/report only | none | none | narrower questions or broader mutation corpus | low | safest if authority/compatibility policy is undecided, but leaves confirmed execution gap |

### Gate 4 recommendation

Select **A, narrowly scoped to source completeness for the existing scalar
slice across BOTH direct and staged admission**, with an explicit invalid/
recovered outcome as the necessary part of C. Start from the reproduced
typed-placement, initializer-tail, and missing-delimiter cases; do not broaden
canonical ownership. Decide how inspection exports remain available without
giving their incomplete projections executable authority. Preserve older
artifact policy explicitly rather than silently mandating new proofs.

Then consider D as a prerequisite to typed-target identity/semantic migration.
B is the stronger long-term representation candidate, not permission for an
immediate AST-wide rewrite. E should precede syntax retirement. Defer is valid
if the reviewer does not authorize one of these bounded changes.

**No recommended correction was implemented.**

## 13. Gate status

GATE 4: WAITING FOR HUMAN REVIEW
