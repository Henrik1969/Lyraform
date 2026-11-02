# Checkpoint: tagged scalar Value boundary selected

The ConfigResolve reference requires an exclusive five-alternative scalar
value: null, Boolean, integer, finite or policy-governed floating point, and
string. Ordinary records can carry the fields but cannot enforce that exactly
one payload is active, so they are not an honest substitute.

The next implementation boundary is therefore a generic closed tagged carrier.
The first executable slice may cover Null, Bool, Int, and Text; Double remains
refused until its unresolved equivalence and admissibility choices are decided.
The existing Text outcome carrier supplies backend and ownership evidence but
does not define the general Value type.

No source syntax or runtime shortcut was added in this checkpoint. The mission
state records the required semantic questions before implementation.
