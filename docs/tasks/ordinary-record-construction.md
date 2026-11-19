# Complete ordinary record construction

ID: `lyraform.ordinary-record-construction`; revision 1. Owner Henrik Sørensen.
Selected continuation on `experiment/configresolve-lyraform` after the owner's
2026-10-17 adoption of complete record construction. Preserve unrelated dirty
work and the read-only ConfigResolve reference; no commits, push or merge.

## Authority and intent

ADR 0068 supplies construction authority; ADR 0052 supplies independent values.
The stage protocol, mission convergence rule, autonomous next-stage selection
and EXECUTION-001 govern execution. General records enable application-defined
Fact/Result-like data without putting ConfigResolve logic in the compiler.
The accepted decision is not evidence of implemented support.

## Measurable goal

Actual Lyraform source constructs and projects complete scalar and finite nested
ordinary records, passes and returns them as independent values, and executes
with matching native/TinyVM observations. Reordered named fields preserve field
identity. Missing, repeated, unknown and incompatible fields are rejected before
executable admission. Effectful initializers and unsupported recursive, reference
or resource-bearing records are refused explicitly. Backend layout stays private.
No application names or behavior enter compiler dispatch. Invalid or unsupported
artifacts cannot bypass admission through a downstream tool.

## Tasks and convergence

1. Scout existing declarations, literals, projections, type identities and scalar
   admission; map semantic and shared contract owners before edits.
2. Implement generic field validation and typed construction/projection facts,
   preserving unsupported-bound refusals. Reuse existing value machinery.
3. Implement native and TinyVM construction, projection, copies, parameters and
   returns for the proved bound; test nested values and renamed source programs.
4. Verify positive, negative and hostile cases, backend parity and relevant
   sanitizer checks; run canonical doctor/build/test and policy validation.
5. Checkpoint exact evidence, limitations and the next bounded dependency.

Member writes, new root write authority, resource lifetimes, public layout/FFI,
collections and effectful initializer ordering are not established here. A new
semantic ambiguity requires a decision brief only for the dependent work.

## State and dependencies

Canonical definition: this Markdown file. Isolated state lives in
`.mission-state/lyraform.ordinary-record-construction/1/`, bound to its SHA-256.
No earlier mission's completion is inherited. Source recovery execution remains
incomplete and separately recorded; its callable ABI tests do not satisfy this
mission or the complete ConfigResolve binary goal.
