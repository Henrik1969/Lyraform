# Tagged producer return and native admission — partial recovery mission checkpoint

Mission `lyraform.source-recovery-execution`, revision 1; branch
`experiment/configresolve-lyraform`. Existing working-tree changes retained.
No commit, merge or application-specific compiler behavior.

Scout found that the source producer's return was serialized as a conversion
from `TextOutcome` to its declared success type `Text`. That describes erasing
the disposition tag, contrary to the accepted atomic tagged-result boundary.
The source declaration still correctly names its normal success type; its
runtime carrier must retain the full tagged value. Before/after source-derived
facts are preserved in `experiments/configresolve/evidence/tagged-return/`.

The semantic producer and callable now carry the same explicit
`atomic_tagged_result`/`TextOutcome` result projection. The return operand is the
exact tagged owner, with no conversion to normal success. Shared validation ties
producer function, return operation, carrier and owner together and refuses
foreign callable projections. Graph version 3 retains that projection; older
versions remain inspectable non-executable contracts. Native structured callable
loading now reads the declared runtime carrier. Ordinary calls to this producer
outside its bounded source graph are refused because separate call-site failure
routes are not yet admitted. This is an explicit support boundary, not a new
implicit failure propagation rule.

The corrected native signature exposed an independent defect: native lowering
returned success for a prepared artifact whose lowering plan was still blocked.
The regression's LLVM-emission request was accepted; no linked executable was
used as evidence. Previously an unsupported signature had accidentally caused
refusal. Native prepared-artifact handling now explicitly rejects blocked plans
and missing callable-plan admission status, preserving historical plan-v1
compatibility. Both lowerers again refuse the graph without creating an artifact.

Validation: focused 4/4 CTests; full Igor doctor/build/test, 192/192 CTests.
The graph regression now rejects 200 hostile mutations across two policy
selections and four pipeline stages, including erased tags, changed return
owners and foreign result projections. Source-derived runtime tests pass two
Clang-18 ASan/UBSan cases (`detect_leaks=0`, same documented ptrace limitation).
The ConfigResolve scalar probe retains 96-case native/TinyVM parity, renaming
control and hostile-oracle rejection. Governance audit: 41/41, zero errors/drift;
`git diff --check` clean.

No source recovery execution claim follows. Remaining work in this mission:
load immutable envelope parameters using an actual envelope ABI (both callable
loaders still use payload carriers); generate the producer/selected-response/
exclusive-rejoin calls; prove success-path ownership and final cleanup; supply
caller-owned runtime evidence storage; return structured host results and prove
native/TinyVM parity. The native signature-loading change is preparatory and is
not evidence of successful graph execution. Transformation chains, the complete
ConfigResolve port and the overall language goal remain unfinished.
