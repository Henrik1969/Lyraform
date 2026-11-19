# Provider-result forwarding slice — 2026-11-05

## Result

The owned `Text` provider result contract now survives one additional
forwarding function:

```text
flow_text_concat -> join() -> relay() -> main owner -> dispose(owner)
```

Flowbind checks the direct transfer from `join` to `relay`, the direct transfer
from `relay` to `main`, and the cleanup of the final owner. Direct provider to
entry calls remain covered by the same fixture. A generated third-hop fixture
is refused before lowering, proving that the bound is not silently generalized.

External resources are unchanged: ncurses cleanup remains capability-based
because its provider contract describes an external/global window lifetime,
not an owned `Text` value.

## Verification

Build directory:

```text
.tmp/build
```

Focused command:

```text
ctest --test-dir .tmp/build \
  -R '^(text_runtime_boundary|tinyvm_runtime_text_parity)$' \
  --output-on-failure
```

Result:

```text
2/2 passed
```

The focused runtime script also verifies the generated third-hop refusal and
the unchanged bounded exhaustion trap. The complete canonical suite remains
the required gate before publication.

## Honest boundary

This is one bounded forwarding hop for provider-owned `Text` results, not
general owned-parameter semantics, arbitrary forwarding graphs, shared or
copied ownership, fan-out, asynchronous cleanup, or exception unwinding.

