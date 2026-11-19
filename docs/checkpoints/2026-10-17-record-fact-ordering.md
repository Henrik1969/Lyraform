# Record facts before admission — partial checkpoint

Mission `lyraform.ordinary-record-construction`, revision 1 remains active.
Question: can statement admission inspect the exact typed record facts that are
serialized for backend consumption, without performing type resolution late in
report printing?

Previously, catalog/local type discovery and typed operand resolution occurred
after statement coverage and admission. They now precede coverage. The analyzer
captures an immutable operand map keyed by expression identity and expected type;
serialization only looks up those facts. One helper selects operation argument
expected types for capture and output, removing duplicated selection logic. The
existing later TextOutcome producer projection remains separate. This is an
internal ordering correction, not new source semantics or a public schema change.

Evidence: six record sources in both plan versions produce twelve byte-identical
reports, including identical exit codes and stderr, compared with the pre-change
analyzer executable. The comparison script and hashes are curated in
[the evidence directory](../../experiments/configresolve/evidence/record-fact-ordering/).
The script uses a temporary baseline executable and the current build; its log
records this run, not a portable claim that that temporary executable persists.
Canonical build/doctor and 193/193 tests pass, including focused record admission
and native/TinyVM units. The 96-case scalar ConfigResolve probe passes renamed
source and hostile-oracle checks with 911 protected files verified. Policy audit:
41/41 contexts, zero errors/drift; whitespace verification passes.

Gate: PASS for fact ordering only. No additional program is admitted by this
refactor. The next implementation must derive a supported local record execution
bound from these facts and enforce it through shared downstream validation,
including scalar range/division safety. Function record ABIs and broader
initializers remain outstanding. The complete ConfigResolve port remains active.
No application-specific dispatch, commit, push or merge was introduced.
