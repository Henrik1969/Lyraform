# Ordinary-record readiness — reconnaissance PASS, execution unsupported

Nine generic cases were captured: valid, reordered, missing field, extra field,
wrong field type, unknown record, unknown member, copy and renamed record.
[Before/after evidence](../../experiments/configresolve/evidence/record-readiness/).

Before correction, seven cases received ready callable-plan-v2 and failed native
lowering; unknown record and unknown member were rejected by Flowanalyst.
Record literals were emitted as type `unsupported` with no field facts, while
source coverage called their value-definition operations lowered. This proves
missing execution admission and missing semantic construction facts, not merely
an unimplemented LLVM layout.

A separate [refusal mission](../tasks/ordinary-record-admission-refusal.md)
now rejects record-literal-containing v2 operations before ready publication.
All nine readiness cases now stop at Flowanalyst. Existing plan-v1 declarative
field inspection is retained; it does not establish executability.

Accepted meaning: ADR 0052 independent aggregate values, existing scalar carrier
and operation laws. Implemented coverage: parser structure and field/type name
lookup; generic scalar algorithm already verified by the preceding port probe.
Proposed meaning: exact complete named construction and read-only value projection
with private layout. Undefined/unadmitted: missing-field defaults, effectful
initializer ordering, recursive/resource fields and expanded mutation/lifetimes.

[Concrete decision brief](../development/ordinary-record-construction-decision.md)
presents complete construction, partial/defaulted construction and deferral.
Henrik's selection is pending; the proposed rule has not become authority.
The missing-field and wrong-type cases would falsify a premature semantic-support
claim. Renaming/reordering exercises name independence without adding semantics.

What became usable: precise generic blocker evidence and truthful refusal.
No executable record support is claimed. Reused existing audit, source coverage,
scalar machinery and backend pipeline; no alternative configuration runtime added.
Next dependent step: implement accepted record construction after the semantic
ruling. Independent next work: already-accepted disposition Graph IR preservation
under Mission 07 Gate 4. Full ConfigResolve and language goals remain incomplete.
