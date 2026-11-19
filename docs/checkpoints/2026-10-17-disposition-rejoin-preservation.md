# Gate 4 progress: resolved rejoin identity preservation

Stage: [source disposition Graph IR preservation](../tasks/source-disposition-graph-preservation-gate4.md).
Status: ACTIVE; Gate 4 is not complete. Branch experiment/configresolve-lyraform,
base HEAD b50534c380879a5a70a77ed5c1dd442bfc4142bd plus preserved working tree.

## Observed gap and correction

The Gate 3 topology retained rejoin wire and type but discarded the resolved
destination function and parameter. Flowanalyst now records a `rejoin` witness
at the original successful lookup: node, function symbol, parameter symbol,
input type, wire and provenance. No later stage performs a second name lookup.

The shared topology validator checks wire/type/provenance consistency; disposition
plan validation ties function and parameter to the exact ordinary function
signature. This is an additive witness in topology v1. Historical topology-only
Gate 2/3 evidence can omit it; the forthcoming graph projection must require it.
No graph/execution readiness claim is set by this change.

## Verification

The existing canonical TextOutcome fixture with explicit binding grants traverses
Flowbind, Flowparallel, Flowoptimize and flowprepare. Rejoin witness is byte-equal
through every plan-producing stage. Binding verifies the input; its report does
not yet carry the full disposition graph. Missing binding grants were correctly
refused during reconnaissance; tests now supply exact libc/TextOutcome grants.

Hostile report tests reject foreign function/parameter, wrong input type, foreign
wire and invalid provenance. Each downstream plan artifact rejects a mutated
parameter witness. Existing fixed and supplied route-policy tests remain green.
Full suite: **190/190 PASS**, 18.43 seconds. Scalar application regression:
96/96 comparisons pass native and TinyVM, renamed source passes and corrupted
oracle fails. [Logs and report](../../experiments/configresolve/evidence/disposition-rejoin/).
Shared governance 41/41, no errors/drift; diff whitespace check passed.

No ConfigResolve application behavior was added to base code. Reviewed generic
file hashes are recorded by the experiment. No commits, push or merge occurred.
No new sanitizer result is claimed for this partial stage; full Gate 4 evidence
requirements remain outstanding.

## Remaining Gate 4 work

1. Project validated source topology and selected response into canonical graph
   nodes/edges with exact references, including typed rejoin and containment.
2. Carry explicit source identity for transformed-failure successors before any
   projection claims them; never fabricate an unsuccessful sink.
3. Require graph presence/identity at every consumer once that projection is
   admitted, with hostile deletion and stale/foreign identity tests.
4. Preserve serial disposition boundaries through binding/scheduling/optimization
   and preparation. Runtime lowering remains blocked until Gate 5/6.
5. Run full stage verification and consolidate support/evidence claims.

The current source_graph contract serves ordinary producer/receiver graphs;
source-disposition topology is a separate semantic input, not yet that executable
Graph IR. Reuse the canonical graph machinery where its roles/contracts match;
do not disguise a copied metadata blob as completed graph projection.

What became usable: exact typed rejoin identity across existing boundaries.
What remains: graph projection and source-level recovery/quarantine execution.
Simplification: preserve the original lookup result rather than re-resolve names.
Falsifier: changing the function/parameter/type/wire without rejection; tests cover
these contradictions. This checkpoint supplies evidence, not successor completion.
