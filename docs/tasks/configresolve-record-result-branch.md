# ConfigResolve record-valued branch returns

ID: `lyraform.configresolve-record-result-branch`; revision 1. This bounded
mission continues the finite fact-resolution work on
`experiment/configresolve-lyraform`.

## Measurable goal

An ordinary finite record returned from either side of a source conditional
must remain an independent value when consumed by another function. Native LLVM
and TinyVM must agree for both branch outcomes, with no uninitialized storage.

## Generic boundary

TinyVM allocates one stable private result tree for a record-returning callable
and copies each control-flow return into that tree. The rule is generic and
does not encode ConfigResolve names or policies. Member writes, resources,
recursive records, and public layout remain outside the mission.

## Evidence

`experiments/configresolve/record-result-branch.flow` and
`experiments/configresolve/run-record-result-branch-probe.py` exercise both
branch outcomes. The dated checkpoint records parity and remaining scope.
