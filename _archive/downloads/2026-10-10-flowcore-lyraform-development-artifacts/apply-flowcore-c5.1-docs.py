#!/usr/bin/env python3
from pathlib import Path
import argparse
import subprocess
import sys

parser = argparse.ArgumentParser(description='Apply Flowmini C5.1 status documentation updates.')
parser.add_argument('--check', action='store_true', help='validate all expected source text without writing')
args = parser.parse_args()

root = Path.cwd()
try:
    git_root = Path(subprocess.check_output(['git', 'rev-parse', '--show-toplevel'], text=True).strip())
except (subprocess.CalledProcessError, FileNotFoundError):
    print('error: not inside a Git repository', file=sys.stderr)
    raise SystemExit(2)
if git_root != root:
    print(f'error: run from repository root: {git_root}', file=sys.stderr)
    raise SystemExit(2)

paths = {
    'status': root / 'Flowmini/flowmini_v24_explicit_ast/docs/v0.24-explicit-ast-status.md',
    'sitrep': root / 'Flowmini/flowmini_v24_explicit_ast/docs/v0.24-shallow-expression-ast-sitrep.md',
    'current': root / 'Flowmini/CURRENT.md',
}
for path in paths.values():
    if not path.is_file():
        print(f'error: missing expected file: {path}', file=sys.stderr)
        raise SystemExit(2)

texts = {key: path.read_text(encoding='utf-8') for key, path in paths.items()}

def replace_once(text: str, old: str, new: str, label: str) -> str:
    n = text.count(old)
    if n != 1:
        raise RuntimeError(f'{label}: expected exactly 1 match, found {n}')
    return text.replace(old, new, 1)

# Current explicit-AST status: add C5.1 guarantees and update current golden count.
old = '''generic/array initializer boundaries
function return types and bodies after both `:` and `->` signatures
```
'''
new = '''generic/array initializer boundaries
function return types and bodies after both `:` and `->` signatures
canonical return value-expression ownership
return expression_ids as a derived compatibility projection
unified function/main/nested statement-body parsing
```
'''
texts['status'] = replace_once(texts['status'], old, new, 'explicit status guarantees')

old = '''The validator requires complete expression and type-reference payload shapes,
valid expression references, source locations, and exact agreement between
canonical payload roles and their generic compatibility projections.
'''
new = '''The validator requires complete expression and type-reference payload shapes,
valid expression references, source locations, and exact agreement between
canonical payload roles and their generic compatibility projections. During C5
it also checks statement-role ownership as those roles become canonical; return
value ownership is the first migrated statement role.
'''
texts['status'] = replace_once(texts['status'], old, new, 'explicit status validator paragraph')

texts['status'] = replace_once(
    texts['status'],
    'AST golden tests: PASS (11)',
    'AST golden tests: PASS (12)',
    'explicit status golden count',
)

# The shallow-expression SITREP is partly historical: preserve its old PASS(8)
# checkpoint text. Only update the live roadmap section that has already evolved
# through C1-C4.
old = '''### Road C5: statement deepening

Purpose:

```text
Give statements stronger expression ownership and richer structure.
```

Needed:

```text
let initializer expression
assignment value expression
if condition expression
while condition expression
return value expression
else blocks
flow statements if kept
```
'''
new = '''### Road C5: statement deepening

Purpose:

```text
Give statements stronger expression ownership and richer structure.
```

Status: in progress.

Completed:

```text
C5.1 canonical return value-expression ownership
C5.1 return expression_ids compatibility projection derived from that ownership
C5.1 unified function/main/nested statement-body parsing
C5.1 direct and nested return regression coverage
```

Current law:

```text
A statement owns expressions according to their semantic role.
Generic expression lists may remain as compatibility projections during the
migration, but they are not the canonical meaning of a migrated statement role.
```

Remaining:

```text
typed-binding initializer ownership
assignment value ownership
if condition ownership
while condition ownership
else blocks
expression statements
flow statements if retained
```
'''
texts['sitrep'] = replace_once(texts['sitrep'], old, new, 'SITREP C5 roadmap')

# CURRENT is explicitly supposed to describe the live tree, so refresh it rather
# than preserving its old shallow-expression checkpoint language.
old = '''Current milestone:

```text
Flowmini v0.24 explicit AST
shallow expression AST checkpoint
```

## Status

```text
build: expected OK
AST golden tests: PASS (8)
suite: PASS (76 / 76)
bad: 0
```

Flowmini is still experimental and unfinished.

The current v0.24 line provides an observable, regression-guarded AST with a first shallow expression graph pass.
'''
new = '''Current milestone:

```text
Flowmini v0.24 explicit AST stabilization
Road C5 statement deepening in progress
C5.1 return expression ownership complete
```

## Status

```text
build: PASS
AST golden tests: PASS (12)
suite: PASS (76 / 76)
bad: 0
```

Flowmini is still experimental and unfinished.

The current v0.24 line provides an observable, regression-guarded AST with
canonical expression payloads, canonical type-reference payloads, and the
first canonical statement-expression role: return value ownership.
'''
texts['current'] = replace_once(texts['current'], old, new, 'CURRENT milestone/status')

old = '''cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug -j20
```

Adjust `-j20` to match your machine.
'''
new = '''cmake -S . -B cmake-build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug -j"$(nproc)"
```
'''
texts['current'] = replace_once(texts['current'], old, new, 'CURRENT build commands')

texts['current'] = replace_once(
    texts['current'],
    'AST golden tests: PASS (8)',
    'AST golden tests: PASS (12)',
    'CURRENT test golden count',
)

old = '''## Not yet complete

The following are still future or incomplete work:

```text
recursive expression population
operator precedence and associativity
else blocks
full type-reference parsing
semantic name resolution
symbol table integration
type checking
Graph IR lowering
runtime execution semantics
provider/capability resolution
```
'''
new = '''## Not yet complete

The following are still future or incomplete work:

```text
remaining statement-role ownership beyond return
else blocks
semantic validation of type references
generic value arguments
pointer/reference source semantics if adopted
semantic name resolution
symbol table integration
type checking
Graph IR lowering
runtime execution semantics
provider/capability resolution
```
'''
texts['current'] = replace_once(texts['current'], old, new, 'CURRENT incomplete list')

if args.check:
    print('C5.1 documentation preflight successful: all expected live-document fragments matched; no files changed.')
    raise SystemExit(0)

for key, path in paths.items():
    path.write_text(texts[key], encoding='utf-8')

print('C5.1 documentation updates applied successfully.')
for path in paths.values():
    print(f'  {path.relative_to(root)}')
