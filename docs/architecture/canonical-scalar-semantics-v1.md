# Canonical scalar semantic authority, Phase 2

This is the bounded compiler-unification slice implemented for Mission 02.
It is subject to Gate 2 review. It does not define the rest of Lyraform v1.

## Admitted scope

The source type identities are exactly `int` and `Bool`. Evidence comes from
the active parser's explicit initializer checks, `canAssignType`, integer and
boolean examples, and the same-revision declaration reconnaissance. ABI
carriers, other numeric widths, aliases, refinements, Text and aggregate types
are not added to this semantic slice. `c_int` and `bool` in existing serialized
operands remain backend representations; they are not new source aliases.

Covered destinations are local declarations with explicit initializers and
subsequent identifier placements into those declarations. Covered source
expressions are integer/boolean literals, resolved scalar identifiers, and
the existing scalar unary/binary operators whose input and result types can
be established within this slice. Literal forms of other kinds are refused
as initializers or values for these destinations. No implicit Bool/int
conversion is admitted, including inside an expression.

Declaration-free writes, writes preceding the destination declaration, and
self/forward reads in this bounded analysis are refused. This is a lexical
establishment check, not control-flow definite-initialization analysis.
Declarations without initializers, member/index destinations, calls, ABI
conversions, and other expression/type families remain outside this phase's
proof. Their existing paths continue to have their existing support limits;
absence of a scalar fact is not a positive scalar-admission result. In
particular, a placement whose declaration initializer is outside this slice
does not acquire a Phase 2 fact.

## Shared rules and adapters

`Flowcontracts/include/flowcontracts/scalar_semantics.hpp` contains the
`lyraform::scalar` type identity, operator-result and compatibility rules,
plus an origin-bearing `Fact`. It is a small header component, independent
of JSON, AST layouts, backend carriers and application names. Both
Flowanalyst and the compatibility parser invoke these rules. The legacy
parser retains its syntax, runtime and out-of-slice rules.

`Flowanalyst/src/scalar_analysis.hpp` adapts the structural AST, symbol/scope
resolution and source map to that component. It computes source types before
operand emission can apply a destination carrier. A refusal becomes
`FLOWANALYST_SCALAR_FLOW_REFUSED`, exit 2, semantic report `error`, and a
non-ready lowering plan. Diagnostics point to the statement and originating
file/line/column. Reports still contain inspectable refused facts where an
operation exists; those facts cannot authorize execution.

This is partial convergence: primitive scalar rules are shared, while source
parsing, general name resolution, functions, aggregates and runtime behavior
still have the separate authorities recorded in the Phase 1 report.

## Versioned projection

A covered `value_definition` or `assignment` operation carries `scalar_fact`:

```json
{
  "format": "lyraform.scalar_fact",
  "version": 1,
  "kind": "declaration",
  "statement_id": 0,
  "declaration_statement_id": 0,
  "expression_id": 0,
  "destination_symbol_id": 2,
  "source_type": "int",
  "destination_type": "int",
  "compatibility": "admitted",
  "provenance": {
    "source": "example.flow",
    "ast_path": "/statement_pool/0",
    "line": 3,
    "column": 5
  }
}
```

For a placement, `kind` is `placement` and `declaration_statement_id` links
to the initialized destination's declaration. Identities are local to the
captured frontend/report, not stable across separate analyses. The initializer
identity is `expression_id` on a declaration fact. Compatibility is either
`admitted` or `refused`; refused/outside types are never treated as a proof.

This independently versioned optional field uses the existing additive
lowering-plan v1/v2 policy. Existing format IDs and envelope versions are
unchanged. Captured artifacts lacking this extension remain compatible and
make no claim to Phase 2 proof. This is not authentication: removing all new
evidence can produce a legacy artifact, and fully rewriting consistent input
facts describes a different program. Requiring proof on all imported programs
would need a separate version/admission-policy decision at a later gate.

`scalar_facts.hpp` serializes facts and checks imported evidence using the
same type algebra. Checks include version, compatibility, declaration and
operation identities, destination type consistency, operand type consistency,
known referenced declaration types and required provenance. Flowbind,
Flowparallel, Flowoptimize, independent `flowvalidate`, `flowprepare`, LLVM and
TinyVM reach this validation through their existing plan/artifact boundaries.
An outer `ready`/`ok` flag does not override a retained refused fact.

## Stage borders and backend limits

Frontend export/import, semantic analysis, binding, planning, optimization,
backend-artifact capture, and each backend remain separately invocable.
Facts live in the preserved lowering plan, so saving and resuming at these
borders retains the same semantic decision. No Igor commands were introduced.

A backend may still refuse a valid expression. For example, the legacy
boolean `not` operation has a shared semantic result, while the current LLVM
lowerer does not implement that unary operation. This phase does not add
backend operator support.

The identifier-initializer probe also records an existing compatibility
runtime limitation: `lowerExprToPath(identifier)` returns the source path,
while scalar declaration callers ignore that return and do not copy into the
new destination. A later read can fail with `missing record path`. The shared
type check accepts the same-type initialization; the LLVM/TinyVM test verifies
its result independently and records the legacy failure explicitly. This
mission does not change that runtime lowering behavior.

## Verification

`lyraform_scalar_semantics` exercises the shared type/compatibility algebra.
`lyraform_scalar_authority` tests twelve invalid cases with both lowering-plan
versions, four valid LLVM/TinyVM result comparisons with both versions
(three legacy matches and one recorded legacy initializer failure),
deterministic analysis, independent canonical artifact round-trips, preserved
facts, mapped source origins, and six malformed or contradictory captured
artifacts refused before executable publication. The canonical suite also
retains the earlier captured-artifact, ABI, graph, callable, Text and runtime
coverage. Exact execution results are in the Phase 2 Gate 2 report.
