# Flowbind

Flowbind is the first external-world boundary after Flowanalyst. It consumes a
`flowanalyst.semantic_report` and verifies the report's declared external
library and symbol requirements with the host dynamic loader.

The v0.1 provider performs discovery only at the Lyraform external boundary:

```text
FlowMini → Flowanalyst → Flowbind
```

It uses `dlopen` and `dlsym` to prove that a declared library and symbol are
available. It also verifies the v0.1 supported C signature family
(`c_int`, `c_long`, `c_ulong`, `c_size_t`, `c_string`, and `c_pointer`) and reports host
layout facts. `c_pointer` is an opaque, contract-bound carrier for admitted
providers; it does not admit pointer arithmetic, guessed layouts, arbitrary
native signatures, or general FFI.
It never calls a foreign function. A ready report is the authorization input
for downstream generic lowering. The profile-free `flowcat` example uses exact
`libc.so.6` grants for `open`, `read`, `write`, and `close`.

Ready reports also contain a `capabilities` array. Each entry preserves the
declared contract, provider library, symbol, calling convention, effect, ABI
types, and authorization status for downstream inspectors and lowerers.

An optional `--abi-manifest manifest.json` consumes provider-owned aggregate
layout evidence. Flowbind reports `aggregate_abi: verified` and carries the
verified layout facts downstream when the manifest matches the semantic
aggregate declaration. Direct non-graph aggregate calls remain blocked until
their record-literal lowering is implemented; provider-verified aggregate
payloads in the bounded native graph path are admitted and tested.

Required CLI invariants:

```text
-h, -?, --help
-a, --about
-v, --version
```

The binding report is a versioned consumer boundary. A ready report means only
that provider discovery succeeded; `execution` is explicitly
`not-performed`.

External use is denied unless an exact capability grant is supplied:

```text
allow libc.so.6 strlen c pure c_string c_size_t
allow libc.so.6 puts c io
```

The optional parameter and result fields bind a grant to the declared parameter and
return ABI types. Older four-field grants remain accepted for compatibility,
but do not make a signature-specific claim.

The ABI summary reports carrier-type support as `carrier_types_supported`.
Provider-exact signature evidence is explicitly reported as `not-provided`
until a provider manifest supplies it.

Pass the policy with `--policy path`. The policy is intentionally small and
explicit; environment and configuration discovery belong to a later policy
boundary.

Generated grants append one additional evidence field after the return type:
`flowcore.generated_binding.v1:SPEC_SHA256:PROVIDER_SHA256`. It must exactly
match the declaration, semantic requirement and operation. Source import aliases
do not alter this identity. Historical grants cannot authorize an operation with
generated evidence. Reports preserve it in each capability; the ABI summary
`provider_signature_evidence` still reports `not-provided` because symbol
discovery does not independently prove a C prototype.

For generated evidence, Flowbind also verifies the SHA-256 of the loaded library
and the file owning the resolved symbol. Its `provider_evidence` array records
the loaded path, hash and `loaded-bytes-verified` status. This requires the Linux
loader interface and OpenSSL Crypto. It does not pin a provider against changes
after binding, or independently prove its C prototype.

If an exact policy grant names a provider that is absent during inspection,
Flowbind returns a blocked `flowbind.binding_report` with a `library
unavailable` failure, `code: FLOWBIND_PROVIDER_FAILURE`, and no ready binding
artifact. Policy and ABI rejections likewise carry stable condition codes. The generated-binding
acceptance test also replaces a digest-bound provider and proves that the old
evidence is rejected. These are inspection-time controls; provider replacement
or degradation after binding and native invocation remain outside the admitted
surface.
