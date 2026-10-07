# Canonical provider/effect authority — Stage 2E

Date: 2026-09-27.

## Objective

Audit the existing provider/effect requirement and Flowbind authorization path
for duplicate semantic authority. Connect only the already-admitted external
operation and native-graph provider identities.

New ABI/FFI surface, provider selection semantics, effects, scheduling,
failure routing, dynamic replacement policy, and runtime invocation remain
excluded.

## Scout result

Flowanalyst already produces provider identity from resolved external callable
declarations: contract, library, symbol, calling convention, declared effect,
parameter carriers, return carrier, and evidence. It projects that same tuple
into `binding_requirements`, external lowering operations, and native graph
provider nodes.

Flowbind correctly treats policy, provider availability, symbol discovery, ABI
carrier support, and generated-provider evidence as authorization facts rather
than language semantics. It also checked that every external operation matched
a semantic binding requirement.

The check was only one-way. A hostile canonical report could append a duplicate
or otherwise unused binding requirement. Flowbind could then authorize a
capability that no admitted operation consumed. Downstream exact authorization
validation would eventually reject some projections, but the binding boundary
itself could emit an over-broad ready report.

## Implementation

Shared Flowcontracts validation now establishes exact set equality between:

```text
Flowanalyst binding requirements
    == external-call and TextOutcome operation providers
     + native-graph provider/count-provider identities
```

Duplicate requirement identities are refused. Missing and unused requirements
are refused. Flowbind invokes the same shared validation before policy or host
provider discovery. Shared semantic-report consumers apply it as part of normal
artifact validation.

Plan-less Flowbind inspection reports remain accepted as explicit compatibility
inputs for bounded provider probing. They are not canonical semantic reports
and cannot enter the staged Lyraform execution path.

## Initial focused evidence

- exact provider/source identity: PASS;
- TextOutcome provider/disposition regression: PASS;
- shared Flowcontracts JSON validation: PASS;
- end-to-end identity preservation: PASS;
- Flowbind provider boundary, including duplicate and unused requirement
  attacks: PASS;
- combined focused result: **5/5 PASS**, 11.72 seconds.

## Final verification

- canonical Igor suite: **170/170 PASS**, 85.32 seconds;
- ASan/UBSan focused gate: **5/5 PASS**, 6.29 seconds;
- Valgrind shared semantic consumer (`flowparallel`): **0 errors**, 0 bytes
  in 0 blocks at exit;
- Valgrind Flowbind authorization boundary: **0 errors**, 0 bytes in 0
  blocks at exit;
- exact ready-capability set equals the semantic requirement set: **PASS**;
- repeated Flowbind report emission: byte-identical;
- current-documentation drift guard: **PASS**;
- canonical-authority drift guard: **PASS**;
- legacy-deprecation drift guard: **PASS**;
- Igor doctor: **PASS**;
- repository diff whitespace check: **PASS**.

The first complete-suite pass exposed one handcrafted graph-planner fixture
that described no external operations but omitted `binding_requirements`. The
fixture now states the matching empty requirement set. No production contract
was weakened.

## Gate 2E

**PASS.** Flowanalyst remains the sole semantic producer of bounded
provider/effect identity. Shared validation proves exact use in both directions,
and Flowbind supplies only policy, ABI-support, provider-availability, symbol,
and evidence authorization. A canonical artifact can no longer broaden its
capability request with duplicate or unused requirements.

Recommended next stage: begin Stage 3 with a bounded operation-connectivity
inventory. Prove which existing scalar, target, guard, disposition, ownership,
and provider facts already share one operation identity, then connect only the
smallest factual gap. Member execution, new Graph IR meaning, scheduling,
failure flow, and ABI/FFI expansion remain excluded.
