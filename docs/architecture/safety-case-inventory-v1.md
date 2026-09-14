# Lyraform safety-case inventory v1

**Status:** baseline inventory, active safety mission
**Date:** 2026-09-14
**Authority:** current executable gates and admitted architecture contracts

This is an assurance inventory, not a certification. A passing test supports
only the named boundary; it does not prove safety outside that boundary.

## Classification

Each row uses the following status vocabulary:

```text
implemented       enforced and covered for the named boundary
compatibility     retained legacy behavior with a limited contract
provisional       approved direction not yet complete or stable
future             intentionally outside the current admission surface
not-claimed       no safety claim is made
```

## Admitted compiler and artifact boundaries

| Hazard | Control/enforcement point | Evidence | Status | Residual risk |
|---|---|---|---|---|
| Malformed or truncated stage artifact is consumed | Typed version/field/identity validation at each consumer, including Flowparallel CPU and graph-reference consumers | `flowcontracts_json`, `flowvalidate_artifacts`, `tinyvm_artifact_v2_hostile`, `flowparallel_cpu_provider`, `flowparallel_graph_reference` | implemented | New artifact fields require the same hostile coverage |
| Hostile plan/report/source input exhausts an active boundary | Current Flowparallel planning/report readers, active artifact tools, Flowanalyst bundle inputs, and the active Flowmini source/stdin readers cap input at 16 MiB before parsing or lexing; oversized input emits structured no-artifact failure | `flowparallel_exception_containment` oversized-ingress cases; artifact boundary suites; `flowanalyst_pipeline`; `flowmini_utf8_source_boundary` | implemented for covered active ingress points | Other native readers and historical implementation snapshots require separate treatment |
| Binding requirement cardinality exhausts the governed ABI boundary | Flowbind caps binding requirements at 100,000 entries before requirement expansion and emits structured no-artifact failure | `flowbind_provider` over-cardinality case | implemented for current binding boundary | Other artifact collections retain their own schema-specific bounds |
| Hostile JSON structure exhausts parser resources | Canonical parser caps nesting at 256 levels, collections at 100,000 entries, and total values at 1,000,000 nodes before consumer validation | `flowcontracts_json` deep, wide-array, and node-count cases | implemented for the canonical parser | Consumers may still require stricter schema-specific bounds |
| Provider handle leaks across an exceptional binding path | Flowbind owns every admitted `dlopen` handle with an explicit RAII deleter, including failure and allocation-unwind paths | `flowbind_provider` normal, missing-symbol, and hostile ABI cases | implemented for current binding inspection | Native provider execution remains separately bounded and provisional |
| Invalid source bytes reach lexical semantics | UTF-8 validation before tokenization with byte-offset diagnostic and no frontend artifact | `flowmini_utf8_source_boundary` | implemented for Stage 0 source ingress | Flow-written UTF-8 source-reader closure remains a bootstrap gap |
| Duplicate keys or ambiguous authority alter meaning | Strict artifact parsing and duplicate-key refusal | Flowcontracts and fuzz gates | implemented | Schema expansion can reintroduce parser gaps |
| Provider symbol exists but is unsafe or unauthorized | Exact provider/library/symbol/convention/carrier/effect policy grant; unsupported carriers, return types, and calling conventions are refused before authorization | `provider_call_identity`, `flowbind_provider`, native binding tests | implemented | General ABI/FFI remains refused |
| Binding evidence is mistaken for execution permission | Flowbind separates discovery, authorization, and lowering | `native_binding_inventory`, binding boundary tests | implemented | Broader policy resolver integration remains incomplete |
| Target silently falls back to another backend | Versioned target-policy resolution with explicit fallback mode | `flowtarget_policy_boundary`, `flowtarget_cross_compile`, install-shape tests | implemented | More targets need independent evidence |
| Backend receives forged aggregate layout | Independent size/alignment/offset/carrier validation | `tinyvm_aggregate_parity`, `native_aggregate_graph`, wide aggregate tests | implemented | Larger/mixed/padded layouts remain refused |
| Portable artifact contains host authority | Opaque handles; no raw pointers or computed labels in artifact | TinyVM artifact and ISA hostile tests | implemented | Runtime provider bridges remain platform-specific |
| Native package metadata causes unbounded observation allocation | Read-only package providers cap each dpkg status paragraph at 1 MiB, each APT metadata file (`InRelease`, `Release`, `.sources`, and `.list`) at 4 MiB, and each result collection at 100,000 entries; oversized records/files, cardinality overflow, unavailable files, malformed provider rows, and provider exit failure are diagnosed | `frankencore_packages_probe` oversized-record, oversized-metadata, oversized-list, cardinality, unavailable-source, and provider-failure cases | implemented for current package-provider collections | Broader provider schemas may require independently sized limits |
| A downstream stage silently loses an operation or identity | Complete versioned artifacts with provenance and identity preservation | `flowcontracts_identity_preservation`, pipeline matrix, pass corpus | implemented | General transformation provenance is incomplete |
| Unsupported graph semantics are projected as executable | Explicit graph-lowering refusal, typed execution-plan parsing, and bounded activation contracts | `graph_lowering_refusal`, `flowparallel_cpu_provider`, `source_receiver_frames`, native graph tests | implemented | General joins, reentrancy, effectful parallelism, and branching streams refused |
| Fan-out duplicates computation or changes signal meaning | One activation per input; one result signal; distinct delivery identities | `native_source_graph`, TinyVM graph parity and trace tests | implemented | General scheduler queues and reentrant delivery remain open |

