# Flowcore product-scale and self-hosting plan

**Status:** architectural direction and staged acceptance plan  
**Date:** 2026-08-26  
**North-star application:** FlowOpenOffice  
**North-star compiler proof:** Flowmini compiles Flowmini

## Purpose

Flowcore should eventually support both:

1. a large, long-lived application composed from many libraries, targets,
   providers and user-facing subsystems; and
2. a compiler implementation written in the language it compiles.

These goals test different forms of maturity. FlowOpenOffice tests product
scale, architecture, platform integration and evolution. Self-hosting tests
language expressiveness, deterministic compiler construction and freedom from
unacknowledged host-language privileges. Neither goal alone proves the other.

## Meaning of success

“FlowOpenOffice port” does not mean adding an application profile to a compiler
switch. It means an independently developed family of ordinary Flow packages
can express and ship an office system through public language, package,
provider and target contracts.

“Flowmini compiles Flowmini” does not mean one generated executable happens to
accept one fixture. It means a staged compiler can rebuild its own source,
produce independently valid boundary artifacts, pass the same conformance
suite, and reproduce its next-stage output without hidden dependence on Stage
0 implementation internals.

## Permanent laws

1. Canonical semantic and Graph IR contracts remain authoritative. The office
   application and compiler are consumers, never sources of secret semantics.
2. Every compiler stage remains a separate tool communicating through complete,
   durable, versioned files.
3. A self-hosted consumer must run from captured input after its producer has
   exited and without the producer binary or private data structures present.
4. New applications, compiler components and targets must not require dispatch
   by source name, package name, fixture name or application profile.
5. Target selection is an explicit policy artifact. Changing its name may
   select another admitted backend, ABI and deployment envelope without source
   conditionals or silent fallback.
6. Host C++ remains a declared bootstrap dependency until replacement is
   independently proven. It is not deleted to manufacture a self-hosting claim.
7. A bootstrap mismatch is evidence to investigate, not output to normalize
   away.
8. Product features cross the same capability, effect, resource, provenance and
   failure boundaries as small programs.

## Maturity model

```text
language closure
  -> library and package closure
  -> compiler-construction closure
  -> partial staged bootstrap
  -> reproducible self-hosting
  -> product-scale platform closure
  -> FlowOpenOffice family
```

The work may overlap, but no later claim bypasses an earlier proof.

## Gate 0 — define and preserve the Stage 0 seed

The current C/C++ toolchain is Stage 0. Record exact source, compiler, standard
library, build tools, external providers and generated artifacts required to
build it from a clean checkout.

- Freeze a bootstrap corpus covering every accepted syntax and semantic form.
- Capture all public artifacts between stages.
- Record deterministic and intentionally non-deterministic fields.
- Produce a host-feature inventory for every compiler component.
- Keep a buildable tagged Stage 0 until two later independent stages agree.

**Exit:** Stage 0 can be rebuilt and its public outputs replayed without relying
on sibling build directories or undocumented machine state.

## Gate 1 — language closure inventory

Inventory what the current compiler and a large office system require but the
language cannot yet express. Classify each need as language semantics, standard
library, provider capability, tool support or application code.

At minimum examine:

- modules, visibility, imports, namespaces and separate compilation;
- records, enums, tagged variants, tuples and recursive data;
- generic containers and algorithms without application-specific compiler code;
- strings, Unicode scalar values, byte buffers and validated text encodings;
- option/outcome types, diagnostics and explicit failure propagation;
- ownership, borrowing/observation, lifetimes and resource cleanup;
- functions, closures/callbacks and interface/trait-style contracts;
- bounded iteration, iterators and graph/stream processing;
- effect/resource identities, ordering constraints, cancellation, delivery,
  and queue semantics needed to derive safe execution schedules;
- stable serialization, hashing, paths, time and numeric facilities;
- reflection or generated metadata needed by tooling, without unrestricted
  runtime semantic mutation.

**Exit:** every required host feature has an owner and gate. Missing language
semantics are not disguised as library calls or compiler intrinsics.

## Gate 2 — packages, libraries and build graph

