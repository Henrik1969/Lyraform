# Frankencore revision probe

`Frankencore::Provenance` is the canonical C++20 API for constructing and
validating mutation records. It owns the semantic record and its invariants;
`to_json()` is an inspectable projection, not the canonical state model.

Successful publications use `MutationRecord`; failed attempts use the separate
`MutationRejection` type. Both share `MutationAttempt` metadata and can be
emitted into one event stream without confusing rejection with published
state. Every result has a mandatory `event_id`; every attempt has mandatory
`attempt_id` and `correlation_id` values so retries can be reconstructed later.
The core provides `generate_ulid()` and `is_valid_ulid()` for this identity
paperwork; callers do not define a competing identifier grammar. Boundary
consumers should use `generate_ulid_checked()`, which converts generator
initialization, entropy, allocation, and unknown failures into an explicit
`UlidResult` without allowing a C++ exception to escape. The throwing helper
remains as a Stage 0 compatibility API.

`frankencore_revision_probe` is a deliberately small reference producer using
that API. It emits one versioned JSON evidence record; it does not choose a
storage model or claim to be the runtime mutation log.

Consumers link the `Frankencore::Provenance` target and include:

```cpp
#include <frankencore/provenance.hpp>
using namespace frankencore::provenance;

MutationRecord record{ /* identity, revisions, evidence, and policy */ };
if (const auto result = validate(record); !result.valid) {
    // reject without publishing the mutation
}
const std::string inspectable = to_json(record);
```

The API deliberately does not perform persistence, rollback, retention, or
provider dispatch. Those remain policy and storage-layer responsibilities.

The record demonstrates an immutable old revision, a newly published revision,
the responsible actor and provider, policy, before/after evidence, atomicity,
recoverability, rollback state, causes, and derived entities.

```sh
frankencore_revision_probe --old-revision 7 --new-revision 8 \
  --old-value stale --new-value current
```

The producer has the standard `-h`/`-?`/`--help`, `-a`/`--about`, and
`-v`/`--version` options.
