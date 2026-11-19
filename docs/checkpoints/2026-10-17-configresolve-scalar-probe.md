# ConfigResolve scalar resolution probe — PASS

Branch: `experiment/configresolve-lyraform`; base HEAD unchanged at
`b50534c380879a5a70a77ed5c1dd442bfc4142bd` plus preserved working tree.
[Mission](../tasks/configresolve-scalar-resolution-probe.md).

Application logic is [Lyraform source](../../experiments/configresolve/scalar-resolution.flow).
It implements two integer candidate selection and diagnostic classification for
four conflict modes. The independent C++ oracle calls the pinned public resolver.
The port binary does not call it. No compiler, backend or base-runtime changes.
911 protected baseline files rehashed unchanged.

Measured: 96 cases matched (64 small cross-product cases, 32 deterministic held-out
inputs). Native comparison exit 0; renamed program/function exit 0; corrupted
oracle exit 1 as required. TinyVM compiled the same comparison through flowprepare
and completed with result 0, trap 0. Standalone native probe is retained at
`/tmp/configresolve-scalar-proof/standalone/program`, returns candidate identity 2.
The comparison binary is `/tmp/configresolve-scalar-proof/comparison/program`.
Hashes, commands and result records are in the [evidence](../../experiments/configresolve/evidence/scalar-probe/report.json).

Reproduction after building the documented Lyraform and reference baselines:

```sh
python3 experiments/configresolve/run-scalar-probe.py --build /tmp/lyraform-capabilities-health-20261017 --reference-build /tmp/configresolve-port-reference --output /tmp/configresolve-scalar-proof
```

The first parser attempt used reserved `policy` as a parameter; renaming that
application identifier corrected it without a compiler change. TinyVM correctly
refused direct optimization input; using existing flowprepare produced the
required backend-neutral artifact. Neither was a missing language capability.

The preceding baseline's 189/189 Lyraform and 13/13 reference tests remain the
whole-system evidence; no protected code changed since those runs. This stage's
new evidence is the 96-case differential test, rename test, corrupted-oracle
refusal, native linkage inspection and TinyVM execution. No claim of new full
suite run or clean committed checkout.

What became usable: actual independently checked Lyraform application algorithm
in native and TinyVM binaries. What remains: complete Fact/Value/Result records,
arbitrary fact sets, all scalar domains, loaders, schema, store/view lifecycle,
full diagnostics/provenance and library composition. This binary is a bounded
probe, NOT the complete ConfigResolve port. Scalar result codes are test/probe
representation, not newly ratified public failure types.

Simplification: reused all existing language/backend machinery; no new provider
or application intrinsic. Falsifiers exercised: contradictory oracle, arbitrary
names and independently generated inputs. Full portability claims remain bounded
to admitted integer input and exact nonzero precedence. Failure result exposes
only diagnostic class; no provisional winner has been promoted to authority.

Next: investigate a generic ordinary-record construction/projection slice for
Fact and Result representation, under ADR 0052. Determine missing canon separately
from missing implementation before editing the compiler. Existing disposition
Graph IR/recovery/quarantine missions remain required for full failure flow.