Large systems require independently versioned units rather than one enormous
source file.

- Define public package/module manifests and dependency identities.
- Support interfaces separate from implementations where useful.
- Make program, library, plugin and tool artifacts selected roots of the same
  graph model.
- Add deterministic dependency resolution, lock evidence and cycle diagnostics.
- Support incremental and parallel builds without changing semantic output.
- Preserve source/package/module/symbol identity through every artifact.
- Add compatibility, version and deprecation contracts.

**Exit:** changing one library rebuilds the necessary graph only, while a clean
build and incremental build produce equivalent public artifacts.

## Gate 3 — product-grade value and resource model

Complete the semantics required for durable applications and compilers:

- typed aggregate layout independent of any one host ABI;
- owned, shared, observed and borrowed values with explicit lifetime rules;
- bounded and dynamic storage expressed as distinct capabilities;
- deterministic destruction/cleanup across return, failure and cancellation;
- immutable and persistent data structures where they improve graph reasoning;
- safe slices/views with bounds and provenance;
- opaque handles for OS, GUI, document and device resources;
- explicit fallible allocation and resource exhaustion behavior.

Target policy may reject dynamic storage, sharing or post-initialization
allocation without removing these capabilities from the language.

**Exit:** no product or compiler feature needs an untyped pointer escape to
express its ordinary ownership and cleanup behavior.

## Gate 4 — effect, failure and scheduling closure

- Make outcome edges and final dispositions explicit and checkable.
- Model filesystem, process, network, UI, clock, randomness and device effects.
- Define operation, effect/resource, stream, cancellation, delivery and join
  contracts needed for Graph IR to derive dependencies and conflicts.
- Keep serial, parallel, accelerator and asynchronous-dispatch policy separate
  from graph meaning and activation semantics; do not add source scheduling
  syntax.
- Support deterministic reference scheduling for tests.
- Prove race/resource-conflict refusal before parallel placement.
- Carry diagnostic versus operational payload identity.

**Exit:** the compiler and office system can perform their required work without
hidden exceptions, global side effects or scheduler accidents.

## Gate 5 — standard library and provider surface

Build small, target-neutral contracts with independently replaceable providers:

- core values, collections, text/Unicode, bytes and algorithms;
- serialization and canonical artifact I/O;
- filesystem/path and stream I/O;
- process/environment and command execution where policy permits;
- diagnostics, source spans and structured logging;
- cryptographic digest provider contracts;
- threading/event-loop/UI abstractions;
- compression, archive and document-format primitives;
- testing, fuzzing and benchmark support.

The standard library defines behavior. Linux, Windows, TinyVM, bare-metal and
special-device implementations are providers selected by target policy.

**Exit:** the same library conformance corpus runs against every admitted
provider, with explicit unsupported results for absent capabilities.

## Gate 6 — compiler-construction kit

Before rewriting Flowmini, prove the language can build compiler-shaped tools:

1. byte reader and UTF-aware source reader;
2. tokenizer with exact spans and lossless token output;
3. recursive data structures and arena/index-based storage;
4. parser with structured recovery and multiple diagnostics;
5. symbol tables, scopes and stable identities;
6. typed AST and semantic facts;
7. Graph IR construction and validation;
8. deterministic artifact readers/writers;
9. target-policy and provider selection clients;
10. test harnesses, golden artifacts, mutation tests and fuzz entry points.

Each is first an ordinary Flow program consuming and emitting public files.

**Exit:** a compiler laboratory written in Flow can process a meaningful
language subset without privileged runtime hooks.

## Gate 7 — staged Flowmini replacement

Replace components vertically, never through a flag day:

```text
Stage 0 C++ producer -> Flow consumer -> public artifact
Stage 0 C++ producer -> C++ consumer  -> public artifact
```

For lexer, parser, symbol projection, semantic analysis, validation and lowering
in turn:

- implement the component in Flow;
- feed it the same captured files as the C++ component;
- compare canonical output and structured diagnostics;
- run positive, malformed, adversarial and fuzz corpora;
- switch the default only after differential parity;
- retain the previous provider as a bootstrap/reference escape hatch.

