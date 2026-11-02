# Entry result and host mapping — proposal for owner review

Date: 2026-11-01. Parent: [stabilization G2](contract-stabilization-g2.md).
Model authority: [ADR 0073](../architecture/decisions/0073-explicit-entry-result-host-mapping.md).
**This is a concrete design proposal, not implemented or accepted syntax.**

## Recommended boundary

1. Main declares its language result using the same result annotation as an
   ordinary function: proposed `main(): int64 { ... }`.
2. A mapping is an ordinary, explicitly typed function. Its parameter must have
   exactly the main result identity. It receives the full result once.
3. The explicitly selected target's chain policy names the mapping function
   and its host contract. Selection resolves to identities during analysis;
   backends cannot choose by name or insert a default.
4. A generated host wrapper invokes main and then the selected mapping. It
   returns only a value admitted by the selected host contract. The mapping
   result must match the host ABI result identity exactly. That ABI identity
   alone does not prove that the externally observed status preserves it.
5. Missing mapping, ambiguous selection, incompatible target, unaccounted result,
   unsupported route, or unproved host representability prevents publication.

Main's result is a language value, not inherently an exit code. A source that
chooses int32 is not changed to int64. A host's use of c_int is not an alias for
int32, even on a target where their carriers have the same width.

## Source example

Only the main result annotation below is a proposed extension. The ordinary
function, comparison, branches, typed parameter and return forms already exist.
The sample explicitly classifies an application result into two status values;
it does not numerically convert 42 into 0. Those choices belong to this sample,
not to Lyraform or an implicit compiler policy.

```text
program result_demo

fn process_status(result : int64): c_int {
    if result == 42 {
        0 -> return
    } else {
        1 -> return
    }
}

main(): int64 {
    result : int64(42)
    result -> return
}
```

A different application's mapper may perform an explicit exact numeric
conversion. That conversion follows ADR 0072. A dynamic int64-to-c_int conversion
cannot become executable merely by placing it inside a mapper. If the value
cannot be proven in range and the language lacks its admitted runtime failure
route, refuse it. No masking, modulo, saturating conversion, private trap, or
fallback exit code is generated.

## Selection proposal — review required

Reuse ADR 0046's target chain-policy direction, rather than adding a special
mapping body, handler language, or compiler-known application function name.
Its schema is provisional, so the following fields are a proposed extension,
not a claim about current tooling:

```json
{
  "entry_result": {
    "mapping_function": "process_status",
    "host_contract": "<explicit selected host contract identity>"
  }
}
```

This object belongs to one selected target. The contract identity is deliberately
not assigned a fabricated standard name here: the implementation must bind it to
an inspected provider/host contract, or refuse selection. The contract must
state its ABI result identity and the admitted externally observable status
values. For the first executable specimen, require a selected host contract
that preserves both 0 and 1; do not generalize that evidence to all c_int values
or all operating systems.

Function lookup happens once in the selected target's lexical/import scope.
The semantic report carries resolved main, mapping-function and host-contract
identities, signature, target and provenance. Later consumers validate these
facts. A CLI may select an existing target/policy, but must not silently select
or synthesize a mapping.

## Why this choice

- Ordinary function semantics own the computation; policy only selects it.
- The full language result remains available to the mapper on LLVM and TinyVM.
- Source reads clearly; the target policy exposes the host-specific choice.
- There is one mapping invocation after normal main completion, not a second
  error-handling system. Existing failure/disposition obligations still apply.
- Different targets can select different explicit mappings without editing the
  application computation or changing its integer width.

An alternative is to put the selector in the source target declaration. That
makes the selection visible in the same file, but requires another source
construct and couples that source target to the selected host contract. The
recommended first implementation uses explicit target policy selection and
reports that selection in build provenance.

## Implementation accounting

- Frontend: add optional return_type to MainBlock; use the existing signature
  parser and type-reference projection. Reject missing explicit result types
  for the new entry contract; do not infer them from returned expressions.
- Analysis: remove the inserted c_int result for this contract; resolve the
  mapper and host contract and require exactly one compatible parameter.
- Shared admission: validate identities, exact signature, target ownership,
  mapping availability, admitted result domain and failure obligations.
- Native: emit the language main as a typed internal callable; emit a separate
  host wrapper. Graph entry variants must invoke the same selected mapping.
- TinyVM: preserve the typed entry result and invoke the same mapping; distinguish
  full language value from host process-status observation in tests.
- Compatibility: mark old implicit-main behavior historical. Migrate active
  executable fixtures to explicit selections; never count the old path as
  proof of the new model. Do not blanket-convert all existing mains to int64.

No new numeric failure carrier or routing spelling is included. If the existing
host-contract machinery cannot carry the necessary domain, record that exact
schema gap for review; do not silently turn the placeholder into a new contract.

## Falsifiable acceptance evidence

Positive: int64 main returns 4294967297; mapper distinguishes it from 1 and
returns a permitted status. Native O0/O2 and TinyVM must observe the same full
value and selected status. Also exercise int32 main, explicit nonnegative exact
conversion where provable, two targets with different mappings, and graph entry.

Negative: missing explicit main type or mapping, wrong parameter identity,
mapper result/host ABI mismatch, unresolved contract, out-of-domain status,
unproved dynamic narrowing, duplicate/ambiguous selection, unaccounted failure.

Hostile imports: change mapping function, main result, parameter/result types,
host contract, target ownership, status-domain evidence and provenance; strip
new contract evidence or downgrade artifact versions. Every executable consumer
must refuse before publication, regardless of optional readiness claims.

## Decision requested

Approve this bounded extension: **main uses an ordinary result annotation;
the selected target's explicit chain policy selects an ordinary mapper and
host contract; no implicit mapping or conversion is supplied.**

This review approves the declaration/selection design, not a promise that
unimplemented numeric failure routes or every host contract can execute.
