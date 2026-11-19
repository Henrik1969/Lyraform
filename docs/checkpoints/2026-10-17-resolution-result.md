# Checkpoint: typed ConfigResolve resolution result

The finite resolver now returns `Resolution{status, value, chosen}` after
policy validation. Every branch constructs the named fields with their declared
scalar types; the result is consumed through `read_result(Resolution)`.

The complete source pipeline produces native exit result 33 and TinyVM result
33 with `trap:0`. The four policy cases, six conflict-policy cases, and two
record-branch cases remain green, as does the canonical 193-test suite.

This is still a finite pure-record boundary. Diagnostics, member writes,
resources, collections, and the remaining ConfigResolve model are outside this
checkpoint.
