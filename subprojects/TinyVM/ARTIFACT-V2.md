# `flowcore.tinyvm_artifact` binary format version 2

**Status:** implementation contract  
**Byte order:** little-endian  
**Purpose:** sectioned, independently validatable executable projection

Version 2 does not reinterpret version 1 reserved bytes. A consumer either
implements the exact selected format/ISA pair or rejects it.

## File layout

```text
fixed header (512 bytes)
section directory (N × 32 bytes)
zero alignment padding
section payloads in ascending type order
```

The file has no trailing bytes outside the final declared section. Every gap and
reserved field is zero. All offsets and sizes are unsigned 64-bit byte counts
from the beginning of the file and are checked for overflow before arithmetic.

## Header

The first 416 bytes retain the version 1 identity envelope. Fields changed or
added by version 2 are below.

| Offset | Bytes | Field |
|---:|---:|---|
| 0 | 8 | `FLOWTVM\0` magic |
| 8 | 4 | artifact format version (`2`) |
| 12 | 4 | ISA version |
| 16 | 4 | header size (`512`) |
| 20 | 4 | artifact flags; zero in version 2 |
| 24 | 1 | byte order (`1`, little-endian) |
| 25 | 1 | instruction size (`32`) |
| 26 | 6 | reserved zero |
| 32 | 8 | entrypoint instruction index |
| 40 | 8 | instruction count, repeated from code section |
| 48 | 8 | declared data words |
| 56 | 8 | declared stack words |
| 64 | 64 | artifact identity |
| 128 | 64 | source identity |
| 192 | 64 | target-policy identity |
| 256 | 64 | lowering-plan identity |
| 320 | 64 | optimization identity |
| 384 | 32 | SHA-256 with this field zeroed |
| 416 | 8 | section-directory offset (`512`) |
| 424 | 4 | section count |
| 428 | 4 | directory-entry size (`32`) |
| 432 | 8 | exact total file size |
| 440 | 72 | reserved zero |

Identity fields use the restricted version 1 identity alphabet and canonical
zero padding.

## Section directory

Each 32-byte entry contains:

| Offset | Bytes | Field |
|---:|---:|---|
| 0 | 4 | section type |
| 4 | 4 | flags; zero unless defined by that section |
| 8 | 8 | payload offset |
| 16 | 8 | payload byte size |
| 24 | 8 | logical record count |

Entries are strictly ascending by section type, unique, non-overlapping and
8-byte aligned. Unknown section types are rejected. Empty optional sections are
omitted rather than represented with zero size.

## Section types

| Type | Name | Required |
|---:|---|---|
| 1 | code | yes |
| 2 | typed constants | no |
| 3 | strings | no |
| 4 | storage declarations | no |
| 5 | authorized imports | no |
| 6 | instruction provenance | yes |
| 7 | graph activation metadata | no |

### Code — type 1

Exactly `count` 32-byte instruction words. The header instruction count must
match. Opcode and operand validation is selected by the declared ISA version.
No serialized instruction contains a host pointer or computed-label address.

### Typed constants — type 2

Fixed 24-byte records:

```text
u64 constant identity
u32 carrier: 1=i1, 2=i32, 3=i64, 4=opaque_handle
u32 flags: zero
u64 canonical value bits
```

Identities are nonzero and unique. `i1` is exactly zero or one. Unused upper
bits of `i32` are the canonical sign extension. An opaque-handle constant may
only be zero; non-null handles are created by validated runtime operations.

### Strings — type 3

The payload begins with `count` fixed 24-byte records followed by the shared
byte area:

```text
u64 string identity
u64 byte-area-relative offset
u64 byte length
```

Identities are nonzero and unique. Ranges are ordered, non-overlapping and
within the byte area. Bytes are length-delimited; no implicit terminator or
encoding claim exists. A later typed value states whether bytes represent
UTF-8, another encoding or opaque data.

### Storage declarations — type 4

Fixed 32-byte records:

```text
u64 storage identity
u64 byte size
u64 alignment
u32 kind: 1=read_only, 2=writable, 3=stack, 4=arena
u32 flags: zero
```

Identity and size are nonzero. Alignment is a nonzero power of two. Target
policy and runtime capacity must admit every declaration before execution.

### Authorized imports — type 5

Fixed 576-byte records. Each record contains a nonzero `u64` import identity
followed by eight canonically zero-padded 64-byte restricted-text fields:

1. contract identity;
2. library/provider identity;
3. calling convention;
4. symbol identity;
5. effect identity;
6. comma-separated parameter carriers;
7. result carrier;
8. authorization-evidence identity;

The final 56 bytes are reserved zero. Import records are requirements, never
permission by themselves. The runner resolves them only against active policy
and provider evidence. Process addresses are stored in runtime state, not the
artifact.

### Instruction provenance — type 6

Fixed 168-byte records:

```text
u64 instruction index
u64 operation identity
u64 block identity
u64 symbol identity (zero when not applicable)
u32 source line
u32 source column
64-byte source artifact identity
64-byte derivation identity
```

There is exactly one record per instruction, ordered by instruction index.
Operation and block identities are nonzero. Source coordinates are one-based.
The source identity must agree with the header. Derivation identifies the
lowering/optimization evidence responsible for this emitted instruction.

### Graph activation metadata — type 7

Fixed 368-byte records describe the validated graph identity carried by a
`TV1_GRAPH_ACTIVATE` instruction:

```text
u64 metadata identity
u64 activation identity
u64 input activation identity (`UINT64_MAX` represents -1)
u64 input signal identity
u64 output signal identity
u64 delivery identity
five 64-byte restricted-text fields: kind, node, wire, input port, output port
```

Metadata identities are nonzero and strictly ordered. Empty source/root
identity fields are represented by `-`. The section is optional and is emitted
only for graph artifacts; it contains no host pointers or runtime addresses.

## Validation order

An independent consumer validates without executing:

1. magic, complete header and supported format/ISA pair;
2. canonical header fields and exact file size;
3. SHA-256;
4. directory ordering, uniqueness, bounds, alignment and complete coverage;
5. required sections and header/section count agreement;
6. each section's canonical records and unique identities;
7. all cross-references between code, constants, strings, storage, imports and
   provenance;
8. target-policy compatibility and declared resource envelope;
9. ISA-specific control flow, operands and explicit terminal behavior.

Failure reports identify format, version, section, record/index or identity and
reason. Consumers never continue with a partially validated artifact.

## Determinism

The writer sorts sections by type and records by identity except provenance,
which is ordered by instruction index. It zeroes all padding, uses canonical
integer encodings and hashes the final byte sequence with the digest field
zeroed. Equivalent authoritative input must produce byte-identical output.

## Compatibility

- Format 1 + ISA 0 remains the recovered prototype envelope.
- Format 2 may carry ISA 0 for contract testing and ISA 1 for Flow-capable code.
- Adding a section, changing record meaning or accepting new unknown-field
  behavior requires a new artifact format version.
- Changing opcode or carrier semantics requires a new ISA version.
