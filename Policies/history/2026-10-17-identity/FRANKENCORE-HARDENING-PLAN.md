# Frankencore Hardening and Improvement Plan

**Status:** Proposed architectural baseline  
**Audience:** Codex, maintainers, contributors, architecture review  
**Purpose:** Convert the current Frankencore/FrankenPOP architecture into enforceable contracts, invariants, and implementation gates.

---

## 1. Executive Summary

Frankencore has reached the point where its main risk is no longer lack of ideas. The project now has a coherent architectural philosophy, but too much of that philosophy still exists as discussion rather than enforceable law.

The next phase should therefore prioritize **hardening over feature expansion**.

The immediate goal is to make it possible to answer mechanically:

> Is this component Frankencore-conformant?

Frankencore should move from a collection of compatible architectural ideas to a system with:

- canonical terminology;
- explicit invariants;
- directional dependency rules;
- a minimal semantic meta-model;
- a capability model;
- lifecycle and error semantics;
- introspection;
- conformance testing;
- substrate isolation;
- reference implementations.

The preferred milestone name is:

> **Frankencore Constitutional Baseline v0.1**

Until that baseline exists, experimental development may continue, but new architectural ideas should not silently become canonical.

---

## 2. Canonical Project Boundaries

The following distinction is architectural law unless explicitly revised.

### FrankenPOP!

FrankenPOP! is the umbrella architecture and ecosystem.

It includes:

- Frankencore;
- Flowcore;
- presentation systems;
- capability providers;
- adapters;
- tools;
- libraries;
- services;
- user-facing applications;
- future system components.

### Frankencore

Frankencore is the **constitutional core and substrate mediation layer**.

Its responsibilities include:

- semantic contracts;
- capabilities;
- provider contracts;
- policy/mechanism separation;
- identity;
- lifecycle;
- introspection;
- substrate abstraction;
- conformance laws.

Frankencore must not become synonymous with the entire operating environment.

### Flowcore

Flowcore is a language, orchestration, transformation, and execution architecture that lives within FrankenPOP!.

Flowcore may consume Frankencore contracts.

Frankencore's fundamental contracts must **not require the Flowcore language or runtime**.

This is a critical dependency rule.

---

## 3. Architectural Thesis

Frankencore follows this model:

> Everything interesting has semantic identity and a type. Types may expose properties, relationships, capabilities, actions, and observable changes.

However:

> Uniform semantics do not require uniform representation, storage, inheritance, or implementation.

A process, monitor, filesystem resource, network endpoint, device, widget, or Flowcore node may participate in the same semantic model without inheriting from a universal `FrankencoreObject`.

Avoid creating a universal object hierarchy containing arbitrary property maps and dynamically typed values.

Prefer:

- contracts;
- protocols;
- traits;
- interfaces;
- typed composition;
- adapters.

Representation must not define identity.

A semantic object may be represented as:

- CLI output;
- TUI content;
- GUI content;
- JSON;
- a filesystem-like projection;
- a Flowcore node;
- a network resource;
- a database record.

Those are projections, not the object itself.

---

## 4. Canonical Vocabulary

The following terms should be standardized before further architectural expansion.

| Term | Canonical Meaning |
|---|---|
| FrankenPOP! | Entire architecture/ecosystem |
| Frankencore | Constitutional core and substrate mediation layer |
| Flowcore | Typed language/orchestration/execution architecture |
| Substrate | External mechanism Frankencore builds upon |
| Capability | Something an entity can truthfully perform |
| Provider | Implementation satisfying a capability contract |
| Backend | Lower-level mechanism used by a provider |
| Adapter | Translation between incompatible contracts or substrate semantics |
| Policy | Rules selecting, preferring, allowing, or denying mechanisms |
| Type | Semantic classification and contractual structure |
| Identity | Stable semantic identity of an entity |
| Property | Observable characteristic or state |
| Action | Invocable semantic behavior |
| Signal/Event | Observable occurrence or state transition |
| Relationship | Typed semantic connection between entities |
| Projection | Representation of semantic state or capabilities |
| Surface | Presentation capability boundary |
| Canvas | Drawable/composable semantic space |
| Workspace | User/task context containing presentation spaces |
| Viewport | View into a canvas or larger presentation space |
| Lifecycle | Rules governing existence, availability, replacement, and failure |
| Result/Error | Semantic description of success or failure |

