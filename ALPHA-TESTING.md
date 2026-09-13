# Lyraform Alpha Testing

Lyraform is experimental, unstable, and not production-ready.

This alpha is not primarily about collecting feature requests or positive impressions.

It is about finding where the project is wrong.

We want testers who are willing to challenge assumptions, break the compiler, falsify architectural claims, find contradictions, and point out where Lyraform adds more complexity than value.

If something feels over-designed, underspecified, misleading, fragile, or simply unnecessary, please say so.

That is useful feedback.

---

## What Lyraform is

Lyraform is an experimental programming-language and systems-architecture project built around:

- explicit contracts;

- compiler-visible graph structure;

- typed ports and connections;

- explicit effects and resources;

- provider and adaptor boundaries;

- governed lowering;

- provenance and revision lineage;

- executable projections derived from explicit semantic artifacts.

The project was previously named Flowcore.

Historical Flowcore and Flowmini names remain in old documents and compatibility identifiers where they are still correct.

The current human-facing toolchain driver is:

```text
Igor
```

---

# What we need from alpha testers

We are looking for critical testers, not cheerleaders.

Useful alpha feedback includes:

- architectural claims that are not supported by the implementation;

- invalid programs that the compiler accepts;

- valid programs that the compiler rejects;

- incorrect diagnostics;

- missing validation;

- graph inconsistencies;

- contract holes;

- effect or authority leaks;

- hidden dependencies on local machine state;

- unreproducible builds;

- undocumented assumptions;

- misleading terminology;

- stale documentation;

- unnecessary abstraction;

- simpler designs that achieve the same result;

- prior art that Lyraform should acknowledge or compare against;

- performance or complexity costs hidden by the current prototypes.

If you think a core design premise is wrong, that is valid alpha feedback.

---

# What is especially valuable

The following kinds of criticism are particularly useful:

## “This invariant is false”

If documentation says something is always true and you can construct a counterexample, please report it.

Examples:

```text
This connection is supposed to be rejected, but it compiles.

This node claims to be pure, but it can invoke an effectful provider.

This artifact claims complete provenance, but this field has no traceable origin.
```

---

## “This abstraction costs more than the problem”

Lyraform explicitly tries to avoid architecture becoming more expensive than the problems it solves.

Please challenge this.

If a normal C/C++ function, library call, state machine, build rule, or ordinary API solves a problem more clearly than the Lyraform mechanism, explain why.

We are interested in cases where:

```text
architectural cost > practical benefit
```

---

## “This only makes sense if you already know the project”

This is extremely useful.

The project has grown through long-running design discussions, which means terminology can accidentally become obvious only to people who were present when it was invented.

If something is unclear on first contact, report it.

Do not assume you are missing something.

The documentation may simply be bad.

---

# Current alpha baseline

At the time this document was written, a clean fresh clone is expected to build and test successfully from zero local project state.

Current verified baseline:

```text
clean configure: PASS
clean build: PASS
registered tests: 106
CTest: 106/106 PASS (fresh GCC)
```

The project remains experimental despite passing its current tests.

Passing tests means only that the currently tested behavior is behaving as expected.

It does not mean the design is complete or correct.

---

# Getting started

Clone the repository:

```bash
git clone git@github.com:Henrik1969/Lyraform.git
cd Lyraform
```

or use HTTPS:

```bash
git clone https://github.com/Henrik1969/Lyraform.git
cd Lyraform
```

Start with:

```bash
./igor doctor
```

Then:

```bash
./igor build
./igor test
```

Useful commands currently include:

```text
igor build
igor check
igor test
igor run
igor doctor
```

Run:

```bash
./igor --help
```

for the current command surface.

---

# Alpha Mission A — Cold clone

Pretend you know nothing about the project.

Use only the public repository.

Do not rely on prior conversations, local backups, or undocumented environment setup.

Try:

```bash
git clone ...
./igor doctor
./igor build
./igor test
```

Report:

- operating system;

- compiler version;

- CMake version;

- Ninja version;

- exact failure;

- whether README instructions were sufficient;

- anything you had to guess.

Success criteria:

```text
A stranger can get from clone to green build without project archaeology.
```

---

# Alpha Mission B — Newcomer comprehension

Read only:

```text
README.md
```

Then answer for yourself:

1. What problem is Lyraform trying to solve?

2. What is a node?

3. What is a port?

4. What is a wire?

5. What is a contract?

6. What is a provider?

7. What does Igor do?

8. What does “compiler-visible architecture” mean?

9. What does Lyraform currently implement?

10. What is still only planned?

If these answers are unclear or contradictory, report that.

Do not read the deeper architecture docs until after doing this exercise.

