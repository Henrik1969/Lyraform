# TinyVM current-LLVM parity inventory

**Date:** 2026-08-26  
**Authority:** exact current public lowering/provider tuples; no application-name dispatch

This inventory closes the governed-provider audit without claiming unsafe
runtime parity. `flowtinylower` emits a structured `unsupported` result and no
executable for every tuple outside the admitted set below.

## Executed with differential LLVM parity

| Contract/effect | Exact typed operations |
|---|---|
| provider-free | empty, typed literals, strings/storage handles, conversions, unary/binary arithmetic, comparisons, definitions, assignments, branches, loops, checked static arguments, return |
| `libc` / `pure` | `abs(c_int)->c_int`, `labs(c_long)->c_long`, `strlen(c_string)->c_size_t`, `strnlen(c_string,c_size_t)->c_size_t` |
| `libc` / `io` | `puts(c_string)->c_int` |
| `kernel` or `linux` / `readonly` | `getpid`, `getuid`, `getgid`, `geteuid`, `getegid`, `getppid`, `getpgrp` as `()->c_int` |
| `kernel` or `linux` / `readonly` | `getpgid(c_int)->c_int`, `getsid(c_int)->c_int`, `getpriority(c_int,c_int)->c_int` |

Every import requires an active policy match before execution and a named typed
thunk after admission. Missing policy, effect drift, carrier drift, library
drift and unimplemented tuples fail closed.

## Structured unsupported inventory

| Surface | Exact operations | Missing governed semantics |
|---|---|---|
| readonly mutable output | `clock_gettime(c_int,c_pointer)`, `uname(c_pointer)`, `getrandom(c_pointer,c_size_t,c_int)` | provider-owned layout, bounded writes and initialized-byte evidence |
| kernel filesystem | `openat`, `read`, `write`, `lseek`, `unlinkat`, `rmdir` with the signatures in `std/abi/kernel.flow` | descriptor/resource identities, buffer mutation, cleanup and failure disposition |
| file-I/O compatibility | `open`, `read`, `write`, `sendfile`, `close` with the signatures in `std/abi/file_io.flow` | the same resource laws plus offset/partial-transfer semantics |
| process/socket IPC | `pipe2`, `fork`, `waitpid`, `socketpair` | child/process lifecycle, multi-result storage and cleanup |
| loopback networking | `socket`, `bind`, `listen`, `poll`, `accept4`, `connect` | sockaddr/poll layout evidence, descriptor ownership and bounded mutation |
| namespaces | `unshare`, `sethostname`, `gethostname` | privilege/capability policy, mutable output and environment lifecycle |
| memory | `memcpy`, `memmove`, `memset`, `memcmp` | handle ranges, alias/overlap laws and initialized-byte tracking |
| ncurses/TUI | `initscr`, `endwin`, `noecho`, `cbreak`, `waddnstr`, `wrefresh`, `wgetch`, `keypad` | external window lifetime, terminal ownership, cleanup and interactive evidence |
| provider aggregates | current `testabi` aggregate probes | provider-owned layout is verified upstream but aggregate call lowering is not implemented |

These are exact remaining implementation slices, not silently substituted LLVM
fallbacks. Target-policy work may select LLVM explicitly for them, but may not
pretend TinyVM admitted the tuple.
