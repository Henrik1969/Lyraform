# PLACEMENT-001 — Lyraform current identity and development directory

- Identifier: Lyraform/PLACEMENT-001
- Version: 1.0
- Status: active owner-authorized placement and identity ruling
- Owner: Henrik Sørensen
- Effective: 2026-10-17
- Parents: [GOV-001 v1.1](../../Policies/rules/GOV-001.md), [ENG-005 v1.1](../../Policies/rules/ENG-005.md), [AGENT-001 v1.0](../../Policies/rules/AGENT-001.md)
- Authority: Henrik's explicit instruction in this conversation to consolidate under `LyraForm`, with current project name `Lyraform`.

## Ruling and scope

The current project name is **Lyraform**. Its single canonical Development
directory is **LyraForm/**. Current components remain distinct beneath that
root; their different purposes and copies are not merged by directory placement.
Linked development worktrees belong in `worktrees/`. External dated examples,
old workspaces and recovery material belong in `history/`, explicitly as evidence.

Flowcore, Flowmini and earlier version labels refer to historical naming,
lineage or compatibility provenance. They do not name a competing current
project or select current authority. Current work must cite the actual checkout,
selected mission and revision-specific evidence rather than adopt an old version
as ambient truth. The old top-level Flowcore directory and Lyraform alias are retired.

Existing executable names, serialized identifiers, component paths and APIs
may retain legacy spelling as compatibility interfaces. This placement decision
does not authorize breaking those interfaces. Such identifiers are not evidence
that the historical project identity remains current. Native Git branch names
and retained task records are provenance; moving them does not activate a mission.

## Boundaries and provenance

FrankenCore remains architecture/policy authority for its governed family;
FrankenPOP remains a possible future system. Shared libraries outside LyraForm
retain independent ownership. Compiler-local and sibling TokenTree/SymbolTable
copies remain distinct. No commit, merge, deduplication or implementation rewrite
is authorized by this policy.

[Move manifest](../../Policies/missions/2026-10-17-lyraform-consolidation/moves.json)
records exact prior/current locations. Before-edit navigation documents are
preserved under Development `Policies/history/2026-10-17-lyraform-placement/`.
Historical source bytes are preserved; prior mission definitions/state remain
bound to their original identity and evidence paths. Use the relocation map to
locate old evidence; do not rewrite old completion or authorization records.

## Review and history

Review when placement changes, a new worktree is created, or a compatibility
identifier is deliberately migrated. Retain the relocation/provenance map.
2026-10-17 v1.0: owner-authorized directory consolidation and current naming.
