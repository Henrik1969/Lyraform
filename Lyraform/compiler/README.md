# Lyraform v0.29 compiler — Reusable Native Language Chain

This is the active Lyraform implementation line on `main`:

```text
main
```

It inherits the tagged `flowmini-v0.24-frontend-border` implementation and the
historical v0.25 projection milestone. The current v0.29 line carries that
frontend boundary through semantic analysis, capability binding, lowering, and
native execution.

## Purpose

The historical v0.25 milestone matured that structural projection boundary:

- broaden and harden AST-to-SymbolTable projection coverage;
- preserve factual metadata and source provenance;
- make AST-to-symbol and AST-to-scope origins precise and testable;
- expand projection and independent-consumer goldens;
- harden the versioned frontend bundle contract;
- prepare a trustworthy input boundary for later semantic analysis.

The current v0.29 line extends beyond that projection boundary with semantic
analysis, capability binding, Graph IR/lowering boundaries, and native runtime
proofs. The historical projection contract remains the documented input
boundary.

The historical projection status is documented in
[v0.25 SymbolTable projection status](docs/v0.25-symboltable-projection-status.md).

Source ingress validates UTF-8 before lexing. Valid non-ASCII source is
accepted; malformed, truncated, overlong, surrogate, and out-of-range byte
sequences fail with a byte-offset diagnostic and no frontend artifact. The
boundary is covered by the `flowmini_utf8_source_boundary` CTest. This is a
Stage 0 ingress guarantee, not yet the Flow-written source reader required for
self-hosting.

FlowIR, AST-symbol, token-tree, and FlowIR-symbol file outputs are rendered
before publication, written and synchronized through a private sibling file,
and atomically renamed over the requested destination after a successful close.
Write, sync, close, or rename failure preserves an existing destination and
returns `FLOW_OUTPUT_FAILURE` at stage `output` in structured mode.
Parent-directory crash durability and abrupt-death orphan cleanup are not
claimed; stdout dump modes remain streaming outputs rather than file artifacts.

## Build

From this directory:

```bash
cmake -S . -B cmake-build-debug -G Ninja
cmake --build cmake-build-debug -j20
```

## Gates

```bash
cmake --build cmake-build-debug --target flowmini_frontend_bundle_tests
cmake --build cmake-build-debug --target flowmini_symbol_projection_tests
cmake --build cmake-build-debug --target flowmini_ast_golden_tests
cmake --build cmake-build-debug --target flowmini_suite
ctest --test-dir cmake-build-debug --output-on-failure
# includes flowmini_utf8_source_boundary
```

Opening baseline:

```text
AST golden tests:          26/26
Symbol projection tests:   11/11
Frontend bundle tests:      5 golden, 1 isolated, 11 negative
Flowmini suite:             78/78
CTest:                       2/2
```

Build trees, runtime build output, test reports, and cache files are local
artifacts and must not be committed.
