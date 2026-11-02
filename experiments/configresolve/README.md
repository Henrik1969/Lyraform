# ConfigResolve in Lyraform — branch experiment

Owner-selected application port; implementation belongs here, outside the language
base. Branch: `experiment/configresolve-lyraform`. Comparison/merge is deferred.
[Mission](../../docs/tasks/lyraform-language-maturation.md) governs the campaign.

`reference-manifest.json` pins the inspected ConfigResolve working tree, including
uncommitted documentation and experimental test inputs. `contract-matrix.json`
maps every public header, source and test file into behavior families. It is a
coverage inventory, not a claim that every API has been ported or approved.
A bounded two-integer-fact algorithm now runs as native and TinyVM binaries.
See [probe checkpoint](../../docs/checkpoints/2026-10-17-configresolve-scalar-probe.md).
This does not complete any full behavior family in the inventory.

## Compatibility boundary

Preserve documented stable behavior; use current implementation as an oracle only
after classifying each observation. Defaults are facts; schema validates shape;
policy selects resolution; AccessPolicy governs mutation/export/secrets/reset.
No application meaning belongs in the compiler, backend or generic runtime.

Unresolved: floating equivalence/NaN/infinity/signed-zero rules; winner exposure
on failed resolution; target-language identity/lifetime requirements for stores
and views; host ABI/packaging compatibility. Do not settle these through a test
oracle alone. Integer/string-only probes can proceed independently.

## First executable probe (verified)

Use ordinary already-admitted scalar functions to evaluate two integer candidates
for one already-valid key. Compare unique highest-precedence, explicit FirstWins
and LastWins, equal-priority equal values and ambiguity diagnosis. Return selected
candidate identity and diagnostic status through separately tested scalar queries;
this is a lowering/algorithm probe, not the public Result representation or full
port. Test both permutations and values not used when authoring the source.
Reference harness calls the existing C++ resolver; the Lyraform binary must not
link or call it. Any compiler changes must be generic and independently exercised.

The bounded mission and exact observable contract are recorded in the checkpoint. Full records, arbitrary fact sets, loaders, store/view
lifecycle and failure-routing closure remain required subsequent work. A small
working executable does not complete this campaign.
