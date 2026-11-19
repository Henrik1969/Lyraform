# Source disposition policy selection — Gate 3

Date: 2026-10-13

Mission: `docs/tasks/source-disposition-policy-selection-gate3.md`

Branch: `codex/disposition-policy-selection-gate3`

Starting commit: `b50534c380879a5a70a77ed5c1dd442bfc4142bd`

## Result

**PASS — WAITING FOR HUMAN REVIEW.**

The bounded source-disposition path now has one versioned policy-selection
authority without making disposition execution available:

```text
lyraform.source_disposition_policy_selection/v1
lyraform.host_disposition_policy_selection/v1
```

For a consumer with one legal route, Flowanalyst emits a deterministic
`fixed_single_route` selection from the exact Gate 2 semantic identities. It
refuses an externally supplied route selection as extra policy. For the new
two-route `TextOutcome` specimen, Flowanalyst requires one supplied artifact
and embeds that exact canonical artifact unchanged in the blocked semantic
report.

The separate host artifact binds the exact module revision, containment node,
and activation scope. Its only admitted action is ADR 0067's
`report_and_fail`. Missing host policy or any other host action is refused.

## Preserved semantic boundary

Source remains the sole owner of:

- legal response functions and route identities;
- incoming failure and outgoing result type identities;
- response class and obligation transition;
- consumer declaration and instance identities;
- containment identity and scope; and
- all source provenance.

Policy owns only one exact choice plus its profile, revision, authority, and
provenance. The test proves that the Gate 2 topology before selection and after
selection is byte-for-meaning identical. A policy cannot add a function,
route, transition, type, obligation, containment scope, host action, or success
meaning.

The route and host artifacts both retain `execution: unsupported`. The
lowering plan remains `blocked`; every topology execution claim remains false.
No Graph IR, backend, runtime dispatch, or activation quarantine execution was
implemented.

## Positive and negative evidence

- existing one-route source: deterministic compiler-owned route 0 / response
  function 40;
- new two-route source: supplied route 1 / response function 43;
- repeated analyses: byte-identical reports;
- supplied route artifact: external and embedded canonical JSON identical;
- supplied host artifact: external and embedded canonical JSON identical;
- two-route source without route policy: refused;
- source disposition without host policy: refused;
- externally supplied route policy for the fixed one-route source: refused.

## Hostile evidence

The focused test rejects:

- 26 route-policy mutations covering foreign module, stale revision, wrong
  selection kind, zero or extra selections, foreign consumer identities,
  changed failure type, unauthorized route, forged response function,
  changed response class/type/obligation transition, and missing policy
  identity fields or injected undeclared fields;
- 14 host-policy mutations covering foreign module, stale revision, changed
  containment node/scope, unsupported actions, and missing policy identity
  fields or injected undeclared fields; and
- 10 independently validated semantic-report mutations covering changed route,
  function, transition, revision, host action, missing route or host artifact,
  injected undeclared policy fields, and a forged ready plan.

## Verification

Pre-change fresh baseline:

- Igor doctor: PASS;
- clean configure and 324-step build: PASS;
- canonical suite: **187/187 PASS** in 60.97 seconds.

Final fresh external build:

- directory: `/tmp/lyraform-gate3-final-build.hAxRI7`;
- Igor doctor: PASS;
- clean configure and 324-step build: PASS;
- focused Gate 2 and Gate 3 tests: PASS;
- canonical suite: **188/188 PASS** in 62.38 seconds;
- current-documentation guard: PASS;
- agent/workspace hygiene guard: PASS;
- `git diff --check`: PASS.

Focused Clang 18.1.3 ASan/UBSan verification:

- fresh build of `flowmini`, `flowanalyst`, and `flowvalidate`: PASS;
- Gate 3 positive, negative, and hostile matrix: PASS with both sanitizers set
  to halt on error and leak detection disabled for the host environment.

The build retains the two pre-existing `-Wunused-function` warnings in
`Flowparallel/src/graph_cuda.cpp`. No new compiler warning was introduced by
Gate 3.

## Maturity boundary

```text
source-declared legal route topology       YES (Gate 2)
fixed one-route policy selection           YES (declarative)
supplied two-route policy selection        YES (declarative)
external/embedded artifact identity        YES
report_and_fail host selection             YES (declarative)
executable Graph IR disposition            NO (Gate 4)
LLVM/TinyVM failure routing                NO (Gate 5)
runtime activation quarantine              NO (Gate 6)
end-to-end executable closure              NO (Gate 7)
```

## Safety accounting

```text
Gate 4 work started:    NO
new language syntax:    NO
exception path added:   NO
implicit default added: NO
policy-created success: NO
master touched:         NO
FlowLFS touched:        NO
force used:             NO
commit/push performed:  NO
```

The branch stops at Gate 3 pending human review. This checkpoint does not
authorize Gate 4, commit, push, merge, or branch deletion.
