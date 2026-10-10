# Source-declared disposition authority — Gate 2

Date: 2026-10-10

Mission: `docs/tasks/disposition-foundation-and-source-authority-gate2.md`

Branch: `codex/disposition-foundation-source-gate2`

Gate 2 starting commit: `a3482169b7ca39e9ab0ed9c1c5f829546144b7e2`

## Result

**PASS — semantic source authority is complete for the one bounded
`TextOutcome` specimen and remains unsupported for execution.**

Flowanalyst is the sole legality producer for the new
`lyraform.source_disposition_topology/v1` projection. Flowcontracts validates
the projection and its embedding in a semantic report without reconstructing
meaning from names or layout. The structural frontend remains the owner of
syntax, structural IDs, and import-aware source locations.

The accepted source is
`Lyraform/compiler/examples/disposition/text_outcome_consumer.flow`. Its
`compose_text` function declares closed `fails` and `faults` sets, returns one
owned `TextOutcome` carrier through its declared boundary, names an immutable
typed response function, closes the consumer set, connects success/failure to
one same-attempt junction, reconnects recovered `Text`, and routes the fault
to activation containment.

The emitted lowering plan is deliberately `blocked`, the semantic topology
says `execution: unsupported`, all five execution claims are false, and the
legacy executable `disposition_facts` array is empty. The report itself is a
valid semantic artifact; it is not an executable plan.

## Factual authority trace

| Link | Exact bounded fact | Maturity |
|---|---|---|
| source/import span -> declaration | function line 9, response line 14, consumer line 25, nodes lines 29–32, wires lines 34–37; source-map origins include the source plus `libc.flow` and `text.flow` | source-derived |
| declaration -> producer | operation 0, disposition 0, function symbol 38, owner symbol 39, return operation 5, obligation `operation:0:outcome` | source-derived |
| producer -> typed endpoints | `Success<Text>`, closed `Failure<TextFailure>`, closed `Fault<TextIntegrityFault>`, `atomic_tagged_result`, exactly-one completion | source-derived |
| response -> transition | function symbol 40, immutable `failure TextFailure`, `recover Text`, transition `close_original` | source-derived |
| consumer -> route | declaration 10, instance structural ID 1 / node `handled`, route 0, fixed single route awaiting Gate 3 | source-derived; policy not selected |
| same-attempt junction | `disposition-wire:0` success, `disposition-wire:1` failure, exclusive pairing, `disposition-wire:3` typed `Text` rejoin | validated declarative |
| fault containment | `disposition-wire:2` to node `quarantine`, activation scope, `halt_and_quarantine` | validated declarative; execution unsupported |
| semantic topology -> bridge | module `module:text_outcome_consumer`, revision `frontend-v2:disposition-v1:decls-12:ops-9`, exact producer and four-wire mapping | validated declarative |
| bridge -> policy / Graph IR / backend | no policy artifact, executable Graph IR, LLVM/TinyVM route, or runtime quarantine | not implemented |

Caller-supplied numeric reference plans remain separate declarative test
fixtures. They are not accepted as source authority. The updated
`docs/architecture/disposition-source-authority-map-v1.json` records this
separation and the now-implemented source links.

## Early refusal and hostile evidence

The focused Gate 2 test proves deterministic output and refuses ten invalid
source variants at Flowanalyst with stable machine codes and original source
locations:

- wrong recovery/rejoin type;
- incorrect immutable-envelope payload;
- declared/derived carrier-set disagreement;
- missing failure route;
- duplicate success route/co-emission claim;
- fault entering an ordinary consumer;
- unknown response function;
- transformed failure with no successor route;
- cyclic composition; and
- mixed producer authority at one bounded junction.

Flowvalidate rejects 23 topology mutations and six embedded semantic-report
mutations covering operation, disposition, function, owner and return IDs;
type identity; envelope mutability; transition law; consumer set and route;
node and wire endpoints; containment scope; source origin and source position;
module/revision; false `ready`; and executable claims. A forged source topology
cannot substitute for its exact TextOutcome operation in the containing plan.

## Verification

Tool versions:

- CMake 3.28.3
- Ninja 1.11.1
- GCC/G++ 13.3.0
- Clang 18.1.3
- Valgrind 3.22.0

Normal build tree: `/tmp/lyraform-disposition-foundation-build`.

- `./igor --build-dir /tmp/lyraform-disposition-foundation-build doctor`:
  PASS.
- `./igor --build-dir /tmp/lyraform-disposition-foundation-build build`:
  PASS.
- `./igor --build-dir /tmp/lyraform-disposition-foundation-build test`:
  **186/186 PASS** in 60.67 seconds.
- focused `lyraform_source_disposition_gate2`: **1/1 PASS**; one deterministic
  positive topology, ten verified-location refusals, 23 topology mutations,
  and six embedded-report mutations.
- focused Clang ASan/UBSan test: PASS with `halt_on_error=1` and leak detection
  disabled for the host environment.
- focused Valgrind probes for frontend export, semantic analysis, and contract
  validation: PASS, zero errors and no definite leaks. The semantic-analysis
  probe freed all 25,817 allocations and had zero bytes in use at exit.
- `git diff --check`: PASS.
- tracked `.codex-run-state`: byte-exact `CONTINUE\n`; it is not this mission's
  runner state.

The normal build retains two pre-existing `-Wunused-function` warnings in
`Flowparallel/src/graph_cuda.cpp`; they are unrelated to this gate.

## Maturity boundary

```text
recognized structural syntax                 YES
import-aware structural provenance           YES
resolved producer/function/type identities   YES
closed consumer and route legality           YES
same-attempt exclusive junction              YES (semantic contract)
source-derived declarative bridge            YES
policy selected                              NO (Gate 3)
executable Graph IR disposition              NO (Gate 4)
LLVM/TinyVM failure routing                  NO (Gate 5)
runtime activation quarantine                NO (Gate 6)
end-to-end executable closure                NO (Gate 7)
```

## Preserved questions and protected boundaries

Mission 07 Gates 3–7 remain unresolved by design: versioned policy selection,
Graph IR preservation, backend lowering, runtime quarantine/result receipt,
and complete executable closure. General retry, propagation, cancellation,
parallel fallible effects, host exit policy, and general failure carriers also
remain outside this gate.

No exception unwinding, dynamic handler lookup, implicit propagation,
diagnostic-and-drop, policy-created success, or ordinary fault recovery was
introduced. `main`, `master`, FlowLFS, Flowselection, the historical runtime,
`meta-discusions.md`, and `output/` were not modified.

Final branch SHA is the commit containing this checkpoint and is reported in
the mission handoff after the ordinary push.
