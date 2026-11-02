# ConfigResolve port contract inventory — stage T0

ID: `lyraform.configresolve-contract`; revision 1. Selected by Henrik's
2026-10-17 branch-experiment instruction. Owner: Henrik Sørensen.
Canonical definition: this Markdown; state binds its SHA-256.

Intent: drive language maturation from a real existing configuration engine.
Goal: pin all public headers, implementation, tests and current contract records;
map every public behavior family and test family to the port, classify differences,
and derive the next smallest executable slice from measured compiler evidence.

Scope: inspect ConfigResolve read-only; add Lyraform experiment contracts,
fixtures, test harness and stage records. No reference implementation edits.
Preserve current dirty Lyraform baseline (private manifest/archive under
`.mission-state/configresolve-experiment-baseline/`). No commit/push/merge.

Tasks: inventory -> baseline verification -> compatibility matrix -> next-stage
selection. Completion requires zero unmapped public header/test files, intact
reference hashes, measured baseline builds/tests, and an explicit next-stage
endpoint with falsifying tests. A family-level map is not API parity proof.

Follow root AGENTS.md, both projects' policy maps and the stage/convergence rules.
Accepted ConfigResolve contract owns application behavior; Lyraform accepted ADRs
own language meaning. Current source and tests provide compatibility evidence,
not new semantics. Float equality defects, nonfinite values, error-result authority
and resource/mutation meaning remain explicit decision boundaries.

Reuse source tests and canonical compiler chain. Retire no implementation in
this inventory stage. Positive: resolve distinct-precedence integer facts with
correct chosen provenance. Negative: required absent/type-invalid key diagnoses
failure. Hostile: equal-priority disagreement and reordered candidates reveal
accidental insertion-order authority. Source-hash mutation invalidates the oracle.

Reproduce reference with CMake/Ninja and CTest in a fresh temporary directory;
Lyraform with Igor doctor/test and a matching build directory. Keep exact logs.
Gate proves a prerequisite inventory, not a functioning port. Before successor
work publish the matrix, measured results, unresolved cases and next mission.
Each successor has fresh state; no inherited completion. Compiler-name special
cases or a port delegating resolution to C++ falsify the eventual experiment.