---

# Alpha Mission C — Break the compiler

Try to construct programs that should obviously be invalid.

Examples include:

- connecting incompatible port types;

- reversing input/output direction;

- leaving required inputs unconnected;

- referencing nonexistent nodes or ports;

- duplicating identifiers;

- violating declared effects;

- using unavailable providers;

- malformed source;

- malformed artifacts;

- impossible contract combinations;

- invalid graph topology.

For every case record:

```text
expected:
actual:
diagnostic:
exit status:
```

We especially want cases where Lyraform silently accepts something that should be rejected.

---

# Alpha Mission D — Find valid programs it rejects

Try normal, minimal programs that appear semantically valid.

Reduce any unexpected compiler rejection to the smallest example you can.

Please distinguish:

```text
I dislike the syntax
```

from:

```text
This program appears valid under the documented rules but is rejected.
```

Both are useful, but they are different issues.

---

# Alpha Mission E — Attack effect boundaries

Lyraform wants effects and authority to be visible rather than silently available.

Try to violate that assumption.

Look for ways to:

- perform filesystem I/O without the required declared effect;

- reach effectful code through an apparently pure path;

- invoke providers that should not be authorized;

- bypass effect validation through indirection;

- create inconsistent effect declarations;

- make diagnostics lose the responsible call/node/provider.

A useful report demonstrates:

```text
declared authority
versus
actual authority exercised
```

---

# Alpha Mission F — Attack graph validation

Try to create graphs that are structurally questionable.

Examples:

- incompatible endpoints;

- missing required inputs;

- invalid terminal nodes;

- duplicate deliveries;

- unexpected fan-out;

- cycles where they are not supported;

- disconnected executable components;

- ambiguous provider selection;

- graph structure that lowers differently from its apparent meaning.

We want to know whether the graph that the compiler accepts actually corresponds to the graph the user wrote.

---

# Alpha Mission G — Challenge provenance

Lyraform treats provenance as a first-class concern.

Pick a generated artifact or diagnostic and ask:

```text
Where did this fact come from?
```

Try to trace it backward.

Report places where provenance:

- disappears;

- becomes ambiguous;

- references stale data;

- loses source location;

- cannot distinguish transformation stages;

- claims more certainty than the implementation has.

A useful artifact should be able to explain enough of its history to be audited.

---

# Alpha Mission H — Challenge determinism and reproducibility

Run clean builds more than once.

Try:

```bash
rm -rf build-directory
./igor build
./igor test
```

or equivalent clean builds.

Compare results.

Look for:

- generated files depending on previous builds;

- local paths leaking into outputs;

- nondeterministic ordering;

- tests depending on execution order;

- timestamps where they should not matter;

- missing dependencies;

- untracked files required for success.

A clean clone should not accidentally depend on the developer’s workstation history.

---

# Alpha Mission I — Attack diagnostics

Diagnostics are part of the language interface.

Try to make them fail.

Look for:

- wrong source positions;

- misleading error causes;

- multiple errors collapsed into one;

- implementation terminology leaking into user-facing diagnostics;

- errors without actionable context;

- graph errors that fail to name the relevant node/port/wire;

- effects errors that fail to identify the required authority;

- unstable diagnostic identifiers.

A bad diagnostic is a real bug even if the compiler rejected the program correctly.

---

# Alpha Mission J — Try a simpler solution

Pick one Lyraform mechanism.

Examples:

```text
graph connection
provider selection
contract declaration
effect declaration
artifact provenance
lowering plan
```

Then implement or describe the same task using a conventional approach.

Examples:

- plain C++;

- ordinary function calls;

- a state machine;

- configuration tables;

- dependency injection;

- a normal build system;

- a message queue;

- an existing compiler IR.

Then compare:

```text
clarity
complexity
debuggability
testability
performance
auditability
amount of code
```

If Lyraform loses, tell us.

---

# Alpha Mission K — Prior-art attack

Lyraform does not claim to have invented graph computation, contracts, effects, provenance, process networks, interface checking, or compiler IRs.

Read:

```text
REFERENCES.md
```

If you know relevant work we have missed, please report it.

Especially useful references include:

- papers;

- languages;

- compiler architectures;

- formal models;

- dataflow systems;

- effect systems;

- component models;

- contract systems;

- provenance systems;

- graph runtimes.

Please include a link or citation where possible.

---

# Alpha Mission L — Compare against another system

If you know another language or system well, compare one Lyraform mechanism with it.

Good comparison targets include:

- Rust;

- Zig;

- Flix;

- Koka;

- Ptolemy;

- CSP/process-network systems;

- LLVM/MLIR;

