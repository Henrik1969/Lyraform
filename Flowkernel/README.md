# Flowkernel

Flowkernel is the first isolated Linux-kernel boundary probe brick. Version
0.1 deliberately covers only:

- read-only `getpid`, `clock_gettime`, `uname`, and `getrandom` probes;
- a private `/tmp` directory transaction using `openat`, `write`, `read`,
  `lseek`, `unlinkat`, and `rmdir`.
- child-process IPC through `pipe2`, `fork`, `read`, `write`, and `waitpid`;
- local Unix-socket IPC through `socketpair`, `send`, `recv`, and `waitpid`.
- local-only TCP loopback through `bind`, `listen`, `poll`, `accept4`, and
  `connect` on an ephemeral `127.0.0.1` port.
- child-only user/UTS/IPC namespace creation through `unshare`.

It performs no privileged operations, device access, mounts, external networking,
namespace creation, cgroup changes, reboot, or host shutdown operations.

Every report states its format, version, probe, effects, individual results,
and status. The temporary filesystem probe cleans up its own directory and
file before returning. Callers that need complete evidence can add
`--require-complete`; it returns a nonzero status when any requested probe is
skipped, while the default mode remains a diagnostic report of environmental
limitations.
Malformed command and probe input can request `--diagnostics json` to receive
a stable failure record on stderr with no probe artifact on stdout.
