# Uniform owned-obligation transfer — 2026-09-26

## Baseline and decision

Continuation of post-Gate-2 consolidation on `main`, HEAD and previously
verified origin/main `6ba9db303db453af952327069814c3a607b26d03`. The existing
mission, Gate 2 and consolidation worktree changes were retained. No files were
staged, committed or pushed. Historical branches and FlowLFS were untouched.

The previous scout found that parser and backend carriers could express a
return, but source accounting refused it. Unique transfer, copied/shared
ownership and deferral were compared in the return-transfer decision brief.
Henrik accepted unique transfer and required uniform mechanisms across all
compiler stages. ADR 0060 records that law; ordinary independent values remain
under ADR 0052.

Bounded question: can one owned completion cross a direct function return with
its identity and obligation intact, and one new owner, without backend-specific
ownership semantics?

## Implementation and authority

`Flowcontracts/ownership_transfer.hpp` owns the generic relation and legality
checks. `lyraform.ownership_transfer` v1 records producer, return, call, source
and destination owner/function identities, value type and obligation identity.
The relation is nested in the existing canonical disposition obligation.
Its transfer replaces the current owner; the original completion, commit law
and publication routes remain intact.

Flowanalyst resolves the source identities and invokes the shared law. Its
existing accounting proof now applies at the caller destination. Serialized
projections reconstruct the resolved relations and invoke the same law through
shared disposition validation. Flowvalidate, Flowbind, Flowparallel,
Flowoptimize, Flowprepare, LLVM lowering and TinyVM lowering all check this
contract. Carrying semantic/parallel/optimized/lowering artifacts preserve the
fact exactly; binding validates the semantic authority and emits its binding
projection.

The shared transfer law contains no Text dispatch. The first executable carrier
is the already-supported TextOutcome; generic contract tests use OwnedBuffer,
OwnedSocket and Outcome<Packet,IoFailure> labels. These are contract tests,
not claims of executable support for three new source types.

The first native return exposed an invalid aggregate fallback terminator in
LLVM emission. Generic `zeroinitializer` now supplies the correct zero value for
all carrier shapes. No backend ownership rule was added.

## Evidence

- Six programs execute with matching expected behavior on LLVM and TinyVM,
  including returned success and returned failure.
- 23 invalid source programs are refused, including copied return owners,
  multiple callers, copied caller destinations, wrong return type, duplicate
  returns and unhandled caller outcomes.
- 41 hostile mutations are rejected at each of seven format-correct consumer
  boundaries (287 rejection checks). Thirteen target transfer evidence,
  including missing facts, shared mode, identities, type, obligation identity,
  return operands and unexpected call arguments.
- Canonical disposition/transfer facts are compared exactly across three
  carrying projections. Three unrelated carrier labels test the generic law.

## Verification

Results are recorded after the final implementation build; builds and tests
against the same tree were separated. An earlier overlapping rebuild caused
transient executable permission failures and was discarded as invalid evidence.

- `./igor doctor`: PASS.
- `./igor build`: PASS.
- `./igor test`: **166/166 PASS**, 77.86 seconds.
- Clang ASan/UBSan focused outcome, ownership and guard tests: **3/3 PASS**,
  15.09 seconds. LeakSanitizer could not operate under sandbox tracing; the
  approved run outside that restriction passed with leak detection enabled.
- Valgrind analysis, validation, returned native program and returned TinyVM
  program: **4/4 clean**, zero errors and zero bytes remaining at exit.
- Shell syntax and `git diff --check`: PASS.

Logs are retained in
`~/.local/state/lyraform-uniform-transfer-20260926/`. Existing unrelated
`graph_cuda.cpp` unused-function warnings remain.

**Gate: PASS** for the bounded uniform direct-return projection.

## Boundaries and next stages

Supported: one nullary producer, one direct return, one entry caller, and full
local accounting by that caller. Source-owner reuse and duplicate ownership
remain refused. No new copy, sharing, lifetime or concurrency model is implied.

Further maturation should extend this same authority to forwarding chains,
then parameter boundaries and additional represented owned types, with a
separate bounded admission proof and hostile evidence at each step. General
control flow, containers, parallel fan-out, runtime guard routing and policy
sinks remain outside current executable coverage. Each expansion must retain
the accepted unique-transfer meaning; decisions about shared ownership or
policy destinations require their own semantic authority.
