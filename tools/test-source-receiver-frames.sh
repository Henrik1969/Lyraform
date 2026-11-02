#!/bin/sh
set -eu
flowmini=${FLOWMINI_BIN:?FLOWMINI_BIN is required}
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
cat > "$tmpdir/receiver.flow" <<'FLOW'
program arbitrary_frames
producer source : stdin.text
node parse : parse.int
node receiver : fn transform
node left : fn observe
node right : fn observe
wire source.out => parse.in
wire parse.out => receiver.in
wire parse.out => receiver.in
wire receiver.out => left.in
wire receiver.out => right.in
fn transform(value : int): int {
    local : int(0)
    local + value -> local
    local -> return
}
fn observe(value : int): int {
    print value
    value -> return
}
main { marker : int(1) }
FLOW
printf '3\n' | "$flowmini" --trace true "$tmpdir/receiver.flow" > "$tmpdir/out" 2> "$tmpdir/log"
printf '3\n3\n3\n3\n' > "$tmpdir/expected"
cmp "$tmpdir/out" "$tmpdir/expected"
test "$(grep -c 'enter receiver with Int' "$tmpdir/log")" -eq 2
test "$(grep -c 'enter left with Int' "$tmpdir/log")" -eq 2
test "$(grep -c 'enter right with Int' "$tmpdir/log")" -eq 2
python3 - "$tmpdir/log" <<'PY'
import re, sys
lines = open(sys.argv[1]).read().splitlines()
routes = [re.search(r'route receiver.out => (left|right).in \[(wire:\d+)\] \[(signal:\d+)\] \[(delivery:\d+)\]', line).groups() for line in lines if 'route receiver.out =>' in line]
assert len(routes) == 4
assert routes[0][2] == routes[1][2] != routes[2][2] == routes[3][2]
assert len({r[3] for r in routes}) == 4
internal = [re.search(r'\[([^\]]+/signal:\d+)\]', line).group(1) for line in lines if 'route __' in line and '/signal:' in line]
assert len(internal) == len(set(internal)), internal
PY
# The second activation cannot read the first activation's conditional result.
python3 - "$tmpdir/receiver.flow" "$tmpdir/missing.flow" <<'PY'
import sys
s = open(sys.argv[1]).read().replace('wire parse.out => receiver.in\nwire parse.out => receiver.in', 'node change : fn increment\nwire parse.out => receiver.in\nwire parse.out => change.in\nwire change.out => receiver.in')
s = s.replace('    local -> return', '    if value == 3 {\n        local -> return\n    }')
s += '\nfn increment(value : int): int {\n    value + 1 -> return\n}\n'
open(sys.argv[2], 'w').write(s)
PY
if printf '3\n' | "$flowmini" --trace true "$tmpdir/missing.flow" > "$tmpdir/missing.out" 2> "$tmpdir/missing.log"; then
    echo 'missing result leaked from previous activation' >&2; exit 1
fi
test "$(grep -c 'route receiver.out =>' "$tmpdir/missing.log")" -eq 2
grep -Eq 'failure at receiver.in via wire:[0-9]+ signal:[0-9]+:' "$tmpdir/missing.log"
# Source body failure has no output activation or downstream invocation.
sed 's/local + value -> local/value div 0 -> local/' "$tmpdir/receiver.flow" > "$tmpdir/failure.flow"
if printf '3\n' | "$flowmini" --trace true "$tmpdir/failure.flow" > "$tmpdir/failure.out" 2> "$tmpdir/failure.log"; then
    echo 'failing receiver succeeded' >&2; exit 1
fi
! grep -q 'route receiver.out =>' "$tmpdir/failure.log"
! grep -q 'enter left with' "$tmpdir/failure.log"
grep -Eq 'failure at receiver.in via wire:[0-9]+ signal:[0-9]+:.*delivery:' "$tmpdir/failure.log"
# Type, port, cycle, definition and role validation happen before execution.
for mutation in type port cycle unresolved role; do
    case "$mutation" in
        type) sed 's/fn transform(value : int)/fn transform(value : Bool)/; s/local + value -> local/1 -> local/' "$tmpdir/receiver.flow" > "$tmpdir/bad.flow" ;;
        port) sed 's/receiver.in/receiver.wrong/g' "$tmpdir/receiver.flow" > "$tmpdir/bad.flow" ;;
        cycle) cat "$tmpdir/receiver.flow" > "$tmpdir/bad.flow"; printf '\nwire left.out => receiver.in\n' >> "$tmpdir/bad.flow" ;;
        unresolved) sed 's/node receiver : fn transform/node receiver : fn missing/' "$tmpdir/receiver.flow" > "$tmpdir/bad.flow" ;;
        role) sed 's/node receiver : fn transform/producer receiver : fn transform/' "$tmpdir/receiver.flow" > "$tmpdir/bad.flow" ;;
    esac
    if "$flowmini" "$tmpdir/bad.flow" </dev/null > "$tmpdir/bad.out" 2> "$tmpdir/bad.log"; then
        echo "invalid receiver accepted: $mutation" >&2; exit 1
    fi
    test ! -s "$tmpdir/bad.out"
    case "$mutation" in
        type) grep -Fq 'type mismatch on wire' "$tmpdir/bad.log" ;;
        port) grep -Fq 'is not an input port' "$tmpdir/bad.log" ;;
        cycle) grep -Fq 'cyclic source receiver graph is unsupported' "$tmpdir/bad.log" ;;
        unresolved|role) grep -Fq 'receiver requires a defined one-input function and node role' "$tmpdir/bad.log" ;;
    esac
done
# Scalar carrier checks are independent of names and operation order.
python3 - "$tmpdir/receiver.flow" "$tmpdir/bool.flow" <<'PYTHON'
import sys
s = open(sys.argv[1]).read().replace('arbitrary_frames', 'renamed_boolean_graph')
s = s.replace('fn transform(value : int): int', 'fn transform(value : int): Bool')
s = s.replace('    local -> return', '    local == 3 -> return')
s = s.replace('fn observe(value : int): int', 'fn observe(value : Bool): Bool')
open(sys.argv[2], 'w').write(s)
PYTHON
printf '3\n' | "$flowmini" "$tmpdir/bool.flow" > "$tmpdir/bool.out" 2> "$tmpdir/bool.log"
printf 'true\ntrue\ntrue\ntrue\n' > "$tmpdir/bool.expected"
cmp "$tmpdir/bool.out" "$tmpdir/bool.expected"
cat > "$tmpdir/text.flow" <<'FLOW'
program text_receiver
producer source : stdin.text
node identity : fn copy_text
node parse : parse.int
node observer : fn observe
wire source.out => identity.in
wire identity.out => parse.in
wire parse.out => observer.in
fn copy_text(value : c_string): c_string {
    value -> return
}
fn observe(value : int): int {
    print value
    value -> return
}
main { marker : int(1) }
FLOW
printf '42\n' | "$flowmini" "$tmpdir/text.flow" > "$tmpdir/text.out" 2> "$tmpdir/text.log"
test "$(cat "$tmpdir/text.out")" = 42
# Legacy serialization must refuse rather than dropping function bodies.
if "$flowmini" --emit-flowir "$tmpdir/result.flowir" "$tmpdir/receiver.flow" > /dev/null 2> "$tmpdir/export.log"; then
    echo 'legacy export erased receiver body' >&2; exit 1
fi
grep -Fq 'legacy FlowIR export is unsupported' "$tmpdir/export.log"
echo 'Source receiver frames: PASS'
