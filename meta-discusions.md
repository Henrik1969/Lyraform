I am widening this discusion a bit because we actually have several points of fail ...
The basic syntax error ;) that is an early one to detect at compile time ... 
but the why an variable x produce the wrong sematical meaning at some point or a logical error where we produce an a/0 error in some unguarded code somewhere are runtime cases 

Yes — and that distinction is crucial. We probably need to stop talking about “errors” as one bucket and instead model failure classes by when they can be known and what they mean.

A useful first split is:

Class	Example	Detectable when?	Meaning
Syntax failure	malformed declaration	parse time	program is not structurally valid
Static semantic failure	Bool -> int where incompatible	analysis time	program is structurally valid but semantically illegal
Static logical contradiction	guarded x > 0, then -1 -> x	analysis time	program violates a proven invariant
Runtime domain failure	a / 0 where denominator becomes zero at runtime	runtime	operation is semantically valid in general, invalid for this concrete value
Runtime guard failure	sensor returns -4 into x guarded x > 0	runtime	attempted state transition violates an active invariant
External/provider failure	file missing, network gone	runtime	environment could not fulfill operation
Resource/system failure	allocation fails	runtime	execution environment cannot complete operation
Programmer assertion failure	assert impossible_condition fails	runtime	claimed invariant was false

That already gives us a cleaner architecture.

Syntax errors aren't really runtime failures

For example:

x : int(

There is no meaningful program to execute.

So:

source
  ↓
parser
  ↓
SYNTAX REFUSAL

That belongs to diagnostics, not the program's own failure graph.

Likewise:

true -> x

where:

x : int

and there is no valid Bool→int conversion.

Again, the compiler already knows this cannot be legal.

STATIC SEMANTIC REFUSAL

No runtime failure node should be generated.

But a / b is different

Suppose:

a : int
b : int

a / b -> result

There is nothing intrinsically semantically wrong with that operation.

If b is statically known:

b : 0

then we can reject it immediately.

But if b comes from:

input() -> b

then the program is valid, but one possible runtime value makes the operation undefined.

That is where failure becomes part of ordinary execution.

Conceptually:

           ┌── result
a / b ─────┤
           └── DivisionByZero

The division operation itself has two possible outcome classes.

That is very different from exceptions being secretly thrown somewhere.

And guard fits the same general model

Runtime guard enforcement is just another operation with success/failure outcomes:

candidate state
      ↓
guard check
      ├── accepted candidate
      └── GuardViolation

So guard doesn't need a special error architecture.

It needs to plug into the same architecture as:

divide
parse
open-file
network request
provider call
allocation
validation

That's important.

“Wrong semantic meaning” needs another distinction

Suppose:

x : int

contains 12, but logically x represents:

age_of_person

and somehow it becomes:

-500

The type system alone says:

-500 : int

Perfectly legal.

But the domain semantics say it is nonsense.

That is precisely where things such as:

valid_age : guard x >= 0 && x <= 150

come into play.

Now we've moved knowledge from:

programmer knows this ought to be true

into:

compiler/runtime has an explicit contract saying this must be true.

Without such a declared constraint, the runtime cannot magically know -500 is semantically wrong.

That's worth preserving as a language principle:

The system can enforce semantic meaning only when that meaning has been expressed as a type, contract, guard, invariant, provider contract, or other canonical fact.

Otherwise an int is just an int.

So I think we're discovering three broad layers
Layer 1 — Program construction failures

These prevent the program becoming executable:

lexical
syntax
name resolution
type errors
contract contradictions
provable guard violations
unsupported semantics

Result:

COMPILATION / ADMISSION REFUSED

These are compiler diagnostics.

Layer 2 — Expected runtime failures

Operations that are legitimate but may fail for particular data/environment:

division by zero
guard rejection
parse failure
file not found
network timeout
provider refused
bounds failure
conversion failure

These should be first-class flow outcomes.

Conceptually:

operation
  ├── value
  └── failure

These aren't necessarily bugs.

Layer 3 — Broken-program / system failures

Examples:

assertion failure
contract corruption
invalid compiler artifact
impossible runtime state
out-of-memory
backend invariant violated

These may need different policy because continuing normally may not be safe.

And even here, we should probably distinguish:

recoverable resource failure

from:

internal invariant corruption

instead of lumping them together.

This suggests an important language distinction

We probably need two concepts that people often incorrectly call “errors” interchangeably:

failure

A legitimate outcome of an operation.

Example:

divide(20, runtime_zero)
    → DivisionByZero

The program is still behaving according to its semantics.

fault

Something has violated the execution model itself.

For example:

artifact says int
backend discovers impossible Bool representation

or internal runtime corruption.

That is not an ordinary business/program outcome.

So we might eventually have:

success
failure
fault

as distinct concepts.

Not necessarily keywords — just semantic categories.

Division demonstrates the whole model nicely

Consider:

a / b -> result
Case 1

Compiler knows:

b = 2

Then:

division safe

No runtime check.

Case 2

Compiler knows:

b = 0

Then:

compile-time refusal

There is no legitimate execution.

Case 3

Compiler knows only:

b : int

Then:

runtime division check required

and execution becomes conceptually:

candidate divisor
      ↓
b != 0 ?
 ├── YES → calculate result
 └── NO  → DivisionByZero failure flow

This is exactly the same prove/refuse/runtime-check pattern as guard.

That's significant.

We may have found a general compiler rule

Instead of guards being special, there is a broader Lyraform rule:

Whenever an operation has a semantic precondition, the compiler must prove it, prove its violation, or lower enforcement.

Formally:

operation requires predicate P

Then:

P provably true
    → emit operation without check

P provably false
    → refuse program/transition statically

P unknown
    → runtime check + explicit failure path

Examples:

division:
    denominator != 0

array access:
    0 <= index < size

guarded placement:
    active predicates remain true

conversion:
    value representable by destination

provider call:
    capability / availability requirement

possibly arithmetic:
    no-overflow policy

Now we're no longer merely designing guard.

We're beginning to define Lyraform's general admission and runtime-failure semantics.

And that is exactly why choosing D for guard right now is sensible.

Don't implement a one-off guard failure system.

First define this general mechanism:

semantic precondition
    ↓
prove / refuse / runtime enforce
    ↓
explicit failure flow

Then guard, division, indexing, conversions, providers, etc. all use one coherent model.

That seems like the next language concept worth maturing before we hand another implementation stage to Igor.
