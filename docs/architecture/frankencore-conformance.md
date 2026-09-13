---
title: Flowcore FrankenCore Conformance Declaration
status: historical-checkpoint
kind: architecture-declaration
verified: 2026-08-16
repository-revision: 2927c981fa029433952ca699d567a5986771eca5
---

# Flowcore FrankenCore conformance declaration

This declaration records the observed architecture of the canonical Flowcore
repository at the revision above. It is retained as historical evidence; the
current implementation summary is
`docs/checkpoints/2026-08-19-language-chain-status.md`.
It follows the canonical FrankenCore
architectural laws maintained in the private sibling repository at
`/home/henrik/Projekter/Udvikling/FrankenCore`. It does not copy or replace
that authority.

Flowcore is a work in progress. This declaration distinguishes implemented
facts from binding direction, provisional mechanisms, compatibility paths, and
experiments. It is not a production-readiness claim.

## Project identity

- Canonical project name: Flowcore.
- Repository root: `/home/henrik/Projekter/Udvikling/Flowcore`.
- Canonical remote: `git@github.com:Henrik1969/Flowcore.git`.
- Active branch at verification: `v25-symboltable-projection`.
- Active implementation: `Lyraform/flowmini_v25_symboltable_projection/`.
- Purpose: explore a language and system architecture in which source-level
  computation becomes typed, contract-governed, compiler-visible graph
  structure.
- Current maturity: experimental; not production-ready.
- Accountable maintainer: Henrik Sørensen (`Henrik1969`).
- Governing project documents: `docs/architecture/flowcore-core-promise.md`,
  `docs/architecture/compiler-transformation-revision-model.md`, and
  `docs/development/verification-gates.md`.

## Scope of the present implementation

Flowmini is the executable prototype used to test Flowcore ideas. The current
v0.25 line matures this implemented flow:

```text
Flowmini source
  -> lexer and TokenTree
  -> parser and raw AST
  -> factual SymbolTable projection
  -> versioned frontend bundle
  -> independent reference consumer and validation gates
```

The following are not implemented v0.25 authority:

- semantic type resolution and alias normalization;
- identifier legality, overload choice, mutability, or assignability analysis;
- contract or refined-type satisfaction;
- canonical Graph IR construction and graph optimization;
- revisioned runtime state and mutation provenance;
- general ABI lowering;
- replacement of the current ModuleSpec/FlowIR runtime path.

## Authority and responsibility

### Owned facts and contracts

Flowcore owns its language and architecture definitions, accepted source forms,
AST schema, structural SymbolTable projection rules, frontend bundle contract,
diagnostic behavior, and the gates used to support its maturity claims.

For the active v0.25 boundary:

- source text supplied by a caller is authoritative input, not a fact owned by
  Flowcore;
- the raw AST is authoritative for parsed source structure and spelling within
  one exported frontend revision;
- the SymbolTable projection is authoritative only for factual structural
  symbol and scope metadata derived from that AST;
- structural-origin entries identify the AST provenance of projected facts;
- the frontend JSON bundle is a versioned exposure contract containing those
  facts, not an independent source of truth;
- consumer-generated lowering skeletons and reverse-origin indexes are
  projections and have no authority over the embedded AST or symbol facts.

Numeric AST, symbol, and scope IDs are stable only within one exported revision.
Cross-edit persistent identity is not currently claimed.

### Transformations

The active frontend tokenizes and parses source, builds an arena-owned AST,
projects factual symbol/scope metadata, and serializes a deterministic bundle.
The independent consumer validates the bundle and derives a lowering skeleton
and reverse-origin index without linking Flowmini internals or reparsing source.

Future semantic interpretation, Graph IR transformation, policy-directed
selection, target lowering, and emission are architectural direction described
in project documentation. They are not present v0.25 transformations.

### Exposure surfaces

Implemented exposure includes the `flowmini` command, diagnostics, AST and
SymbolTable dumps, the `flowmini.frontend_bundle` version 2 JSON contract, test
targets, and documented CMake workflows. These surfaces expose facts or
behavior; they do not own the source or canonical compiler model.

### Explicitly external responsibility

Flowcore does not own FrankenCore governance, consumer implementations,
provider-controlled shared libraries, target platform behavior, user source
files, or future independently implemented semantic and tooling providers.
Dependency and consumption do not transfer ownership.

## Capability and provider boundaries

