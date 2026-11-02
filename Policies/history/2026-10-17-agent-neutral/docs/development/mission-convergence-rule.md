# Lyraform standing mission convergence rule

Date: 2026-10-10

Intended repository path: `docs/development/mission-convergence-rule.md`

## Purpose and authority

Every future Lyraform compiler, language, contract, graph, policy, backend, or
runtime mission must move the implementation toward one coherent, usable
language model. Apply this rule alongside `AGENTS.md`, the Stage Execution
Protocol, accepted ADRs, and the explicitly selected mission. This rule
constrains how work is framed and proven; it grants no commit, push, merge,
syntax, policy, or semantic authority by itself.

The five standing requirements are **vertical slice, canonical law,
canonization, simplification and reuse, and consolidation**. Every mission
must state how each applies. A mission may explicitly narrow a requirement
for a prerequisite stage, but must explain the dependency and the exact
maturity level it can prove. Silence does not create an exception.

## 1. Prefer a vertical slice

Choose a small behavior that begins in actual Lyraform source or an explicitly
identified external input and reaches its admitted observable boundary.
Trace the same semantic fact through each stage it traverses: syntax, semantic
authority, typed graph, contracts, policy selection if applicable, lowering,
backend, runtime, and diagnostic or result. Test the successful path and a
nearby invalid or unsuccessful path. A source example, clean checkout command,
and expected observations belong in the gate evidence.

Some prerequisite missions necessarily end at parsing, semantic analysis, or
a declarative bridge. Such a mission must name the dependent executable slice,
prove its own bounded boundary, label every later step unsupported, and stop at
that boundary. A reference executor, hand-authored plan, or fixture can prove
its own contract but cannot substitute for a source-to-runtime slice.

## 2. Check every change against canonical law

Before implementation, list the accepted ADRs and invariants the slice relies
on, and assign one producer of each semantic fact. For each proposed change,
show whether it preserves, implements, refines, or conflicts with an accepted
law. An implementation accident, legacy behavior, passing test, or generated
document does not override that law. If accepted records conflict, identify
the exact contradiction and stop dependent work for a decision.

During validation, mutate the facts most likely to be mistaken for one
another: type, owner, graph/wire identity, producer operation, policy
selection, source provenance, attempt, signal, delivery, disposition,
obligation, scope, and commit state as applicable. Require refusal at the
earliest authoritative boundary. Keep a trace from accepted law to producer,
consumer, test, and observable result. Do not multiply semantic authorities
between compiler stages.

## 3. Identify what remains to be canonized

Maintain a short decision register with four distinct labels:

- **Accepted meaning:** exact ADR or other human-approved authority.
- **Implemented coverage:** exact source forms and execution boundaries proved.
- **Proposed meaning:** alternatives and consequences awaiting a decision.
- **Undefined or unsupported:** the point where the current pipeline refuses.

When implementation requires choosing language or architecture meaning that
has not been accepted, write a decision brief with alternatives, migration
effects, recommendation, and the precise choice for Henrik. Continue only
independent work. Do not create a public spelling, failure route, ownership
rule, policy power, or runtime continuation by inference. Once a decision is
accepted, record the authority before implementing it and update the register.

## 4. Simplify, reuse, and generalize deliberately

Before adding a schema, adapter, emitter, test harness, or reference layer,
identify the existing component that might express the same fact. Prefer one
authority and several validators over repeated derivations. Reuse mechanisms
only when their semantics and identities actually match; superficial naming
similarity is insufficient. Generalize after two independently meaningful
uses establish the common rule, not by predicting hypothetical future cases.

At every gate, ask what code, compatibility branch, special case, duplicate
contract, stale document, or transitional fixture can now be deleted. Compare
the simplest viable implementation with the proposed one and record why any
additional layer earns its maintenance cost. Avoid profile selectors, parallel
schemas for the same authority, and growing test suites that merely restate
their own implementation. Preserve historical evidence without keeping
obsolete mechanisms executable.

## 5. Consolidate before expanding

At each gate, reconcile the branch's current truth: source forms, contract
formats, consumers, refusals, tests, status pages, mission state and known
unsupported paths. Keep published branch history and working-tree status
clear. Reproduce the relevant command from a clean checkout or record the
exact environmental reason it could not be reproduced. Use independent or
hostile evidence that is capable of falsifying the claim; a test written from
the same assumption as the code is insufficient on its own.

Do not start another adjacent feature while the prior slice has an
unexamined integration gap. If a feature has acquired more reference models,
policy files, or documentation than executable paths, prioritize connecting
or retiring those artifacts. Report capability maturity honestly: recognized,
structurally validated, semantically resolved, admitted, preserved, executable,
and optimized are different claims.

## Required mission and gate record

Every new mission must include a compact **convergence contract**:

1. The user-visible behavior or bounded prerequisite and the exact end point.
2. The accepted law and one owner for each new semantic fact.
3. The unresolved decisions that may require Henrik, with stop conditions.
4. Existing components to reuse and transitional components to retire.
5. A positive example, a negative example, and a hostile mutation.
6. The clean-checkout command or reproducible artifact and the claimed
   maturity level.
7. The specific evidence that would falsify the implementation's claim.
8. The consolidation work required before the next slice.

Each gate report answers: **What became usable? What remains declarative or
unsupported? What became simpler or was removed? Which accepted law was
checked, and what evidence could have proved us wrong?** Report exact facts
and counts, not adjectives or commit volume. A gate may pass as a bounded
prerequisite while its dependent executable slice remains open; state that
relationship explicitly.

## Repository wiring

To make this rule durable, add this sentence beside the Stage Execution
Protocol link in root `AGENTS.md`:

> Every explicitly selected maturation mission follows the [Mission
> Convergence Rule](docs/development/mission-convergence-rule.md): prioritize
> vertical slices, check accepted law, identify missing canon, simplify and
> reuse, and consolidate at each gate. A prerequisite stage must name its
> executable dependency and state its narrower maturity claim.

The link makes the rule part of the normal Codex instruction chain. An
explicitly selected mission still controls its own Git authority and exact
definition of done.
