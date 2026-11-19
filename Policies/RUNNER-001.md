# RUNNER-001 — Legacy Codex loop adapter boundary

- Identifier: Lyraform/RUNNER-001
- Version: 1.0
- Status: active adapter classification; no new mission-state exception
- Owner: Henrik Sørensen / Lyraform maintainer
- Recorded/effective: 2026-10-17
- Parents: [STATE-001 v1.0](../../Policies/rules/STATE-001.md), [AGENT-001 v1.0](../../Policies/rules/AGENT-001.md), [EXECUTION-001 v1.0](EXECUTION-001.md)
- Decision provenance: user requested consolidation of policy-driven execution; classification preserves existing runs without promoting their control flags.

## Scope and rule

`tools/run-autonomous-codex` is a product-specific legacy loop adapter. It binds
its control directory to a task path/content digest. Its CONTINUE, BLOCKED and
DONE flags describe that adapter's loop, not the portable mission's measured
goal status or permission to run a new mission. A DONE sentinel alone MUST NOT
be reported as goal achievement under WORK-001 or STATE-001.

Use the portable work records and guarded state writer for new shared mission
state. When using the legacy adapter for an existing run, retain its manifest
and flags as adapter-local evidence. Before claiming a goal, switching executor
or resuming through the portable mechanism, establish the exact canonical
mission definition, authorization, measurements and evidence explicitly under
EXECUTION-001. No automatic conversion of DONE/BLOCKED to achieved/blocked is
permitted. No existing live run was modified by this classification.

## Evidence and limitation

[Legacy runner](../tools/run-autonomous-codex), observed 2026-10-17;
SHA-256 `d817f6a4a9be928fe51feb120648f26e3fc169b4595bcfebbbbd8aa23c8c9ce2`. Its focused test uses a fake executor and tests loop isolation,
not goal measurement. The runner itself does not validate portable JSON records.
That limitation remains visible; this record is not a compatibility certificate.

## Review and history

If the adapter is later integrated with portable state, require isolation,
measurement and handoff tests before changing this classification.
2026-10-17: explicit boundary recorded; runner implementation unchanged.
