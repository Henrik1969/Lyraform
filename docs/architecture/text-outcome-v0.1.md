# Text outcome contract v0.1 — proposed

**Status:** proposed backend-neutral contract
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

The next implementation slice should add a typed backend-neutral `text_outcome`
operation and a provider contract that reports `TextFailure` without exposing a
host pointer. Existing `flow_text_concat` may remain as a compatibility
adapter while success, exhaustion, and provider-unavailable mappings are
differentially tested. Until then, the bounded provider and backend traps stay
explicitly transitional.
