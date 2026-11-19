# ConfigResolve Lyraform experiment — end report

**Experiment branch:** `experiment/configresolve-lyraform`  
**Freeze date:** 2026-10-17  
**Acceptance target:** a real ConfigResolve port, beginning with its scalar
`Value` model  
**Status:** frozen at the declaration boundary; runtime carrier work is
intentionally deferred

## Why this experiment was useful

The experiment pressed Lyraform against an existing implementation instead of
allowing language design to remain self-referential. That exposed which parts
of the language were already usable and which apparent conveniences would have
been cheats.

The finite path is real: ordinary `Fact`, `Policy`, `Resolution`, and related
records can be constructed, copied, returned through functions, projected, and
executed with native LLVM/TinyVM agreement. Immutable `Text` fields also cross
that bounded record path. The current acceptance probes report 193/193 suite
checks passing, with the focused record, policy, resolution, and Text probes
passing on both backends.

## Main language lessons

1. **Real code separates records from exclusive values.** ConfigResolve's
   `Value` is a closed `std::variant`, not a record with a tag and optional
   fields. Ordinary records cannot enforce the one-active-alternative law.
2. **Text had to become a language value at the record boundary.** Treating
   strings as a globally selected scalar regressed unrelated scalar-flow
   behavior. The correct boundary is record-specific typed Text admission,
   with owned immutable semantics and no pointer identity.
3. **Backend agreement is a language test.** Native success alone was not
   enough. Function returns, branch results, copies, and Text values required
   the same observable result in LLVM and TinyVM.
4. **Bounded refusal is valuable evidence.** Unsupported collections,
   mutation, resource lifetimes, diagnostics, loaders, schemas, and floating
   point behavior remained explicit boundaries instead of being smuggled in as
   implementation shortcuts.
5. **The next abstraction must be generic.** A ConfigResolve-specific compiler
   special case would hide the language gap. The experiment therefore stopped
   at a generic closed `variant` declaration.

## What was frozen

Lyraform now parses and validates this generic declaration shape:

```text
type Value variant {
    case Null
    case Bool : Bool
    case Int : int
    case Text : Text
    case Double : float64
}
```

The parser enforces unique case names and known, non-sealed payload types. A
positive source probe and hostile duplicate/unknown-payload probes pass. The
declaration lowers alongside an independent integer main path, proving that
the new syntax does not disturb the existing graph path.

The freeze does **not** claim that `Value` can be constructed, projected, or
executed. Those operations remain refused until the runtime carrier exists.
`Double` is declared for fidelity with the reference model but remains outside
the admitted execution slice until finite values, signed zero, NaN, infinity,
and equality/ordering are specified.

## What the main branch should carry forward

- Keep the accepted semantics in
  `docs/architecture/decisions/0069-closed-tagged-scalar-carrier.md`.
- Keep the source-boundary evidence in
  `experiments/configresolve/tagged-value-declaration.flow` and its probe.
- Do not replace `variant` with an ordinary record encoding.
- Implement constructors, explicit case tests, checked projections, and
  independent copies as generic language operations.
- Admit `Null`, `Bool`, `Int`, and `Text` first; keep `Double` refused until
  its numeric contract is explicit.
- Require native LLVM/TinyVM parity before calling the carrier executable.
- Resume ConfigResolve only after the generic carrier is usable; loaders,
  stores, views, schemas, collections, and diagnostics remain later missions.

## Reproducible evidence

From the Lyraform project root:

```text
python3 ../Policies/tools/audit.py --root ..
python3 experiments/configresolve/run-tagged-value-declaration-probe.py <build-dir>
git diff --check
```

The policy audit at freeze is clean: 41/41 contexts, zero errors, and zero
evidence drift. The working tree remains intentionally dirty because this
branch contains the broader Lyraform maturation work; freezing this experiment
does not authorize a merge, commit, reset, or cleanup of unrelated changes.

## Final conclusion

The experiment succeeded as a language test even though it stopped before a
binary `Value` carrier. It proved a usable finite record/Text foundation,
identified the missing generic sum-type abstraction, and left a precise,
discoverable boundary for the main branch to continue from.
