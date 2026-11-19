# Failure-envelope argument admission checkpoint

Branch: `experiment/configresolve-lyraform`. Mission:
`lyraform.disposition-graph-preservation`, revision 1. Partial prerequisite;
Gate 4 remains active and no recovery-execution claim is made.

A source probe passed a bare `TextFailure` value into a `failure TextFailure`
parameter. Before this change the semantic report was `ok` with no diagnostics;
the lowering plan remained blocked by the existing disposition execution limit.
This was an admission defect, not evidence of successful bad runtime execution.

Flowanalyst now distinguishes the parameter's source envelope form from its
payload carrier and rejects bare variables, literals and incompatible envelope
arguments with `FLOWANALYST_CALL_FAILURE_ENVELOPE`. Direct forwarding of an
existing same-type envelope parameter remains admissible. This uses the accepted
immutable typed-envelope law (ADR 0062/0066); it introduces no source syntax,
application policy or runtime shortcut. General envelope expressions, local
storage and the runtime ABI are not established by this check.

Validation: canonical build passed; 191/191 CTests passed, including two new
negative source cases and a positive ordinary-response forwarding case. The
ConfigResolve integer probe retained its 96-case native/TinyVM checks. Evidence:
`experiments/configresolve/evidence/envelope-admission/`. The before observation
was captured using a host policy rebound to the probe's actual module revision;
stale-host-policy rejection was not mistaken for this admission defect.

Next dependency: preserve explicit envelope parameter identity and its immutable
projection in callable/graph contracts, with hostile cross-stage validation,
before implementing an executable generic envelope representation. Source
transformation successor chains are still unsupported; full Gate 4 is incomplete.
The complete ConfigResolve port and full language target remain unachieved.
