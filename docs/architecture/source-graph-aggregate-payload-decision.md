# Source-graph aggregate payloads

Status: implementation phase opened by Henrik after the bounded persistent-state
checkpoint (2026-09-13).

## v0.32 bounded contract

The first aggregate surface is an immutable, provider-verified record payload
carried through one direct native graph path. It uses the existing aggregate
layout inventory and ABI manifest; a type name or successful parsing is not
layout authority.

- The aggregate is a named provider-owned record with a verified ordered field
  list, size, alignment, offsets, and field carriers.
- The first admitted field carrier is `c_int`, with at most 16 fields and no
  nested records, pointers, flexible arrays, ownership transfer, or hidden
  padding assumptions.
- A provider returns the aggregate by value. A source receiver accepts exactly
  that aggregate type and returns an existing scalar carrier. The receiver gets
  one fresh activation per delivery; fan-out reuses the output signal.
- The exact aggregate manifest identity is carried through Flowbind,
  Flowparallel, Flowoptimize, backend preparation, and Flowlower. Every stage
  rejects changed field order, offsets, size, alignment, ownership, lifetime,
  or evidence.
- Aggregate values are immutable graph payloads for this phase. No aggregate
  mutation, persistent aggregate state, aggregate streams, joins, or parallel
  aliasing is admitted.

The initial implementation targets the existing `Point { c_int x, c_int y }`
provider manifest as a generic layout instance, not as a compiler special case:
the implementation must resolve the named layout and field facts from the
captured ABI artifacts. Other aggregate shapes are admitted only when they pass
the same generic layout contract.

## Required implementation evidence

The implementation must add positive and hostile evidence for:

1. exact manifest verification and immutable field-preserving delivery;
2. native by-value provider and receiver ABI agreement;
3. fan-out without aggregate re-execution or mutation;
4. rejection of forged field order, offset, size, alignment, carrier, and
   manifest evidence; and
5. preservation through optimization and backend preparation.

Aggregate payloads remain separate from the next reentrant/parallel scheduling
phase. TinyVM aggregate graph execution remains last in Henrik's ordered plan.

The next slice is to carry verified aggregate layout facts into the generic
graph provider/receiver contract without admitting a handwritten `Point` path.
