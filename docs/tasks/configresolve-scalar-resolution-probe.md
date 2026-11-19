# ConfigResolve scalar resolution probe

ID: `lyraform.configresolve-scalar-probe`; revision 1. Owner: Henrik Sørensen.
Selection: authorized next stage of the 2026-10-17 branch experiment, after T0
baseline verification passes. Canonical definition: this Markdown, hash-bound
in its own state. No prior completion or permissions are inherited.

Intent: establish the first independently checked executable application logic
from Lyraform source, while discovering concrete language blockers for the port.
Goal: a real native binary compiled through the canonical staged chain evaluates
bounded integer candidate selection identically to the pinned ConfigResolve
reference, with zero compiler/base-runtime application code changes.

Scope: experiments/configresolve source, reference-only C++ test harness,
fixtures and reproduction script. Existing scalar functions/comparisons/branches
and exact integer operations only. Two valid application facts for one fixed
valid key, explicitly supplied nonzero precedence, insertion order and conflict
policy. No generic records/collections or new error syntax are inferred.

Observable contract: selected candidate identity plus diagnostic classification
for unique-highest, FirstWins, LastWins, RejectConflict and highest-tier ambiguity.
Queries may be scalar functions for this probe; this does not establish the
port's public Result type. Do not consume a failed result as authoritative config.
Reference evidence: src/resolver.cpp and docs/API_FREEZE_CANDIDATE.md in the
pinned ConfigResolve manifest. On failed cases compare diagnostics; provisional
winner exposure stays a compatibility observation, not required semantics.

Tasks: define oracle cases -> compile generic Lyraform scalar functions -> compare
native results -> hostile/independence checks -> consolidate checkpoint.
Required criteria: all declared cases pass; altered value/order cases pass;
source and executable hashes recorded; no ConfigResolve library dependency in
the port executable; compiler/base files unchanged against the experiment
baseline; canonical suite remains green. Also attempt equivalent TinyVM path
using existing tooling and record exact limits rather than infer parity.

Convergence contract: this is an executable bounded prerequisite to the full
port, not completion. Existing language facts retain their semantic owners;
ConfigResolve policy remains application source. Reuse canonical stage tools
and public reference resolver. No new runtime/provider, syntax or policy override.
Stop if already-admitted syntax cannot express the probe; report the exact
refusal, and select a separate generic compiler mission under accepted law.

Positive: higher precedence chooses its candidate regardless of insertion order.
Negative: equal-priority unequal values diagnose ambiguity; RejectConflict
rejects disagreement even at different priorities. Hostile: permute candidate
order, rename program/functions, vary input constants and mutate oracle outputs;
comparison must detect wrong answers and compiler-name dependence.

The checked-in reproduction script must build a retained native artifact from
Lyraform source using flowmini -> Flowanalyst -> Flowparallel -> Flowoptimize ->
Flowlower -> LLVM/clang, requiring complete source-operation coverage. Reference
C++ code runs only as an independent test oracle. Archive exact commands/logs.
Any output table, config-specific intrinsic or delegation in the compiler/runtime
falsifies the claim. At gate update experiment inventory and checkpoint, name
remaining unsupported behavior and derive the next smallest mission from it.
