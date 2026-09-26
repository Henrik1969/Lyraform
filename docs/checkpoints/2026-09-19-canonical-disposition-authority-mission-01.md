# Canonical Disposition Authority — Mission 01 checkpoint

Date: 2026-09-19.

Gate 1: **PASS**.

## Baseline and protected scope

- branch: `main`;
- starting `HEAD`: `6ba9db303db453af952327069814c3a607b26d03`;
- starting `origin/main`: `6ba9db303db453af952327069814c3a607b26d03`;
- mission:
  [Canonical Disposition Authority — Mission 01](../tasks/canonical-disposition-authority-mission-01.md);
- `master` and `flowlfs-v0.1-alive`: untouched;
- no staging, commit, push, branch mutation, or repository-host mutation.

The mission continued the existing unstaged bounded static-guard stage. No
unrelated user work was overwritten; `meta-discusions.md` remained untouched.

## Accepted authority

ADR 0058 establishes one canonical operation-disposition fact with validated
contextual projections.

The first implementation introduces `lyraform.disposition_fact` version 1 for
an already-admitted, statically proven guarded scalar transition. It records:

```text
stable fact and lowering-operation identities
statement, expression, scope, and available function-owner identities
exactly-one completion cardinality
the canonical successful scalar payload type
atomic destination commit and destination route
the eliminated Failure<GuardViolation> disposition
the exact lyraform.guard_fact proof identities that eliminated it
source, AST, line, and column provenance
```

This is a bounded semantic-proof fact. It is not a general runtime carrier and
does not assert that arbitrary operations are infallible.

## Implementation

- Flowanalyst assigns stable `fact_id` values to guard facts.
- Proven guard-transition facts are grouped by affected lowering operation.
- A ready plan emits one disposition fact per bounded guarded scalar
  transition, with the complete guard-proof set.
- Blocked plans emit no executable disposition facts.
- Lowering-plan v2 facts additionally preserve owning function identity.
- Flowcontracts independently validates operation, scalar type, success route,
  atomic commit, exclusive completion, exact guard-proof set, owner, and
  provenance consistency.
- Removing disposition authority from an artifact that carries guard authority
  is rejected as a downgrade.
- Artifacts predating guard/disposition authority remain compatible by absence
  and make no disposition claim.
- Flowparallel, Flowoptimize, Flowprepare, LLVM preparation, and TinyVM
  preparation preserve the lowering plan rather than recomputing the fact.

## Developer evidence

Flowvalidate reports a missing disposition fact at the exact
`lowering_plan.disposition_facts` path with the reason:

```text
proven guarded operation lacks disposition authority
```

This explains the failed semantic relation without treating the diagnostic as
a route or guessing source intent. Human and structured diagnostic maturation
continues under ADR 0057.

## Hostile evidence

The focused gate rejects **11 guard-fact mutations** and **20 disposition-fact
mutations**. Disposition attacks cover:

```text
missing authority field and empty authority set
duplicate fact
invalid fact identity, format, or version
invented operation
changed statement, expression, or scope identity
weakened completion cardinality
missing or reclassified possible disposition
changed success payload type, commit law, or destination route
missing or changed eliminated GuardViolation
missing, duplicate, or invented guard-proof linkage
invalid provenance
```

Flowvalidate, LLVM preparation, and TinyVM preparation all refuse the forged
facts.

## Verification

- `./igor doctor`: **PASS**;
- focused disposition/guard lifecycle: **1/1 PASS**, 0.97 seconds before the
  final hostile expansion and **1/1 PASS** after expansion;
- related validator, identity-preservation, scalar-source, and guard set:
  **4/4 PASS**, 7.36 seconds;
- `./igor build`: **PASS**;
- canonical `./igor test`: **165/165 PASS**, 83.80 seconds;
- Clang ASan/UBSan focused suite: **1/1 PASS**, 2.16 seconds, with
  LeakSanitizer disabled because of the known host ptrace-wrapper limitation;
- Valgrind producer and validator probes: **PASS**, no reported errors under
  `--error-exitcode=99 --leak-check=full`;
- changed shell script: `sh -n` **PASS**;
- `git diff --check`: **PASS**.

The full build retained a pre-existing unused-function warning in
`Flowparallel/src/graph_cuda.cpp`; this mission did not change that file.

An attempted focused build through the repository-local `build/` directory
encountered its stale pre-rename `Flowmini/...` CMake path. Verification used
Igor's canonical `/tmp/lyraform-build` directory instead; no user build cache
was deleted or rewritten.

## Non-goals preserved

- Runtime-dependent guards remain compile-time refused.
- No recovery, propagation, policy-sink, fault-containment, effect, or graph
  failure-port syntax was introduced.
- No exceptions, hidden unwinding, implicit termination, or diagnostic-and-drop
  behavior was introduced.
- No existing `TextOutcome` or provider response was silently promoted to
  universal authority.

## Gate result

```text
canonical disposition fact v1                         PASS
bounded deterministic producer                        PASS
independent validation                                PASS
exact current-stage preservation                       PASS
hostile LLVM/TinyVM refusal                            PASS
runtime-dependent guard remains refused                PASS
canonical suite                                        PASS
```

GATE 1: PASS

## Next-stage selection

Initial review suggested projecting the already established bounded
`Outcome<Text,TextFailure>` operation into the same authority. Focused scouting
found a semantic blocker: the repository does not establish whether storing a
failure in a tagged value discharges its route obligation or transfers a
must-account obligation that must be closed before the value's lifetime ends.

Treating storage alone as a complete route could permit ignored tagged failures
and recreate dangling wires. The continuation therefore stops for the
[tagged-outcome obligation decision](2026-09-19-tagged-outcome-obligation-decision-brief.md).

Runtime guard execution remains separately blocked on explicit route and
policy syntax.
