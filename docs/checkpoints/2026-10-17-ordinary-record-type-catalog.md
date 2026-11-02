# Ordinary record type catalog — partial implementation checkpoint

Mission `lyraform.ordinary-record-construction`, revision 1 remains active.
The analyzer projects finite int/Bool/nested ordinary record declarations into
an explicitly non-executable private type catalog in the lowering plan. It
preserves source declaration/symbol identities and named typed fields without
foreign ABI byte offsets. Shared validation rejects altered meaning, unsupported
scalars, missing references, cycles and duplicate field/type identities.

The actual source fixture contains shared nested records, an unsupported recursive
record and a resource-bearing record. Only the supported finite types appear.
The catalog survives semantic analysis, scheduling, optimization and preparation
unchanged. Forty hostile mutations are rejected across the four artifact stages;
consumer tools also reject the altered incoming contracts tested at their input
boundaries. Ordinary record execution remains refused.

Verification: canonical Igor build, doctor and 193/193 tests pass, including the
existing 100 record cases and new catalog tests. The separate scalar ConfigResolve
probe passes 96 cases with renamed-source and hostile-oracle checks; 911 protected
files verified. Policy audit 41/41, zero errors/drift; diff whitespace check passes.
[Logs](../../experiments/configresolve/evidence/ordinary-record-type-catalog/).

The [projection contract](../architecture/ordinary-record-type-projection.md)
records precise limits. Typed construction/projection/copy operands, linkage to
this catalog, function value ABIs and native/TinyVM execution remain unfinished.
No full record mission or ConfigResolve port completion is claimed. No application
logic was added to compiler/runtime; no commit, push or merge was performed.