### Provider is not backend

Example:

```text
Capability:
    TextRendering

Provider:
    PangoTextProvider

Possible backend mechanisms:
    FreeType
    HarfBuzz
    Cairo
```

This distinction must remain explicit.

---

## 5. Constitutional Invariants

The initial constitution should contain roughly 10–20 strong laws rather than a large descriptive specification.

The following are recommended starting invariants.

### FC-I01 — Semantic identity precedes representation

An entity is not defined by whether it appears as a file, widget, TUI row, Flowcore node, JSON structure, database record, or network resource.

Those are representations.

### FC-I02 — Capabilities are explicitly discoverable

Consumers must be able to query supported capabilities without trial-and-error execution.

### FC-I03 — Capability claims are truthful

If a provider or object claims support for a capability, the declared contract must actually be satisfiable.

### FC-I04 — Policy and mechanism are separate

A mechanism describes what can be done.

A policy describes what should, may, must, or must not be selected.

The two concepts must not be conflated.

### FC-I05 — Core semantics are presentation-independent

GUI, TUI, CLI, network, or other presentation layers may disappear without destroying domain semantics.

### FC-I06 — Substrate specifics terminate at adapter boundaries

Linux-specific details such as:

- `/proc`;
- `/sys`;
- `/dev`;
- ioctls;
- udev;
- D-Bus;
- systemd;
- Wayland;

must not progressively leak into higher semantic layers.

They may be exposed through typed semantic objects and explicit substrate-specific extensions.

### FC-I07 — Mutation does not necessarily destroy identity

Where appropriate, semantic identity may persist across revisions.

Example:

```text
NodeId 42 revision 7
NodeId 42 revision 8
```

History-preserving or revision-aware semantics should remain possible.

### FC-I08 — Uniform semantics do not imply common inheritance

No architectural requirement shall force all Frankencore entities into a universal base class.

### FC-I09 — Capability does not imply authority

These are separate questions:

1. Can the target perform operation X?
2. Is the caller allowed to request operation X?

Authorization must not be encoded merely as capability presence.

### FC-I10 — Flowcore is optional to Frankencore

Frankencore contracts must be usable by components that do not embed or depend upon Flowcore.

### FC-I11 — Public contracts are versionable

Public semantic contracts must have explicit identity and compatibility/version semantics.

### FC-I12 — Architectural decisions must be introspectable where practical

The system should be able to explain not only what provider or mechanism was selected, but why.

### FC-I13 — Provider failure must have defined semantics

Provider disappearance or failure must not produce undefined semantic behavior.

### FC-I14 — Projections do not own canonical domain state

A projection may cache or adapt state, but it must not become the sole semantic owner unless explicitly defined as such.

### FC-I15 — Dependency direction is enforceable

Core layers must not acquire arbitrary dependencies on presentation, application, parser, or substrate-specific implementation layers.

---

## 6. Dependency Law

The intended conceptual dependency direction is:

```text
FrankenPOP applications
        |
        v
     Flowcore
        |
        v
 capability APIs
        |
        v
   Frankencore
        |
   +----+----+
   |         |
 policy   providers
             |
             v
          adapters
             |
             v
          substrate
```

Not every component must traverse every layer.

However, dependency direction should generally move downward.

### Explicitly reject dependency cycles such as:

```text
Frankencore
    -> Flowcore parser
    -> GTK
    -> Frankencore
```

or:

```text
core
    -> Wayland
    -> application policy
```

unless those dependencies exist behind explicit adapter/provider boundaries.

Architecture erosion must be prevented mechanically where possible.

---

