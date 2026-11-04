# Activation frame runtime — private backend ABI

This C library stores one bounded serial activation's disposition and evidence.
It implements accepted ADR 0062/0063/0065/0067 invariants; it is not another
semantic authority, route selector, source interpreter or public Lyraform ABI.

The generated executor must supply authority already checked by the shared
source/graph validators. `flow_activation_begin` copies that descriptor into a
zero-initialized frame. All strings and payload storage remain caller-owned,
immutable and alive until the host consumes the result. No payload layout,
ordinary record semantics or source lifetime rule is established here.

Legal transitions are:

- active -> published, for an ordinary successful producer;
- active -> pending failure envelope -> published, after the exact selected
  response completes with the declared success type and its own attempt;
- active -> quarantined, for the declared fault and containment authority.

The frame permits only `atomic_tagged_result` and `no_commit` unsuccessful
outcomes in this initial bound. It does not handle response-attempt faults,
transformation, partial commits, siblings or concurrent access. Recovery closes
one obligation and retains its originating envelope in the result. Quarantine
has no normal value and retains the fault envelope and containment identity.
A terminal frame admits no further transitions, including reinitialization.

Every function returns explicit admission of the requested ABI transition.
Zero means a caller contract violation with byte-for-byte unchanged frame,
not a synthesized language failure or fault. Generated code must handle that
transport refusal explicitly; ignoring it is not authorized continuation.
There is no exception transport, allocation, evidence truncation, implicit
callback, TLS, name lookup or process exit in the frame API. The bounded native
TextOutcome bridge uses only explicit generated callable and cleanup callbacks;
it does not turn callbacks into a general runtime dispatch mechanism. Evidence
accessors return const pointers. The frame's C storage is trusted
implementation detail, not writable source data.

Current verification feeds facts from real compiler-produced, validated graph
versions 2/3 fixtures into the linked C library. It checks evidence retention,
exclusive terminal publication, identity/provenance refusals, independent
frames, quarantine transition refusal and long evidence. The C++ test driver
uses exceptions only to report test/admission failures. Production C code does
not use them. The frame driver supplies test payloads; the separate generated
bridge test executes the admitted response body through compiler-emitted
callables.

Source execution remains blocked. A bounded native callable unit now generates
one `flow.source.disposition` bridge for the validated one-route `TextOutcome`
specimen. It supplies real producer/response/rejoin values and the preserved
authority to this runtime; the public source graph is still refused. The bridge
does not establish general response-result ownership: its admitted response is
a static/borrowed `Text` literal, while provider-owned successful Text is
disposed through the generated cleanup callback. Native graph execution,
TinyVM parity, receipts and general control-flow readiness must still be proven
before the source disposition status changes.
