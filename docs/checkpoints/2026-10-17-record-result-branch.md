# Checkpoint: record-valued branch returns

TinyVM now gives a record-returning callable one stable private result tree and
copies every branch return into that tree. This prevents branch-local metadata
from selecting the last statically compiled field storage and fixes the
uninitialized-slot failure for a record returned from a conditional.

The source probe chooses `Outcome{value:11}` or `Outcome{value:22}` through
`choose(flag): Outcome`, then passes the result to `read(Outcome): int`. Both
branch outcomes match in native LLVM and TinyVM, with TinyVM `trap:0`.

Evidence: `experiments/configresolve/evidence/record-result-branch/` and
`experiments/configresolve/run-record-result-branch-probe.py`.

This remains a finite scalar-record boundary. It does not admit member writes,
resource lifetimes, recursive records, collections, or the full ConfigResolve
result/diagnostic model.
