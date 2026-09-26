# Post-Gate-2 consolidation

Date: 2026-09-26.

Stages 1–2: **PASS**. Stage 3 scout: complete; ownership decision pending.

## Baseline and scope

Continue from the [Gate 2 hardening PASS](2026-09-26-gate-2-execution-hardening.md)
on `main`, HEAD `6ba9db303db453af952327069814c3a607b26d03`.
The user authorized planning and continued maturation/consolidation. Existing
unstaged work is preserved; no staging, commits, pushes, branch changes,
FlowLFS changes, or `meta-discusions.md` edits were made.

Plan: [post-Gate-2 maturation sequence](../tasks/post-gate-2-consolidation.md).

## Stage 1: guard/disposition evidence

The older guard lifecycle tests had the same wrong-format rejection weakness
as the pre-hardening Text outcome test. Their backend calls passed semantic
reports directly and treated any nonzero exit as sufficient rejection.

The repaired gate now accepts an optimized control at LLVM and a backend
lowering control at TinyVM before injecting hostile plans into those formats.
Flowvalidate receives the semantic artifact. All three consumers must report
a guard/disposition contract rejection. Forged-ready plans from refused source
programs are checked by the same mechanism.

Coverage: 34 hostile plans (11 guard mutations, 20 disposition mutations,
3 forged-ready plans) at three consumers: **102 expected contract rejections**.
Existing deterministic analysis, lowering versions 1/2, composition and
selective unguard, and exact preservation through four artifacts remain.
Historical checkpoint claims are left intact; this checkpoint supplies the
corrected backend evidence.

## Stage 2: diagnostic and status consolidation

Dangling-outcome guidance now describes supported local handling and explicitly
states that propagation and policy sinks are not yet supported. The stable
diagnostic code, subject identity, root-cause classification, and structured
provenance remain intact. The seventeen source-refusal cases assert the
supported/deferred distinction.

The function-return scout also exposed an original-file location defect:
a nine-line source file was reported at expanded line 51 after importing the
Text ABI. The frontend already supplies the mapping to original line 4.
Guard facts, disposition facts, and their statement diagnostics now use one
shared projection of that source map. Null/unmapped generated lines retain
the existing fallback rather than inventing an original-file location.

Tests assert original line and column for all Text outcome producer facts and
refusals, plus an imported static-guard success and refusal. Valid mapped
evidence remains identical through the carrying stages. The return probe now
points at the producer's original line 4.

The root README, Flowanalyst current status, and Flowcontracts documentation
now link to current evidence and distinguish implementation from proposals.

## Stage 3: completed scout, ownership decision pending

The source probe confirms that a `TextOutcome` return signature and return
statement are recognized but not admitted as obligation transfer. The parser
succeeds and Flowanalyst refuses with the dangling-outcome diagnostic.

[The decision brief](2026-09-26-outcome-return-transfer-decision.md) recommends
unique transfer through one direct return into one caller owner. It compares
that proposal with copied/shared outcomes and deferring return support for a
general ownership model. No proposed transfer semantics were implemented.

## Verification

- Canonical doctor and build: PASS.
- Focused Text outcome and guard suites: **2/2 PASS**, 5.76 seconds.
- Focused Clang 18 ASan/UBSan suites: **2/2 PASS**, 6.40 seconds, with
  LeakSanitizer disabled for the established host ptrace-wrapper limitation.
- Valgrind imported outcome-refusal probe: expected analyzer exit 2,
  **zero errors and zero outstanding bytes**; source-line assertion PASS.
- Shell syntax and `git diff --check`: PASS.
- Final canonical `./igor test`: **165/165 PASS**, 78.78 seconds.

## Next action

Select the ownership model in the decision brief before implementing return
transfer. Stages 1 and 2 consolidate existing authority; Stage 3's proposal
would establish new meaning and remains a decision gate under the standing
stage protocol.
