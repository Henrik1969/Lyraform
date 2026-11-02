# Lyraform stage execution

- Identifier: `LyraForm/STAGE-001`
- Version: 1.1
- Status: active reference to standing project procedure
- Owner: project maintainer; shared-rule exceptions require the affected policy owner
- Recorded: 2026-10-17
- Effective: existing source rules retain their effective status; unresolved changes are not effective
- Authority: Henrik's instruction to map policies and identify local changes explicitly; no new exception approval
- Parent rules: [GOV-001 v1.1](../../Policies/rules/GOV-001.md), [QA-001 v1.1](../../Policies/rules/QA-001.md), [TOOL-002 v1.1](../../Policies/rules/TOOL-002.md)

## Purpose and scope

Explicitly selected Lyraform maturation missions; no stage is selected by this inventory.

## Local wording, rationale and decision

Follow the stage protocol and mission convergence rule under the policy hierarchy and [EXECUTION-001](EXECUTION-001.md). A stage-specific override must additionally have a local Policies file naming its parent, mission/revision, activation condition, termination/review condition and evidence. The mission links to that file. A Git branch name alone never activates a policy. Existing mission records remain intact; no historical exception is silently approved by this map. This is a local application of GOV-001, not promotion of Lyraform procedures to every project.

## Evidence and limitations

- [Flowcore/AGENTS.md](history/2026-10-17-agent-neutral/AGENTS.md) — observed 2026-10-17; SHA-256 `9518897d015eff8303d917e4738c81a2d6477220622fb4c50d55cdf497bd7784`.
- [Flowcore/docs/development/stage-execution-protocol.md](history/2026-10-17-agent-neutral/docs/development/stage-execution-protocol.md) — observed 2026-10-17; SHA-256 `51db869aa92883bee35973bafed61ec32af280f82b295216388c522950112ed3`.
- [Flowcore/docs/development/mission-convergence-rule.md](history/2026-10-17-agent-neutral/docs/development/mission-convergence-rule.md) — observed 2026-10-17; SHA-256 `06e38211da964729166320c5aeb884bcf126e7ae4161f9ebc4257315511b8795`.
- [Flowcore/docs/development/autonomous-next-stage-selection.md](history/2026-10-17-agent-neutral/docs/development/autonomous-next-stage-selection.md) — observed 2026-10-17; SHA-256 `a0e55fe43e137bc86a5e689895b111d163c0d1b55194262a5275ee41ada75cd4`.

## Review and revision

Revisit on parent/source revision changes. Confirm any proposed exception with
the affected policy owner and record the decision before treating it as effective.
2026-10-17: mapped existing requirements or identified mismatch; no implementation change.

2026-10-17 (v1.1): linked executor-neutral adaptation; prior source hashes below identify pre-change snapshots, not the edited current files.