## Runtime, resource, and failure boundaries

| Hazard | Control/enforcement point | Evidence | Status | Residual risk |
|---|---|---|---|---|
| Receiver-local state leaks across activations | Fresh activation frame per delivery | `source_receiver_frames`, native graph tests | implemented | Persistent state has only bounded admitted forms |
| Activation or queue growth exhausts memory | Explicit activation-record budget and bounded FIFO | TinyVM activation budget/FIFO evidence; graph parity | implemented | General async queue/backpressure contract not implemented |
| Unconnected input/output produces hidden behavior | Required-input validation and explicit output-drop diagnostics | `flowcore_graph_routing`, source graph tests | implemented | Broader graph topology remains bounded |
| Arithmetic or provider failure becomes a normal result | Structured trap/outcome; failed activation emits no normal result | graph failure tests, TinyVM ISA tests, Text outcome tests | implemented | Some native process-level failures remain terminal |
| Text allocation failure is represented as success | Tagged `Outcome<Text,TextFailure>` boundary; bounded provider allocation failure returns `FLOW_TEXT_EXHAUSTED` without publishing a value | `text_outcome_boundary`, `tinyvm_runtime_text_parity`, `flowtext_runtime_allocation_failure` | implemented for the bounded Text provider | Wider owned-text semantics and other native allocators remain scoped |
| File/terminal resources leak or close twice | Path-aware cleanup validation and explicit ownership metadata | `file_resource_boundary`, `file_io_boundary`, `sel_tui_pipeline` | implemented | Full cancellation cleanup is not yet admitted |
| Invalid pointer/storage or partial transfer corrupts state | Bounded storage, initialized-byte and partial-transfer checks | `tinyvm_memory_parity`, file-I/O boundary tests | implemented | Compatibility pointer semantics remain narrower than final language design |
| Bootstrap implementation exception crosses a public semantic boundary | Stage 0 C++ exceptions must be translated to structured stage/runtime results before artifact or language semantics are exposed; the runtime capability probe contains discovery and projection failures with empty artifact output | `structured_cli_diagnostics`, `flowparallel_exception_containment`, Flowanalyst/Flowbind/Flowoptimize/Flowlower/Flowprepare/Flowtarget hostile boundary tests, `frankencore_runtime_probe`, `frankencore_runtime_probe_fault`, `runModuleChecked`, and throwing CUDA-cleanup callback test; current audit still finds internal throws/catch/rethrow in compatibility paths | provisional | Provider/Frankencore API containment beyond these boundaries, stable per-condition stage codes, and injected allocation/library exception coverage remain open |
| Retry or cancellation creates duplicate effects | No implicit retry; CPU provider, CUDA provider, and runtime planner refuse cancellation, async, backpressure, and reentrant effectful execution with no fallback artifact | source graph decisions, flowparallel_cpu_provider, flowparallel_cuda_provider, flowparallel_runtime_planner | future | Must define idempotence, commit, cancel, and recovery contracts |
| Torn history or recovery erases committed facts | Append-only durable history with atomic append, per-record and total byte bounds, `fsync`/parent-directory synchronization, explicit `uncertain` result when a partial or zero-progress write or either synchronization barrier changes the file without complete durability, abrupt-exit/torn-tail recovery test, quarantine, explicit repair, typed mutation replay, and read-only branch reconciliation | ADR-0002/0003; `frankencore_error_state_history` including injected partial-write, zero-progress-write, file-`fsync`, parent-directory-`fsync`, abrupt-exit, and oversized-record cases | provisional | Deeper crash fault injection and retention remain required before mutation expansion |

## Provider, trust, and deployment assurance

