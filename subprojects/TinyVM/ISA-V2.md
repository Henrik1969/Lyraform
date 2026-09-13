# TinyVM typed ISA version 2 execution inputs

ISA 2 preserves every ISA 1 opcode and carrier semantic unchanged. It adds an
execution-input convention and the graph activation observation opcode without
adding serialized host state:

- slot 0 is initialized to the process-style argument count as canonical
  `i32`;
- slot `1 + n` is initialized, when argument `n` exists, to opaque handle
  `0x03_00000000000000 | n`;
- absent argument slots remain uninitialized and fault if read;
- the artifact contains neither argument bytes nor host addresses.

The first argument is the artifact/program identity, matching native `argv[0]`.
Thus `flowtinyrun program.tvm selected` exposes count 2 and argument handle 1
for `selected`. A future governed string provider may resolve an argument handle
to bytes; the portable VM value remains an opaque identity.

Lowering computes the highest statically indexed argument, reserves the needed
slots, and emits a checked-count guard returning `i32(64)` before any indexed
access. Dynamic argument indexing is not admitted. Artifacts without argument
intrinsics continue to use ISA 1, unless the artifact contains graph activation
metadata.

Graph artifacts use ISA 2 and may contain `TV1_GRAPH_ACTIVATE` instructions.
The instruction's `a` operand names a record in optional artifact section 7;
`b` is either `-1` for a static activation or a slot containing a canonical
`i64` stream index, and `pad` is zero. A configured runner observer receives
the record at execution time, including activation, signal, delivery, wire and
port identities. The context retains a copied runtime record with a monotonic
execution sequence and stream index. Without an observer the instruction is a
validated no-op; the artifact still carries the same metadata for independent
inspection.

Portable-switch and computed-goto execution initialize identical values and
their complete post-execution states remain differentially tested.
