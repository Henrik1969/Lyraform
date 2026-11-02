# ConfigResolve typed resolution result

ID: `lyraform.configresolve-resolution-result`; revision 1. This bounded
mission extends the finite resolver on `experiment/configresolve-lyraform`.

## Measurable goal

Expose checked resolution through a typed `Resolution` record and execute its
branch-selected value through both native LLVM and TinyVM. The acceptance
program must return 33 with no TinyVM trap and must contain no ConfigResolve
specific compiler or runtime shortcut.

## Boundary

The record is a pure finite scalar record with private layout and independent
value copies. Member writes, resources, collections, diagnostics, and broader
ConfigResolve behavior remain separate decisions.

## Evidence

`experiments/configresolve/finite-resolution.flow` and the finite policy and
resolution probes provide the source and parity evidence.
