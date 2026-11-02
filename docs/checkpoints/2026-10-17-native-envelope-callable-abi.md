# Native envelope callable ABI — bounded checkpoint

2026-10-17. Mission `lyraform.source-recovery-execution`, revision 1 remains
incomplete. Native source callable emission now preserves immutable failure
envelope pointers, rejects payload/envelope confusion and unsupported writes,
and preserves the tagged producer result. This is generic compiler machinery.

Three actual source variants (direct response, forwarded response, renamed and
changed source body) execute in a C unit host. Forwarding checks exact envelope
pointer identity through a separate link boundary. The test helper exercises
internal callable emission; it is not installed and does not establish source
graph execution. Public lowering of the blocked graph remains refused. TinyVM
recovery execution, generated graph control, ownership closure and parity remain
outstanding.

Verification: canonical build and doctor passed; Igor tests 193/193; focused
checks 4/4; all three callable variants passed ASan/UBSan with LeakSanitizer
explicitly disabled for the ptrace environment. All three passed separate
Valgrind checks with zero errors and zero remaining allocations. The scalar
ConfigResolve probe passed 96 cases with renamed-source and hostile-oracle
checks; 911 protected compiler files were verified. This remains a two-integer
probe, not the complete port. Policy audit: 41/41, zero errors or evidence drift.
`git diff --check` passed.

[Curated verification logs](../../experiments/configresolve/evidence/native-envelope-callable-abi/).

The owner separately approved complete record construction, now ADR 0068.
The next selected mission is `docs/tasks/ordinary-record-construction.md`, with
fresh isolated state. Recovery completion is not inherited or claimed. Record
construction implementation and verification remain pending.
