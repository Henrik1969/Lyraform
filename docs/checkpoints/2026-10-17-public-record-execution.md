# Public finite record execution — partial checkpoint

Mission `lyraform.ordinary-record-construction`, revision 1 remains active.
Authority remains ADR 0068 and ADR 0052; this checkpoint adds no source law.

The finite scalar/nested ordinary-record bound now has an explicit
`lyraform.ordinary_record_execution` version-1 contract. It is emitted only for
one sane target after typed fact resolution, structured control-flow proof and
complete source-operation coverage. Shared validation requires the exact
`finite_scalar_nested`, `independent_value`, `private` contract on ready plans;
blocked plans retain the earlier inspection-only behavior. Missing, altered or
blocked ready contracts are rejected.

The public admission test compiles target-wrapped source containing reordered
named nested fields, independent whole-record copy, nested projections, scalar
operators and a branch. Flowanalyst, Flowparallel, Flowoptimize and Flowlower
produce LLVM with private aggregate types and no application dispatch. Clang
builds a real binary which exits 12. Flowprepare plus production TinyVM lowering
and execution of the same optimized artifact completes with result 12 and trap
0. Existing non-target record probes remain blocked, and unsupported shape,
operand, projection and control-flow mutations remain refused.

Focused evidence: the record admission CTest passes, including the public LLVM
and TinyVM binaries and three hostile execution-contract mutations; the native
callable/record CTest passes. Canonical doctor passes and the 193-test suite is
193/193. The 96-case ConfigResolve scalar probe passes, including renamed-source
and hostile-oracle checks with 911 protected files. Native ASan and Valgrind
checks are clean; the public TinyVM Valgrind run reports zero errors. [Logs](../../experiments/configresolve/evidence/public-record-execution/).
Policy audit is 41/41 with zero errors or drift, and whitespace verification
passes. No application-specific compiler/runtime logic, commit, push or merge was
introduced.

Gate: PASS for public local finite scalar/nested record construction, copying and
projection. Function record parameters/returns, broader initializer forms,
member writes/resources and complete arithmetic range/division admission remain
dependencies for the full ConfigResolve port.
