# Aggregate ownership/lifetime composition vertical slice — 2026-11-02

## Scope

This slice makes the existing bounded ordinary-record value law explicit at
the semantic-artifact boundary. A finite ordinary record is an independent
value, and the lifetime of that value and its nested fields is
`enclosing_value`:

```text
ordinary_record_types:
    semantics: independent_value
    ownership: value
    lifetime: enclosing_value
```

The same facts are carried by each projected record identity and by the
`ordinary_record_execution` contract. Flowcontracts requires exact agreement
at every consumer, so a later stage cannot silently reinterpret a nested value
as a shared reference, borrowed field, escaped resource, or process-lifetime
object.

## Composition law

Construction creates an independent aggregate value. Nested finite records
compose recursively under the same law. Copying, projection, and returning a
record preserve value identity semantics without introducing aliasing. A field
does not outlive the enclosing value in this bounded model.

This slice does not admit resource-bearing fields, borrowed references, shared
identity, move semantics, member writes, cancellation, or general cleanup. Such
forms require a separate ownership and lifetime authority with explicit
obligation identity and backend support. The existing unique owned
`TextOutcome` return path remains a distinct contract and is not folded into
ordinary record values.

## Verification

The record catalog regression now checks the new ownership/lifetime facts and
rejects altered values across the semantic, parallel, optimized, and prepared
stages:

```text
python3 Flowanalyst/tests/run-record-catalog-tests.py \
  /tmp/lyraform-refinement-clean
```

Result:

```text
PASS: record catalog preserved across 4 stages; 48 hostile contracts rejected
```

The focused aggregate matrix also remains green, including cross-field
invariants and nested refined aggregates.

## Remaining boundary

The canonical law now distinguishes value-owned aggregate composition from the
separate owned-resource transfer law. The next lifecycle expansion should
introduce one explicitly represented resource-bearing aggregate boundary only
if its construction, transfer, cleanup, failure, and backend contracts can be
implemented together. No such resource-bearing aggregate is admitted by this
slice.
