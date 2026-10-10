# Disposition structural syntax — Gate 1

Date: 2026-10-10
Mission: `docs/tasks/disposition-foundation-and-source-authority-gate2.md`
Branch: `codex/disposition-foundation-source-gate2`
Baseline: `d6973e28c94797ead83d96c3bbb1c032286edc28`

## Decision and result

Gate 1: **PASS**.

The canonical frontend now captures the bounded ADR 0066/0067 source forms
without granting semantic or executable authority:

- closed function `fails { ... }` and `faults { ... }` clauses;
- immutable `failure E` parameter form;
- explicit `recover T` and `transform E2` result forms;
- closed `consumer` declarations containing ordinary function references;
- producer-function, consumer-instance and activation-containment nodes; and
- explicit `success`, `failure` and `fault` endpoints and wires.

The frontend publishes these facts in the separate versioned
`flowmini.disposition_syntax/v1` projection with `status: structural` and
`execution: unsupported`. Existing `flowmini.graph_syntax/v1` keeps its prior
meaning; disposition nodes and wires do not enter it. Parser recognition does
not select policy, establish semantic legality, create an executable route, or
fall back to the deprecated runtime.

## Structural authority

`flowmini.disposition_syntax/v1` carries deterministic structural IDs for
functions, consumers, nodes and wires. Every declaration, type, consumer
member, node, wire and endpoint retains its mapped source path, line and
column. Imported response and consumer declarations retain the imported unit's
origin instead of acquiring the importing program's location.

The structural specimens are:

- `Lyraform/compiler/examples/disposition/text_outcome_consumer.flow`;
- `Lyraform/compiler/examples/disposition/imported_response.flow`; and
- `Lyraform/compiler/examples/disposition/imported_disposition.flow`.

The selected specimen deterministically projects four nodes and four wires.
The imported specimen proves a `transform OtherFailure` response and its
consumer originate in `imported_response.flow`.

The canonical source-form audit now classifies top-level `consumer` as
recognized but explicitly unsupported beyond structural capture. Gate 2 owns
the separate decision to resolve and admit its semantic facts.

## Parser refusal and regression repair

The focused regression refuses ten malformed classes before semantic
analysis: incomplete and duplicate closed sets, duplicate consumer members,
malformed failure envelopes, malformed response results, wrong declaration
position, missing wire arrows, malformed endpoints and malformed containment.

Initial imported-fixture testing exposed a pre-existing structural scanner
defect: graph declaration tokens were not top-level boundaries, so an embedded
`fn` or `consumer` implementation selector could be re-read as a declaration.
The smallest root-cause repair makes graph declaration keywords explicit
top-level boundaries. Existing AST golden output and graph tests remain green.

## Verification

Tool versions:

- CMake 3.28.3
- Ninja 1.11.1
- Clang 18.1.3
- Valgrind 3.22.0

Normal evidence:

- `./igor doctor`: PASS.
- `./igor build`: PASS, 50 build steps in the existing clean build tree.
- `./igor test`: **185/185 PASS** in 61.22 seconds.
- focused parser/frontend/graph compatibility selection: **9/9 PASS**.
- AST golden suite: **28/28 PASS**.
- frontend bundle suite: **8 golden + 1 isolated positive + 19 negative PASS**.
- disposition syntax suite: two deterministic positive projections, imported
  spans, and **10/10 malformed forms refused**.
- `git diff --check`: PASS.

Focused Clang ASan/UBSan evidence:

- fresh Clang 18 Debug tree with address and undefined-behavior sanitizers;
- rebuilt the `flowmini` target;
- disposition syntax suite PASS with `halt_on_error=1` and leak detection
  disabled for the host environment.

Focused Valgrind evidence:

- selected source projection: 0 errors, 0 leaks, 1,468 allocations freed;
- imported source projection: 0 errors, 0 leaks, 456 allocations freed.

## Maturity boundary

```text
recognized                  YES
structurally validated      YES
import-aware provenance     YES
semantically resolved       NO (Gate 2)
legality/admission proven   NO (Gate 2)
policy selected             NO
backend executable          NO
```

No runtime route, Graph IR operation, scheduler rule, policy artifact,
LLVM/TinyVM behavior or fault quarantine was added. The tracked
`.codex-run-state` remains byte-identical to the `main` base (`CONTINUE\n`).
The unrelated `meta-discusions.md` and `output/` paths remain untouched.

Gate 2 must now establish one semantic owner for the closed typed topology,
derive the declarative bridge from source facts, attack that contract at its
actual consumer and preserve explicit later-stage execution refusal.
