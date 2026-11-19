# EXECUTION-001 — Portable Lyraform mission execution

- Identifier: Lyraform/EXECUTION-001
- Version: 1.0
- Owner: Henrik Sørensen
- Status: active local adaptation, explicitly authorized by the current request
- Effective: 2026-10-17
- Parents: [AGENT-001 v1.0](../../Policies/rules/AGENT-001.md),
  [WORK-001 v1.0](../../Policies/rules/WORK-001.md),
  [STATE-001 v1.0](../../Policies/rules/STATE-001.md),
  [GOV-001 v1.1](../../Policies/rules/GOV-001.md).
- Changed local sources: AGENTS.md and standing stage procedures, preserved in
  [the pre-change snapshot](history/2026-10-17-agent-neutral/AGENTS.md).
- Decision provenance: Henrik instructed executor independence and implementation
  using policy and agent definitions on 2026-10-17.

## Scope and exact local change

Lyraform maturation missions retain their Markdown definitions directly under
`docs/tasks/` and all existing semantic, convergence and verification gates.
Their measurable goal criteria and task references must be explicit. The portable
JSON representation may accompany a mission; it does not become a second owner
of language semantics. The declared canonical representation and its digest must
be recorded, and divergent representations require reconciliation before resume.

New mission state uses `.mission-state/<mission-id>/<revision>/` with documented
records. Codex, another AI, a human or automation may operate it. The old
`.git/codex/runs/` path and `/tmp/lyraform-codex-*` naming remain legacy adapter
locations, not mandatory semantics or default state for new missions.
Existing live records are not moved, renamed, re-bound or declared complete.
Migration requires validating identity, exact source revision, evidence and
permission scope, then an explicit handoff record. Missing legacy fields must
be established from evidence, not invented from old status labels.

Within project governance: shared and applicable local policies govern AGENTS.md;
standing project procedures guide a selected mission; checkpoints and run state
are evidence. A mission's workflow exception must link to an explicit local
Policies file rather than silently supersede a standing rule. Platform/system
instructions and actual permissions still constrain every executor.

## Evidence and verification

Pre-change copies preserve the earlier Codex-specific storage and authority
wording. Existing local semantic laws are unchanged. The shared validator checks
portable records, not all historical Markdown/agent-native formats. New validation
must not be misrepresented as migration of existing runs or proof of their goals.

## Review and history

Review when a parent policy or the stage protocol changes. 2026-10-17: adopted
executor-neutral storage and policy-first project instruction hierarchy; all
existing live mission state remains untouched.
