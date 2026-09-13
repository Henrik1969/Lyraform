# Text outcome contract v0.1 — proposed

**Status:** proposed backend-neutral value contract; typed artifact boundary implemented
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

The lowering plan now marks the bounded construction operation as
`text_outcome`, and validates a serializable `result_outcome` declaration with
the `Outcome<Text,TextFailure>` shape. `flow_text_concat` remains the current
provider adapter: its non-null pointer is still the backend carrier for the
success variant, while null maps to the tested backend failure disposition.
The provider also exposes `flow_text_concat_outcome` and
`flow_text_outcome_dispose`, which carry the explicit code and owned success
value in a runtime-local struct. The LLVM and TinyVM `text_outcome` paths now
consume that tagged transport and observe its code before mapping failure to
their transitional trap/fault behavior.

Flow code can now use the separate `flow_text_concat_status(Text,Text):c_int`
probe to branch on the stable failure code before requesting the owned Text;
the probe disposes its temporary success allocation internally. This is an
explicit recovery facade, not yet the atomic `Outcome<Text,TextFailure>` value:
the success value and failure code are still produced by separate provider
operations, and cleanup remains provider-activation scoped. The atomic carrier
and last-owner cleanup are the next slice.

For TinyVM, `flowtinyrun` now preserves the observed failure as an execution
record field:

```json
{"outcome":{"type":"Outcome","failure_type":"TextFailure","failure_code":"exhausted"}}
```

This is an external execution result, not yet a Flow value. LLVM native
execution still exposes the transitional nonzero process disposition.
