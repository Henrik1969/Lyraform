# Mission 02 — canonical scalar semantic authority

Date: 2026-09-18. Authority: Phase 2 only. Required stop: Gate 2.

## Baseline

Started on clean `main` at `c03079e46e938e98b04f3bad21c991cc4f58eee4`.
`git fetch origin` confirmed `origin/main` at the same SHA. That commit adds
the two Phase 1 reconnaissance reports. One canonical worktree was present.
Changes remain uncommitted for review; no push, history rewrite, branch
deletion, FlowLFS change, or `master` change was performed.

## Result

The required `int`/`Bool` mismatch now fails in Flowanalyst before a ready
execution plan. Both literal and identifier Bool-to-int initialization or
placement produce `FLOWANALYST_SCALAR_FLOW_REFUSED`, semantic status `error`,
exit 2, and a non-ready lowering plan. Invalid scalar operations do not
project executable operands. Binding, planning and optimization reject the
refused report. Changing the outer status does not override its retained
refused fact.

The shared component is
`Flowcontracts/include/flowcontracts/scalar_semantics.hpp`, namespace
`lyraform::scalar`. Its type identity, compatibility and operator-result
rules are called by Flowanalyst, the retained compatibility parser, and
artifact import validation. It has no AST, JSON or backend dependency.

`Flowanalyst/src/scalar_analysis.hpp` is the structural frontend adapter. It
resolves the bounded destinations using current symbol/scope facts, checks
named AST declaration types against their SymbolTable projection, infers
covered source types before carrier emission, and returns origin-bearing
declaration/placement facts. Expression inference has a recursion bound,
cycle detection and per-statement memoization.

`Flowcontracts/include/flowcontracts/scalar_facts.hpp` projects those facts as
`lyraform.scalar_fact` version 1 and validates imported evidence. The common
lowering validators and Flowbind invoke it. Thus LLVM and TinyVM receive the
same checked source/destination type facts in the backend artifact, with no
backend-specific decision about Bool/int compatibility.

## Scope and compatibility

- Source scalar identities: exactly `int` and `Bool`.
- Destinations: explicitly initialized local declarations and later identifier
  placements into them.
- Source expressions: integer/boolean literals, resolved scalar identifiers,
  and the existing scalar operators covered by the shared type algebra.
- Other literal kinds are refused for these destinations. Missing destinations,
  lexical use before declaration and self-initialization are refused in this
  bounded path.
- Calls, ABI conversions/carriers, refined types, aggregate/list/array
  operations, uninitialized declarations and new source syntax remain outside
  the proof. There is no definite-initialization or first-placement design.

The extension is an independently versioned field on existing lowering-plan
v1/v2 operations. Existing serialized IDs and envelope versions are unchanged.
Older captured artifacts lacking facts retain compatibility and make no Phase
2 proof claim. Requiring evidence on every imported artifact would need a
separate admission/version decision. This phase does not claim authenticated
source provenance or rejection of every possible legacy artifact.

The precise contract is documented in
[`canonical-scalar-semantics-v1.md`](../architecture/canonical-scalar-semantics-v1.md).

## Preserved stage borders

Frontend, Flowanalyst, Flowbind, Flowparallel, Flowoptimize, flowprepare,
flowvalidate, LLVM and TinyVM remain independently invocable. Tests save each
stage artifact, import it into the next process, canonicalize it independently,
and compare all scalar facts. Facts retain statement, declaration, expression
and destination identity plus original file/line/column and AST path.
Source-map tests distinguish original file coordinates from expanded lines.
Igor and its CLI are unchanged.

## Evidence

Build environment: GCC 13.3.0; Clang 18.1.3; CMake/Ninja root build.

| Check | Result |
|---|---|
| Fresh baseline configure/build, `/tmp/lyraform-phase2-build` | PASS |
| Baseline canonical CTest | 156/156 PASS, 57.28 seconds |
| Final implementation configure/build | PASS |
| Final canonical CTest | 158/158 PASS, 62.45 seconds |
| Final focused scalar CTests, GCC | 2/2 PASS, 1.41 seconds |
| Final focused scalar CTests, Clang ASan/UBSan | 2/2 PASS, 3.30 seconds |
| Shell syntax and tracked diff whitespace checks | PASS |

Focused sanitizer configuration used Debug, `-fsanitize=address,undefined`,
`-fno-omit-frame-pointer`, `ASAN_OPTIONS=detect_leaks=0:verify_asan_link_order=0`
and `UBSAN_OPTIONS=halt_on_error=1`. Leak detection was disabled. The complete
sanitizer CTest suite was not run; the two new scalar gates exercised the
instrumented stage executables.

The new end-to-end gate covers:

- Twelve invalid source cases under both lowering-plan versions, including
  the required Bool-to-int initializer/placement defect, reverse mismatch,
  mismatched identifier values, invalid mixed expressions, missing targets,
  self-initialization and a write before declaration.
- Four valid LLVM/TinyVM cases under both plan versions; three also agree with
  the legacy direct runtime. Repeated analysis is byte-identical.
- Exact scalar-fact preservation through semantic, execution, optimization
  and backend artifacts, including independent canonical serialization.
- Six malformed/contradictory captured artifacts rejected by flowvalidate,
  LLVM and TinyVM before executable publication.
- Mapped source origins and a conflicting AST/SymbolTable type projection.

Reproduction:

```sh
cmake -S . -B /tmp/lyraform-phase2-build -G Ninja
cmake --build /tmp/lyraform-phase2-build -j 6
ctest --test-dir /tmp/lyraform-phase2-build --output-on-failure
ctest --test-dir /tmp/lyraform-phase2-build -R '^lyraform_scalar_' --output-on-failure
```

## Findings retained for review

1. **Legacy identifier-initializer execution defect.** The fourth valid case
   uses same-type identifier initializers for int/Bool. The compatibility
   runtime fails with `missing record path`, while LLVM and TinyVM return the
   expected result. The unchanged `lowerExprToPath(identifier)` implementation
   returns the source path, while declaration callers ignore that result.
   This helper is identical at the baseline. The test records the observed
   failure explicitly; runtime lowering was not repaired in this phase.
2. **LLVM boolean `not` support.** The shared semantic rule recognizes the
   existing boolean operation. LLVM currently refuses its unary lowering.
   Backend refusal remains permitted; no operator implementation was added.
3. **Partial convergence.** Syntax, general resolution, non-selected types,
   function semantics and runtime behavior still have the separate paths
   inventoried in Mission 01. Sharing this bounded semantic rule does not
   retire either parser or establish complete Lyraform v1 semantics.
4. **Compatibility imports.** Proof-less old artifacts remain readable. A
   future phase must explicitly decide when canonical scalar evidence becomes
   mandatory at import, with versioning and captured-artifact migration.

## Gate 2

The requested mismatch is closed for the selected source slice and the
implementation is available as an uncommitted diff. Review the scope,
independently versioned facts, compatibility-import policy, and retained
runtime findings before selecting another merger slice. No subsequent phase
is authorized by the test results.

```text
GATE 2: WAITING FOR HUMAN REVIEW
```
