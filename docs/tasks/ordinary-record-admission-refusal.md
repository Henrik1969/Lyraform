# Ordinary-record execution admission refusal

ID: `lyraform.record-admission-refusal`; revision 1. Owner Henrik Sørensen.
Selected under authorized experiment continuation. Canonical Markdown definition.

Goal: canonical callable-plan-v2 must not call unsupported ordinary-record literal
operations executable. Refuse before ready-plan publication with source provenance
and refused coverage. Retain existing declarative plan-v1 field analysis and ABI
behavior. This implements the existing source-form audit's unsupported boundary;
it does not choose record construction or layout semantics.

Tasks: characterize false-ready cases; add a generic operation admission check;
verify positive scalar case, negative record cases, deterministic repeated output;
run canonical tests; checkpoint and return to generic record readiness.

Scope Flowanalyst admission and tests only, plus mission/evidence records. Existing
Gate 3 edits are protected. Flowanalyst owns source admission, lowerers validate
rather than invent execution. No application name or ConfigResolve code enters
base. Reuse source-operation coverage diagnostics; no alternate validator model.

Convergence endpoint is truthful refusal, not record support. Falsifier: a v2
record initializer remains ready, or existing scalar/ABI/declarative tests regress.
Reproduce via CTest plus the nine readiness probes. Before expansion record exact
coverage and rejected-source diagnostics, preserving pre-edit source hashes.
No commits/push/merge. Record implementation stays pending the separate semantic
decision; no duplicate-field or defaulting meaning is introduced here.
