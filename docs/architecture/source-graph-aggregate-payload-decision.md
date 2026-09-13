# Source-graph aggregate payloads

Status: bounded direct native implementation checkpoint (2026-09-13).

## v0.32 bounded contract

The first aggregate surface is an immutable, provider-verified record payload
carried through one direct native graph path. It uses the existing aggregate
layout inventory and ABI manifest; a type name or successful parsing is not
layout authority.

- The aggregate is a named provider-owned record with a verified ordered field
  list, size, alignment, offsets, and field carriers.
- The admitted field carriers are `c_int`, `c_long`, `c_ulong` and `c_size_t`,
  with at most 16 fields and no
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
  mutation, aggregate streams, joins, or parallel aliasing is admitted.

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

Aggregate payloads remain separate from the next reentrant/parallel aggregate
scheduling phase. The bounded TinyVM aggregate graph path is now covered; its
general activation-record extension remains later work.

The implementation carries the verified layout array through Flowbind,
Flowparallel, Flowoptimize, backend preparation, and Flowlower. On the current
Linux x86-64 C ABI, verified packed integer aggregates up to 8 bytes are
lowered to their provider ABI `i64` carrier; the carrier is derived from
manifest size facts, not from the aggregate name. Native execution proves one
provider activation, fan-out to two fresh receivers, and payload-derived
results for both the existing `Point` and dedicated `LongValue` coverage.

Direct non-graph aggregate calls remain blocked until their record-literal and
general ABI lowering path is separately completed. Aggregate streams, joins,
and parallel/reentrant aggregate delivery remain out of scope for this
checkpoint; persistent aggregate state is admitted only through the bounded
schedule-v3 one-64-bit carrier contract.

The next slice is to broaden the direct graph contract beyond one startup root
and mature aggregate payload handling alongside the user-ordered scheduling
work, while preserving the verified layout authority.
