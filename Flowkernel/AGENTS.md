# Lyraform component Flowkernel — executor entry point

Read [Policies/README.md](Policies/README.md), then the
[resolved context](Policies/context.json) and [audit](Policies/AUDIT.md).
Follow the [shared executor instructions](../../AGENTS.md) and
[AGENT-001](../../Policies/rules/AGENT-001.md). These instructions implement
policy; a local override must be its own policy file with parent and evidence.

Also follow the containing context’s [agent instructions](../AGENTS.md); this does not transfer implementation ownership.

## Project boundary

Component-specific contracts stay in their existing source documentation. Lyraform standing development rules apply; this record does not invent independent umbrella ownership.

## Working procedure

Use the project’s existing documentation and build/test instructions identified
in the audit. Establish actual scope, permissions, working directory and
protected changes before acting. The audit records inspected commands, not a
successful build of the current code. Open GAP files remain unresolved.

A mission needs a measurable goal and one or more subtasks. Validate its exact
identity/revision before resuming state; use the
[portable work tools](../../Policies/work/README.md) for the supported JSON format.
Private state belongs under `.mission-state/<mission-id>/<revision>/`; verify
that the owning repository ignores it before writing. No old agent session,
status flag or neighboring mission supplies completion or authorization.

For an executor without automatic file discovery, supply the referenced files
explicitly. If shared policy files are unavailable, report the missing dependency;
do not guess their requirements. Third-party and historical material retains its
own provenance. A dependency is not ownership or approval to mutate another project.

This file authorizes no commit, publication, deployment, device operation or
policy exception. User authorization and actual platform constraints govern work.
