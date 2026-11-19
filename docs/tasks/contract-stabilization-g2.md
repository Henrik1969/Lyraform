# Stabilization G2 — numeric and predicate agreement

Parent: [mission revision 1](contract-enforcement-stabilization.md).
Prerequisite: [G1 passed](../checkpoints/2026-11-01-contract-stabilization-g1.md).
Selected under the owner's existing mission execution instruction.

Implement the [owner's numeric rulings](contract-stabilization-numeric-decision.md):
explicit-width concrete integer types (ADR 0071), no bare-int default, no
implicit integer-identity conversions, authorized overflow conversion or
failure route, otherwise admission refusal. Bounded arithmetic promotion is
admitted only where its operator contract explicitly says so.
Do not add syntax, short aliases, numeric failure types or invented host policy.

Inventory literals, arithmetic, conversions, parameters/results, records,
control flow and predicates. Reuse shared scalar semantics and artifact checks;
remove accidental backend width and conversion choices. Preserve raw spelling.
Verify source, hostile imported artifacts, native O0/O2 and TinyVM full results;
keep unresolved families explicitly non-executable. Account separately for the
host ABI entry boundary before changing its carrier. A missing accepted host
mapping is a human decision, not permission to truncate language values.

Close only with the parent G2 inventory, zero unexplained differences or invalid
admissions, exact source evidence and required canonical/pressure checks.
