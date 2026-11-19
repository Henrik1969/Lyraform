# ConfigResolve contract inventory — T0 gate PASS

Branch: `experiment/configresolve-lyraform`, forked from current Lyraform HEAD
`b50534c380879a5a70a77ed5c1dd442bfc4142bd` with existing dirty content preserved.
No commit, push or merge. Original file hashes/archive are private under
`.mission-state/configresolve-experiment-baseline/`. This is a working-tree
baseline, not a clean committed checkout or main integration claim.

Reference HEAD: `35e5aae9974c796e26e797f144570bb9f18122d0`; manifest includes
working-tree changes. Source hashes were rechecked after reference testing.
[Inventory](../../experiments/configresolve/contract-matrix.json) maps all 41
public header/source/test files across 13 families, with zero unmapped files.
This is family coverage, not completed symbol-level compatibility or port parity.

Measured baseline: ConfigResolve fresh configure/build succeeded and 13/13 tests
passed. Lyraform Igor doctor/build/test succeeded, 189/189 passed. Exact
[logs](../../experiments/configresolve/evidence/t0/) accompany the observation.
Shared policy audit: 41/41 contexts, zero errors or drift. No compiler/base edits
were made by T0. ConfigResolve remains read-only.

Accepted application rules distinguish facts, resolution policy, schema and
runtime access policy. Tests and implementation show explicit FirstWins/LastWins
ignore precedence; highest-tier disagreement is diagnosed. Float-equivalence
accidents and provisional winner exposure are recorded separately and remain
unresolved for the full port. The experiment cannot silently canonize them.

What became usable: pinned reference plus complete file/family map to drive
application-derived compiler work. Unsupported: the port itself, general records,
collections and store/view lifecycle in Lyraform. Simplification: one reference
manifest and one map reuse existing contracts/tests rather than inventing a
second configuration specification. A changed source hash or unmapped file
would falsify this inventory gate.

Next selected bounded mission: [scalar resolution probe](../tasks/configresolve-scalar-resolution-probe.md).
It uses already admitted pure scalar operations and independent reference outputs.
It does not bypass or declare completion of pending disposition Graph IR/backend
work; that remains required before broader typed failure and resource support.
No new language semantics are needed for this probe. It produces a retained
native binary as a first prerequisite, not the completed ConfigResolve port.
