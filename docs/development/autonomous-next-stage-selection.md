# Codex Meta-Mission — Autonomous Next-Stage Selection

## Purpose

Extend the standing Stage Execution Protocol so Codex can determine and execute the **next logical maturation stage** without requiring a separately authored mission each time.

The standing rule is:

> Use the latest durable checkpoint and gate result to determine the next bounded stage.

Codex should continue autonomously when the next stage is technically and semantically obvious.

Codex should stop for human review only when progress requires a genuine language, architecture, or policy decision.

---

# 1. Determine the Next Stage

At the end of every completed stage:

1. Review:

   * current gate result;
   * remaining gaps;
   * deferred semantics;
   * explicit non-goals;
   * recommended next steps;
   * dependencies exposed during reconnaissance.

2. Identify the **smallest next stage** that advances maturity without combining unrelated unresolved concerns.

3. Frame that stage using the standing Stage Execution Protocol.

Do not create broad roadmap missions when one bounded stage is sufficient.

---

# 2. Autonomous Continuation

If the next stage is obvious and does **not** require inventing new language or architectural semantics:

> Proceed directly.

Examples include:

* propagating an already-defined semantic fact through another stage;
* adding missing deterministic validation;
* adding negative tests for an already-defined rule;
* implementing a backend operation whose semantics are already canonical;
* extending an established resolution rule to another structurally equivalent case;
* correcting a contract-preservation defect;
* tightening refusal of malformed artifacts.

Codex should:

```text
identify next stage
→ state its bounded objective
→ execute normal stage protocol
→ verify
→ produce checkpoint
→ determine next stage again
```

No human confirmation is required merely because a new stage has begun.

---

# 3. Decision Gate

Stop autonomous implementation when the next stage requires choosing between materially different semantic or architectural models.

Examples:

* value versus reference semantics;
* mutable versus immutable aggregates;
* ownership model;
* aliasing behavior;
* copy versus move;
* implicit versus explicit conversion;
* declaration write authority;
* lifetime semantics;
* concurrency guarantees;
* public language syntax where multiple meaningful alternatives exist.

In these cases:

> Do not choose by implementation convenience.

Instead produce a **Decision Brief**.

---

# 4. Decision Brief Format

When blocked on a decision, report:

## Problem

State the exact unresolved question.

Example:

```text
When `b = a` for an aggregate value, does `b` receive an independent value
or another reference to the same mutable object?
```

## Why the decision is required

Explain what cannot safely proceed until it is resolved.

Identify affected areas such as:

* semantic analysis;
* assignment legality;
* aliasing;
* optimization;
* function parameters;
* backend lowering;
* concurrency.

## Current Evidence

Separate:

```text
canonical facts
implementation behavior
legacy behavior
undefined areas
```

Do not present historical behavior as authority.

## Candidate Solutions

Present the viable candidates.

For each candidate include:

### Candidate <X> — <name>

**Meaning**

What semantic model it establishes.

**Pros**

Concrete benefits.

**Cons**

Concrete costs and risks.

**Consequences**

What it implies for:

* language semantics;
* compiler contracts;
* optimization;
* parallelism;
* backends;
* future features.

**Migration impact**

Whether existing syntax/tests/runtime behavior would need to change.

---

# 5. Recommendation

Codex may identify a technically preferred candidate if the evidence supports one.

The recommendation must be explicit about why.

Example:

```text
Recommendation:
Candidate A appears most compatible with current Flow semantics because ...
```

But:

> Do not implement the recommendation until the human decision is given when the choice changes canonical language semantics.

If there is no clear preferred candidate, say so.

---

# 6. Human Decision Request

End the Decision Brief with a compact choice.

Example:

```text
DECISION REQUIRED

A. Immutable aggregate value / reconstruction
B. Unique mutable ownership
C. Explicit reference mutation
D. Shared mutable object
E. Defer and define prerequisite semantics first
```

Then stop.

Do not continue into dependent implementation.

---

# 7. What Does Not Require a Human Decision

Do not stop merely because:

* multiple implementation techniques exist but preserve identical semantics;
* one refactoring is cleaner than another;
* test organization could vary;
* internal helper APIs could be shaped differently;
* evidence needs expansion;
* a bug has an obvious correction;
* contract validation needs tightening.

Those are engineering decisions Codex should make autonomously.

Human review is reserved for choices that change or establish **meaning**, public contracts, or major architecture.

---

# 8. Automatic Stage Chaining

After a stage reaches PASS:

```text
PASS
  ↓
inspect remaining gaps
  ↓
identify next bounded dep
```