## 7. Minimal Semantic Meta-Model

Before building a large implementation library, define the minimum semantic model.

Recommended concepts:

```text
Type
Identity
Property
Capability
Action
Signal/Event
Relationship
Provider
Policy
Projection
Lifecycle
Result/Error
```

For every concept, document both:

1. what it is;
2. what it is explicitly not.

Negative definitions are important because they prevent conceptual overlap.

Example:

> A capability is not automatically permission.

> A projection is not identity.

> A provider is not necessarily a backend.

> A property is not necessarily mutable.

---

## 8. Capability Model

Capability semantics are currently the highest architectural coupling risk and should be hardened early.

A capability should eventually support more than a Boolean `supported`.

Potential model:

```text
CapabilityId
CapabilityVersion
Requirements
Parameters
Limits
Provider
Quality/Constraints
```

Recommended capability relations and operations:

```text
supports
requires
provides
prefers
conflicts
implies
fallback
version
```

Recommended runtime operations:

```text
discover
query
negotiate
invoke
observe
```

### Capability example

Instead of:

```text
supports_animation = true
```

prefer:

```text
Capability:
    Animation

Properties / constraints:
    max_fps
    timing_precision
    synchronization_support
```

Likewise:

```text
Capability:
    RasterImage

Limits:
    max_width
    max_height

Formats:
    PNG
    JPEG
```

### Capability composition

A consumer may eventually express requirements such as:

```text
TextRender >= 2
AND
Geometry.Continuous
AND
Pointer
AND
(Color.TrueColor OR Color.Indexed256)
```

Do not over-engineer this immediately, but preserve the design space for negotiation and composition.

---

## 9. Capability vs Quality vs Authority

These concepts must remain distinct.

### Capability

What operation is possible?

Example:

```text
FormatFilesystem
```

### Quality / constraint

What limits or characteristics apply?

Example:

```text
max_resolution
latency
max_fps
precision
supported_formats
```

### Authority

May the caller perform the operation?

Example:

```text
Device capability:
    FormatFilesystem = available

Caller authority:
    FormatFilesystem = denied
```

Do not collapse these into a single property model.

---

## 10. Lifecycle Semantics

Real systems are dynamic.

Devices disappear.

Providers crash.

Displays reconnect.

Processes terminate.

Network services vanish.

Capabilities change.

Therefore contracts must define lifecycle behavior.

At minimum, the semantic model must be able to describe:

```text
discovery
identity lifetime
availability
activation
degradation
failure
replacement
detachment
removal
recovery
```

A universal state enum is not necessarily required.

Different object types may have different lifecycle state machines.

What is required is a common framework for describing lifecycle contracts.

---

## 11. Error Semantics

Do not allow every subsystem to independently invent incompatible error behavior through:

```text
bool
errno
nullptr
exceptions
optional
status enums
```

Frankencore should define a common semantic failure model.

Conceptually:

```text
Result<T, Error>
```

An error should be able to describe:

```text
domain
code
cause
message
recoverability
context
```

Individual implementation languages do not need to use the exact same concrete type.

The semantic contract is what matters.

---

## 12. Introspection and Explainability

Observability should be an architectural property, not an afterthought.

A future CLI should be able to perform operations similar to:

```bash
fc inspect display:0
```

returning:

```text
type
identity
properties
capabilities
provider
backend
policy
relationships
state
revision
```

A particularly valuable operation is:

```bash
fc why display:0.renderer
```

Possible response:

```text
Selected provider:
    VulkanRenderer

Candidates:
    VulkanRenderer     accepted
    OpenGLRenderer     accepted
    SoftwareRenderer   accepted

Decision:
    policy graphics.performance
    preferred hardware acceleration
```

The architectural concept of **why** should be preserved.

Frankencore should aim to expose enough decision provenance that important provider/policy decisions can be explained.

---

## 13. Substrate Model

The Linux filesystem and pseudo-filesystems are substrate representations, not Frankencore semantics.

