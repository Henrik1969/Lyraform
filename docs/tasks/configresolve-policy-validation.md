# ConfigResolve finite policy validation

ID: `lyraform.configresolve-policy-validation`; revision 1. This bounded
mission follows finite fact resolution on `experiment/configresolve-lyraform`.

## Measurable goal

Lyraform source must carry a finite `Policy` record with required/range fields,
validate admitted integer facts before conflict resolution, and return explicit
valid, out-of-range, and missing outcomes. Native LLVM and TinyVM must agree.

The slice is limited to integer range validation. It does not claim tagged
ConfigResolve values, strings, diagnostics, loaders, stores, views, bindings,
or the public C API.

## Generic boundary

Policy and fact names remain ordinary source records. The compiler only handles
generic typed record construction, projection, calls, and control flow; no
ConfigResolve-specific dispatch or runtime shortcut is introduced.

## Evidence

Source: `experiments/configresolve/finite-resolution.flow`.
Runner: `experiments/configresolve/run-finite-policy-probe.py`.
