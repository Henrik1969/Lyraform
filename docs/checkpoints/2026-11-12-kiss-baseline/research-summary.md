# Curated research findings and completed Slice 5 synthesis

Baseline: `9459705a9de14a91d62f796aa6bcd7f727fac165`.
This preserves the completed conversation reports as a concise synthesis;
it is not a newly executed semantic investigation or a verbatim report export.
Source references are repository-relative at that revision.

## Established observations

VERIFIED: ordinary domain records and operations execute (`A_float_calls`,
native/TinyVM result 6). Concrete replacement-style containers execute while
preserving the earlier independent value (`B_box`, result 8). Concrete Boolean
specialization works. Direct record construction bypasses a factory convention.
These establish useful composition, not genericity or representation hiding.

VERIFIED: all 52 Slice 2 source hashes matched the original manifest before
preservation. Thirteen saved paired native/TinyVM executions matched expected
results with zero TinyVM traps. Original results are preserved separately from
the fresh canonical 207-test publication run.

VERIFIED: bounded literal refinement arguments/returns pass; invalid returns
and assignments are refused. `consume(make())`, a refined local passed onward,
and an unchanged refined-parameter return encounter unproven obligations.
See Flowanalyst/src/main.cpp:1005,1263–1349. Existing checks do not yield a
complete reusable evidence chain; scalar compatibility facts are not predicate
proofs. No separate proof-authority subsystem was shown necessary.

VERIFIED: the stronger-to-weaker refinement probe is refused; the literal
control executes with result 9 and the invalid producer is refused. This does
not establish an implication calculus or prove that one must be added.

VERIFIED: swapped record arguments and uninitialized record use reach ready
plans and validators before backend refusal. The record call validator checks
result identity without the complete argument/signature relation; the read
check recognizes record_value but misses the demonstrated generic identifier
path. See Flowcontracts/include/flowcontracts/ordinary_record_operands.hpp:143,
250–256 and Flowanalyst/src/main.cpp:3717.

VERIFIED: replacing guard predicate IDs with nonexistent 999999 still passes
the examined artifact validator. Reference consistency and predicate truth are
different guarantees. This experiment does not bypass canonical source production.
See Flowcontracts/include/flowcontracts/guard_facts.hpp:44.

VERIFIED: named guard removal does not erase a refinement; a saved Boolean
condition does not enforce future mutations. Their lifecycles cannot simply
be merged. ADR 0053 and the D_combined/D_check_once probes preserve this distinction.

## Limitations and classification

- Missing evidence production/consumption: identical-refinement forwarding and
  unchanged refined returns. INFERRED implementation connection, not a proven
  missing language primitive.
- Missing canonical admission validation: incompatible record calls and
  uninitialized reads. Reuse existing catalogs and operand relationships.
- Admission/support mismatch: member placement reaches readiness before
  member/index execution refusal. ADR 0052 defines reconstruction under separately
  proven root-write authority; ADR 0068:32 does not admit member writes in its
  bounded construction slice. Backend work alone is not sufficient authority.
- Implementation boundary: some scalar/record literal constructions fail where
  helper-produced values work. Minimize and diagnose before extending semantics.
- UNRESOLVED: genericity, representation hiding, arbitrary implication,
  runtime-dependent enforcement and provider-dependent evidence continuity.
- Documentation qualification: the 2026-11-02 refinement-boundary checkpoint
  explicitly describes statically proved specimens. Its historical 164/202 test
  result is not the fresh publication result. Broad documentation drift was not
  demonstrated merely by comparing bounded examples with later ambitions.

## Architectural economy

VERIFIED: Flowbind's ordinary no-manifest successful path parses the same report
five times (Flowbind/src/main.cpp:211,234,644,654,787). CONSOLIDATE immutable parse
sharing, preserving errors and lifetimes. No speedup has been measured.

VERIFIED: validate_lowering_plan and validate_lowering_authority repeat common
scalar/record/coverage checks. CONSOLIDATE shared checks, but KEEP distinct
admission contracts and independently supplied artifact validation. See
Flowcontracts/include/flowcontracts/validate.hpp:39,127 and artifacts.hpp:500;
Flowlower/src/main.cpp:169 calls both paths.

VERIFIED: three sampled lowering plans remain equal through semantic, parallel,
optimized and prepared artifacts. Repeated serialization exists; independent
replay and envelope-specific authorization still serve distinct purposes.