- actor systems;

- dataflow languages;

- workflow systems;

- contract-based component systems.

Do not compare slogans.

Compare concrete mechanisms.

For example:

```text
Lyraform effects vs Flix effects
Lyraform providers vs dependency injection
Lyraform graph validation vs Interface Automata-style compatibility
Lyraform lowering artifacts vs MLIR
```

---

# Reporting issues

Use GitHub Issues:

```text
https://github.com/Henrik1969/Lyraform/issues
```

A useful report contains:

```text
Title:
short factual description

Environment:
OS
compiler
CMake
Ninja
commit SHA

Steps to reproduce:

Expected result:

Actual result:

Diagnostics/output:

Minimal reproduction:

Why this matters:
```

Include logs or small repro files where useful.

Avoid uploading secrets, credentials, private source code, or confidential material.

---

# Architecture criticism

Architecture criticism is explicitly welcome.

A useful architecture issue can use this structure:

```text
Claim being challenged:

Relevant document/code:

Why I think the claim is wrong:

Counterexample:

Simpler or safer alternative:

Trade-offs:

References:
```

You do not need to propose a replacement to report a real problem.

“This abstraction has no demonstrated benefit” is valid criticism if you can explain why.

---

# Security-related findings

If you find a security problem that creates a real exploitable vulnerability rather than simply an unfinished experimental boundary, please do not publish sensitive exploitation details immediately.

Open a minimal issue describing that there is a security-sensitive problem, or contact the maintainer privately.

Remember that Lyraform is explicitly experimental and not currently intended for security-critical or production use.

---

# What not to expect

Lyraform currently does **not** claim to be:

- production-ready;

- memory-safe in the Rust sense;

- formally verified;

- a complete effect-system implementation comparable to mature research languages;

- a general distributed runtime;

- a replacement for C/C++;

- a finished operating system;

- a complete package ecosystem;

- performance-proven across real workloads.

If documentation appears to make one of those claims, please report it.

That is likely a documentation bug.

---

# Historical names

You may encounter:

```text
Flowcore
Flowmini
```

in old checkpoints, compatibility schemas, binaries, artifacts, and historical documentation.

This is intentional where the names describe real historical states or compatibility contracts.

Current project identity is:

```text
Lyraform
```

The current toolchain driver is:

```text
Igor
```

Do not report every historical Flowcore reference as a bug.

Do report a Flowcore/Flowmini reference if it appears to claim current authority incorrectly.

---

# Research references

See:

```text
REFERENCES.md
```

for published work that influenced or informed the project.

Lyraform is an original experimental synthesis, not an implementation of any single referenced formalism.

If a reference is missing, incorrectly described, or overstated, please report that too.

---

# Testers we especially want

We would particularly value feedback from people with experience in:

- compiler implementation;

- programming-language design;

- type/effect systems;

- formal methods;

- C/C++ systems programming;

- ABI and native-code work;

- build systems;

- Linux tooling;

- dataflow/process-network systems;

- graph runtimes;

- distributed systems;

- security;

- static analysis;

- developer tooling;

- documentation for complex technical systems.

Experience is useful but not required.

A newcomer who cannot understand the README can uncover a problem an expert may completely miss.

---

# Expected behavior between releases

Alpha means breaking changes are expected.

The following may change:

- syntax;

- compiler behavior;

- architecture;

- internal formats;

- diagnostics;

- commands;

- directory layout;

- provider interfaces;

- standard-library structure.

Compatibility identifiers that are intentionally retained will be documented explicitly.

Do not build production dependencies on Lyraform at this stage.

---

# Code of feedback

Please be direct.

Good:

```text
This design is unnecessarily complicated because X already solves the same
problem with fewer states and clearer failure semantics.
```

Good:

```text
The documentation says this is impossible, but this input produces it.
```

Good:

```text
I could not understand what a provider was until I read four architecture files.
```

Less useful:

```text
This sucks.
```

Also less useful:

```text
Looks awesome!
```

We appreciate encouragement, but alpha testing benefits most from evidence.

---

# The main question

The central experiment behind Lyraform is approximately:

> Does keeping architectural intent explicit and compiler-visible provide enough analyzability, accountability, and compositional value to justify the additional structure?

Everything in the project is allowed to be challenged in service of answering that question honestly.

If the answer for some part of the architecture is “no,” we want to know now.

---

# Thank you

Lyraform exists because people published compilers, papers, languages, operating systems, formal models, tools, and failures that other people could learn from.

Critical testers are part of the same tradition.

Please break things.

Igor will try to put them back together.

If the test suite goes green afterward:

```text
IT'S ALIVE.
```