| Capability | Direction | Provider or consumer | Contract | Ownership boundary | Failure behavior |
| --- | --- | --- | --- | --- | --- |
| Token structure | consumed internally | TokenTree static library | C++ library interface | TokenTree owns its implementation; Flowmini owns its use in the frontend | build failure or diagnostic |
| Symbol storage/projection | consumed internally | SymbolTable static library plus Flowmini projection | C++ API and `symboltable.snapshot` v1 | SymbolTable owns generic storage; Flowmini owns projection rules and origins | build failure, diagnostic, or gate failure |
| Frontend facts | provided | independent tools and reference consumer | `flowmini.frontend_bundle` v2 | Flowmini owns schema and produced facts; consumers own their derived projections | unsupported or malformed contracts are rejected |
| ABI test provider | consumed by compatibility runtime | `flowmini_testabi` | focused C ABI and legacy source-tree shared-library path | provider owns compiled library; Flowmini owns test expectation | preparation, load, symbol, or diagnostic failure |
| Runtime stages | provided by prototype | built-in stage registry | current ModuleSpec/FlowIR runtime conventions | Flowmini owns prototype behavior; this is not canonical future Graph IR | diagnostic failure |

Direct linking to internal frontend implementation is not part of the exported
frontend contract. Independent consumers are expected to consume the versioned
bundle.

## Boundary contracts and evidence

### Source to raw AST

- Input authority: caller-provided source and imported source units.
- Handling: lexing, TokenTree construction, parsing, and AST construction.
- Output authority: `flowmini.ast.v2` for the parsed revision.
- Evidence: 26 AST golden tests and categorized accepted/rejected examples.
- Current limit: the gate proves tested structural output, not semantic
  completeness.

### Raw AST to SymbolTable projection

- Input authority: raw AST facts.
- Handling: factual projection without type resolution or legality inference.
- Output authority: projected structural symbols, scopes, and typed origins.
- Policy: source structure and spelling remain AST responsibility; semantic
  interpretation is deferred.
- Evidence: 12 SymbolTable projection tests and the recorded origin maturity
  audit.

### Frontend bundle to independent consumer

- Input authority: embedded AST, SymbolTable snapshot, source map, and typed
  structural origins.
- Contract: `flowmini.frontend_bundle` version 2 with explicit rejection of
  unsupported or malformed versions and origins.
- Output: non-authoritative lowering skeleton and reverse-origin projection.
- Evidence: seven golden bundles, one isolated-consumer test, and nineteen
  negative attacks.
- Readiness gate: Tier 2 for the current integration baseline; recorded Tier 3
  Firetest evidence is required before a greater-border closure claim.

## Mutation and provenance

### Implemented structural provenance

The v0.25 frontend records source filenames, expanded-source locations, source
maps, typed AST paths, structural roles, and matching AST IDs where the arena
owns an ID. Independent validation attacks missing, duplicated, mismatched, and
wrong-kind origins.

The exported frontend bundle is treated as one published revision. The
projection and consumer do not mutate the embedded AST facts.

### Incomplete mutation provenance

The compatibility runtime mutates payload records, lists, arrays, fields, and
stage outputs. Current mutation helpers use stage names for diagnostics, but
successful mutations do not retain a complete provenance record containing:

- before and after revision identity;
- accountable actor or provider identity;
- the policy authorizing the mutation;
- durable evidence linking the two states;
- a general recovery or rollback contract.

This is architectural debt, not a v0.25 frontend-border violation. Revisioned
identity and non-destructive mutation are binding direction, while their exact
mechanism remains provisional and unimplemented.

## Compatibility and adapters

| Path or surface | Classification | Canonical replacement or model | Consumers | Retirement condition |
| --- | --- | --- | --- | --- |
| ModuleSpec/FlowIR runtime representation | named compatibility execution target | future canonical Graph IR and explicit lowering boundary | current Flowmini runtime | a governed runtime/backend consumes Graph IR or a later lowered representation |
| `build/libflowmini_testabi.so` | legacy test-provider path bridge | declared prerequisite/provider resolution | ABI examples and tests | consumers resolve the provider through a stable declared contract |
| packed verified integer aggregate ABI carrier path | bounded compatibility probe with generic layout identity | future general ABI type and call mechanism | ABI test path | general ABI lowering is specified and gated |
| `Lyraform/flowmini_v24_explicit_ast/` | frozen historical implementation checkpoint | active v0.25 line | maintainers and historical comparison | preservation remains intentional; no deletion condition declared |
| root superbuild references to `Handwritten_V1`, `flowcheck`, and `flowoptimize` | legacy build configuration | v0.25 directory CMake scope | historical/root workflows | explicit future root-build decision |

Compatibility paths remain named and must not define the future canonical
model accidentally.

## State and lifecycle

