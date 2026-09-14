# Structured CLI diagnostics checkpoint — 2026-09-14

## Change

The `flowmini` public CLI now supports:

```text
flowmini --diagnostics json <program.flow>
```

The mode is opt-in and keeps normal human-readable diagnostics unchanged. A
failure is emitted as one JSON record on stderr; artifact-producing stdout
remains empty and the process returns status 1.

Current conservative codes are:

```text
FLOW_DIAGNOSTIC_ERROR
FLOW_RESOURCE_EXHAUSTED
FLOW_UNEXPECTED_EXCEPTION
```

All failures use `status: failed` and `disposition: no_artifact`. This is a
CLI projection boundary, not a new language exception mechanism.

## Evidence

```text
focused test: structured_cli_diagnostics — 1/1 PASS
canonical suite: 108/108 PASS
build targets: 150
git diff --check: PASS
```

The focused test verifies a semantic type error, empty artifact stdout, stable
JSON fields, nonzero exit status, and explicit rejection of an unsupported
diagnostic format.

## Remaining boundary work

The compiler/runtime internals still use C++ exceptions as a Stage 0
implementation mechanism. Provider bridges, runtime APIs, Frankencore public
APIs, allocation-injection coverage, and stable per-condition stage codes
remain open Gate 2 work. No production or safety-certification claim follows
from this checkpoint.
