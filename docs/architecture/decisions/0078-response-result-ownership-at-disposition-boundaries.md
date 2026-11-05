# ADR-0078: Response-result ownership is explicit at disposition boundaries

Status: accepted bounded implementation slice — 2026-11-05

## Decision

Every source-disposition response carries an explicit `result_resource`
contract alongside its typed outgoing result. The contract records:

- the result type;
- ownership (`borrowed` or `owned`);
- read access and nullability;
- lifetime (`call` for a borrowed result or `owned` for a transferred result);
- the cleanup capability;
- the origin and the proof supporting the ownership decision.

The disposition boundary defaults to `owned`. A response is classified as
`borrowed` only when the compiler proves the current bounded case: one `Text`
string literal returned through the synchronous typed rejoin. That result has
call lifetime and `cleanup_capability: none`.

All other response bodies receive the explicit owned-default contract. An
owned result without a resolved cleanup capability remains a semantic fact but
is refused by the private native bridge. Ownership ambiguity therefore cannot
silently become a borrowed callback result or a leak.

The native bridge now receives two cleanup capabilities. The producer's owned
successful `TextOutcome` is disposed independently of the response result. A
borrowed response passes no response disposer; an owned response must name the
supported `flow_text_dispose` capability before native lowering is admitted.
The runtime disposes an owned response only after the final rejoin use, and
also disposes it if recovery admission itself refuses.

## Boundary

This does not make source disposition graph execution ready. The source
topology and TinyVM paths remain `execution: unsupported`. It also does not
claim that arbitrary response bodies can yet be lowered: the current private
native bridge still admits only the one-route `TextOutcome` shape and the
statically verified response body. General owned response lowering remains a
follow-up slice.

## Rationale

The response result crosses a semantic ownership boundary even when the
response is written as an ordinary source function. Treating the current
static literal as implicitly borrowed made the lifetime rule live only in the
fixture-specific emitter. Carrying the fact through the topology, policy
selection, lowering gate, and runtime makes the decision inspectable and keeps
the refusal boundary honest while lifecycle canon matures.