Example Linux sources:

```text
/dev/nvme0n1
/sys/block/nvme0n1
/proc/...
```

Frankencore may instead expose:

```text
Device
{
    id
    type = Storage
    properties
    relationships

    capabilities:
        Read
        Write
        Trim
        SMART
}
```

That semantic object may then have projections through:

```text
CLI
GUI
TUI
Flowcore
network
filesystem-like namespace
```

This intentionally differs from Plan 9's model.

Plan 9:

> Everything is a file.

Frankencore:

> Everything has semantic identity that may have multiple projections.

---

## 14. Display Architecture Hardening

The emerging display hierarchy is useful:

```text
display service
    |
    v
workspaces
    |
    v
canvases / surfaces
    |
    v
viewports
    |
    v
drawable entities
```

However, future work must preserve separation between:

```text
semantic object
layout object
scene object
render object
pixel output
```

For example, a semantic `Button` is not inherently:

```text
rectangle + text + mouse callback
```

It may instead expose:

```text
Button
    label
    enabled
    activation
```

Possible projections:

```text
GTK widget
terminal region
web component
voice command
remote API
```

Presentation mechanisms must not become the semantic definition.

---

## 15. Flowcore Integration Law

Flowcore and Frankencore are strongly complementary but must not collapse into each other.

Desired relationship:

```text
Frankencore object
       ^
       |
       v
Flowcore binding
```

Flowcore may provide powerful access to Frankencore graphs, for example:

```text
system.devices
    |> where(.type == Storage)
    |> where(.capabilities contains SMART)
    |> map(.temperature)
```

But this must remain a binding over semantic contracts.

Frankencore may not require the Flowcore parser, AST, runtime, or execution engine to exist.

---

## 16. Versioning

Contract versioning should begin before provider count becomes large.

Use contract identity/version concepts similar to:

```text
frankencore.display.surface/1
frankencore.input.pointer/2
frankencore.device.storage/1
```

These are contract versions, not project release versions.

Contracts should define:

```text
name
version
compatibility
```

Where appropriate, provider selection may negotiate compatible versions.

---

## 17. Reference Implementation Strategy

Do not begin architectural validation with the display stack.

Start with a deliberately simple semantic object.

Recommended first reference object:

> **Clock**

Possible model:

```text
Type:
    Clock

Properties:
    realtime
    monotonic
    resolution

Capabilities:
    Read
```

Linux adapter:

```text
clock_gettime()
```

CLI projection:

```bash
fc show clock:monotonic
```

Possible Flowcore projection:

```text
clock.monotonic.read()
```

This small reference object should exercise the full architecture:

```text
Linux substrate
    |
    v
adapter
    |
    v
semantic object
    |
    v
capabilities
    |
    v
policy
    |
    v
projection / CLI
```

The purpose is not clock functionality.

The purpose is to expose architectural contradictions at low implementation cost.

---

## 18. Conformance Harness

Frankencore architecture should become executable law.

The Firetest project is a strong candidate for enforcing Frankencore conformance.

Conceptually:

```text
test <component> against <laws>
```

Example laws:

```text
LAW FC001
Capabilities shall be introspectable.

LAW FC002
Provider failure shall have defined semantic behavior.

LAW FC003
Projection shall not own canonical domain state.

LAW FC004
Substrate types shall not cross the adapter boundary.
```

Potential command:

```bash
firetest component ./storage-device \
    against frankencore-laws
```

This is strategically important because it converts architecture from documentation into governance.

---

## 19. Architectural Attack Tests

After initial contracts exist, deliberately attack them.

Required hostile scenarios should include:

```text
provider disappears during invocation
device changes capability
two providers claim the same semantic object
property retrieval fails
policy conflicts
capability version mismatch
circular relationship
projection crashes
backend falsely claims a capability
adapter returns malformed state
object disappears while referenced
provider is replaced during operation
authority changes during operation
```

Each case should eventually have defined semantics.

A design is not hardened merely because the happy path works.

---

