# Text value contract v0.1 — proposal

**Status:** proposed; not current language semantics
**Scope:** smallest provider-independent Text slice for Lyraform
**Related:** [`Lyraform/backlog.txt`](../../Lyraform/backlog.txt)

## Why this proposal exists

Lyraform already carries string literals and the `c_string` ABI carrier through
selected compatibility paths. That carrier describes a native call boundary;
it does not define an owned language value. Promoting it silently would make
ownership, encoding, allocation failure, and backend representation accidental.

This proposal establishes the laws that must be accepted before implementing a
first-class `Text` value.

## Proposed language meaning

- `Text` is an immutable, owned sequence of Unicode scalar values encoded as
  canonical UTF-8.
- A Text value owns its storage for the duration of its value lifetime. A
  function may return Text without borrowing from a temporary or provider
  buffer.
- Text has no public pointer identity and cannot be mutated through a native
  pointer escape.
- Text literals produce Text values. The source spelling is decoded once and
  invalid UTF-8 or invalid escape sequences are rejected with source
  provenance.
- `Text + Text` produces a new Text value, preserving left-to-right operand
  order. Concatenation does not mutate either operand.
- `print` accepts Text and emits the value through a declared text-output
  capability. It does not reinterpret Text as an integer or opaque pointer.
- There is no implicit conversion between `Text` and `c_string`. Conversion at
  an explicitly declared native boundary must state its ownership and lifetime
  contract.

## Failure and resource laws

Text construction and concatenation may fail for invalid encoding or exhausted
bounded storage. Failure is explicit and preserves the source operation and
operand provenance; it is not represented by a null pointer or a truncated
string.

The initial v0.1 implementation may use a target policy to impose a maximum
Text byte length. The limit and failure disposition must be visible in the
policy/artifact evidence. A later dynamic-storage policy must not change the
meaning of existing programs.

## Backend projection

The backend-neutral lowering artifact should carry a typed Text value or an
opaque validated Text handle, never a process-local host pointer.

- LLVM/native lowering may materialize owned UTF-8 storage and produce a
  temporary nul-terminated view only for an explicitly authorized provider
  call.
- TinyVM should carry a validated artifact/runtime handle and resolve a native
  address only inside an authorized provider bridge.
- Both backends must agree on scalar content, concatenation order, output,
  failure outcome, and provenance. Native layout and allocation strategy are
  target details, not language meaning.

## Minimum acceptance corpus

The implementation is not ready for admission until it has positive, negative,
and adversarial coverage for:

1. empty and non-ASCII literals;
2. Text assignment and return ownership;
3. Text + Text ordering and repeated concatenation;
4. invalid source encoding/escapes;
5. bounded-storage exhaustion with explicit failure;
6. print output and embedded newline behavior;
7. rejection of implicit `Text`/`c_string` confusion;
8. preserved source operation identity through semantic, binding, optimized,
   and backend artifacts;
9. LLVM/TinyVM differential behavior where both backends admit the slice;
10. renamed source units and unused capabilities not changing Text semantics.

## Compatibility boundary

Existing `c_string` ABI declarations and fixtures remain unchanged while this
proposal is evaluated. They continue to mean borrowed, read-only native string
carriers at authorized external calls. No current program gains `Text` behavior
until the language syntax, artifact schema, providers, lowerers, and acceptance
corpus are implemented together.

## Open review questions

These are deliberate review points for alpha testers and critics:

- Is Unicode-scalar-value validation the right v0.1 boundary, or should the
  language initially expose validated UTF-8 bytes instead?
- Is immutable owned Text the smallest useful model, or does it impose too much
  allocation cost before a borrowing/view model exists?
- Should resource exhaustion be a normal outcome edge, a bounded compile-time
  refusal, or policy-selectable between the two?
- Does a dedicated text-output capability improve auditability enough to justify
  another provider contract?

Until these questions are resolved by evidence, this file is a proposal and
not authority for compiler behavior.
