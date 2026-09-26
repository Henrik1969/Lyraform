# Uniform owned-obligation transfer

Date: 2026-09-26. Authority: Henrik accepted unique transfer with one uniform
mechanism across compiler stages; ADR 0060 records the decision.

Objective: prove that one owned completion can cross a function return without
copying or discharging its obligation, using one shared contract from analysis
to both executable backends.

Baseline: post-Gate-2 consolidation PASS; main at
`6ba9db303db453af952327069814c3a607b26d03`, existing dirty mission and hardening
work preserved. No staging, commits, pushes, branch or FlowLFS changes.

Scope: shared generic ownership relation; source projection and independent
artifact validation; preservation through carrying stages; one nullary producer,
one direct return and one entry caller with complete local outcome handling.
TextOutcome supplies the first supported physical carrier. No per-type transfer
law or new backend ownership interpretation is permitted.

Evidence: type-independent contract cases, returned success and failure on LLVM
and TinyVM, source refusals, hostile relation mutations at every relevant
consumer, exact fact preservation, canonical suite, sanitizers and leak probes.

Non-goals: copying/sharing owned resources, forwarding chains, parameters,
containers, recursion, fan-out, new recovery syntax and runtime policy sinks.
Ordinary independent-value semantics remain unchanged.

Deliverable: [implementation checkpoint](../checkpoints/2026-09-26-uniform-owned-transfer.md).
