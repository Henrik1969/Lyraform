# Canonical authority envelope — Stage 4B

Date: 2026-09-27.

## Objective

Take the remaining current callable-plan-v2 semantic facts across every active
carrying boundary without creating a parallel model or expanding language
meaning. Every fact must be preserved in the canonical envelope, deliberately
consumed by a separately validated branch, or remain an explicit refusal.

## Authority disposition

| Fact family | Representation and Stage 4B disposition |
|---|---|
| source identity | top-level `source`; preserved exactly and structurally validated |
| source-operation coverage | inside `lowering_plan`; preserved and linked to exact operation identities |
| scalar declaration and placement facts | inside lowering operations; preserved and revalidated by lowering authority |
| target and member-path facts | top-level target catalog plus operation facts; preserved and identity-linked; unsupported execution remains refused |
| guard lifecycle and proof | inside lowering operations; preserved and revalidated; runtime-dependent transitions remain refused |
| disposition and TextOutcome accounting | inside lowering operations; preserved with obligation and ownership relationships |
| ownership transfer | inside `lowering_plan`; preserved and revalidated under the bounded direct/one-hop law |
| provider/effect authority | semantic requirements are consumed by Flowbind's separately validated policy branch; exact effect evidence is preserved by carrying stages |
| source-call and deferred-candidate authority | top-level exact arrays plus lowering operations; preserved and recomputed as established by Stage 4A |
| ABI type contracts and aggregate layouts | top-level exact arrays; preserved and structurally validated; an empty plan-v2 layout set remains explicit |
| schedule authority | no schedule admitted; deferred candidates grant no execution authority |
| plan v1 | historical compatibility; no Stage 4B envelope claim and no wire-shape change |

Flowparallel and Flowoptimize preserve the canonical envelope. Flowprepare now
retains even an empty `aggregate_abi_layouts` array for plan v2, because an
explicit empty authority set is not equivalent to silently dropping the field.
Its plan-v1 omission behavior remains unchanged for compatibility.

## Closed defects

- optimization-report validation now checks source provenance shape, the target
  catalog, and ABI contracts rather than merely requiring a target array;
- lowering-plan validation links source-operation coverage to the exact carried
  operation set at every consumer;
- backend preparation no longer discards an empty plan-v2 aggregate-layout
  authority set;
- the preservation gate compares the full current authority envelope and
  attacks source, target, ABI, aggregate-layout, source-coverage, scalar, and
  target identities at downstream consumers;
- LLVM and TinyVM receive the same validated prepared authority; neither may
  reinterpret a hostile mutation as valid.

## Non-carrying and refused routes

Flowbind remains a non-carrying authorization branch. It consumes the exact
canonical provider requirements, applies policy, and emits capabilities whose
identities are paired with lowering operations by backend validation. It is not
a second language-semantic authority.

Member execution, ordinary aggregate construction, runtime-dependent guards,
general failure routing, and runtime parallel scheduling remain unsupported or
decision-blocked exactly as before. No syntax, source extension, ABI surface,
aggregate construction law, failure behavior, or concurrency meaning changed.

## Verification

- focused preservation and adjacent boundary suite: **5/5 PASS**, 10.77 seconds;
- hostile source, target, ABI, layout, source-coverage, scalar, and target-fact
  mutations: refused before output publication;
- exact semantic → execution → optimization → prepared-backend comparison:
  byte-identical for both call/candidate and scalar/target fixtures;
- canonical integration build and suite: **172/172 PASS**, 82.06 seconds;
- fresh GCC 13.3.0 debug build and suite: **172/172 PASS**, 71.14 seconds;
- fresh Clang 18.1.3 debug build and suite: **172/172 PASS**, 56.66 seconds;
- ASan/UBSan hostile end-to-end preservation gate: PASS with
  `ASAN_OPTIONS=detect_leaks=0`; LeakSanitizer remains unavailable under the
  host ptrace wrapper, so leak evidence is supplied separately by Valgrind;
- Valgrind 3.22.0 Flowoptimize, Flowprepare, LLVM Flowlower, and TinyVM lowering:
  **0 errors**, 0 bytes in 0 blocks at exit for each consumer;
- repeated Flowparallel execution-plan, Flowoptimize optimization-report, and
  Flowprepare backend-artifact emission: byte-identical;
- current-documentation, canonical-authority, and legacy-deprecation guards:
  PASS;
- Igor doctor: PASS;
- repository diff whitespace check: PASS.

The first attempt to construct fresh GCC and Clang trees concurrently exhausted
the host `/tmp` filesystem and both compilers stopped with `No space left on
device`. Only disposable earlier Lyraform build trees were removed. The fresh
builds were then repeated sequentially from configuration and both completed
with the results above. The builds retain the pre-existing unused
`json_escape` warning in `Flowparallel/src/graph_cuda.cpp`; Stage 4B did not
touch that file or introduce a new warning.

## Gate 4B

**PASS.** The complete current callable-plan-v2 authority envelope crosses all
active carrying stages without silent discard or downstream reinterpretation.
Policy authorization remains isolated, plan-v1 compatibility is unchanged, and
unsupported meaning remains fail-closed.