**Exit:** every stage has a Flow implementation capable of rebuilding the
complete admitted compiler subset while preserving file boundaries.

## Gate 8 — reproducible bootstrap

Use conventional staged terminology:

```text
Stage 0: trusted existing C/C++ Flowmini builds Stage 1
Stage 1: Flow-written Flowmini builds Stage 2
Stage 2: Flow-written Flowmini rebuilds Stage 3
```

Required proof:

- Stage 1, 2 and 3 pass the same language and artifact-contract suites.
- Stage 2 and Stage 3 produce byte-identical deterministic artifacts, or every
  permitted difference is independently identified and excluded from authority.
- Diverse bootstrap compares outputs produced through at least two independent
  seeds/providers where practical.
- Rebuilding one target never silently uses host-native artifacts from another.
- Captured intermediate files allow every stage to be audited and replayed.

**Exit:** Flowmini can compile its own complete source and reaches a reproducible
fixed point without Stage 0 being present at execution time.

This is the first legitimate “Flowmini writes Flowmini” checkpoint.

## Gate 9 — cross-target self-hosting

- Build the self-hosted compiler for LLVM/native and TinyVM.
- Run the same frontend and validator artifacts through both.
- Cross-build for another supported architecture using only a changed target
  policy name and available toolchain/provider evidence.
- Compare canonical stage outputs across hosts and backends.
- Separate target-dependent executable bytes from target-independent meaning.

**Exit:** self-hosting is a property of the language and contracts, not an
accident of one x86-64 Linux process.

## Gate 10 — FlowOpenOffice vertical product slices

Build the product as ordinary packages in increasing slices:

1. application shell, command model, settings and target-independent document
   lifecycle;
2. text document model with load/save, undo/redo and a headless renderer test;
3. accessible GUI editor through an authorized UI provider;
4. canonical ODF subset with hostile-file and round-trip tests;
5. spreadsheet cells, formulas, dependency graph and deterministic recalc;
6. presentation document and layout/rendering pipeline;
7. printing/export, clipboard, drag/drop and platform integration;
8. localization, Unicode shaping, accessibility and input methods;
9. plugin/extension boundary with capability isolation and version contracts;
10. collaboration/revision model and recovery from partial writes or crashes.

Every slice must build headlessly, keep document meaning separate from GUI
projection, and run on at least two target/provider combinations before being
called platform-neutral.

**Exit:** a multi-package FlowOpenOffice build is maintainable, independently
testable, target-selectable and free of application-specific compiler dispatch.

## Gate 11 — scale, evolution and release evidence

- Measure clean/incremental build time, memory, artifact size and diagnostics.
- Test thousands of modules and large symbol/graph populations.
- Prove deterministic builds and content-addressable caching.
- Add migration tools for language, package and document format revisions.
- Sign source, policy, provider evidence and emitted artifacts.
- Reproduce releases from captured inputs on clean builders.
- Maintain backwards-compatibility and deprecation tests.

**Exit:** the language can evolve without making old products or its own
compiler unauditable.

## Relationship between the two north stars

Shared requirements include modules, aggregates, generics, text/bytes,
collections, deterministic serialization, diagnostics, resource laws,
concurrency, providers, package graphs and reproducible builds.

The compiler should not be delayed until all office features exist. The office
system should not wait for full self-hosting. The efficient route is alternating
vertical slices that strengthen both:

```text
compiler-shaped library slice
  -> self-hosted component proof
  -> product-shaped library slice
  -> FlowOpenOffice proof
  -> repeat
```

## Immediate sequence

1. Finish the TinyVM artifact and typed-ISA gates already underway.
2. Produce the language-closure inventory from actual Stage 0 dependencies.
3. Select the smallest missing shared facility—likely aggregate values plus
   byte/text collections—through an explicit language decision.
4. Build one compiler-construction probe and one headless document-model probe
   using it.
5. Preserve both as permanent cross-backend acceptance programs.

This avoids prematurely rewriting Flowmini while ensuring every language
extension moves both north-star systems closer to reality.
