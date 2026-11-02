# Explicit integer conversion implementation — 2026-11-01

Parent: [contract stabilization G2](../tasks/contract-stabilization-g2.md).
Authority: [ADR 0071](../architecture/decisions/0071-explicit-integer-width.md)
and [ADR 0072](../architecture/decisions/0072-explicit-value-preserving-integer-conversion.md).
This is a bounded implementation checkpoint, not G2 or mission closure.

Explicit typed initialization now carries conversion intention for identifiers,
expressions and ordinary call results. Assignment, argument passage, return,
mixed arithmetic, call-result placement and argv indexing cannot authorize an
integer type change. Equal machine widths are not type identity. Contextual
literal typing uses a declared destination or callable signature; an existing
typed value keeps its identity until an explicit conversion.

Shared admission checks source/destination identities, signature and target
facts, initializer context, expression identity, range and immutable evidence.
It rejects missing/forged conversion markers and compatibility downgrades that
lose the declared result contract. Native LLVM and TinyVM preserve explicit
int32/int64 carriers. Signed widening preserves every source value; narrowing
requires a proven representable value. Known nonnegative values and unchanged
signed values under a direct nonnegative guard can convert to the selected
64-bit size/unsigned ABI carrier. Writes in the guard block or nested control
flow invalidate that proof. Unknown narrowing/signedness cases remain refused
without an admitted runtime failure route. No new numeric failure type, policy
syntax, implicit route or handler was introduced.

Verification: the source matrix passes 10 positive and 19 negative cases, with
41 hostile pipeline boundaries. Native O0/O2 harnesses inspect full function
results. TinyVM compares values inside the source program, including an
independent 32-bit threshold to detect systematic truncation of a wide value
and its equality literal. Positive/negative widening and narrowing, runtime
parameter widening, call-result conversion, guarded size conversion, invalid
source literals, assignments to uninitialized destinations, nested writes,
arguments, returns, mixed arithmetic and indexing are covered.

Freshly rebuilt GCC and Clang consumers pass the conversion and explicit-width
matrices. The selected flowcat, selector and TextOutcome integration tests also
pass on both. Two further GCC boundary tests preserve their original memory
fault and unsupported-provider targets after correcting their result carriers.
Full current GCC suite: **166/196 passed, 30 failed**. This is not a green
integration baseline. The bare-int migration remains open; provider-parity
fixtures such as abi_strlen_main also still return a wider result through the
old host-status boundary without explicit conversion. Failures are retained,
not relabeled as feature execution or hidden behind generic refusal tests.

The shared checks reuse existing operand, target, function, block and operation
facts. The actual branch establishes a bounded range proof; an optional proof
flag cannot replace it. The source emitter no longer labels arbitrary placements
or returns as explicit typed initializers. Flowcat's sendfile comparisons now
use an explicitly declared c_long zero. The negative-provider sign test uses
its actual c_int status carrier; separate c_long provider coverage remains.

Limits: general numeric/predicate conformance, broader explicit-width migration,
all signedness/range proofs, dynamic narrowing failure routes, record conversion,
legacy oracle migration and the main/host-result contract are not closed here.
No G1–G5 completion follows from this matrix. The original G1 snapshot remains
preserved, and current C1/C2 measurements remain open until the migrated stack
passes their complete criteria.

The mission evidence retains exact tested sources (before this report), per-file
hashes, GCC/Clang commands and raw outputs, focused observations and the failed
full-suite log. Reproduce the matrix with `python3 tools/test-integer-conversions.py
--build BUILD`. Branch/HEAD provenance remains the dirty
`experiment/configresolve-lyraform` checkout at
`b50534c380879a5a70a77ed5c1dd442bfc4142bd`; HEAD alone is not the tested source.
