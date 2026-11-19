# Contract stabilization G1 — 2026-11-01

Mission: [contract stabilization revision 1](../tasks/contract-enforcement-stabilization.md).
G1 passed; G2–G5 remain open. Branch `experiment/configresolve-lyraform`,
HEAD `b50534c380879a5a70a77ed5c1dd442bfc4142bd`, including preserved dirty work.

Refined value use with unenforced predicates and unproved integer division now
refuse before executable publication. This includes inherited, shadowed and
generic refinements, nested record operands, arguments and placements.
Constant-safe division, ordinary scalar updates and static guards remain usable.
Shared Flowcontracts admission runs independently of optional scalar facts and
claimed readiness at separately invocable artifact consumers. Source diagnostics
retain operation and original statement/location provenance.

The executable matrix has 9 positive and 20 negative source cases, and 42
hostile artifact boundaries, including stale identity, missing type, version and
readiness changes. Native O0/O2 full results agree with TinyVM. Any invalid
admission, emitted executable for a refused case, or result mismatch fails it.
Zero invalid admissions were observed. Run `python3 tools/test-contract-stabilization.py
--build BUILD` against the exact snapshot.

Fresh GCC and Clang builds each pass 194/194 CTests. Clang ASan/UBSan with
LeakSanitizer enabled passes 194/194 outside the ptrace sandbox. Canonical Igor
doctor/build/test pass. Frontend gates: 28 AST, 14 symbol projection, 8 frontend
golden, 1 isolated, 19 negative and 18 malformed attacks plus 1 attack control.
Support-inclusive Valgrind run passes 58/58; pipeline probes cover all compiler
stages, VM execution and hostile refusal. 24 concurrent projections are identical.
The corpus checks 98 sources: 57 admitted, 41 source-operation-gap refusals and
2 contract refusals, with overlapping refusal categories.

The required pressure checks exposed and repaired Text-example ownership leaks,
GPU allocation-fault fixtures loading unnecessary vendor constructors, sanitizer
link flags in the callable harness, and stale negative diagnostic expectations.
These repairs do not establish new ownership or numeric semantics.

Pager tests currently prove honest refusal of unproved provider-dependent
division; they do not prove paging execution. General refinement enforcement,
dynamic division routing, G2 numeric alignment, G4 source recovery and G5
composition remain unsupported or pending. G1's current 32-bit native `int`
carrier is an implementation limit; the owner's subsequent fixed signed-int64
ruling is the next gate's required alignment, not inferred from these tests.

Exact tested sources (before this report), their per-file hashes, raw logs and
matrix observations are retained in the mission's portable evidence directory.
The guarded state records their SHA-256 digests and verifier. The original dirty
baseline is separately retained; this is not reproducible from HEAD alone.

Reused mechanisms: existing typed operands, source provenance, scalar admission
and stage validation. No optional proof flag, new syntax, private numeric trap
route or fixture-specific compiler dispatch was introduced.
