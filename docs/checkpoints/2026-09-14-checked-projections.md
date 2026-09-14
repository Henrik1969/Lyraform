# Checked public projection checkpoint

**Date:** 2026-09-14  
**Status:** implemented and verified on `main`

The Frankencore Packages and Runtime public JSON projections now provide
`to_json_checked(...) noexcept` wrappers. A successful wrapper returns the
serialized projection; allocation, standard, and unknown failures return an
invalid result with an empty JSON payload and an explicit error string.

The Runtime probe uses the checked path. The Packages reference probe verifies
that the checked and compatibility projections agree on valid input. The
compatibility `to_json(...)` APIs remain available while callers migrate.

Verification:

```text
clean incremental build: PASS
complete CTest gate: 109/109 passed
```

This does not yet provide failure injection for serialization allocation or
translation failure. Those cases remain part of the adversarial verification
campaign; the wrapper nevertheless fails closed if they occur.
