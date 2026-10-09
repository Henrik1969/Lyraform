# Disposition lanes: source and containment decision brief

Date: 2026-10-09. Base: `86a5c69d58bacb09f393cef2a3d4e798ac9fe3ad`.
Selected mission: [canonical disposition lanes and wire closure](../tasks/canonical-disposition-lanes-and-wire-closure.md).
Status: **BLOCKED at source admission; no source/backend lane claim**.

## Exact missing source fact

`TextOutcome` already has a complete bounded binary disposition and local
must-account proof. It is the first plausible producer, not an absent or
unimplemented failure mechanism. A fresh analysis of
`Lyraform/compiler/examples/text/text_outcome.flow:7:5` produces operation 0,
disposition fact 0, owner 38 and obligation `operation:0:outcome`. Its failure
route is `tagged_owner`, commit law `atomic_tagged_result`, and failure behavior
is operation 3 under complementary branches. This establishes neither an
ordinary function taking a read-only failure envelope nor a declared recovery
transition and policy-selected graph route. Reinterpreting that branch as an
ADR 0062 consumer would add meaning.

Guard analysis admits proven preservation and eliminates `GuardViolation`;
runtime-dependent guard transitions remain refused. Existing provider ABI
selection grants exact calls, not failure-envelope adaptation or recovery
meaning. No current admitted form supplies the mission's complete association.
The reference plans are hand-authored contract inputs and C++ callbacks. They
are not source producers or backend function lowering.

## Alternatives

A. **Reviewed external semantic declarations for existing TextOutcome calls.**
Attach exact operation/function/consumer/transition/policy identities to the
canonical source analysis, with an explicitly reviewed envelope parameter
projection and commit-law adaptation. This avoids new source tokens and reuses
existing TextOutcome production. It adds a compiler input/trust boundary:
imports, stale identities, module revision binding and declaration ownership
must be specified. Binding and both backends must preserve the resulting facts;
optimizers must retain the route. Serial execution remains required. Existing
programs need no migration unless they opt into the declaration.

B. **Reviewed source associations and envelope signature projection.**
Specify source-visible associations using existing declaration structures where
possible, with new spelling only in a separately authorized mission. This makes
contracts locally reviewable but establishes language syntax and typing meaning.
It needs parser, import and semantic diagnostics, explicit envelope lowering,
route scheduling and backend parity. Existing tagged programs remain valid;
new declarations opt into the route. This mission cannot implement new spelling.

C. **Retain the present boundary.**
Keep tagged recovery executable and the bridge declarative until the language
boundary is selected. There is no migration or invented meaning, but no native
failure lane or general guard routing becomes available.

Recommendation: **A**, as a bounded next decision, because TextOutcome already
has source-derived producer/type/commit/ownership evidence. Approval must cover
the missing association, read-only envelope projection and commit proof; a JSON
file naming a function is not sufficient authority. This recommendation has not
been implemented.

## Exact fault boundary

ADR 0065 and the reference containment contract authorize one activation to halt
and quarantine, preserve evidence, suppress publication and never rejoin. Native
`flow_graph_fail` instead records a diagnostic and calls `exit(70)`. The current
compiler graph does not expose a declared fault endpoint or receipt-bearing
quarantined activation result. There is no corresponding source/TinyVM contract.
Replacing the exit with return or catch/recover would invent continuation.

The narrow future choice is a reviewed activation-result protocol that can carry
`halted_quarantined` plus receipt while scheduling excludes that activation.
An outer process policy is a separate choice if the host cannot represent that
state. Whole-graph/process quarantine, restart and implicit termination remain
unselected. The new declarative bridge validates only activation scope; it does
not implement this protocol or claim that process termination is containment.

## Decision required

Select A (external semantic declarations), B (source associations), or C
(defer), and review the envelope/commit boundary before source implementation.
Separately select an observable activation containment protocol before native
fault execution. Stages 3 and executable 5 depend on these decisions; completed
independent bridge, diagnostic and reference evidence work is retained.
