# Text outcome contract v0.1

**Status:** bounded backend-neutral value and must-account contract implemented; wider ownership remains scoped
**Scope:** failure representation for owned Text construction

The current `flow_text_concat(Text,Text): Text` provider shape is useful for
the bounded slice, but a null pointer is not a portable language failure
value. LLVM currently traps through `llvm.trap`; TinyVM currently reports an
import trap. Those are tested transitional mappings, not the final semantic
contract.

## Proposed value

Text construction should produce a tagged outcome:

```text
Outcome<Text, TextFailure>
```

`Text` is present only in the success variant and owns its storage. `TextFailure`
contains a stable code and the originating operation identity. The initial
codes are:

| Code | Meaning | Retry expectation |
|---|---|---|
| `invalid_input` | a supplied Text handle or encoding violates its contract | no |
| `exhausted` | the selected byte/storage limit or allocator capacity was exceeded | policy-dependent |
| `provider_unavailable` | the authorized provider could not perform the operation | policy-dependent |

The outcome is backend-neutral and serializable as a value; it contains no
host pointer, allocator address, or backend trap number. A backend may retain a
diagnostic trace with its native trap details, but must map the semantic result
to the same failure code.

## Boundary rules

- Producing a tagged outcome creates one canonical disposition identity.
- Storing the outcome transfers its must-account obligation to the local
  semantic owner; storage does not discharge failure.
- The currently admitted source shape projects `.code` exactly once and uses
  complementary `== 0` and `!= 0` sibling branches.
- The projected tag and any named zero comparator remain unchanged in the
  containing function. Call return values use a separate status local.
- Production (or an admitted return call), projection, and the two sibling branches must follow their
  established statement order in one containing block. Nested producer routes,
  additional entries to the handling blocks, and exits that bypass accounting
  are refused in this bounded slice. Handling branches must fall through.
- `.value` is available only in the proven success branch and its owned Text is
  disposed exactly once there, after every admitted direct use. Cleanup is
  identified by its canonical provider contract, not the source callee name.
  Other admitted direct uses must be read-only borrowed external-call
  arguments with call lifetime; copying the payload into a local or passing it
  to another owning boundary remains outside this slice.
- The failure branch must perform explicit behavior, must not access `.value`,
  and must not be empty.
- One nullary producer may directly return an owned outcome to one entry
  caller under [ADR 0060](decisions/0060-uniform-owned-obligation-transfer.md).
  The generic transfer fact retains the obligation identity, invalidates the
  old owner, and requires complete local accounting by the caller.
- An uninspected, partially inspected, copied, aliased, indirectly returned, nested, or
  otherwise unproved outcome is refused as a dangling outcome wire in this
  bounded stage.
- The maximum byte length is target-policy data, not a compiler profile.
- A provider must not return a null pointer as a successful `Text` value.
- A failed construction emits no normal Text result and does not transfer
  ownership of partially allocated bytes.
- A successful result may be returned, assigned, repeated, or delivered to
  multiple consumers; each consumer observes the same immutable content.
- Cleanup is attached to the owned success value and runs once after the last
  owner, independently of LLVM or TinyVM representation.
- The existing borrowed `puts_text(Text): c_int` boundary remains valid for the
  duration of its call and does not become an owner.

## Migration shape

The lowering plan marks the bounded construction operation as `text_outcome`
and validates a serializable `result_outcome` declaration with the
`Outcome<Text,TextFailure>` shape. `concat_outcome` returns the provider's
tagged `{code,value}` carrier atomically. LLVM represents that carrier as a
local `{i32,ptr}` value; TinyVM represents it as a checked opaque outcome
handle whose fields are projected only by the governed runtime. Neither
representation serializes a host pointer.

Flowanalyst additionally emits one `lyraform.disposition_fact` v1 for the
producer. It records the exclusive `Success<Text>` and
`Failure<TextFailure>` possibilities, atomic tagged publication, owner and
code-projection identities, complementary branch proof, exact success-value
uses and disposal, exact failure-recovery operations, stable failure codes,
and source provenance. Flowcontracts validates those relations independently;
all carrying stages preserve the fact exactly, and both executable backends
reject missing or hostile authority.

`flow_text_concat` remains a compatibility adapter for existing programs. The
provider also exposes `flow_text_concat_outcome` and
`flow_text_outcome_dispose` for its runtime-local pointer form. A successful
Flow value is consumed by its final `puts_text` call and then passed exactly
once to the explicit `dispose` capability. TinyVM clears the corresponding
runtime-owned value before provider teardown, so teardown cannot free it a
second time.

Flow code can still use the separate `flow_text_concat_status(Text,Text):c_int`
probe as a compatibility recovery facade. New code should use the atomic
`TextOutcome(concat_outcome(left,right))` value and branch on `.code` before
consuming `.value`; the status probe and atomic call are not combined in the
same path.

For TinyVM, `flowtinyrun` now preserves the observed failure as an execution
record field:

```json
{"outcome":{"type":"Outcome","failure_type":"TextFailure","failure_code":"exhausted"}}
```

This is an external execution result in addition to the Flow value. LLVM
native execution returns the normal process result after Flow-level recovery;
TinyVM retains its machine-readable result and governed fault fields for the
older adapter's compatibility path.
