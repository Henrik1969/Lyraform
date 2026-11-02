# Gate 4 progress: transformed-failure successor admission audit

Stage remains ACTIVE; no full Gate 4 or language-completion claim.
Branch experiment/configresolve-lyraform, unchanged base HEAD plus reviewed edits.

Three counterexamples were reproduced before editing: transformed failure wired
to a missing node, an ordinary success function and activation containment.
Each received semantic status ok with no diagnostic. The implementation checked
wire presence but never resolved/accounted its destination. Shared topology
validation also admitted transform response facts without successor authority.

Flowanalyst now diagnoses unknown and wrong-kind successors explicitly. A declared
consumer-chain successor remains unsupported until its precise type and obligation
linkage are admitted by a bounded chain stage. Shared single-consumer topology
validation rejects transform claims without that authority. This enforces ADR
0066 and no-dangling-failure law; it introduces no replacement failure semantics.
Transform signatures can still be inspected separately. No known executable
transform implementation was removed: none exists in this source pipeline.

Evidence: 3/3 focused test groups pass, full **191/191 PASS**, 17.83 seconds.
The existing source suite now includes the three source counterexamples; report
mutation also cannot manufacture a linked-successor transform. Scalar application
regression still passes 96 comparisons on native and TinyVM, source renaming and
corrupted-oracle refusal. [Logs and counterexamples](../../experiments/configresolve/evidence/transform-admission/).

What became usable: truthful refusal of counterfeit successor routes. What
remains: actual finite consumer-chain semantics projected from source, and runtime
recovery/containment. Simplification: existing source diagnostics and shared
contract validator enforce one authority; no new runtime or ConfigResolve code.
The counterexamples falsified the earlier acceptance claim and now remain tests.

## Next implementation facts

The recovery-only graph is preserved and tested independently of transformation
chains. Source response facts identify immutable_failure_envelope, but callable
plan parameters currently retain only the TextFailure payload carrier. Native
lowering maps that carrier to i32. Making a graph executable at this point would
lose producer/attempt/obligation/commit/provenance evidence even if the example
response ignores its parameter. Do not enable readiness or hide this in a helper.

Next bounded step within the active compiler stage: preserve the response parameter
identity and envelope projection in canonical callable/graph facts, validate their
agreement at every boundary, then define a generic backend envelope ABI under the
accepted immutable-envelope law. Reuse existing FailureEnvelope/receipt concepts;
do not create application-owned failure handling inside the compiler.
The broader transformation work remains required, not silently dropped. The
separate ordinary-record construction ruling remains pending owner selection.