## 20. Explicit Non-Goals for the Current Phase

### Do not build a huge Frankencore library yet

The API has not earned stability.

Prototype contracts before producing large implementation surfaces.

### Do not create a universal `FrankencoreObject`

Avoid a base class whose main purpose is:

```cpp
PropertyMap properties;
CapabilityMap capabilities;
Variant anything;
```

This would create weakly typed global coupling.

### Do not make Flowcore the mandatory system runtime

Flowcore should be an exceptionally powerful FrankenPOP! component, not a constitutional dependency of Frankencore.

### Do not over-design security yet

Preserve the distinction between capability and authority now.

A complete authority/security model can follow later.

### Do not over-design distributed operation yet

Versioning, identity, lifecycle, and provider contracts should preserve the possibility of remote providers without requiring immediate distributed-system implementation.

---

## 21. Hardening Roadmap

### Phase 0 — Architectural Freeze

Continue experimentation, but classify architectural concepts explicitly as:

```text
EXPERIMENTAL
PROPOSED
CANONICAL
```

New concepts must not silently become constitutional.

---

### Phase 1 — Constitution

Create:

```text
FRANKENCORE-CONSTITUTION.md
```

Keep it deliberately small.

Target approximately 10–20 invariants and preferably under 15 pages.

It should define:

```text
scope
canonical terminology
invariants
dependency law
identity rules
compatibility/version rules
```

---

### Phase 2 — Canonical Meta-Model

Define:

```text
Type
Identity
Property
Capability
Action
Signal/Event
Relationship
Provider
Policy
Projection
Lifecycle
Result/Error
```

Include negative definitions.

---

### Phase 3 — Capability Specification

Define the initial capability contract.

At minimum:

```text
CapabilityId
CapabilityVersion
requirements
parameters
limits
provider identity
```

Define:

```text
discover
query
invoke
observe
```

Preserve room for later negotiation.

---

### Phase 4 — Clock Reference Object

Implement the complete architectural path for `Clock`.

The implementation must demonstrate:

- Linux substrate isolation;
- adapter boundary;
- semantic identity;
- typed properties;
- explicit capability discovery;
- invocation;
- introspection;
- error handling;
- lifecycle semantics where applicable;
- CLI projection;
- optional Flowcore binding.

---

### Phase 5 — Conformance Harness

Create executable Frankencore laws.

Integrate with Firetest where practical.

The first reference object must pass the conformance suite.

---

### Phase 6 — Dependency Enforcement

Add automated checks preventing forbidden architectural dependencies.

Examples to reject:

```text
core -> GTK
core -> Wayland
core -> Flowcore parser
core -> application code
```

except through explicitly declared lower-level adapters or permitted interfaces.

---

### Phase 7 — Linux Substrate Package

Formalize Linux adapters.

Possible scope:

```text
/proc
/sys
/dev
udev
systemd
D-Bus
Wayland
input
networking
processes
```

Target conceptual package:

```text
frankencore-linux
```

The architecture should preserve the theoretical possibility of:

```text
frankencore-freebsd
frankencore-windows
frankencore-baremetal
```

Even if they are never implemented.

---

### Phase 8 — Display Reference Architecture

After the semantic model has survived smaller objects, implement:

```text
Display
Workspace
Canvas
Viewport
Surface
Renderable
InputTarget
```

Use actual capability negotiation and provider selection.

---

### Phase 9 — Flowcore Bridge

Once Frankencore semantics stabilize, expose them naturally to Flowcore.

Flowcore should treat the Frankencore semantic system as typed graph data, not as special parser-owned runtime magic.

---

## 22. Priority Matrix

