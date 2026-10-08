# Architecture Notes

Project-wide architecture notes belong here.

## Document authority

Architecture documents distinguish:

- **Binding** architectural laws and approved boundaries that implementations
  must preserve.
- **Provisional** mechanisms, syntax, or engineering defaults that express an
  approved direction but still require focused design and evidence.
- **Exploratory** alternatives and research notes that are not decisions.
- **Historical/current-state** evidence that records what was observed at a
  particular checkpoint rather than permanent law.

A document may contain more than one class, but it must label the boundary
between them. Stated intent is preserved; an implementation sketch does not
become binding merely by appearing beside a binding law.

Current intended compiler direction:

```text
source files
  -> import/source loading
  -> lexer
  -> TokenTree
  -> parser / AST builder
  -> complete raw AST
  -> structural SymbolTable projection
  -> semantic analysis / facts / contracts / diagnostics
  -> Graph IR
  -> graph analysis / pruning / transformation / policy
  -> optimized Graph IR
  -> target lowering
  -> target IR
  -> emitter
  -> artifact/runtime
```

The important architectural boundaries are documented before their full
implementations exist so the current default path does not accidentally become
the permanent definition of Lyraform. Flowcore remains the historical project
name in dated architecture records.

Current active implementation is a verified Lyraform v0.29 typed artifact-contract
language-chain slice. It
inherits the closed v0.24 raw frontend/export border, establishes semantic
analysis and policy-binding consumers, preserves an optimization boundary, and
lowers the current supported contract surface to executable LLVM/ELF artifacts.
Its public stage artifacts are independently validated and identity-tested.
Canonical Graph IR, generalized optimizer transforms, executable safety
profiles, certification, and self-hosting remain future expansion work.

Planned and active architecture topics:

- pipeline boundaries
- TokenTree
- SymbolTable
- semantic AST
- semantic checker split
- Graph IR
- target-independent transformation and pruning
- policy-directed optimization
- revisioned IR and transformation provenance
- target lowering and emitters
- runtime backends
- bytecode
- effects and failure-flow

Foundational notes:

- [Frankencore Constitutional Baseline v0.1](FRANKENCORE-CONSTITUTION.md)
- [Frankencore contract inventory](frankencore-contract-inventory.json)
- [Current Frankencore conformance](frankencore-current-conformance.md)
- [Current v0.28 typed artifact-contract status](../checkpoints/2026-08-26-v0.28-typed-artifact-contracts.md)
- [Mutation-provenance contract](FRANKENCORE-MUTATION-PROVENANCE.md)
- [Repository audit](FRANKENCORE-AUDIT-2026-08-20.md)
- [ADR-0001: Mutation event envelope](decisions/0001-mutation-event-envelope.md)
- [ADR-0002: Provenance metadata and history](decisions/0002-provenance-metadata-and-history.md)
- [ADR-0003: Error-state lifecycle events](decisions/0003-error-state-lifecycle-events.md)
- [ADR-0004: Capability discovery without promotion](decisions/0004-capability-discovery-without-promotion.md)
- [ADR-0005: Policy authority and disclosure](decisions/0005-policy-authority-and-disclosure.md)
- [ADR-0006: ConfigResolve as policy substrate](decisions/0006-configresolve-as-policy-substrate.md)
- [ADR-0007: Policy-resolution provider contract](decisions/0007-policy-resolution-provider-contract.md)
- [ADR-0008: Language-neutral policy contract](decisions/0008-language-neutral-policy-contract.md)
- [ADR-0009: User sovereignty and liberal defaults](decisions/0009-user-sovereignty-and-liberal-defaults.md)
- [ADR-0010: Operating profile and conformance](decisions/0010-operating-profile-and-conformance.md)
- [ADR-0011: Signed profile declarations](decisions/0011-signed-profile-declarations.md)
- [ADR-0012: Public-key profile trust](decisions/0012-public-key-profile-trust.md)
- [ADR-0013: RSA profile signatures](decisions/0013-rsa-profile-signatures.md)
- [ADR-0014: Acquisition provenance and global trust](decisions/0014-acquisition-provenance-and-global-trust.md)
- [ADR-0015: Delegation and system coherence](decisions/0015-delegation-and-system-coherence.md)
- [ADR-0016: Provider-neutral layered isolation](decisions/0016-provider-neutral-isolation.md)
- [ADR-0017: Trust and isolation admission](decisions/0017-trust-and-isolation-admission.md)
- [ADR-0018: Contextual and earned trust](decisions/0018-contextual-earned-trust.md)
- [ADR-0019: Scoped trust bootstrap](decisions/0019-scoped-trust-bootstrap.md)
- [ADR-0020: ConfigResolve trust arbitration](decisions/0020-configresolve-trust-arbitration.md)
- [ADR-0021: Minimal trust-store schema](decisions/0021-minimal-trust-store-schema.md)
- [ADR-0022: Trust discovery and authority negotiation](decisions/0022-trust-discovery-and-negotiation.md)
- [ADR-0023: Authority-question capability](decisions/0023-authority-question-capability.md)
- [ADR-0024: Secret-bearing questions](decisions/0024-secret-bearing-questions.md)
- [ADR-0025: Trusted intermediary broker](decisions/0025-trusted-intermediary-broker.md)
- [ADR-0026: Adaptive broker assurance](decisions/0026-adaptive-broker-assurance.md)
- [ADR-0027: Operation risk and assurance selection](decisions/0027-operation-risk-and-assurance-selection.md)
- [ADR-0028: Provider risk and renewed trust](decisions/0028-provider-risk-and-renewed-trust.md)
- [ADR-0029: Versioned substrate adaptation](decisions/0029-versioned-substrate-adaptation.md)
- [ADR-0030: Privileged substrate-change observation](decisions/0030-privileged-change-observation.md)
- [ADR-0031: Practical assurance boundary](decisions/0031-practical-assurance-boundary.md)
- [ADR-0032: Official release trust bootstrap](decisions/0032-official-release-trust-bootstrap.md)
- [ADR-0033: Substrate verification adapter contract](decisions/0033-substrate-verification-adapter-contract.md)
- [ADR-0034: Owner-controlled attestation](decisions/0034-owner-controlled-attestation.md)
- [ADR-0035: Normalized verification evidence](decisions/0035-normalized-verification-evidence.md)
- [ADR-0036: Canonical package data over native package substrates](decisions/0036-substrate-package-model.md)
- [ADR-0037: Native verification delegation](decisions/0037-native-verification-delegation.md)
- [ADR-0038: Transparent substrate replacement](decisions/0038-transparent-substrate-replacement.md)
- [ADR-0039: Compatible command with explicit policy control](decisions/0039-compatible-command-with-policy-control.md)
- [ADR-0040: Executable substrate facade pattern](decisions/0040-executable-substrate-facade.md)
- [ADR-0041: Localized capability monikers](decisions/0041-localized-capability-monikers.md)
- [ADR-0042: Reversible localized script projection](decisions/0042-reversible-localized-script-projection.md)
- [ADR-0043: Language-neutral canonical authoring](decisions/0043-language-neutral-canonical-authoring.md)
- [ADR-0044: Language dialect and profile selection](decisions/0044-language-dialect-and-profile-selection.md)
- [ADR-0045: Inspectable language-map manifest](decisions/0045-language-map-manifest.md)
- [ADR-0046: Declarative language-chain build policy](decisions/0046-declarative-chain-build-policy.md)
- [ADR-0047: Mechanically resolvable laws and philosophies](decisions/0047-mechanically-resolvable-laws.md)
- [ADR-0048: Substrate-wide facade forest](decisions/0048-substrate-wide-facade-forest.md)
- [ADR-0049: Runtime capability discovery](decisions/0049-runtime-capability-discovery.md)
- [ADR-0050: Runtime specialization and JIT](decisions/0050-runtime-specialization-and-jit.md)
- [ADR-0051: Runtime provider decisions require facts and calibration](decisions/0051-runtime-provider-decision.md)
- [ADR-0052: Ordinary aggregates are independent values](decisions/0052-independent-aggregate-values.md)
- [ADR-0053: Named guards use lifecycle-statement syntax](decisions/0053-named-guard-lifecycle-syntax.md)
- [ADR-0054: Failure is explicit flow](decisions/0054-failure-is-explicit-flow.md)
- [ADR-0055: Refusal, failure, and fault are distinct dispositions](decisions/0055-refusal-failure-fault-dispositions.md)
- [ADR-0056: No dangling failure or fault wires](decisions/0056-no-dangling-failure-or-fault-wires.md)
- [ADR-0057: Diagnostics explain from canonical evidence](decisions/0057-explanatory-diagnostics-from-canonical-evidence.md)
- [ADR-0058: One canonical disposition fact, many projections](decisions/0058-canonical-disposition-fact-projections.md)
- [ADR-0059: Tagged outcomes transfer a must-account obligation](decisions/0059-tagged-outcome-must-account.md)
- [ADR-0060: Uniform transfer of owned obligations](decisions/0060-uniform-owned-obligation-transfer.md)
- [ADR-0061: Legacy implementation is deprecated and oracle-only](decisions/0061-legacy-is-deprecated-oracle-only.md)
- [ADR-0062: Typed failure consumers with policy-selected routing](decisions/0062-typed-failure-consumers-and-policy-routing.md)
- [ADR-0063: Disposition evidence uses bounded epochs](decisions/0063-bounded-disposition-evidence-epochs.md)
- [ADR-0064: Parallel opportunity is derived from the graph](decisions/0064-graph-derived-parallelism-and-policy-scheduling.md)
- [ADR-0065: Fault containment halts and quarantines the exact scope](decisions/0065-fault-containment-halts-and-quarantines-scope.md)
- [Failure-flow semantic--syntax--graph projection map](failure-flow-semantic-syntax-graph-map.tex)
- [Canonical executable failure flow — Mission 01](../tasks/canonical-executable-failure-flow-mission-01.md)
- [Canonical executable failure flow — Mission 02](../tasks/canonical-executable-failure-flow-mission-02.md)
- [Canonical executable failure flow — Mission 03](../tasks/canonical-executable-failure-flow-mission-03.md)
- [Canonical executable failure flow — Mission 04](../tasks/canonical-executable-failure-flow-mission-04.md)
- [Canonical fault containment — Mission 05](../tasks/canonical-fault-containment-mission-05.md)
- [Canonical response fault containment — Mission 06](../tasks/canonical-response-fault-containment-mission-06.md)
- [Canonical language authority map v1](canonical-language-authority-map-v1.json)
- [Frankencore implementation roadmap](FRANKENCORE-IMPLEMENTATION-ROADMAP.md)
- [Frankencore hardening checkpoint — 2026-08-20](../checkpoints/2026-08-20-frankencore-hardening-status.md)
- [Project-local status metadata schema v1](schemas/frankencore-project-statuses-v1.json)
- [Project-local capability catalog schema v1](schemas/frankencore-capability-catalog-v1.json)
- [Flowcore core promise](flowcore-core-promise.md)
- [Transformation and revision architecture](compiler-transformation-revision-model.md)
- [Revisioned node identity](revisioned-node-identity.md)
- [Prerequisites](prerequisites.md)
- [FrankenCore conformance declaration](frankencore-conformance.md)

`flowcore-core-promise.md` is binding foundational intent.
`compiler-transformation-revision-model.md` contains binding stage laws and
provisional implementation mechanisms. `prerequisites.md` contains a binding
environment-contract rule with provisional syntax and taxonomy.

Scheduling follows ADR 0064: canonical graph and effect facts establish
dependencies and conflicts; policy and measured runtime feasibility select an
execution strategy only within that proved freedom. Parallelism and
asynchronous dispatch are not source-language features.

## Executable facade profiles

- [`ls` / GNU coreutils 9.4](facades/ls-coreutils-9.4.md)

## Flowmini and AstLib boundary

Flowmini keeps its dedicated, typed AST and parser. AstLib is an independent,
language-neutral capability intended to mature through other language
experiments. Flowmini is not to be rewritten around AstLib merely to unify tree
storage. A future narrow adapter may be considered after AstLib matures and a
measurable benefit is demonstrated; it must preserve Flowmini's typed semantic
nodes and ownership laws.
