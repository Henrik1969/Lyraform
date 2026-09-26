# `guard` / `unguard` semantic maturation scout

Date: 2026-09-19.

## Baseline

Branch `main`; HEAD and `origin/main` at stage entry:
`6ba9db303db453af952327069814c3a607b26d03`.

The worktree was clean. This stage changes documentation only. No branch,
commit, remote, `master`, or FlowLFS operation was performed.

## Bounded question

What structural and semantic model can represent a named live invariant whose
dependencies must remain valid from activation until selective `unguard` or
lexical scope exit?

The semantic brief already establishes the meaning of the facility. The scout
therefore separates that accepted direction from the public syntax and runtime
failure-routing decisions that remain open.

## Canonical facts

- A guard is a named, scoped live invariant, not a one-time branch or assertion.
- Its predicate may depend on multiple resolved declarations.
- Activation, every dependency-changing transition, selective deactivation,
  and lexical scope exit participate in its lifetime.
- A threatened transition is classified as proven safe, proven violation, or
  not provable.
- A proven violation is refused statically. An unknown transition requires
  pre-commit runtime overwatch or closed refusal.
- Multiple guards compose; removing one does not remove the others.
- Candidate state is checked before commit. Invalid state must never become
  observably current.
- Aggregate guards eventually operate on reconstructed candidate values under
  ADR 0052, not hidden alias mutation.

## Current structural and implementation facts

### Lexer and expressions

`guard` and `unguard` are currently ordinary identifiers. The active expression
AST can represent identifiers, integer/Bool literals, unary expressions and a
single comparison such as `x > 0`. The lexer and expression grammar do not
currently represent `&&` or `||`, so the range example is outside a first
bounded slice.

Expression identity and resolved symbol identity are already available in the
frontend bundle and Flowanalyst. This is enough to derive a dependency set from
identifier leaves without using source spelling as semantic identity.

### Statements and scopes

The statement AST has let, assignment, placement, control-flow, return,
expression, flow and unknown variants. It has no activation or deactivation
node. Blocks and projected scopes provide lexical containment, while block
statement order provides activation/deactivation order for a straight-line
slice.

The symbol system has `Contract` but no dedicated guard symbol kind. A guard
could use a contract identity with an explicit guard role, or the symbol enum
could gain `Guard`; either internal representation preserves the same language
meaning. `unguard` must resolve semantic identity, not compare an arbitrary
string.

### Existing refined types

Refined types carry declaration-wide invariants over a synthetic `value`
identifier. The compatibility parser can evaluate selected integer-literal
refined-type invariants. Those are useful evidence for comparison evaluation,
but they are not scoped guard lifecycle semantics and must not become guard
authority.

### Scalar transitions

Canonical scalar analysis already owns initialized local `int`/`Bool`
declarations and placements, exact source/destination compatibility, statement
origin, and early refusal. It infers types, not a general symbolic value
environment. A first static guard slice therefore needs a separate bounded
constant/refinement evaluator rather than teaching a backend to decide guards.

### Failure infrastructure

The repository has several distinct structured failure boundaries:

- compiler-process diagnostics with `no_artifact` disposition;
- source-graph activation failure;
- bounded tagged `Outcome<Text,TextFailure>`;
- durable FrankenCore error-state history.

None is presently the canonical language-level runtime failure object for a
guard. Selecting one now would invent failure semantics. The first static-only
slice can remain correct by refusing every not-provable transition until a
separate failure-routing decision admits runtime overwatch.

## Source probes

Three temporary source probes were exported through the current frontend.

### Preferred spelling

```lyraform
positive : guard x > 0
```

is projected as an uninitialized ordinary variable named `positive` whose type
is the unresolved name `guard`. The predicate tokens are discarded. Flowanalyst
then reports unknown type `guard`.

### Parenthesized declaration spelling

```lyraform
positive : guard(x > 0)
```

preserves the comparison expression, but projects it as the initializer of an
ordinary variable whose unresolved type is `guard`. Treating that accidental
shape as guard semantics would silently reinterpret typed declarations and
would make a guard appear to be an ordinary runtime value.

### Keyword-led spelling

```lyraform
guard positive : x > 0
```

is not recognized as a statement. Current token skipping later misprojects
`positive : x` as an ordinary variable declaration. This spelling therefore
also requires a dedicated parser node rather than reuse of existing behavior.

For all three probes:

```lyraform
unguard positive
```

is silently omitted from the statement pool. Parse validity remains
`outside_scope`/`unassessed`, not canonical validity. No current spelling has
language authority.

## Canonical authority model after syntax selection

The smallest defensible architecture is:

```text
frontend
    dedicated guard activation/deactivation structure
    complete predicate expression
    lexical source provenance

symbol projection
    named guard identity and lexical resolution

Flowanalyst/shared guard semantics
    Bool predicate typing
    resolved dependency symbol set
    lifetime and active-set tracking
    bounded candidate substitution
    safe / violation / not-provable classification

Flowcontracts
    validate versioned guard facts and operation linkage

later stages
    preserve and validate

backends
    consume admitted overwatch operations only; never decide semantics
```

## Smallest implementation slice after the decision

The first bounded slice should be straight-line, local, scalar and static:

- initialized local `int` dependency;
- one comparison predicate of the form `x <op> integer-literal`;
- activation only when the current known literal state proves the predicate;
- later literal placement classified through candidate substitution;
- proven-safe placement admitted without runtime checking;
- proven violation refused by Flowanalyst;
- not-provable activation or transition refused closed as runtime overwatch is
  not yet admitted;
- selective named deactivation followed by ordinary scalar placement;
- automatic deactivation at the defining block's exit;
- no `if`, loop, call, aggregate, parameter, reference, alias, conjunction,
  runtime handler, or cross-function preservation semantics.

The bounded evaluator should be intentionally finite. It is not a general
theorem prover.

The initial artifact can carry facts equivalent to:

```text
guard identity
activation/deactivation statement identity
predicate expression identity
resolved dependency symbol identities
lexical scope identity
classification
affected transition identity
execution = elided_static | unsupported
provenance
```

Runtime-overwatch operations and failure payloads remain a later stage.

## Stop reason

The semantic brief explicitly asks for a decision brief when syntax presents a
material canonical choice. All viable spellings require new dedicated syntax;
none can be inherited honestly from the current parser. The alternatives
change whether a guard looks like a declaration/value or a lifecycle statement
and therefore affect AST shape, namespaces, diagnostics, future first-class
contract behavior, and grammar compatibility.

See
[the surface-syntax decision brief](2026-09-19-guard-surface-syntax-decision-brief.md).

## Untouched scope

No lexer, parser, AST, symbol, semantic, contract, plan, optimizer, backend, or
runtime code changed. No guard spelling is admitted. Existing unknown-token
behavior was observed, not promoted to semantics.

## Verification

- `./igor doctor`: PASS;
- `./igor build`: PASS, no work required;
- `./igor test`: **164/164 PASS**, 67.65 seconds;
- `git diff --check`: PASS.

Sanitizer and Valgrind runs are not meaningful for this documentation-only
decision stage; no executable source or test changed.

GATE: PASS — GUARD SURFACE SYNTAX

## Resolution after review

Candidate B was accepted on 2026-09-19:

```lyraform
guard positive : x > 0
unguard positive
```

The decision is durable in
[ADR 0053](../architecture/decisions/0053-named-guard-lifecycle-syntax.md).
The untouched-scope and verification statements above describe this scout at
its original decision gate; implementation evidence is recorded separately in
the bounded static-lifecycle checkpoint.