| Priority | Work | Reason |
|---|---|---|
| P0 | Canonical vocabulary | Prevent semantic drift |
| P0 | Constitutional invariants | Define Frankencore identity |
| P0 | Dependency rules | Prevent architecture erosion |
| P0 | Minimal semantic model | Foundation for everything |
| P0 | Capability model | Highest coupling risk |
| P1 | Lifecycle semantics | Required by real dynamic systems |
| P1 | Error semantics | Required for interoperability |
| P1 | Introspection / `why` | High architectural value |
| P1 | Conformance tests | Convert theory into law |
| P1 | Clock reference object | Validate the model cheaply |
| P2 | Authority/security model | Preserve safe capability use |
| P2 | Linux adapters | First serious substrate |
| P2 | Display architecture | Major real-world validation |
| P2 | Flowcore bridge | Ecosystem integration |
| P3 | Remote/distributed providers | Later |
| P3 | Alternative substrates | Portability proof |

---

## 23. Frankencore Constitutional Baseline v0.1 Acceptance Gate

The baseline is complete only when all of the following are satisfied.

- [ ] Canonical vocabulary is defined.
- [ ] FrankenPOP!, Frankencore, and Flowcore boundaries are explicit.
- [ ] 10–20 constitutional invariants are written.
- [ ] Dependency direction is defined.
- [ ] Forbidden dependencies are mechanically testable.
- [ ] Minimal semantic meta-model is defined.
- [ ] Capability contract is defined.
- [ ] Capability and authority are explicitly separate.
- [ ] Lifecycle semantics are defined.
- [ ] Error semantics are defined.
- [ ] Contract versioning rules are defined.
- [ ] Introspection semantics are defined.
- [ ] `why`/decision provenance has a design direction.
- [ ] Clock reference object exists.
- [ ] Linux implementation details remain behind an adapter.
- [ ] CLI projection exists for the reference object.
- [ ] Flowcore is demonstrably optional.
- [ ] Conformance suite exists.
- [ ] Reference implementation passes the conformance suite.
- [ ] Architectural attack tests exist for major failure modes.

---

## 24. Instructions for Codex

When operating on the Frankencore repository:

1. Treat this document as an architectural hardening proposal, not permission to perform large rewrites automatically.
2. Inspect the current repository before proposing structural changes.
3. Compare existing code and documentation against the invariants and roadmap above.
4. Identify contradictions, ambiguous terminology, hidden coupling, substrate leakage, and dependency cycles.
5. Prefer small changes that make contracts explicit over large speculative abstractions.
6. Do not introduce a universal Frankencore base-object hierarchy.
7. Do not introduce a mandatory Flowcore dependency into Frankencore.
8. Preserve existing working behavior unless a documented invariant requires change.
9. Separate:
   - semantic contracts;
   - providers;
   - backends;
   - adapters;
   - policy;
   - presentation.
10. Where current code does not fit the proposed model, report the mismatch before rewriting it.
11. Convert architectural laws into tests where practical.
12. Keep experimental ideas marked experimental until explicitly promoted to canonical status.
13. Prefer reference implementations with low complexity before validating the model against display, networking, or other large subsystems.
14. Record unresolved architectural questions explicitly rather than silently choosing arbitrary semantics.

### Initial Codex audit request

Codex should begin by producing a repository-grounded report with:

```text
1. Current repository structure
2. Existing architectural contracts
3. Existing vocabulary
4. Dependency graph
5. Substrate leaks
6. Flowcore coupling
7. Capability abstractions already present
8. Provider/backend abstractions already present
9. Lifecycle/error semantics already present
10. Existing tests relevant to architectural invariants
11. Conflicts with this hardening proposal
12. Missing pieces
13. Recommended minimal first patch set
```

Do not start a large implementation before that audit is complete.

---

## 25. Guiding Principle

Frankencore should provide a stable architectural genome.

FrankenPOP! should remain free to mutate rapidly, adopt useful ideas from existing systems, replace implementations, add projections, and experiment with new capabilities.

A mutation is acceptable when it preserves the constitutional contracts.

A mutation that violates those contracts must either:

1. be rejected; or
2. trigger an explicit constitutional revision.

The long-term goal is that:

> "When does a system cease to be Frankencore?"

becomes an engineering question with a testable answer rather than a matter of taste.
