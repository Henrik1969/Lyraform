# Ordinary record shape validation — partial implementation checkpoint

Mission `lyraform.ordinary-record-construction`, revision 1, remains active.
ADR 0068 is accepted authority; this checkpoint supplies implementation evidence.

Flowanalyst now diagnoses missing, duplicate and unknown named fields in typed
local ordinary-record construction, including nested literals. Resolution uses
lexical symbols and declaration origins; nested field types resolve from their
declaring record. Duplicate declared fields are diagnosed on construction.
Diagnostics identify the source statement. Both declarative and executable plan
requests receive these semantic diagnostics. Valid construction remains explicitly
blocked for execution until the remaining value machinery is implemented.

Nested probes also exposed silent loss of consecutive record declarations. The
frontend top-level boundary now recognizes `record`, preserving both declarations;
the tests check the actual frontend declaration inventory.

Verification: 24 source cases cover complete/reordered/missing/extra/duplicate
fields, flat and nested construction, and both plan versions. Canonical Igor
build, doctor and all 193 tests passed. The separate scalar ConfigResolve probe
passed 96 cases, with renamed-source and hostile-oracle checks and 911 protected
files verified. Policy audit: 41/41, zero errors/evidence drift. Diff whitespace
check passed. [Logs](../../experiments/configresolve/evidence/record-construction-shape/).

Limits: field typing, purity, finite-type admission, construction in other
expression contexts, independent copy/parameter/return semantics, typed record
operands, hostile downstream artifact checks and native/TinyVM execution remain
unfinished. Neither this mission nor the full ConfigResolve port is complete.
The [implementation map](../development/ordinary-record-implementation-map.md)
identifies owners and the next dependencies. No application-specific compiler
or runtime behavior was added; no commit, push, comparison or merge was made.
