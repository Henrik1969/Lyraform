# Source recovery execution — native and TinyVM

ID: `lyraform.source-recovery-execution`; revision 1. Owner Henrik Sørensen.
Selected authorized continuation on `experiment/configresolve-lyraform` after
the source activation runtime foundation checkpoint. Existing dirty work,
read-only ConfigResolve reference, no commits/push/merge remain protected.

Intent: make application-defined failure recovery real so the ConfigResolve
port can use it. Goal: an actual Lyraform TextOutcome producer and ordinary
source response execute on native and TinyVM, success bypasses response,
failure invokes only the policy-selected response, both enter the same typed
rejoin exactly once, and matching runtime evidence proves identity, origin
commit, obligation closure and provenance. All owned successful Text storage
must be disposed exactly once after its final permitted use. No application
logic may be inserted into the compiler/runtime.

Convergence: source semantic analysis owns legality and the existing graph-v2
facts; shared validators own boundary consistency; backends implement those facts
without name lookup or policy selection. ADR 0062/0063/0065/0066/0067 and Mission
07's accepted expected-failure bound supply the meaning. Reuse the private C
activation-frame ABI, existing TextOutcome provider/ownership authority and
structured callable lowering. No new source spelling, record construction,
implicit conversion or lifetime model. Source transformation remains unsupported
until accountable successor authority exists; this mission does not claim the
full Gate-4 transformation criterion has passed.

Tasks: inventory remaining producer/response/ownership and backend admission
requirements; implement source-derived execution readiness only for fully proved
bounds; emit native and TinyVM calls/results/evidence; exercise real source
success and forced existing-provider failure; test two-route selection and
unselected response nonexecution; inspect generated artifact and evidence;
positive/negative/hostile parity, sanitizers, canonical doctor/build/test;
checkpoint and consolidation. Keep execution refused on either backend until
that backend supports the requested contract. Do not make a blocked declarative
graph executable merely by changing its status.

The native endpoint is a real compiled binary and explicit embedded-host result;
TinyVM must produce equivalent observations and evidence. A callback harness,
reference executor, manually rewritten plan, scalar ConfigResolve probe or C
runtime test cannot satisfy this goal. Source-body changes and renaming must
change executable behavior only as specified, independent of fixture names.

Admission includes exact callable envelope ABI, producer tagged-result ABI,
exclusive junction, publication, ownership and cleanup, no unrelated source
operations silently dropped, and explicit fault refusal or containment. No
reachable `flow_graph_fail`/exit/unwind on a claimed ADR-0067 executable path.
Fault execution beyond the established runtime bound must be separately proven;
never route faults through ordinary recovery or discard a declared fault port.

Accepted meaning: cited ADRs. Implemented baseline: versioned graph preservation
and tested C runtime frame, no source execution. Proposed record construction
awaits owner choice. Unsupported broader composition remains visible. Stop only
dependent work on a real semantic ambiguity and provide a concrete decision
brief; ordinary implementation choices remain autonomous.
