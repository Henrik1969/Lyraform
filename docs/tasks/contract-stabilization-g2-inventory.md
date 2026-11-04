# G2 numeric and predicate inventory — 2026-11-01

## 2026-11-03 migration addendum

The active fixture and generated-golden migration is recorded in the
[explicit-width fixture migration checkpoint](../checkpoints/2026-11-03-explicit-width-fixture-migration.md).
The table below remains an audit snapshot from 2026-11-01; statements that
active fixtures still contain bare `int` are historical observations from that
snapshot. The migration did not close the implementation rows: explicit-width
guard admission, parser-validity propagation, scalar runtime paths, ordinary
record execution, and older provider/corpus boundaries remain open.

Parent: [bounded G2](contract-stabilization-g2.md). This is an open audit, not a
conformance report. [ADR 0070](../architecture/decisions/0070-fixed-integer-default-and-explicit-conversion.md)
records the earlier decisions; [ADR 0071](../architecture/decisions/0071-explicit-integer-width.md) now supersedes the default alias with mandatory explicit widths.

| Behavior | Authority | Current implementation observation | G2 work / verification |
|---|---|---|---|
| Bare `int` | ADR 0071: concrete width required | Source and imported-plan refusal added; historical runtime paths still need migration | Verify all admissions and migrate active uses by intent, not blind replacement |
| Explicit widths | Binding type policy: int32/int64 distinct canonical identities | Shared scalar algebra selects only int; other names remain outside its slice | Resolve aliases without collapsing distinct widths; reject unsupported operations |
| Integer literals | Invoked type range; no implicit narrowing | Several operand paths default to c_int; shared G1 proof bounds division literals only | Full spelling/range checks at source and imported consumers |
| Arithmetic overflow | ADR 0070: authorized widening or failure route | Native add/sub/mul/unary minus use unchecked instructions | Prove range or refuse missing routes; boundary and composed-expression tests |
| Division/remainder | ADR 0074, ADR 0075, and owner overflow law | `div` and `%` are integer-only with bounded LLVM/TinyVM parity; `/` is true division with bounded `float64` promotion and binary64 exceptional/precision parity | Add dynamic integer failure routing, broader conversion policy, and user-selected rounding rules |
| Explicit conversion | Owner intent/policy requirement | Typed initializer uses conversion operand; placements and returns incorrectly receive same tag | Preserve actual source intention; validate range and origin independently |
| Parameters/results | Same identity absent explicit conversion | Argument mismatch and backend carrier reconciliation need repair | Exact signatures, hostile operands, valid explicit conversions |
| Host entry result | ADR 0073: explicit language result and separately selected host mapping | MainBlock lacks result metadata; analyzer inserts c_int; no mapping selector implemented | Model approved; concrete declaration proposal requested before syntax implementation |
| Records | ADR 0068 plus scalar identity laws | Finite record operands have separate expression projection | Apply the same numeric checks recursively and at copied/call boundaries |
| Bool/operator typing | Canonical scalar authority | Shared algebra distinguishes Bool/int | Preserve no numeric-to-Bool conversion and both source projections |
| Initialization/control flow | Existing scalar/guard facts | Current scalar facts and branch/loop consumers | Test uninitialized reads, stale identity, writes, branch/loop range proofs |
| Predicates/guards | Binding refinement policy and static guard lifecycle | G1 refuses unenforced refined values; static guards remain supported | Audit totality and dependencies; effect-free is not proof of safe evaluation |
| ABI integer contracts | Separate ABI type contracts, not source primitive aliases | c_int/c_long/c_ulong/c_size_t carry host assumptions | Verify selected ABI widths and signedness; no implicit source conversion |
| Floating/wider integers | Type identities exist; operations need accepted rules | `float64` literals and bounded mixed `+ - * /` now have shared LLVM/TinyVM evidence; other families remain outside the slice | Inventory unresolved executable paths; do not adopt host semantics |

The source probes for placement, initializer, argument and return conversions
are preserved in mission evidence. The ready-plan conversion tags are reproduced
observations, not authorization. G1's exact closure snapshot remains preserved
before G2 changes. G2 stays open until every row has enforcement and differential
evidence or an explicit, mechanically enforced unsupported boundary.

## Failure reconnaissance after entry-model approval

The preceding publication measured 166/196 passed, 30 failed. The later
numeric-promotion checkpoint records the new focused evidence and the current
full-suite measurement; neither represents a green whole-repository baseline.
The owner requested the [concrete host-mapping proposal](entry-result-host-mapping-proposal.md)
for review before syntax implementation.

Fresh probes of 51 existing source files reached from the failure traces used
canonical frontend bundles and version-2 analysis: 42 accepted, 9 refused.
These are source-analysis observations, not 51 executable conformance tests.
Five refused files contain bare integer types: call_expression_probe,
parallel_independence_probe, matrix_workload, array_demo, fn_canonical_demo.
The latter two aggregate workloads also expose source-operation gaps.
abi_strlen_main refuses an implicit c_size_t-to-entry-result conversion.
abi_libc_demo, abi_pointer_contracts_demo and abi_struct_demo have canonical
source-operation gaps; some older tests inspect them through compatibility
analysis, so version-2 refusal alone does not establish a new regression.

First failing boundaries group as follows; later failures may be masked:

| Test entries | First work area |
|---|---|
| 7, 8, 9, 182, 183 | Corpus/matrix: width-bearing fixtures, explicit result accounting, and existing source-operation gaps |
| 15, 27, 28, 29, 76, 106, 125, 142, 147, 149, 151, 155, 179 | Shared callable/parallel fixtures still use bare int; migrate carrier intent and recheck provider behavior |
| 24 | Graph refusal fixture gains width diagnostics before its intended unsupported-graph boundary |
| 53 | abi_strlen_main returns c_size_t through the compatibility c_int main boundary |
| 105, 107, 109, 111, 112, 113, 114 | Scalar, parse, target, field and guard fixtures plus shared scalar authority still require width migration |
| 117, 119, 186 | Record/scalar execution and G1 positive controls need explicit-width migration through shared record/type contracts |

This accounts for all 30 failing CTest entries, including duplicate registrations
of the corpus and matrix. It does not classify every failure as a stale test:
shared scalar_semantics still selects bare int, target facts have hardcoded int
checks, and record operands probe int/Bool. Those are implementation migrations,
not expectations to relax. Preserve each positive execution/identity/guard
assertion and its invalid counterpart.

Evidence: exact failure trace commands and raw source diagnostic probes are
retained in mission state. Trace-wrapper mistakes from preliminary attempts are
not product failures and are not used as verification results.
