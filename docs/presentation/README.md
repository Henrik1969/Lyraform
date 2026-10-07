# Lyraform presentation authority

This directory provides a compact, versioned input for public-facing project
presentation. It does not replace checkpoints, architecture records, tests, or
Git history. It indexes the evidence a presentation maintainer must use.

`current-status-v1.json` is maintained by the project authority after a verified
checkpoint. A presentation-only agent may consume it but must not update it as
part of a presentation refresh.

If the manifest and its referenced evidence disagree, the conflict must be
reported for human review rather than resolved by inference.

The retained `flowcore.presentation-status/v1` schema name is a compatibility
identifier from the historical lineage. It does not define the current project
identity. Present material must identify the project as Lyraform and link
current claims to the dated authority listed by the manifest.