| Hazard | Control/enforcement point | Evidence | Status | Residual risk |
|---|---|---|---|---|
| Unknown provider is treated as trusted | Verification evidence rejects `allowed` without trusted key, matched integrity, and authenticated/attested identity; weaker outcomes remain explicit | Contracts validation test; verification ADRs | implemented for narrow providers | General signed profile and trust-store implementation is future |
| Local override masquerades as authentication | Override remains policy evidence with diagnostic and provenance | verification contract and policy tests | implemented in contract | Broader admission policy integration remains incomplete |
| Isolation label exceeds actual enforcement | `IsolationClaim` validates assurance/enforcement compatibility and requires explicit resource, identity, filesystem, network, privilege, teardown, provider, and verification fields | Contracts validation test; isolation ADRs | provisional | Provider execution and independent verifier remain incomplete; no production isolation claim |
| Build-host hardware is mistaken for deployment capability | Runtime snapshot separated from compile-time policy | runtime capability and provider-planner tests | implemented | Runtime refresh and broader platform matrix remain open |
| Host memory capability arithmetic wraps or accepts malformed kernel data | Runtime memory facts use strict complete-token parsing and saturating kB-to-byte conversion; malformed fields become zero | `frankencore_runtime_memory`, `frankencore_runtime_probe`, and instrumented runtime probe boundary | implemented for the current Linux memory projection | A broader capability schema needs field-level diagnostics |
| CUDA workload sizing overflows or requests unbounded provider work | CUDA provider accepts only matrix sizes 1..4096 and computes element/byte counts after the bound check, before driver probing | `flowparallel_cuda_provider` complete-token and oversized-size hostile cases | implemented for the provider-selection contract | Device-specific memory admission remains provider/runtime dependent |
| Unsupported platform or skipped capability is presented as complete | Host-specific Linux x86-64 baseline and machine-readable Flowkernel result; default reports skips explicitly, `--require-complete` refuses any skipped probe, and malformed input has structured no-artifact diagnostics | `2026-09-14-platform-assurance-linux-x86-64.md`; `flowkernel_all`; `flowkernel_require_complete`; `flowkernel_structured_diagnostics` | provisional | This host reports loopback `EPERM`; cross-platform assurance and complete permission profile remain open |
| Probe evidence duplicates or omits a requested capability | Flowkernel `all` output requires one unique result for each of the six admitted probes | `flowkernel_all_contract` | implemented for the current probe set | New probes require contract/test updates |
| CPU execution launches an unbounded worker set | The public independent-task executor rejects zero workers and any request above the explicit 256-worker limit before thread creation | `flowparallel_cpu_execution` excessive-worker case | implemented for the current CPU executor | General scheduler admission and cancellation remain refused |
| Graph runtime launches an unbounded worker set | The graph parallel runtime rejects invalid requests, null workers, and counts above 256 with a deterministic process-boundary diagnostic before thread allocation | `flowgraph_runtime_bounds` | implemented for the current graph runtime | Effectful/reentrant parallel scheduling remains refused |
| Graph diagnostics exhaust stderr or log storage | Graph runtime diagnostics share a 16 MiB aggregate output budget, bound raw activation scanning, and emit one structured truncation marker after exhaustion | `flowgraph_runtime_bounds` diagnostic-output and oversized-activation cases | implemented for the current graph runtime | External log retention remains an operator concern |
| Graph worker exception terminates without a semantic outcome | Graph runtime catches standard and non-standard worker failures at the thread boundary and emits the structured graph-failure disposition | `flowgraph_runtime_bounds` throwing-worker case | implemented for the current graph runtime | Provider execution remains process-boundary and not recoverable in-place |
| Partial graph worker launch leaves joinable threads | Graph runtime uses joining thread ownership so vector-growth or later launch failure cleans up already-started workers | `flowgraph_runtime_launch_fault` injected allocation case | implemented for the current graph runtime | Broader native allocation-failure injection remains separate |
| Human approval is silently affirmative | Structured authority questions with expiry and no implicit yes | ADR-0023 | provisional | Interactive trust negotiation is not fully implemented |
| Version requirement parsing consumes unbounded input | The public requirement parser refuses version text and range expressions above 4096 bytes and refuses more than 128 dotted components before comparison | `frankencore_requirements_probe` oversized-input and component-count cases | implemented for the current requirement grammar | Other policy grammars require their own bounds |

## Mission blockers before self-hosting

The current Gate 9 decision and the exact Stage 0 privilege inventory are
recorded in [`2026-09-14-self-hosting-readiness-review.md`](../checkpoints/2026-09-14-self-hosting-readiness-review.md).

The following are not necessarily blockers for every Stage 1 experiment, but
they block any claim that the self-hosted compiler inherits a complete safety
model:

1. The first Stage 1 slice must have explicit outcome, cleanup, boundedness,
   and artifact-validation contracts available in Flow rather than only in
   Stage 0 C++.
2. Any mutation performed by the self-hosted slice needs the mutation-
   provenance fields and recovery disposition defined by contract.
3. Any asynchronous or parallel feature must remain refused until cancellation,
   backpressure, effect, ordering, and commit semantics are admitted.
4. Any foreign provider must pass exact ABI/effect/ownership evidence; arbitrary
   FFI cannot be used as a bootstrap shortcut.
5. Any isolation or trust claim must identify its actual assurance level and
   environmental limitations.

## Inventory maintenance rule

New admitted behavior requires a row here before its implementation is called
safe. A row may move from future or provisional to implemented only when the
contract, enforcement point, positive tests, hostile tests, and residual-risk
statement all exist. A green test without a named hazard does not close a
safety case.
