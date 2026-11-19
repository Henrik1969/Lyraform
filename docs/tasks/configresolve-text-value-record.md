# ConfigResolve text value records

ID: `lyraform.configresolve-text-value-record`; revision 1. This bounded
mission extends the finite ConfigResolve port with the scalar `Text` value
needed by `Value` and provenance-facing records.

## Measurable goal

An ordinary record containing a `Text` field and a `Bool` field must pass the
canonical source pipeline, produce a real native binary, and execute with the
same result in TinyVM with no trap. The compiler and runtimes must remain
generic; no ConfigResolve name may select behavior.

## Boundary

Text is admitted only as an immutable finite record field or literal in this
mission. Text mutation, dynamic allocation policy, resource cleanup, tagged
unions, collections, and the full `Value` API remain separate decisions.

## Evidence

`experiments/configresolve/text-record.flow` and
`experiments/configresolve/run-text-record-probe.py` provide the parity probe.
