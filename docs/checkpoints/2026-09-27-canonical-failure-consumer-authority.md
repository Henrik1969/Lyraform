# Canonical failure-consumer authority — 2026-09-27

## Baseline and scope

This stage began on synchronized `main` at
`ecdac7184236dab24423ca7495e882f771cdf287`, with local `HEAD` equal to
`origin/main`. The accumulated canonical-convergence work already present in
the worktree was preserved. No file was staged, committed, or pushed;
`master`, `flowlfs-v0.1-alive`, and the unrelated untracked
`meta-discusions.md` were untouched.

The bounded mission was to establish the smallest carrier-independent contract
for one typed expected-failure consumer, its closed accepted failure set, its
ordinary response-function identities, and a policy selection constrained to
those routes. This stage did not admit source syntax or executable routing.

## Design and authority

`flowcontracts/failure_consumer.hpp` now defines two versioned declarative
facts:

```text
lyraform.failure_consumer/v1
    one semantic consumer identity
    one declaration-scope identity
    one closed expected-failure payload-type set
    one or more exact routes for every accepted type
    one exact ordinary function-symbol identity per route

lyraform.failure_policy_selection/v1
    exact consumer identity
    exact failure payload type
    exact selected route identity
    policy identity and revision
```

The shared contract requires every response function to be an available
definition with exactly one canonical `failure_envelope` input projection for
the routed failure payload type and a non-void declared result. It rejects
duplicate identities, uncovered accepted types, routes outside the closed set,
naked-payload or wrong-envelope inputs, incompatible or unavailable functions,
and policy selections not already authorized by the consumer.

Both facts require `status: "declarative"`. Readers reject a `ready` status,
an open set, or an unsupported version. Neither fact is part of a ready
callable plan, so neither can authorize source admission or execution.

## Evidence

- Positive tests cover two failure payload types, multiple authorized routes
  for one type, exact policy selection, deterministic serialization, and
  round-trip reconstruction.
- Hostile tests cover empty and duplicate failure sets, missing and extraneous
  routes, duplicate route/function identities, missing functions, incompatible
  envelope projection and payload type, void results, declaration-only functions, wrong
  policy consumer/type/route identities, empty policy provenance, invalid
  consumers, open sets, executable status, and future versions.
- The tests use `GuardViolation` and `ResourceUnavailable` as generic contract
  labels. They do not claim those carriers or runtime guard routing are
  executable source semantics.

## Verification

- Focused GCC build and `flowcontracts_failure_consumer`: **1/1 PASS**.
- Focused Clang build and `flowcontracts_failure_consumer`: **1/1 PASS**.
- Clang ASan/UBSan focused run: **1/1 PASS**, with leak detection disabled for
  the sandbox run and both sanitizers configured to halt on error.
- Valgrind focused run: **0 errors**, **0 bytes in use at exit**, 974
  allocations and 974 frees.
- `./igor doctor`: **PASS**.
- Canonical `./igor test`: **174/174 PASS**, 74.94 seconds.

**Gate 1: PASS** for declarative failure-consumer and policy-selection
authority.

## Deliberate boundary and decision gate

The component proves which ordinary function policy may select for a typed
failure and that the function receives the canonical envelope projection. It
does not prove what that function's result means. A C++ or Lyraform return type
alone cannot establish that a response recovered,
transformed, propagated, terminated, or requested a safe retry.

Candidate A in the linked response-disposition decision brief was subsequently
accepted: an explicit response-transition contract will attach to the ordinary
function identity while the envelope preserves producer evidence. ADR 0063
adds bounded evidence epochs and retention budgets. The next bounded stage
must implement that response-transition contract before source integration,
producer wiring, runtime guard routing, or backend execution.
