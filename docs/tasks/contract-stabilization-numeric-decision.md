# Contract stabilization: numeric authority decision

Parent: [stabilization mission](contract-enforcement-stabilization.md).
Status: owner clarified conversion, overflow and routing authority on 2026-11-01.
Latest authority requires explicit-width concrete integer types; bare int is refused. Earlier answers below remain historical provenance.

## Explicit conversion implementation instruction

[ADR 0072](../architecture/decisions/0072-explicit-value-preserving-integer-conversion.md)
records the owner's final clarification and instruction to implement it: both
widening and narrowing require visible explicit intent. A conversion must preserve
the exact value or follow its declared failure route. No implicit conversion at
assignments, calls, returns, arithmetic or host boundaries is authorized.

## Latest owner ruling — explicit widths required

[ADR 0071](../architecture/decisions/0071-explicit-integer-width.md) supersedes
the earlier default-alias answer below. The owner approved requiring concrete
width-bearing integer names and diagnosing bare `int`. Readers must not need to
infer a type's width from the target or an unseen default policy. Retain the
conversion and overflow-routing decisions. Review uses before migration; do
not blindly replace every int with int64. The separate main/host-status decision
is still pending; the owner requested tradeoffs but has not selected a model.

## Owner clarification — 2026-11-01

The owner states that semantics must be honest: the developer gets what is
written under the invoked contracts and policies. Primitive types have defaults.
Type conversion requires an explicit policy or clearly expressed intention.
An unspecified unsuccessful graph path is an admission problem, not authority
for the compiler to choose a fallback.

This resolves the governing principle. It does **not** authorize replacing the
language with a common-range subset, selecting arbitrary primitive defaults,
or introducing implicit conversions. No such approval is inferred.

ADRs [0055](../architecture/decisions/0055-refusal-failure-fault-dispositions.md),
[0056](../architecture/decisions/0056-no-dangling-failure-or-fault-wires.md), and
[0062](../architecture/decisions/0062-typed-failure-consumers-and-policy-routing.md)
already establish the matching route law: possible unsuccessful dispositions
must have a declared accountable destination; policy selects among authorized
routes; a backend cannot invent a destination or numeric meaning.

## Owner overflow ruling — 2026-11-01

The owner answered the concrete overflow question:

> What you infer is that a int overrun is found. That must follow a conversion
> rout to a larger int type or fail

Applied together with the earlier conversion ruling, an overrun must follow an
explicitly authorized conversion route to a larger integer type or the declared
failure route. It cannot silently wrap, truncate, widen, or use a backend trap
as a substitute for that route. Missing or incompatible routing is an admission
refusal under ADR 0056. An available wider host carrier is not authorization to
change the source type.

## Owner default-width ruling — 2026-11-01

The owner explicitly selected 64 bits for standard `int`, independent of target,
and prefers names showing width such as `int32` and `int64` because target-varying
`int` meanings obstruct porting. Thus `int` resolves to signed `int64` with range
−9223372036854775808 through 9223372036854775807. Written spelling remains in
raw AST. No new `short` spelling is introduced. Explicit conversions and the
above overflow routes remain required. ABI contracts retain their own identities.
This supersedes the provisional int alias entry, not provisional floating laws.

The [binding type policy](../flowmini/v0.24-type-policy.md) and
[scalar authority](../architecture/canonical-scalar-semantics-v1.md) remain
relevant to type identity and provider conformance. The owner's explicit ruling
now supplies the overflow-disposition law absent from those records. It does not
add source syntax, a numeric failure type, or an executable conversion route by
itself. Implement or refuse the existing declared paths; never invent them.

## Conversion reconnaissance after owner clarification

Four fresh canonical source probes were run with the current GCC tools.
`x : int(7); y : c_long(0); x -> y` is accepted and emits a conversion tagged
`explicit_typed_initializer`, although the source operation is a placement.
Returning an `int` local from a `c_long` function receives the same tag.
An actual `y : c_long(x)` initializer receives that tag as well. A cross-type
function argument is accepted by analysis without that conversion projection.
These are observations, not approval of the conversion or proof that every
backend executes it. The first two reveal that the tag alone cannot prove
explicit source intention. Raw probes are preserved in mission evidence.
This is G2 reconnaissance; it does not pass G1 or bypass gate order.

## Accepted-law register at this checkpoint

| Area | Accepted meaning / authority | Implementation coverage | Open or unsupported |
|---|---|---|---|
| Scalar types | Canonical scalar authority: `int` and `Bool` have distinct identities; shared operator type algebra | `scalar_semantics.hpp`, frontend adapter, imported scalar-fact validation | Owner selects signed int64 default and explicit widening-route-or-failure overflow law; G2 implementation pending |
| Type aliases and carriers | Binding v0.24 policy inherited by v0.25; native storage only implements approved language contracts | Existing ABI and backend carriers | `int` is now int64; `float` remains provisional |
| Refinements | A refined type retains its base and ordered invariant obligations; binding type policy | Declaration inspection; first-priority work rejects value uses with unenforced predicates, including inherited predicates | General invariant construction/placement enforcement remains unsupported |
| Guards | Existing static lifecycle checks and their accepted source forms | Existing static guard tests remain positive/negative controls | No new dynamic guard handler or disposition inferred |
| Division | ADRs [0054](../architecture/decisions/0054-failure-is-explicit-flow.md) and [0055](../architecture/decisions/0055-refusal-failure-fault-dispositions.md): explicit dispositions; unknown safety does not authorize execution | First-priority work proves a bounded set of constant-safe divisors and refuses unknown/unsafe cases in shared artifact consumers | Dynamic routes not implemented; private exits and backend traps are not a replacement contract |
| Ownership and recovery | Existing accepted Text ownership, failure consumer, containment and source-disposition authorities remain binding | Existing tests and captured facts; not a new full-stack completion claim | Mission G4 must still establish actual source execution on LLVM and TinyVM |

Proposed meanings stay proposed. Implementation observations stay implementation
observations. Refusal is temporary support coverage, not a new language rule.

## Consequences already observed

The first-priority safety checks expose existing pager examples that divide by a
provider-supplied page size without a compiler-established safety proof. Those
examples are currently refused. The pass-corpus also contains cases newly
refused by the safety checks. These integration failures remain visible; tests
have not been changed to claim those applications still execute.

[G1 closure](../checkpoints/2026-11-01-contract-stabilization-g1.md) records its bounded safety result. The full mission
requires resolving its admission/coverage failures and running the required
Firetest, then completing G2 through G5 in order. A passing focused test alone is
not closure.

## Entry-result decision update — 2026-11-01

The owner approved [ADR 0073](../architecture/decisions/0073-explicit-entry-result-host-mapping.md):
an explicitly typed language result with an explicitly selected host mapping.
The earlier open model choice is resolved. The owner requested a concrete
mapping proposal before adding declaration syntax; that review remains open.
