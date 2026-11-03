# The FlowMini Programmer’s Manual

This manual teaches the current FlowMini language surface through runnable
examples: source units, imports, declarations, expressions, functions,
records, collections, control flow, refined types, and ABI declarations.

## 1. Your first program

```flow
program hello

main {
    answer : int32(40)
    answer + 2 -> answer
    print answer
}
```

`program hello` names the executable source unit. `main` is its root executable
block. A declaration gives a name, type, and initializer. The arrow form
evaluates an expression and places its result in a target.

The simple source form has one root `main` block. For a program that describes
several products, entrypoints are named by `target` blocks rather than by
creating several anonymous root `main` blocks.

```flow
program toolset

target cli {
    main {
        print "command-line tool"
    }
}

target daemon {
    main {
        print "service"
    }
}
```

This gives the language model an important distinction:

```text
program = shared semantic universe
target  = named buildable or runnable projection
main    = entrypoint owned by a target
```

Each target has one meaningful `main`; target names remove the ambiguity that
would arise from several anonymous root mains. Shared declarations may be used
by more than one target, while target-local declarations belong only to that
projection. A plain root `main` remains the compact single-product form. Named
targets are the planned multiplexed-entrypoint form; the active v25 executable
subset still accepts the plain one-root-`main` form only.

## 2. Programs, units, and imports

FlowMini has two source-unit roles:

```flow
program application
unit math_helpers
```

A `program` is the executable root and may contain `main`. A `unit` is an
importable defining source unit and must not contain `main`.

Imports name a unit by path:

```flow
import "../../std/math.flow"
import "../../std/collatz.flow"

program import_demo

main {
    value : int32(5)
    result : int32(0)
    factorial(value) -> result
    print result
}
```

Imported files are units, not programs. The root program is the thing you run;
units supply declarations and functions.

## 3. Comments and layout

Line comments use `//` or `#`:

```flow
// comment
# another comment
value : int32(1) // trailing comment
```

Block comments use `/* ... */` and may be nested:

```flow
/* outer comment
   /* nested comment */
*/
```

Newlines normally separate statements. Braces delimit blocks; indentation is
for readability.

## 4. Declarations and types

A local declaration has the form:

```text
name : Type(initializer)
```

Examples:

```flow
count : int32(0)
ready : Bool(false)
text  : string("hello")
```

The current examples use explicit-width `int32` and `Bool`. Other names may be
declared by the program, imported from a unit, or supplied by an ABI surface.
Written type spellings matter: `int32`, `Bool`, `Point`, `Percent`, and `c_int`
are distinct source names. Bare `int` is rejected; choose a concrete language
width such as `int32` or `int64`. `c_int` remains a separate foreign/host ABI
contract and is not an alias for a language integer.

Boolean literals are `true` and `false`; constructor-style initializers such as
`Bool(false)` also occur in current examples. String literals use double quotes
and support `\n`, `\t`, and `\r` escapes.

## 5. Expressions

FlowMini expressions include identifiers, integer and floating-point literals,
strings, Boolean literals, unary `not` and prefix `-`, arithmetic operators
`+ - * / %`, comparisons `< <= > >= == !=`, parentheses, calls, indexing,
field access, list literals, and record literals.

For integer operands, `div` is integer quotient with truncation toward zero.
`/` is true division and produces `float64`, even when the quotient is exact.
The `%` operator remains reserved until its backend contract is admitted.

```flow
result : int32(0)
square(add(3, 4)) -> result

value : int32(0)
(a + b) * c -> value
```

Calls, indexing, and field access compose as postfix expressions. Examples are
`matrix[row, col]` and `person.address.city`.

## 6. Placement, assignment, and return

The central data-movement form is:

```text
expression -> target
```

Current assignable targets are identifiers, field paths, and indexed targets:

```flow
value -> name
value -> record.field
value -> list[index]
value -> matrix[row, col]
```

For example:

```flow
arr : list<int32>([1, 9, 3])
i   : int32(1)
x   : int32(0)

arr[i] -> x
42 -> arr[i]
```

Equals assignment also exists in the structural/provisional language surface:

```flow
x = expression
```

A function can return with either current spelling:

```flow
fn square(n : int32): int32 {
    n * n -> return
}

fn cube(n : int32): int32 {
    return n * n * n
}
```

The canonical teaching form is the arrow: it reads as “evaluate this, then
place the result there.” Use equals assignment only where the active consumer
explicitly supports that compatibility form. `return` is control transfer, not
an assignable variable.

## 7. Functions

Functions use `fn`, a name, typed parameters, a return type, and a braced body:

