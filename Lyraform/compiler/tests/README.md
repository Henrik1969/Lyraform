# Flowmini test expectations

This directory contains both historical compatibility expectations and current
structural fixture goldens. They are not one semantic authority.

```text
examples/pass/*.flow     must exit 0
examples/fail/*.flow     must exit nonzero
examples/support/*.flow  ignored unless explicitly requested
```

The `examples/pass`, `examples/fail`, and `examples/support` tree is retained
for the deprecated Flowmini compatibility/oracle workflow. The current
canonical language fixtures use explicit-width source integers (`int32` or
`int64`) and are exercised by the AST, frontend-bundle, SymbolTable,
Flowanalyst, and vertical semantic gates. A compatibility parser may accept
the explicit spellings as an oracle convenience, but it does not define their
canonical meaning.

Expected-output files are optional, but when present the suite runner checks them.

```text
tests/expected/stdout/<test-name>.out
    Exact stdout expected from a passing program.

tests/expected/stderr/<test-name>.err
    Exact stderr expected from a test. Use sparingly because diagnostics may include paths.

tests/expected/diagnostics/<test-name>.contains
    One required diagnostic substring per non-empty line. Preferred for negative tests.
```

The expectation set originated in `flowmini_v22_unit_kinds` with stdin value
`5` and was extended through the closed v0.24 checkpoint. The active v0.29
line retains that categorized-suite contract while its structural projection
and downstream language-chain gates mature independently.

Useful commands:

```bash
$TOP/tools/run-flowmini-test-suite.sh
FLOWMINI_VALGRIND=1 $TOP/tools/run-flowmini-test-suite.sh
FLOWMINI_GDB_FAILURES=1 $TOP/tools/run-flowmini-test-suite.sh
```