VERIFIED: direct nongraph semantic input to Flowoptimize produced the same B_box
plan and byte-identical LLVM, with TinyVM result 8/trap 0. The newly generated
LLVM was compared, not newly linked in that experiment. Source graphs require
scheduling and are refused on this direct path (Flowoptimize/src/main.cpp:69).
DERIVE an optional eligible route; do not remove Flowparallel globally.

Measured implementation physical lines: Flowanalyst 4910; Flowcontracts 9421;
Flowbind 842; Flowparallel 1810; Flowoptimize 154; Flowlower including prepare
2097; frontend 13904; TinyVM 2709. Comments/blanks included; frontend includes
legacy; tests and external LLVM excluded. Size is not proof of redundancy.
The original selected-tool build contained 48 actions spanning 57.181 s;
summed overlapping action durations 155.121 s are not CPU time. Warm process
medians on one small case span roughly 1.4–7.9 ms. Runtime savings and large
program scaling are unmeasured.

KEEP independent source admission, provider/capability checks, scheduling,
artifact integrity, backend support, ownership accounting and fault containment.
Declared contracts, established facts, assumptions, runtime checks and unresolved
obligations must remain distinct. Formal correctness does not establish the
truth of a programmer's real-world assumptions.

## Minimum continuation roadmap from completed Slice 5

These are proposals only. Each ends at review; no implementation is authorized
by this archive. Correctness precedes optional economy work.

1. Reject incompatible ordinary calls before ready admission. Reuse function and
   record catalogs; preserve accepted conversions. Accept valid domain calls;
   reject swapped identities, wrong arity and forged/nested mismatches. Stop if
   new conversion or equivalence law is required.
2. Enforce initialization across operand representations. Reuse operations and
   read/dominance checks; invent no defaults. Accept B_box; refuse read-before-
   construction, nested uninitialized projection and invalid branch paths. Stop
   where existing control-flow initialization authority is insufficient.
3. Align unsupported target admission with write authority. Preserve replacement-
   style values; refuse unadmitted member/index placement at the responsible
   boundary. Do not silently enable backend writes. Review future write admission
   separately if it requires an unsettled rule.
4. Preserve/consume identical-refinement evidence. Reuse canonical identities,
   predicates, provenance and validity contracts. Accept producer/consumer,
   local forwarding and unchanged refined returns; retain bad-producer/argument/
   assignment refusals. Hostile cases include rebinding, effectful calls, same
   text with different bound identities and forged provenance. Stop before
   arbitrary implication, circular trust or new validity law.
5. Clarify predicate-reference validation. Determine where authoritative predicate
   data exists, then reject dangling/wrong-owner/stale references there. Do not
   equate reference integrity with proof. Stop for review if transport contracts
   require a broader decision. Coordinate with evidence production in step 4.
6. Diagnose one scalar/record construction mismatch. Compare the failed literal
   case with its helper-based equivalent under accepted numeric rules. Preserve
   width, precision and overflow behavior; test invalid field types and nesting.
   Stop if the supposed equivalent constructions have different existing meaning.
7. After correctness stabilizes, independently review parse-once Flowbind,
   shared validator checks and optional nongraph traversal. Preserve refusal
   behavior, authorization, scheduling and provenance. Test malformed/blocked
   artifacts and source graphs; measure before claiming performance benefits.

Risks: false type refusals, stale evidence, self-justifying contracts, missing
authorization, changed error ordering and loss of replayable provenance.
Dependencies and stop conditions constrain each proposal to existing authority.

## Preserved falsifications and open research

Parsing did not prove support; ready plus artifact-valid did not prove complete
source correctness; refined return declaration did not provide reusable evidence;
one Boolean check did not replace an active guard; one successful shortcut did
not make scheduling unnecessary. Component count, artifact count and source size
did not establish competing semantic authorities or unnecessary mechanisms.

INFERRED: useful domain wrappers and concrete replacement containers are LIBRARY
candidates. UNRESOLVED: general abstraction and runtime-dependent proof support.
HYPOTHESIS: sharing implementation machinery reduces maintenance/runtime cost;
only duplicated work, not resulting savings, was measured.

No examined counterexample establishes a new fundamental primitive as necessary.
That bounded result is not proof of complete expressiveness. Further research
should try to falsify the existing mechanisms before proposing additions.
