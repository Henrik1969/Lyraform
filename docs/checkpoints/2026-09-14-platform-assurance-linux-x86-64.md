# Platform assurance baseline — Linux x86-64

**Date:** 2026-09-14  
**Status:** host-specific baseline; not a portability or certification claim

The current assurance evidence is scoped to this host and must not be
generalized to another platform:

```text
OS/kernel: Linux 7.0.11-76070011-lyraskitty
architecture: x86_64
compiler: GCC 13.3.0; Clang 18.1.3
CMake: 3.28.3
Ninja: 1.11.1
```

`igor doctor` passes and confirms the required Lyraform paths and build tools.
The Flowkernel machine-readable probe reports:

```text
readonly: ok
tempfs: ok
ipc: ok
socket_ipc: ok
loopback: skipped (EPERM)
namespaces: ok
overall: ok-with-skips
```

The loopback skip is an environmental permission result, not evidence that
loopback isolation is available. No execution claim may rely on that probe
until the required permission is present and the probe returns `ok`.
The probe now supports `--require-complete`, which returns status 3 for this
host rather than allowing a caller to treat `ok-with-skips` as complete
evidence. The default report remains useful for diagnosis.
Other platforms, kernels, privilege configurations, and deployment profiles
remain unverified and are explicitly outside this baseline.
