# Disposition source boundary — Gate 0

Date: 2026-10-10  
Mission: `docs/tasks/disposition-foundation-and-source-authority-gate2.md`  
Branch: `codex/disposition-foundation-source-gate2`

## Decision and result

Gate 0: **PASS**.

The bounded source fact is the producer function's explicit
`fails { TextFailure }` clause, matched against the existing canonical
`TextOutcome` carrier at a single owned transfer to that same function's
return boundary. The function success type is `Text`; the carrier facts are
`Success<Text>` and `Failure<TextFailure>`. Admission will require exact
agreement rather than deriving failure from `outcome.code`, function names,
or control-flow shape.

The selected specimen is
`Lyraform/compiler/examples/disposition/text_outcome_consumer.flow`. Its
`compose_text` function transfers the complete tagged carrier once with
`produced -> return`. This gives the future semantic stage one explicit
boundary at which `atomic_tagged_result` can prohibit normal `Text`
publication when the carrier selected `Failure<TextFailure>`. It does not
access `produced.value`, duplicate the carrier, or infer a failure from the
body.

The existing `Lyraform/compiler/examples/text/text_outcome.flow` remains
unchanged. Its complementary `outcome.code` branches continue to account for
one local tagged value. Its local `puts` call is not a response consumer and
its branch is not silently promoted into a function failure.

The response function receives `failure TextFailure`, the accepted immutable
envelope projection, and explicitly declares `recover Text`. The
`text_failures` consumer names that exact ordinary function. The graph names
the producer function, consumer instance, same-attempt success/failure inputs,
typed `Text` rejoin, and activation containment node. None of those forms is
yet parsed or executable at this checkpoint.

## Evidence inventory

`docs/architecture/disposition-source-authority-map-v1.json` records:

- the existing import-aware source declarations for `concat_outcome`,
  `TextOutcome`, and `TextFailure`;
- current AST statement/expression, symbol, scope, operation, disposition,
  owner, obligation, commit and local-accounting identities;
- every intended source association and its current maturity;
- the numeric failure/fault plan identities that still originate only in a
  caller-supplied C++ fixture;
- the future ownership point for each missing identity link;
- an explicit all-false execution boundary for the new source form.

The current executable trace was reproduced from the canonical frontend and
Flowanalyst:

```text
text_outcome.flow:7:5
  -> AST statement 0 / initializer expression 0 / scope 16
  -> concat_outcome symbol 25
  -> tagged owner symbol 38
  -> operation 0 / disposition fact 0
  -> Success<Text> | Failure<TextFailure>
  -> commit atomic_tagged_result
  -> obligation operation:0:outcome (must_account)
  -> complementary local branches (accounted)
```

The bridge fixture's IDs 10–101 are separately labelled caller-supplied
declarative consistency facts. They are not presented as source identities.

## Lexer boundary

No punctuation change is required. The current lexer already retains every
token used by ADRs 0066–0067: parentheses, braces, colon, comma, dot, `->`,
and `=>`. The new words are currently ordinary identifiers, which permits one
canonical parser recognition path without adding aliases. A token-tree dump
of the new specimen succeeded before parser support; semantic admission was
not claimed.

## Remaining boundary

Gate 1 must add complete structural nodes and spans. Gate 2 must resolve and
own legality, prove declared/carrier equality and same-attempt wiring, produce
the declarative bridge from those facts, and refuse all later execution. No
policy, Graph IR route, LLVM/TinyVM dispatch, or fault quarantine behavior has
been added.
