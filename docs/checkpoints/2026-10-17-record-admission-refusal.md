# Ordinary-record admission correction — PASS

Selected [mission](../tasks/ordinary-record-admission-refusal.md).
Branch `experiment/configresolve-lyraform`, original HEAD unchanged. Protected
Gate 3 source changes were preserved. No commits/pushes/merges.

Flowanalyst now marks callable-plan-v2 statements containing structural record
literals refused, with FLOWANALYST_SOURCE_OPERATION_GAP and source provenance.
This is generic admission correction, not ConfigResolve code or record support.
Plan-v1 declarative field analysis remains available. Existing source-form audit
already classifies ordinary record construction as unsupported.

Five literal variants (valid, reordered, missing, extra and wrong-type) refuse
before ready publication; repeat analysis is byte-identical. Scalar positive case
remains ready. Nine exploratory readiness cases now stop at Flowanalyst.

Initial full suite: 189/190 passed. The failing target-preservation test expected
v2 readiness for its unsupported record fixture. It now verifies identical target
facts between versions, v2 admission refusal, and all original v1 pipeline and
hostile artifact checks. No valid executable ABI case was removed or exempted.
Final canonical suite: **190/190 passed**, 17.72 seconds. Focused tests 4/4 passed;
updated target-preservation and repeated-output checks independently passed.
The scalar ConfigResolve probe still passes 96 comparisons on native and TinyVM,
rename invariance and corrupted-oracle rejection.

[Evidence](../../experiments/configresolve/evidence/record-refusal/) retains both
initial failure and final success. `generic-compiler-changes.json` records exact
reviewed base-file hashes and owning mission; unexpected changes fail the scalar
harness. The original compiler manifest is exported for reproducibility. This
review mechanism is evidence, not automatic proof that future changes are generic.

Shared audit: 41/41 contexts, zero errors/drift. The changed CMake test registration
was inspected and its policy evidence explicitly refreshed with prior hash retained.
No sanitizer run: this change affects admission traversal/diagnostics, with existing
allocation-fault/full-suite checks passing; no memory ownership mechanism changed.

What became usable: accurate early refusal with preserved structural evidence.
What remains unsupported: actual ordinary-record execution and the full port.
Simplification: reuse one existing coverage/diagnostic mechanism. No new runtime,
provider, syntax or application dispatch. Missing-field/incorrect-type probes and
preservation tests could falsify the claim; they are retained.

Next dependent stage awaits the record construction decision. Independent stage:
[source disposition Graph IR preservation](../tasks/source-disposition-graph-preservation-gate4.md),
bounded by already accepted ADR 0066/0067 and Mission 07 Stage 4.
