# TinyVM current-LLVM parity inventory

**Date:** 2026-09-13
**Authority:** exact current public lowering/provider tuples; no application-name dispatch

This inventory closes the governed-provider audit without claiming unsafe
runtime parity. `flowtinylower` emits a structured `unsupported` result and no
executable for every tuple outside the admitted set below.

## Executed with differential LLVM parity

| Contract/effect | Exact typed operations |
|---|---|
| provider-free | empty, typed literals, strings/storage handles, conversions, unary/binary arithmetic, comparisons, definitions, assignments, branches, loops, checked static arguments, return |
| `libc` / `pure` | `abs(c_int)->c_int`, `labs(c_long)->c_long`, `strlen(c_string)->c_size_t`, `strnlen(c_string,c_size_t)->c_size_t` |
| `ctype` / `pure` | `tolower(c_int)->c_int`, `toupper(c_int)->c_int` |
| `libc` / `io` | `puts(c_string)->c_int` |
| `file_io` / `io` | `open(c_string,c_int)->c_int`, `read(c_int,c_pointer,c_size_t)->c_long`, `write(c_int,c_pointer,c_size_t)->c_long`, `close(c_int)->c_int` with runtime-owned descriptor tracking, bounded storage, partial-transfer and initialized-byte checks, and cleanup |
| `kernel` or `linux` / `readonly` | `getpid`, `getuid`, `getgid`, `geteuid`, `getegid`, `getppid`, `getpgrp` as `()->c_int` |
| `kernel` or `linux` / `readonly` | `getpgid(c_int)->c_int`, `getsid(c_int)->c_int`, `getpriority(c_int,c_int)->c_int` |
| source graph / serial fresh activation | one authorized `startup_once` provider, typed receiver pipelines and fan-out through graph schedule v1; differential LLVM/TinyVM stdout/result and deterministic artifact checks |
| source graph / finite scalar stream | one authorized `stream` count `()->c_size_t` and item `(c_size_t)->c_int` provider, bounded item loop, direct root-to-receiver fan-out and linear type-continuous receiver-pipeline parity with LLVM |
| source graph / persistent scalar or verified aggregate activation | one startup provider, repeated fresh deliveries, typed `c_long` or one-64-bit verified aggregate state initialization/update and LLVM/TinyVM output parity through schedule v3 |
| source graph / pure parallel activation | validated schedule v4 dependency waves with pure, non-nested receiver bodies; deterministic serial TinyVM projection is equivalent for this effect-free surface |
| verified aggregate payload | packed, no-padding verified `c_int`, `c_long`, `c_ulong` or `c_size_t` layouts up to 8 bytes, aggregate provider return and aggregate parameter call through typed 64-bit payloads; layout/schedule mutations and exact-policy mutations are refused |

Every import requires an active policy match before execution and a named typed
thunk after admission. Missing policy, effect drift, carrier drift, library
drift and unimplemented tuples fail closed.

## Structured unsupported inventory

| Surface | Exact operations | Missing governed semantics |
|---|---|---|
| readonly mutable output | `clock_gettime(c_int,c_pointer)`, `uname(c_pointer)`, `getrandom(c_pointer,c_size_t,c_int)` | provider-owned layout, bounded writes and initialized-byte evidence |
| kernel filesystem | `openat`, `read`, `write`, `lseek`, `unlinkat`, `rmdir` with the signatures in `std/abi/kernel.flow` | descriptor/resource identities, buffer mutation, cleanup and failure disposition |
| file-I/O compatibility | `sendfile` with the signature in `std/abi/file_io.flow` | offset/partial-transfer semantics and descriptor/resource interactions |
| process/socket IPC | `pipe2`, `fork`, `waitpid`, `socketpair` | child/process lifecycle, multi-result storage and cleanup |
| loopback networking | `socket`, `bind`, `listen`, `poll`, `accept4`, `connect` | sockaddr/poll layout evidence, descriptor ownership and bounded mutation |
| namespaces | `unshare`, `sethostname`, `gethostname` | privilege/capability policy, mutable output and environment lifecycle |
| memory | `memcpy`, `memmove`, `memset`, `memcmp` | handle ranges, alias/overlap laws and initialized-byte tracking |
| ncurses/TUI | `initscr`, `endwin`, `noecho`, `cbreak`, `waddnstr`, `wrefresh`, `wgetch`, `keypad` | external window lifetime, terminal ownership, cleanup and interactive evidence |
| provider aggregates beyond packed scalar slice | verified layouts larger than 8 bytes, padded/mixed layouts and aggregate-to-aggregate calls | TinyVM admits only packed verified integer aggregates up to 8 bytes in this slice; `c_long` coverage is exercised by `tinyvm_wide_aggregate_parity` |
| graph activation runtime | effectful/nested schedule v4 parallel waves; branching/merging stream pipelines | pure parallel waves, direct stream fan-out and linear stream pipelines are admitted; actual parallel runtime delivery and generalized activation records are not yet implemented |

These are exact remaining implementation slices, not silently substituted LLVM
fallbacks. Target-policy work may select LLVM explicitly for them, but may not
pretend TinyVM admitted the tuple.