| Path or artifact | Class | Authoritative? | Reproducible? | Retention or recovery policy |
| --- | --- | --- | --- | --- |
| tracked source, headers, schemas, tests, goldens, and architecture documents | source and governed evidence | yes, within their declared scope | from Git history | retain in repository history |
| `Lyraform/flowmini_v25_symboltable_projection/cmake-build-debug/` and other build trees | build output | no | yes | local and ignored; recreate through CMake |
| generated runtime/test files under active build or report paths | scratch/build output | no | normally yes | local and ignored unless promoted as governed evidence |
| recorded Firetest reports in `docs/flowmini/` | test or checkpoint evidence | authoritative for the recorded run only | partly | retain with tested revision and limitations |
| release tags and release-associated reports | release evidence | yes, for repository provenance | Git-reproducible metadata; environment only partly reproducible | durable repository history |
| `_archive/` and preserved older Flowmini stages | preservation history | historical, not current implementation authority | not assumed | retain intentionally; review before any lifecycle change |
| independent-consumer lowering skeleton and reverse-origin index | generated projection | no | yes from a valid bundle and consumer version | regenerate; retain only when needed as test evidence |

Old, generated, or downloadable material is not presumed disposable without a
separate lifecycle decision.

## Policy, evidence, and readiness

- Transformation legality, optimization policy, and implementation provider
  are architecturally separate responsibilities.
- Tier 1 gates cover focused patches; Tier 2 covers integration checkpoints;
  Tier 3 Firetest is required for greater-border closure and public release
  claims.
- The current recorded Tier 2 baseline is: normal build; 26/26 AST goldens;
  12/12 SymbolTable projection tests; seven frontend-bundle goldens, one
  isolated run, and nineteen negative attacks; 78/78 categorized suite; and
  2/2 CTest tests.
- Previous Firetest evidence remains evidence for its recorded revision, not a
  permanent property of the active branch.
- Flowmini v0.25 does not claim semantic completeness, canonical Graph IR,
  runtime provenance maturity, production readiness, or security-critical
  suitability.

## Security and privacy boundary

Flowcore source, tests, and generated frontend structures are not credential
stores. No secret acquisition contract is defined for the active frontend.
Imported or caller-provided source may contain sensitive material, so dumps,
diagnostics, bundles, reports, and test captures must be treated according to
the sensitivity of their inputs. Credential values must not be committed as
examples, goldens, diagnostics, or declaration content.

The prototype may load explicitly named ABI libraries and may read input or
write process output through runtime stages. Those capabilities are current
prototype mechanisms, not evidence of a complete sandbox, least-authority
model, or production security boundary.

## Current conformance assessment

### Strong present conformance

- Projection is separated from authority at the v0.25 frontend boundary.
- AST, structural projection, bundle exposure, and independent consumer have
  explicit responsibilities.
- Cross-process facts use versioned contracts and fail closed under tested
  malformed inputs.
- Structural projection carries explicit and independently checked provenance.
- Compatibility paths are named rather than presented as the future canonical
  model.
- Readiness claims are tiered and tied to recorded evidence.

### Partial conformance and debt

- Runtime mutations lack accountable revisions, complete actor/policy
  attribution, durable mutation evidence, and general rollback.
- Provider selection and policy enforcement are architectural direction more
  than generalized implemented interfaces.
- The ABI provider bridge couples tests to a source-tree output path and cannot
  safely prepare multiple build configurations concurrently.
- Security and capability isolation are not mature enough for production use.

### Intentional experiments

Flowmini versions, TokenTree, SymbolTable, `Pattern_explored/`, architecture
notes, and archived stages may represent different maturity levels. Their
experimental or historical status does not make them disposable or canonical
by accident.

### Decisions still requiring architectural authority

- the canonical Graph IR schema and identity model;
- the first implemented revision/provenance boundary;
- policy and provider-selection contracts;
- runtime replacement or continued compatibility strategy;
- general prerequisite and ABI-provider resolution;
- retention rules for generated evidence beyond existing recorded reports.

### Next smallest reversible improvement

Define the runtime mutation-provenance contract before changing runtime storage
mechanisms. The contract should state required identity, actor, policy,
before/after evidence, atomicity, and rollback semantics without prematurely
selecting persistence machinery.

## Declaration provenance

- Declaration path: `docs/architecture/frankencore-conformance.md`.
- Declared by: project maintainer with Codex-assisted repository audit.
- Based on repository revision:
  `2927c981fa029433952ca699d567a5986771eca5`.
- Last verified: 2026-08-16.
- Verification method: documentation, build definitions, source-boundary
  inspection, test contracts, Git identity, and the previously passed v0.25
  integration gates.

This declaration is governed documentation. It does not replace executable
tests, recorded Firetest reports, repository history, or runtime evidence.
