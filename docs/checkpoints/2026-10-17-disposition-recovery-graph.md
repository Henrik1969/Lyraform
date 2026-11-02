# Gate 4 progress: recovery graph projection

Status: ACTIVE, partial Gate 4. No source recovery execution claim.
Branch experiment/configresolve-lyraform, base HEAD unchanged; no commit/push/merge.

The existing graph-plan-version 2 request now projects selected recovery topology
into a typed non-executable disposition subgraph. Nodes represent producer,
failure dispatch, selected ordinary response, exclusive success junction,
resolved rejoin destination and activation containment. Original source wire
identities and typed lanes survive. Module, policy, obligation and commit authority
remain attached. Serial scheduling and unproved-boundary-transform refusal are
explicit. [Contract](../architecture/disposition-graph-projection.md).

Flowbind retains authority/projection evidence after validating the semantic plan.
Flowparallel/Flowoptimize preserve the graph; flowprepare requires exact agreement
with binding evidence. Shared validation rejects altered projections and deletion
of either graph or its version marker. This is mechanical projection of resolved
facts, not new language meaning, runtime default selection or ConfigResolve code.

## Evidence and correction discovered

Generic tests cover fixed-single and supplied multi-route policy, repeated
byte-identical analysis, 4 plan-stage preservation checks per source, 64 hostile
stage mutations, a valid-but-foreign binding report, corrupted binding and both
backend execution refusals. Transformation is explicitly unadmitted until its
successor authority is preserved. Recovery projection does not pretend it exists.

The first backend test exposed TinyVM ignoring blocked-plan status and emitting
an entry-only binary. TinyVM now refuses all explicit non-ready plans and requires
status for callable v2. Version-1 captured artifacts historically lack status and
retain their prior contract. An initial broad guard broke 4 such fixture tests;
the final guard preserves that legitimate legacy form without admitting blocked v2.
Both failure evidence and final results are retained.

Full suite **191/191 PASS**, 17.98 seconds. ConfigResolve scalar regression:
96 cases agree with C++ on native and TinyVM; rename invariant and corrupted-oracle
refusal pass. Governance 41/41, no errors/drift; whitespace check passes.
[Evidence](../../experiments/configresolve/evidence/disposition-graph/) includes
logs, regression report and a curated source-derived recovery graph snapshot.
No sanitizer result or complete Gate 4 certification is claimed yet.

What became usable: real typed recovery/containment graph projection across the
compiler boundaries, with contradictory evidence rejected. What remains: transformed
failure successor projection, complete gate verification, native/TinyVM recovery
execution and quarantine runtime implementation, plus the rest of the full port.
Simplification: one projection/validator implementation and shared source authority;
no new provider or application-specific backend path. Falsifiers exercised include
changed selected function/edge/schedule/obligation, missing graph, foreign binding
and attempted execution of blocked plans.

Next work: preserve the transform successor's exact semantic endpoint and legal
unsuccessful continuation, extend projection tests, then finish Gate 4 verification.
The record construction decision remains independently pending owner selection.
