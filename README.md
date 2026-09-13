# Lyraform

Lyraform is an experimental programming language and system-architecture
project built around explicit contracts, compiler-visible graph structure,
provider boundaries, provenance, and governed executable projections.

Lyraform was previously developed under the name Flowcore. Flowmini names the
historical prototype and bootstrap lineage; the current language and semantic
model are Lyraform. The project remains experimental, unstable, and not
production-ready.

The primary developer-facing toolchain command is `igor`.

<p align="center">
  <img src="igor.png" alt="Igor, the Lyraform compiler mascot" width="320">
</p>

## Current status

```text
project:          Lyraform
active branch:    main
toolchain:        Igor
status:           experimental / unstable / not production-ready
current lineage:  v0.29 reusable native language chain
```

The current verified boundary includes source-driven scalar/control-flow
lowering, exact generated ABI evidence, durable scalar graph activation,
Flow-owned pager behavior, and governed LLVM/TinyVM backend boundaries. See the
[current verification ledger](docs/checkpoints/2026-09-07-reusable-flow-chain-result.md)
for exact evidence and scoped limitations.

For external evaluation, use the [tester and critic onboarding package](docs/onboarding/README.md).

## Igor

```text
igor --help
igor doctor
igor check
igor build
igor test
igor run -- Lyraform/compiler/examples/pass/fn_demo.flow
```

`igor` is a small, inspectable driver around the existing CMake, compiler, and
CTest entry points. The underlying stage binaries remain available for
stage-specific work and compatibility. `igor check` validates the CMake
configuration; `igor build` builds the canonical root graph; `igor test` runs
the canonical tests; and `igor run` delegates to the current `flowmini`
compatibility executable for a source or artifact path.

## Repository map

```text
Lyraform/compiler/
    current v0.29 compiler implementation

Lyraform/flowmini_v24_explicit_ast/
    preserved historical Flowmini v0.24 checkpoint

Flowanalyst/ Flowbind/ Flowparallel/ Flowoptimize/ Flowlower/
    independent semantic, binding, planning, optimization, and lowering stages

Frankencore/
    governance and constitutional architecture reference material

subprojects/TinyVM/
    independent governed backend experiment

docs/
    architecture, language, development, history, and verification records
```

## Build and test without Igor

```bash
cmake -S . -B /tmp/lyraform-build -G Ninja
cmake --build /tmp/lyraform-build
ctest --test-dir /tmp/lyraform-build --output-on-failure
```

The clean root graph currently registers 81 CTest tests. Individual stage
builds remain useful for focused development; the root graph is the canonical
clean-checkout verification path.

## Intellectual provenance and acknowledgements

Lyraform builds on decades of published work in compiler construction, formal
methods, graph-based computation, contract-based design, provenance,
heterogeneous models of computation, and type/effect systems. See
[REFERENCES.md](REFERENCES.md) for the project’s intellectual provenance and
reading list, and [ACKNOWLEDGEMENTS.md](ACKNOWLEDGEMENTS.md) for software,
tools, and implementation influences.

## Historical naming

The migration from Flowcore to Lyraform is documented in
[docs/history/FLOWCORE_TO_LYRAFORM.md](docs/history/FLOWCORE_TO_LYRAFORM.md).
Historical documents retain Flowcore, Flowmini, and version-era names when
those names describe the state that existed at the time.

## License and disclaimer

This project remains under the existing MIT license. It is not intended for
production, safety-critical, security-critical, financial, legal, medical, or
operational use.
