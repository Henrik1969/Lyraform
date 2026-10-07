# Canonical executable failure flow — Gate 2

Date: 2026-10-07.
Mission: [Canonical Executable Failure Flow — Mission 02](../tasks/canonical-executable-failure-flow-mission-02.md)
Gate result: **PASS for finite closed-set serial routing**

## Baseline and protected scope

The stage began on synchronized `main` at
`9de06b6896dfba1346c553fca22d53e0fe0c7576`, equal to `origin/main`, after
Mission 01 Gate 1. The unrelated untracked `meta-discusions.md` and generated
`output/` tree were preserved untouched.

`master`, `flowlfs-v0.1-alive`, source syntax, Flowanalyst, compiler plans,
LLVM/TinyVM backends, runtime guards, mutation, and parallel execution were not
modified.

## Bounded question and authority

Mission 01 proved one explicit route. Mission 02 asked how one producer may
declare several expected failure types while proving that every member has one
accountable selected route before any member executes.

`lyraform.failure_flow_plan/v2` establishes:

```text
finite producer failure set
    == finite consumer accepted set

each declared type
    -> one explicit typed wire
    -> one policy selection
    -> one authorized route
    -> one existing response transition
    -> one exact function identity
```

The accepted failures remain a semantic set. The plan records a deterministic
producer projection order for corresponding wires and selections, but list
presentation does not create subtype, priority, or nearest-handler semantics.

## Implementation

`flowcontracts/failure_flow_closed_set.hpp` adds:

- a finite closed producer disposition set;
- version-2 plan serialization and reconstruction;
- producer/consumer set-equality validation;
- complete one-wire and one-selection coverage per declared type;
- support for multiple authorized routes per type with exactly one selected
  route;
- complete transition coverage for every authorized route;
- rejection of unreferenced response functions;
- exact concrete-envelope route, transition, and function lookup;
- serial recovery or transformation execution through the selected identity;
- reuse and independent validation of the version-1 envelope and receipt.

The version-1 single-route contract remains unchanged and valid.

## Evidence

Positive evidence uses one producer with `ReadFailure` and
`CredentialFailure`. `ReadFailure` has two authorized recovery routes while
policy selects one; execution proves that the unselected callable is never
invoked. `CredentialFailure` selects a transformation to a linked
`AuthenticationUnavailable` successor obligation.

Negative and hostile evidence refuses:

- empty or duplicate producer failure types;
- producer/consumer closed-set disagreement;
- reordered producer entries without matching wire/selection projection;
- missing wires or policy selections;
- duplicate wire identities;
- wrong wire type or route;
- policy/wire disagreement;
- incomplete response-transition coverage;
- missing or unreferenced response functions;
- an envelope type outside the closed set;
- an unavailable selected callable;
- missing required plan fields and wrong plan versions.

## Verification

- Focused GCC failure authority suite: **4/4 PASS**.
- Focused Clang 18 ASan/UBSan closed-set test: **1/1 PASS** with LeakSanitizer
  disabled for the managed ptraced environment.
- Valgrind 3.22.0: **0 errors**, **0 bytes in use at exit**, 4,753 allocations
  and 4,753 frees.
- Canonical-language authority freeze: **PASS**.
- Current-documentation drift guard: **PASS**.
- `git diff --check`: **PASS**.
- `./igor doctor`: **PASS**.
- `./igor build`: **PASS**.
- `./igor test`: **178/178 PASS**, 58.74 seconds.

## Deliberate boundary

This gate does not admit open failure sets, subtyping, wildcard dispatch,
dynamic consumers, source or compiler projection, backend execution,
propagation, retry, sinks, response-attempt failure, faults, mutation, or
parallel failure composition.

## Next bounded stage

The next mechanically implied stage is a bounded serial transformation chain.
It should consume a transform receipt's exact successor obligation as the next
envelope, preserve correlation and provenance links, enforce an explicit hop
budget, refuse cycles and broken joins, and finish only through a declared
recovery. It must not invent propagation, retry, or response-attempt failure
semantics.

## Gate 2

**PASS.** Every member of a finite producer failure set now has complete,
inspectable, policy-selected serial routing before execution, and no
unselected or undeclared route can run.
