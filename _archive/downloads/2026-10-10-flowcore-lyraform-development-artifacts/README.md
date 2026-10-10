# Historical Flowcore/Lyraform downloaded development artifacts

**Classification:** historical evidence / source archaeology

**Current authority:** none

**Imported:** 2026-10-10 from `/home/henrik/Hentet/`
**Project identity at import:** Lyraform

These files are preserved because they record pre-convergence Flowcore and
early Lyraform development work that had accumulated in the local Downloads
directory. They are not current source, selectable missions, accepted
architecture, build inputs, or executable tooling.

Do not run the Python scripts, apply the patches, or unpack archives into the
active tree merely because they are present here. Current authority comes from
the applicable `AGENTS.md` chain, accepted ADRs, and an explicitly selected
mission under `docs/tasks/`.

## Contents

- two historical review/mission documents;
- ten one-use patch-application helpers;
- six historical patch files;
- one historical autonomous-runner kit;
- one downloaded Flowmini AST source snapshot; and
- one dated Lyraform complex-example bundle.

The example bundle contains `transaction_workflow.flow` and
`maintenance_dispatch.flow` specimens that were not present in the current Git
tree when this archive was created. Preserving the original ZIP avoids losing
that evidence while keeping it outside the active language surface.

`flowcore-c5.1-return-ownership-v2.patch` and its `(1)` download copy are
byte-identical. Both filenames are retained to preserve the downloaded set
exactly. See `SHA256SUMS` for the complete integrity inventory.

## Promotion rule

Nothing in this directory may become current by inference. Reuse requires a
new bounded mission that classifies the artifact, reconciles it with current
canonical law, and promotes only the necessary content to an appropriate
active path with fresh verification.
