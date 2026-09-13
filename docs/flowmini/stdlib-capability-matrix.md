# Flowmini standard-library capability matrix

Status: current narrow integration boundary  
Evidence: `tools/run-flowcore-stdlib-boundary.sh`

This matrix defines the claim precisely. “Basic system integration” means that
the standard capability declarations currently shipped with Flowmini are
parseable, inspectable, policy-gated, and—where the ABI contract is supported—
verified against a real host provider. It does not claim coverage of every
symbol exported by glibc or every platform ABI.

| Module | Role | Current evidence | Status |
|---|---|---|---|
| `std/abi/libc.flow` | C string, scalar, and basic output calls | `strlen`, bounded `strnlen`, `abs`, `labs`, and `puts` are resolved through exact `libc.so.6` grants; LLVM and TinyVM execute the pointer-plus-length `strnlen` slice | ready, bounded string-length slice |
| `std/abi/ctype.flow` | Character classification and conversion | `tolower(c_int)` and `toupper(c_int)` resolve through exact `libc.so.6` grants; LLVM and TinyVM execute the scalar conversion slice | ready, scalar conversion slice |
| `std/abi/text.flow` | Bounded owned Text concatenation and recovery outcomes | `concat_outcome(Text,Text)->TextOutcome` returns one tagged `{code,value}` carrier; LLVM and TinyVM cover success, exhaustion, explicit recovery branching, exact authorization, and one final-owner dispose; the status probe remains compatibility coverage | bounded native/TinyVM slice |
| `std/abi/file_io.flow` | C file descriptor I/O | `open`, bounded `read`/`write`, `sendfile`, and `close` resolve through exact grants; LLVM and TinyVM execute tracked descriptors with eight-byte storage and TinyVM initialized-byte checks, including EOF and `/dev/full` error returns, while `flowcat` uses generic typed-plan loops, branches, mutation, and calls | bounded LLVM/TinyVM resource/I/O slice; Linux-specific `sendfile` example |
| `std/abi/memory.flow` | Bounded C memory operations | `memcpy`, `memmove`, `memset`, and `memcmp` resolve through exact `libc.so.6` grants; bounded `c_pointer(8)` buffers execute through LLVM and TinyVM | bounded LLVM/TinyVM executable slice; partial-overlap proof pending |
| `std/abi/kernel.flow` | Current Flowkernel communication matrix | all 32 symbols resolve through exact effect-aware grants and have individual narrow executable profiles with safe probes; process identity and priority queries are authorized through exact provider contracts and executable proofs | executable boundary verified; 32/32 |
| `std/abi/pointers.flow` | Borrowed buffers and opaque-handle contracts | all three type contracts parse and appear in the symbol inventory; no external call is implied by the type-only module | declaration-only |
| `std/abi/testabi.flow` | Internal struct-ABI provider used by language tests | the provider emits and the boundary test verifies a versioned manifest for `Point` size, alignment, and field offsets; Flowbind still blocks aggregate calls until it consumes that evidence | layout verified, call lowering deferred |

## Contract

Flowmini produces the frontend bundle. Flowanalyst derives semantic binding
requirements. Flowbind then performs the following checks without executing a
foreign function:

1. exact capability-policy grant;
2. supported calling convention and scalar/pointer ABI types;
3. provider library availability through `dlopen`;
4. symbol availability through `dlsym`;
5. an inspectable capability record for every authorized requirement.

The binding report’s `capabilities` array is the consumer-facing artifact. It
retains contract, provider, symbol, convention, effect, types, and authorization
status instead of reducing the result to a list of symbol names.

## What is and is not claimed

The ready boundary is sufficient for the current real integration example:
`flowcat` transfers files through declared libc capabilities and emits a native
ELF artifact without an application-specific emitter. It is not a promise that arbitrary Flow code can call arbitrary
native functions, pass arbitrary aggregates, or use the whole host C library.

The struct provider remains visible and tested as a deliberate partial case.
Its layout is now verified by a provider-owned manifest. Aggregate calls must
still not be admitted until Flowbind consumes that manifest and a lowering/runtime
test proves the call convention. A struct name must never be treated as a
scalar ABI type.

Flowanalyst now exports the first part of that contract as
`aggregate_abi_layouts`: the aggregate name, field names/types, contract, and
the explicit provider-verification requirement. The internal provider now
supplies the first versioned `flowcore.abi_manifest`; Flowbind consumes and
records that evidence. Aggregate lowering remains pending.

The memory slice is deliberately bounded: a positive `c_pointer(n)` declaration
creates local storage of exactly `n` bytes, and the Flow-level `c_size_t` count
must remain within that declared bound. This proves the pointer-plus-length ABI
shape for both backends without claiming that arbitrary native pointers or
unchecked buffer arithmetic are safe. TinyVM storage handles are runtime-owned
and reject counts outside their declared bounds.

## Reproduction

From the repository root, after configuring the root CMake build:

```sh
ctest --test-dir /tmp/flowcore-superbuild --output-on-failure -R flowcore_stdlib_boundary
```
