# Codex Meta-Mission — Stage Execution Protocol

## Purpose

This document defines the default execution pattern for all future Flowcore / Lyraform maturation stages.

Treat it as standing operating procedure.

Individual stage missions should therefore describe only:

* the stage objective;
* the specific question to resolve;
* stage-specific scope;
* stage-specific evidence requirements;
* explicit non-goals;
* any exceptional constraints.

Do not require every stage mission to repeat this protocol.

---

# 1. Operating Principle

Each stage is a **bounded investigation and implementation cycle**.

The goal is not merely to make code work.

The goal is to establish:

```text
current facts
    ↓
semantic / architectural question
    ↓
candidate models
    ↓
chosen authority
    ↓
smallest defensible implementation
    ↓
positive + negative evidence
    ↓
hostile validation
    ↓
whole-system verification
    ↓
gate decision
```

Do not skip directly from a requested feature to implementation.

---

# 2. Stage Entry

Before changing code, establish the baseline.

Record:

* branch;
* HEAD;
* `origin/<branch>`;
* worktree state;
* existing modified/untracked files;
* relevant previous gate/checkpoint;
* known unresolved semantics;
* explicit protected scope.

Unless a stage mission says otherwise:

* do not touch unrelated dirty worktree content;
* do not touch `master`;
* do not touch FlowLFS;
* do not stage;
* do not commit;
* do not push.

Preserve user work.

---

# 3. Phase A — Scout

Every stage begins with reconnaissance.

Determine what the system **actually does now**.

Inspect only the areas necessary to answer the stage question.

Classify every important finding as one of:

```text
canonical language fact
canonical implementation behavior
legacy/historical behavior
test expectation
accidental behavior
undefined
```

Do not promote implementation accidents or legacy behavior into language semantics.

Do not infer intended semantics merely because code currently behaves a certain way.

The scout phase should answer:

```text
What do we know?
What do we not know?
What layer currently owns the behavior?
What assumptions would implementation require?
```

---

# 4. Phase B — Frame the Question

Reduce the stage to one bounded technical or semantic question.

Examples:

```text
What does p.a.b denote?
```

```text
When is a resolved member target assignable?
```

```text
What does updating part of an aggregate mean?
```

```text
How is an indexed target resolved?
```

Avoid vague objectives such as:

```text
implement structs
improve assignments
finish indexing
```

The stage must have a question that can be answered with evidence.

---

# 5. Phase C — Candidate Models

When semantics or architecture are not already canonical, identify competing models before implementing one.

For each viable candidate, evaluate:

* compatibility with existing language semantics;
* compiler architecture;
* determinism;
* static reasoning;
* optimization;
* backend independence;
* runtime consequences;
* parallelism;
* serialization/distribution where relevant;
* future extensibility;
* implementation complexity;
* hidden semantic commitments.

Do not invent a candidate merely because it is easiest to code.

A valid candidate is:

```text
defer / unresolved
```

if the language lacks prerequisite semantics.

---

# 6. Phase D — Choose Authority

Every semantic fact must have one canonical owner.

Typical authority flow:

```text
parser
    → structural syntax/completeness

semantic analysis / shared semantic model
    → identity
    → types
    → legality/admission

shared contracts
    → artifact consistency

later compiler stages
    → preserve + validate

backend
    → execute already-established semantics
```

Avoid duplicated semantic decisions.

In particular:

* backends must not invent language semantics;
* later stages must not repeat semantic lookup unnecessarily;
* strings must not substitute for resolved identity where identity is available;
* runtime behavior must not become language authority by accident.

---

# 7. Phase E — Bounded Implementation

Implement the smallest slice necessary to answer the stage question.

Do not opportunistically expand scope.

Do not implement adjacent features merely because the code is nearby.

Prefer:

```text
one semantic fact
one authority
one contract
one bounded pipeline path
```

over broad partial support.

If the stage exposes a deeper semantic dependency, stop rather than guessing.

---

# 8. Phase F — Positive Evidence

Prove supported behavior with focused cases.

Positive evidence should demonstrate, where applicable:

* structural correctness;
* semantic identity;
* type identity;
* deterministic resolution;
* contract generation;
* preservation through applicable stages;
* expected execution if execution is within scope.

Prefer multiple small cases over one large demonstration.

For hierarchical constructs, include both:

```text
simple case
nested case
```

where relevant.

---

# 9. Phase G — Negative Evidence

Every positive capability should have corresponding refusal cases.

Test invalid forms such as:

* unknown identity;
* wrong type;
* incomplete syntax;
* invalid intermediate structure;
* ambiguous declaration;
* unsupported operation;
* illegal target kind;
* incompatible source/destination;
* missing semantic evidence.

Refusal should happen at the earliest authoritative stage that has enough information to decide.

Do not allow invalid input to drift into later stages merely to fail there.

---

# 10. Phase H — Hostile Artifact Validation

Where semantic facts are serialized or passed between stages, deliberately mutate valid artifacts.

Test contradictory evidence such as:

* wrong declaration identity;
* wrong type identity;
* reordered segments;
* mismatched names;
* impossible target kind;
* inconsistent final type;
* false authority claims;
* missing required identity;
* unsupported execution claims.

Every consumer that relies on the contract must reject contradictory evidence deterministically.

A semantic artifact must be an enforceable contract, not decorative metadata.

---

# 11. Phase I — Pipeline Preservation

