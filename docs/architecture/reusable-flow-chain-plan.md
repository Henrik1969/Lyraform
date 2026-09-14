# Reusable Flow compiler-chain plan

## Objective

The built Flowcore tools must compile new Flow source files without adding a
C++ lowering profile or changing the toolchain binaries. Application semantics
belong in Flow source. C++ supplies reusable language machinery, substrate
bindings, providers, and backends.

```text
Flow source
  -> Flowmini frontend bundle
  -> Flowanalyst semantic report
  -> Flowbind capability authorization
  -> Flowparallel execution plan
  -> Flowoptimize transformed plan
  -> Flowlower backend IR
  -> native executable
```

## Current facts

The native scalar/control-flow and Flow-owned pager migrations are implemented.
`flow_less` uses ordinary source functions for navigation, validation and rendering;
separate native libraries supply raw input and output transport.
Explicit graph v2 supports selected scalar startup providers and native source
receivers through a runtime-owned deterministic activation-record FIFO. The
default graph v1 remains
non-executable evidence; unsupported graphs fail rather than being dropped. See the [activation decision](source-graph-activation-decision.md).

- For the admitted scalar/control-flow surface, every required compiler stage consumes or preserves the versioned structured
  lowering plan without application, source-unit or profile selection.
- Flowbind authorizes exact provider, convention, carrier, effect, resource and
  generated contract/evidence identities.
- Flowlower uses one typed structured emitter for values, calls, branches,
  loops, assignments, checked argv indexing, cleanup and returns; handwritten
  application emitters have been removed.
- The acceptance and migrated native examples build and execute through the
  already-built compiler chain without adding compiler dispatch.

## Migration gates

1. Emit `flowcore.lowering_plan` v1 beside the legacy profile field.
2. Make Flowbind authorize plan operations by exact capability/signature facts.
3. Make Flowparallel and Flowoptimize preserve plan operation identity and
   provenance while transforming it.
4. Make Flowlower consume generic value operations and external calls.
5. Add general control-flow/state lowering, then move `flow_less` paging logic
   into Flow source.
6. Convert existing fixtures one at a time and delete profile dispatch only
   after positive, negative, adversarial, sanitizer, and ELF tests pass.

## Non-negotiable invariants

- A new program must not require a C++ source edit or a new profile name.
- `->` value placement and `=>` graph connection remain distinct.
- Provider and capability authority are explicit and inspectable.
- Unsupported lowering fails with a structured report and provenance.
- Legacy compatibility is temporary and must not become the generic contract.

## Acceptance proof

Flowanalyst emits one operation per resolved call site. An operation records its
callee, resolved provider symbol when present, ABI signature/effect facts,
argument expression IDs, result symbol, and source identity. This is additive to
semantic report v1 and is the input contract for the generic lowerer migration.

`profile_free_getpid` and the generated `gettid` binding prove independent
source and provider selection. Arbitrarily named programs carry their generic
operations through Flowparallel and Flowoptimize, receive exact Flowbind
authorization, and are emitted, linked and executed by Flowlower without a C++
source change or toolchain rebuild.
