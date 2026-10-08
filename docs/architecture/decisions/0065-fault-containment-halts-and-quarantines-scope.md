# ADR 0065: Fault containment halts and quarantines the exact scope

## Status

Accepted architectural law on 2026-10-08.

## Context

ADR 0055 separates expected `Failure<E>` from integrity `Fault<F>`. ADR 0056
requires every fault to have one explicit, statically inspectable route to a
containment authority. The remaining question is what the first containment
authority is allowed to claim after receiving a fault.

Treating containment as ordinary recovery would permit execution to continue
inside a scope whose integrity is no longer trusted. Implicit process
termination would instead create a hidden control path and hide the affected
boundary. Logging alone would leave the fault wire dangling.

## Decision

A bounded fault-containment authority halts and quarantines its exact declared
scope. It suppresses normal publication from that scope, permits no local
continuation, preserves the fault envelope and provenance, and emits a
validated containment receipt.

```text
Fault<F>
    -> exact typed fault wire
    -> declared containment authority for scope S
    -> halt and quarantine S
    -> suppress normal publication
    -> no local continuation
    -> containment receipt
```

The receipt accounts for the fault route. It does not convert the originating
attempt to success, reclassify the fault as an expected failure, prove that
the compromised operation completed, or authorize execution to resume in the
contained scope.

Policy may select only an action and authority already admitted by canonical
fault and containment facts. It may not choose an undeclared scope, weaken
quarantine, create continuation, or substitute logging, dropping, recovery,
or implicit termination.

## First bounded projection

The first executable reference projection contains one fault type, one exact
wire, one activation-scope containment authority, and the single action
`halt_and_quarantine`. Provider, graph-region, process, distributed, and
hardware containment domains require their own later contracts.

## Consequences

- ordinary failure consumers cannot swallow integrity faults;
- the affected scope and containment action remain inspectable;
- a contained fault remains a fault in historical and audit evidence;
- normal output cannot escape after containment;
- escalation, trusted restart, and process termination remain explicit future
  routes rather than runtime defaults.

## Deferred

- propagation to an outer containment authority;
- trusted repair, replacement, or restart;
- provider, graph, process, distributed, and hardware containment scopes;
- top-level termination-policy sinks;
- concurrent fault composition;
- source spelling and compiler/backend integration;
- durable quarantine and audit-provider contracts.
