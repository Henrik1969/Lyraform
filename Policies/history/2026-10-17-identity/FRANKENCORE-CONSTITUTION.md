# Frankencore Constitutional Baseline v0.1

**Status:** proposed constitutional baseline  
**Scope:** Frankencore, the constitutional core and substrate-mediation layer

This document defines the first small set of laws against which Frankencore
components can be audited. It does not require a universal runtime library,
object hierarchy, persistence mechanism, or Flowcore dependency.

## Boundaries

- **FrankenPOP!** is the complete ecosystem and umbrella architecture.
- **Frankencore** is the constitutional core and substrate-mediation layer.
- **Flowcore** is an optional language, orchestration, transformation, and
  execution architecture within FrankenPOP!.
- A **substrate** is an external mechanism exposed through an adapter or
  provider boundary.

## Constitutional invariants

### FC-I01 — Identity precedes representation

An entity is not defined by its CLI, TUI, GUI, JSON, filesystem, Flowcore, or
network projection.

### FC-I02 — Capabilities are discoverable

A consumer must be able to query a claimed capability without relying on
trial-and-error invocation.

### FC-I03 — Capability claims are truthful

A claimed capability must have a satisfiable contract, including its declared
parameters, limits, version, and failure behavior.

### FC-I04 — Policy and mechanism are separate

Mechanism describes what can be done. Policy decides what should, may, must,
or must not be selected.

### FC-I05 — Semantics are presentation-independent

Removing a presentation surface must not destroy canonical domain semantics.

### FC-I06 — Substrate details terminate at boundaries

Linux, GUI, kernel, library, and device specifics may exist in providers or
adapters, but must not leak upward as the core semantic definition.

### FC-I07 — Mutation does not necessarily destroy identity

Where appropriate, an entity may retain identity while its published state
advances through explicit revisions.

### FC-I08 — Uniform semantics do not require common inheritance

Frankencore must not require every entity to inherit from a universal base
object or carry an untyped property/capability map.

### FC-I09 — Capability does not imply authority

What an entity can do and what a caller is allowed to request are separate
questions and must remain separately representable.

### FC-I10 — Flowcore is optional to Frankencore

Frankencore contracts must remain usable by components that do not embed or
depend on the Flowcore parser, AST, runtime, or execution engine.

### FC-I11 — Public contracts are versionable

Every public semantic contract has an explicit identity, version, and
compatibility rule.

### FC-I12 — Decisions are explainable where practical

Provider and policy decisions should retain enough provenance to explain what
was selected, which alternatives were considered, and why.

### FC-I13 — Provider failure has defined semantics

Provider disappearance, rejection, degradation, and replacement must not
produce undefined semantic behavior.

### FC-I14 — Projections do not own canonical state

A projection may cache or adapt state, but it is not the canonical owner unless
that ownership is explicitly declared by contract.

### FC-I15 — Dependency direction is enforceable

Core semantics must not acquire arbitrary dependencies on presentation,
application, parser, or substrate-specific implementation layers.

## Canonical vocabulary

Capability, provider, backend, adapter, policy, type, identity, property,
action, signal/event, relationship, projection, lifecycle, and result/error
have the meanings defined in the hardening plan. A provider is not a backend;
a capability is not authority; and a projection is not identity.

## Change rule

An architectural change that violates an invariant must either be rejected or
accompanied by an explicit constitutional revision. Experimental ideas remain
marked experimental until promoted deliberately.