```flow
fn add_then_square(a : int32, b : int32): int32 {
    sum : int32(0)
    a + b -> sum
    square(sum) -> return
}

main {
    result : int32(0)
    add_then_square(3, 4) -> result
    print result
}
```

Parameters are named and typed. Functions may call local or imported
functions. Keep the return path explicit with `-> return` or `return`.

## 8. Records and field access

Define a record with `type` and `field`:

```flow
type Point {
    field x : int32
    field y : int32
}
```

Construct and use it with named fields:

```flow
p : Point({x:10, y:32})
value : int32(0)
p.x -> value
42 -> p.x
```

Record fields are contextually checked against their declared types. The field
schema carries the width; a numeric literal in a record value is not written as
`int32(10)` because that spelling is a declaration initializer, not an
expression cast.

Records can be passed to functions:

```flow
fn sum_point(p : Point): int32 {
    p.x + p.y -> return
}
```

## 9. Lists and arrays

Declare a list with an element type and list literal:

```flow
numbers : list<int32>([1, 9, 3, 5])
i : int32(1)
x : int32(0)
numbers[i] -> x
42 -> numbers[i]
```

Shaped arrays include extents:

```flow
matrix : array<int32>[2, 3]([1,2,3,4,5,6])
row : int32(1)
col : int32(2)
value : int32(0)
matrix[row, col] -> value
```

The current examples use zero-based indexes. The programmer must supply an
initializer and indexes appropriate to the value.

## 10. Conditions and loops

```flow
if x == 0 {
    print 10
} else if x == 1 {
    print 20
} else {
    print 30
}
```

The `else` branch is optional. `while` repeats a braced block:

```flow
i : int32(0)
five : int32(5)
one : int32(1)
keep_going : Bool(true)

while keep_going {
    print i
    i + one -> i
    i < five -> keep_going
}
```

`break` exits the current loop. `continue` skips to its next iteration:

```flow
while condition {
    if finished {
        break
    }
    if skip_this {
        continue
    }
    work()
}
```

## 11. Refined types

A refined type names a base type and adds invariant clauses:

```flow
type PositiveInteger refines int32 {
    invariant value > 0
}

type Percent refines PositiveInteger {
    invariant value <= 100
}

main {
    load : Percent(87)
}
```

An invariant is part of the type’s contract. Do not assume that every possible
semantic check, conversion, or runtime enforcement is available on every path.

## 12. ABI declarations

An `abi` block describes an external library boundary:

```flow
abi testabi {
    library "./build/libflowmini_testabi.so"
    convention c

    struct Point {
        x : c_int
        y : c_int
    }

    extern fn point_sum(p : Point): c_int {
        symbol "point_sum"
        effect pure
    }
}
```

An ABI block may contain a library path, calling convention, ABI structs, and
extern functions. Names such as `c_int` describe the foreign boundary and are
not automatically interchangeable with every FlowMini type.

After importing an ABI unit, an extern function is called normally:

```flow
result : c_int(0)
point_sum(point) -> result
```

The library must exist and export the declared symbol for the call to work.

## 13. Output and capabilities

Current examples use:

```flow
print value
```

Treat this as the current convenient output spelling. Conceptually, output is a
capability/provider operation, and ordinary calls such as `print(value)` may be
used where the available provider surface supports them. Do not infer a larger
standard library from the keyword alone; use the functions and units actually
provided by the environment.

## 14. Complete example

```flow
program larger_demo

fn larger(a : int32, b : int32): int32 {
    if a > b {
        a -> return
    } else {
        b -> return
    }
}

main {
    values : list<int32>([4, 9, 2])
    first : int32(0)
    second : int32(0)
    result : int32(0)

    values[0] -> first
    values[1] -> second
    larger(first, second) -> result
    print result
}
```

This combines a program, function, list, indexing, placement, conditionals,
return, and output.

## 15. Compact reference

```text
program name
unit name
import "path/to/unit.flow"

main { statements }
target name { main { statements } }
fn name(param : Type, ...): ReturnType { statements }
type Name { field member : Type ... }
type Name refines BaseType { invariant expression ... }
abi name { library "path"; convention name; ... }

name : Type(initializer)
expression -> target
expression -> return
name = expression
if expression { ... } else if expression { ... } else { ... }
while expression { ... }
break
continue

list<Type>([values])
array<Type>[extent, ...]([values])
record.field
value[index]
function(arguments)
```

When learning, begin with a small `program`, add one typed declaration at a
time, and keep expression-to-target placement explicit. The current `pass/`,
`support/`, and `std/` examples are the companion reference for combinations of
these forms.
