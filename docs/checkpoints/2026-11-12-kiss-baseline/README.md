# KISS audit and verified implementation checkpoint

Audited and tested implementation: `9459705a9de14a91d62f796aa6bcd7f727fac165`.
Owner-authorized preservation: 2026-11-12. This archive adds evidence only; it
does not implement recommendations or change accepted language semantics.
The publication commit is a descendant containing this archive and its mission.

## Fresh publication verification

The canonical commands were run on the implementation revision, in order, with
a fresh temporary build selected by `LYRAFORM_BUILD_DIR` and build parallelism
limited to three through `CMAKE_BUILD_PARALLEL_LEVEL`.

| Command | Exit | Recorded wall seconds | Outcome |
|---|---:|---:|---|
| `./igor doctor` | 0 | 0.006 | Readiness checks pass |
| `./igor check` | 0 | 0.803 | 207 registered tests |
| `./igor build` | 0 | 79.229 | 332 actions, build complete |
| `./igor test` | 0 | 119.825 | 207/207 pass; CTest reports 119.78 s |

Toolchain: GCC/G++ 13.3.0, Clang 18.1.3, CMake/CTest 3.28.3, Ninja 1.11.1,
Git 2.43.0 and jq 1.7. See `results/toolchain.txt`.
The build emitted 560 pedantic `__int128` warnings and 52 missing-initializer
warnings, including repeated header instantiations; these are occurrences,
not 612 distinct defects. No compilation failure occurred.

The portable archive scripts were then executed against that same build. All
52 Slice 2 and three Slice 3 cases reproduced the recorded stage statuses,
diagnostic codes and execution outcomes. Direct-path plan/LLVM equality and
TinyVM result 8 were reproduced; the normalized guard artifact still validated.
The measurement and timing scripts also completed. These fresh replay checks
are summarized in `results/archive-replay-check.json`; original measurements
were not overwritten. Generated identities and source paths were not compared.

`results/publication-test.log` is the complete test transcript with local paths
normalized. `publication-verification.json` records original-log hashes;
those hashes deliberately differ from normalized archive files. Archive-byte
hashes are in `SHA256SUMS`. Full original logs remain outside Git locally.

Passing this suite does not invalidate the independently observed research gaps.
Historical checkpoint totals, the Slice 2 selected-tool build, and Slice 4
timings must not be confused with this fresh full canonical verification.

## Evidence inventory and provenance

- `sources/slice2/`: all 52 original source probes, unchanged (17,712 bytes).
- `sources/slice3/`: the three original implication/control/invalid-producer
  sources, unchanged.
- `results/slice2.json`, `slice3.json`: original result projections, including
  diagnostic codes, stage exit statuses and selected backend refusal messages;
  source paths are normalized. No result was invented for an unexecuted probe.
- `results/slice2-manifest.json`: original baseline, source hashes and expected
  execution results; redundant result copies and local build location omitted.
- `results/guard-missing-predicate*.json`: original altered artifact and its
  validation response with paths normalized. This is an artifact-validator
  counterexample, not proof of canonical source acceptance.
- `results/slice4-*`: original size/build-action measurements, timing results
  and TinyVM direct-path execution record. These are prior audit measurements.
- `scripts/`: portable adaptations of the original runner and measurement
  scripts. Explicit CLI paths replace original workstation paths. Their
  original hashes and adaptation descriptions are in `provenance.json`.
- `research-summary.md`: curated Slices 2–4 findings and the completed Slice 5
  conclusions from the conversation, not a verbatim transcript.

No standalone original Slice 0 or Slice 1 report was found in the inspected
audit directories or repository audit search. Their mission specifications are
not substituted as results. Slice 5 was completed in the conversation; this is
a preserved summary of that completed report, not a claim inferred from its
mission specification. The erroneous intermediate `measure.py` hash printed
in that report is not propagated: provenance uses freshly computed hashes.

Omitted: build trees, native/TinyVM binaries, generated LLVM, full repetitive
stage envelopes, the 278 KB extracted trace, and superseded aggregate console
logs. Original source-tree digests and selected omitted-file hashes identify
the retained local originals. There was no standalone Slice 3 orchestrator in
the inspected evidence; the portable probe runner can replay its three sources.

The archive is minimal relative to the original generated evidence: source and
result summaries are retained so successful and falsified alternatives remain
reviewable. It does not promise byte-identical generated artifacts after paths,
toolchain or host change.

## Replay

Run from the repository root, using a checkout of the audited implementation
or an evidence-only descendant. The following commands create fresh temporary
outputs rather than overwriting archived results:

```sh
archive="$PWD/docs/checkpoints/2026-11-12-kiss-baseline"
replay=$(mktemp -d)
export LYRAFORM_BUILD_DIR="$replay/build"
export CMAKE_BUILD_PARALLEL_LEVEL=3
./igor doctor
./igor check
./igor build
./igor test
python3 "$archive/scripts/probe.py" --build "$replay/build" \
  --sources "$archive/sources/slice2" --output "$replay/slice2"
python3 "$archive/scripts/probe.py" --build "$replay/build" \
  --sources "$archive/sources/slice3" --output "$replay/slice3"
python3 "$archive/scripts/direct-path.py" --repo "$PWD" \
  --build "$replay/build" --probes "$replay/slice2" --output "$replay/direct"
python3 "$archive/scripts/measure.py" --repo "$PWD" \
  --build "$replay/build" --probes "$replay/slice2" --output "$replay/measure"
python3 "$archive/scripts/benchmark.py" --repo "$PWD" \
  --build "$replay/build" --probes "$replay/slice2" --output "$replay/timing"
"$replay/build/flowcontracts/flowvalidate" < \
  "$archive/results/guard-missing-predicate.json"
(cd "$archive" && sha256sum -c SHA256SUMS)
```

The runner records refusals without treating them as harness crashes. Compare
results with archived diagnostics and expected executions; process exit zero
from the runner alone does not prove all probes passed. Native results are
process exit codes, not an arbitrary-width output oracle. Clang 18 is used for
native linking. Timings include startup and should not be treated as forecasts.
The measurement script counts all actions in its selected Ninja log: a fresh
full build will differ from the original selected-tool build measurements.

## Publication boundary

Destination: `Henrik1969/Lyraform`, existing branch
`experiment/configresolve-lyraform`; normal fast-forward push only.
Fetched branch inspection before publication reported `protected: false` and
no applicable branch rules. No repository settings were changed.
Main and master are untouched. Integration remains a separate human decision.