Once a semantic fact has canonical authority, later stages should preserve it unless transformation is explicitly part of their responsibility.

Preferred pattern:

```text
resolve once
validate many
preserve exactly
```

Avoid:

```text
resolve
reinterpret
resolve again
guess in backend
```

Where applicable, prove byte-identical or structurally identical preservation.

---

# 12. Phase J — Backend Boundary

Backends execute semantics.

They do not define them.

If a semantic operation is known but backend support is absent:

```text
semantic fact: valid
execution: unsupported
```

is preferable to partial or guessed execution.

Backends must not:

* collapse richer targets into simpler ones;
* invent storage layout;
* infer ownership;
* manufacture aliasing rules;
* assume copy/move behavior;
* reinterpret unresolved semantics.

Explicit refusal is a correct result.

---

# 13. Phase K — Verification

Unless the stage explicitly changes the required verification set, run:

```text
./igor doctor
./igor build
./igor test
```

Also run stage-specific focused tests.

Where relevant:

* normal focused suite;
* Clang 18 ASan/UBSan;
* Valgrind probes;
* `git diff --check`;
* shell syntax checks;
* deterministic/byte-identical analysis checks.

Report exact counts.

Do not report merely:

```text
tests pass
```

Report:

```text
164/164 PASS
8/8 focused PASS
4/4 Valgrind probes clean
```

If a tool cannot operate because of the host environment, document the limitation and use the established substitute where possible.

---

# 14. Phase L — Gate Decision

Every stage ends at a gate.

The allowed outcomes are:

```text
PASS
```

The bounded question has been answered and supported by evidence.

```text
BLOCKED
```

A prerequisite semantic or architectural decision is missing.

```text
WAITING FOR HUMAN REVIEW
```

The implementation/evidence is complete enough for review, but the next step requires human acceptance or selection.

A blocked stage is not a failed stage.

Discovering that implementation would require invented semantics is a successful reconnaissance result.

Never manufacture semantics merely to obtain `PASS`.

---

# 15. Durable Checkpoint

Every completed stage should leave a durable checkpoint report.

The report should contain, as applicable:

* baseline;
* scout findings;
* stage question;
* candidate models;
* selected/rejected models;
* authority accounting;
* implementation changes;
* semantic contract changes;
* positive evidence;
* negative evidence;
* hostile artifact evidence;
* preservation evidence;
* backend behavior;
* verification results;
* untouched scope;
* remaining gaps;
* recommended next stage;
* gate result.

The checkpoint becomes the next stage's factual baseline.

---

# 16. Scope Discipline

Default rule:

> If it is not required to answer the stage question, do not implement it.

Reconnaissance may inspect adjacent areas where necessary to understand consequences.

Inspection does not authorize implementation.

Explicitly distinguish:

```text
observed
considered
selected
implemented
deferred
```

---

# 17. Semantic Discipline

Use these rules throughout all stages.

## Rule 1

Implementation behavior is not automatically language semantics.

## Rule 2

Historical behavior is evidence, not authority.

## Rule 3

A parser recognizing syntax does not prove semantic legality.

## Rule 4

A resolved target is not automatically assignable.

## Rule 5

Assignable does not automatically mean executable by current backends.

## Rule 6

Optimization must preserve language semantics, not define them.

## Rule 7

Do not hide unresolved semantics behind implementation convenience.

## Rule 8

Prefer explicit:

```text
unresolved
unsupported
refused
```

over implicit assumptions.

---

# 18. Evidence Ladder

Use the following maturity ladder when describing a capability:

```text
recognized
    ↓
structurally validated
    ↓
semantically resolved
    ↓
legality/admission proven
    ↓
contract preserved
    ↓
backend executable
    ↓
optimized
```

Do not claim a higher maturity level than the evidence proves.

Example:

```text
p.a.b
```

may be:

```text
recognized                 YES
structurally validated     YES
semantically resolved      YES
assignable                 UNRESOLVED
backend executable         NO
```

This is a valid and useful compiler state.

---

# 19. Stage Mission Format

Future stage missions should therefore be compact.

Recommended template:

```text
STAGE <N> — <NAME>

Objective:
    <one bounded question>

Baseline:
    <previous gate/checkpoint>

Scope:
    <areas allowed>

Required investigation:
    <stage-specific scout questions>

Candidate models:
    <only if stage-specific candidates are already known>

Required evidence:
    <stage-specific positive/negative/hostile cases>

Explicit non-goals:
    <things not to implement>

Additional verification:
    <anything beyond standing protocol>

Expected gate:
    PASS / BLOCKED / WAITING FOR HUMAN REVIEW

Deliverable:
    durable checkpoint
```

Everything else comes from this Stage Execution Protocol.

---

# 20. Core Working Pattern

The standing Flowcore/Lyraform maturation loop is:

```text
SCOUT
  ↓
ESTABLISH FACTS
  ↓
FRAME QUESTION
  ↓
EVALUATE MODELS
  ↓
CHOOSE AUTHORITY
  ↓
IMPLEMENT BOUNDED SLICE
  ↓
PROVE POSITIVE BEHAVIOR
  ↓
PROVE REFUSAL BEHAVIOR
  ↓
ATTACK CONTRACT
  ↓
VERIFY PIPELINE
  ↓
GATE
  ↓
HUMAN REVIEW
  ↓
NEXT STAGE
```

This protocol remains in force unless a future mission explicitly overrides part of it.
