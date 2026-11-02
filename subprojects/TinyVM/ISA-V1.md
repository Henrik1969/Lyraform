# TinyVM Flow-capable ISA version 1

ISA 1 uses the four signed 64-bit fields physically inherited from `InstrWord`
as `opcode`, `a`, `b`, and `c`. Their interpretation is opcode-specific. No
field contains a host pointer or computed-label address.

The artifact header's declared data-word count is the virtual-slot capacity for
ISA 1. Slots contain an initialization bit, carrier identity and canonical value
bits. Slot types are established by defining instructions and checked on every
use. Opaque handles are stable artifact/runtime identities, not addresses.

| Opcode | `a` | `b` | `c` |
|---|---|---|---|
| `NOP` | 0 | 0 | 0 |
| `CONST` | destination slot | constant ID | 0 |
| `MOVE` | destination slot | source slot | 0 |
| `CONVERT` | destination slot | source slot | target carrier |
| `ADD/SUB/MUL/SDIV` | destination slot | left slot | right slot |
| `FADD/FSUB/FMUL/FDIV` | destination slot | left `f64` slot | right `f64` slot |
| comparisons | destination `i1` slot | left slot | right slot |
| `JMP` | absolute instruction target | 0 | 0 |
| `BRANCH` | condition `i1` slot | true target | false target |
| `RETURN` | result slot | 0 | 0 |
| `TRAP` | nonzero trap code | 0 | 0 |
| `HALT` | 0 | 0 | 0 |
| `STRING_HANDLE` | destination slot | string ID | 0 |
| `STORAGE_HANDLE` | destination slot | storage ID | 0 |
| `CALL_IMPORT` | result slot | import ID | first argument slot |

The authorized import record determines the argument count and result carrier.
Arguments occupy consecutive slots beginning at `c`. Runtime resolution is a
separate governed provider boundary. Until it is supplied, `CALL_IMPORT`
deterministically traps as unresolved rather than embedding or discovering a
host symbol opportunistically.

Integer arithmetic operates on equal `i32` or `i64` carriers. `ADD`, `SUB`, and
`MUL` use modulo-2^width results, matching the current LLVM lowerer's plain
integer instructions without `nsw`/`nuw`. Floating arithmetic operates on
equal `f64` carriers and uses IEEE-754 host operations for the currently
admitted finite slice; `FDIV` explicitly traps on a zero divisor. Integer
division by zero and signed minimum divided by minus one are explicit traps
rather than host-language undefined behavior. Future checked arithmetic
requires distinct semantic operations. Conversions support `i1`, `i32`, `i64`
and integer-to-`f64`; narrowing to
`i1` accepts only canonical zero or one. Comparisons are signed for integer
carriers and produce `i1`.

`RETURN`, `TRAP`, and `HALT` terminate execution. `HALT` returns canonical
`i32(0)`. A configurable instruction-step limit converts accidental or
unauthorized nontermination into an explicit runtime trap; target policy will
eventually supply that bound.
