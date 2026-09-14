# Runtime failure-containment checkpoint — 2026-09-14

## Change

The compatibility runtime now exposes `flowmini::runModuleChecked`. It
translates `DiagnosticError`, `std::bad_alloc`, and unexpected `std::exception`
conditions into an explicit `RuntimeResult` containing completion status, code,
stage, and message. The compiler CLI uses this checked entry point.

The existing throwing `runModule` and graph-node internals remain as a
temporary compatibility mechanism. They do not define Lyraform semantics and
are contained by the checked wrapper before the public compiler caller
continues.

## Evidence

```text
focused graph/runtime tests: PASS
canonical suite: 108/108 PASS
git diff --check: PASS
```

## Remaining work

Provider bridges, Frankencore public APIs, legacy direct graph callers,
allocation-injection coverage, and stable per-condition diagnostic codes still
need the same checked-boundary treatment.
