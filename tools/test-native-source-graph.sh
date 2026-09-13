#!/bin/sh
set -eu
root=${FLOWCORE_ROOT:?}
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
sha256sum "$FLOWMINI_BIN" "$FLOWANALYST_BIN" "$FLOWBIND_BIN" "$FLOWPARALLEL_BIN" "$FLOWOPTIMIZE_BIN" "$FLOWLOWER_BIN" > "$tmpdir/tools.sha256"
cat > "$tmpdir/provider.c" <<'C'
#include <stdio.h>
#include <stddef.h>
static int calls;
static const char graph_text[] = "graph text";
extern int flow_graph_raise(int code);
int input_value(void) { ++calls; return 3; }
int other_value(void) { ++calls; return 8; }
size_t stream_count(void) { return 3; }
int stream_item(size_t index) { return (int)(10 + index); }
int stream_failure_item(size_t index) { return index == 1 ? flow_graph_raise(31) : (int)(10 + index); }
long wide_value(void) { return 4294967297L; }
unsigned long ulong_value(void) { return 4294967297UL; }
size_t size_value(void) { return (size_t)4294967298UL; }
const char *text_value(void) { return graph_text; }
int failing_value(void) { return flow_graph_raise(23); }
int input_count(void) { return calls; }
int observe_value(int value) { printf("%d\n", value); return 0; }
int observe_long(long value) { printf("%ld\n", value); return 0; }
int observe_ulong(unsigned long value) { printf("%lu\n", value); return 0; }
int observe_size(size_t value) { printf("%zu\n", value); return 0; }
int observe_text(const char *value) { printf("%s\n", value); return 0; }
C
clang -shared -fPIC "$tmpdir/provider.c" -o "$tmpdir/provider.so"
jq -n --arg path "$tmpdir/provider.so" '{format:"flowcore.native_binding_spec",version:1,unit:"unregistered_graph_provider",namespace:"host",provider:{soname:$path,path:$path,convention:"c"},functions:[{name:"input",symbol:"input_value",effect:"io",parameters:[],return_type:"c_int"},{name:"other",symbol:"other_value",effect:"io",parameters:[],return_type:"c_int"},{name:"wide",symbol:"wide_value",effect:"io",parameters:[],return_type:"c_long"},{name:"ulong",symbol:"ulong_value",effect:"readonly",parameters:[],return_type:"c_ulong"},{name:"size",symbol:"size_value",effect:"readonly",parameters:[],return_type:"c_size_t"},{name:"text",symbol:"text_value",effect:"readonly",parameters:[],return_type:"c_string"},{name:"failure",symbol:"failing_value",effect:"failure",parameters:[],return_type:"c_int"},{name:"count",symbol:"input_count",effect:"readonly",parameters:[],return_type:"c_int"},{name:"observe",symbol:"observe_value",effect:"io",parameters:[{name:"value",type:"c_int"}],return_type:"c_int"},{name:"observe_long",symbol:"observe_long",effect:"io",parameters:[{name:"value",type:"c_long"}],return_type:"c_int"},{name:"observe_ulong",symbol:"observe_ulong",effect:"io",parameters:[{name:"value",type:"c_ulong"}],return_type:"c_int"},{name:"observe_size",symbol:"observe_size",effect:"io",parameters:[{name:"value",type:"c_size_t"}],return_type:"c_int"},{name:"observe_text",symbol:"observe_text",effect:"io",parameters:[{name:"value",type:"c_string"}],return_type:"c_int"}]}' > "$tmpdir/spec.json"
jq '.functions += [{name:"stream_count",symbol:"stream_count",effect:"readonly",parameters:[],return_type:"c_size_t"},{name:"stream_item",symbol:"stream_item",effect:"readonly",parameters:[{name:"index",type:"c_size_t"}],return_type:"c_int"},{name:"stream_failure_item",symbol:"stream_failure_item",effect:"failure",parameters:[{name:"index",type:"c_size_t"}],return_type:"c_int"}]' "$tmpdir/spec.json" > "$tmpdir/stream-spec.json"
mv "$tmpdir/stream-spec.json" "$tmpdir/spec.json"
"$root/tools/generate-flow-bindings.sh" --spec "$tmpdir/spec.json" --flow-output "$tmpdir/provider.flow" --policy-output "$tmpdir/policy" --manifest-output "$tmpdir/manifest.json" >/dev/null
cat > "$tmpdir/selection.json" <<'JSON'
{"format":"flowcore.graph_provider_map","version":1,"providers":[{"implementation":"injected.batch","source_callable":"host.input","activation":"startup_once","output_port":"out"}]}
JSON
cat > "$tmpdir/program.flow" <<'FLOW'
import "provider.flow" as host
program fresh_native_graph
producer source : injected.batch
node receiver : fn transform
node left : fn observe_left
node right : fn observe_right
wire source.out => receiver.in
wire source.out => receiver.in
wire receiver.out => left.in
wire receiver.out => right.in
fn transform(value : c_int): c_int {
    local : c_int(0)
    local + value -> local
    return local
}
fn observe_left(value : c_int): c_int {
    result : c_int(0)
    host.observe(value) -> result
    return result
}
fn observe_right(value : c_int): c_int {
    result : c_int(0)
    next : c_int(0)
    value + 1 -> next
    host.observe(next) -> result
    return result
}
main {
    calls : c_int(0)
    host.count() -> calls
    return calls - 1
}
FLOW
compile() {
    "$FLOWMINI_BIN" --dump-frontend-bundle "$tmpdir/program.flow" > "$tmpdir/frontend.json"
    "$FLOWANALYST_BIN" --lowering-plan-version 2 --graph-plan-version 2 --graph-providers "$tmpdir/selection.json" < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
    "$FLOWBIND_BIN" --policy "$tmpdir/policy" < "$tmpdir/semantic.json" > "$tmpdir/binding.json"
    "$FLOWPARALLEL_BIN" < "$tmpdir/semantic.json" > "$tmpdir/execution.json"
    "$FLOWOPTIMIZE_BIN" < "$tmpdir/execution.json" > "$tmpdir/optimization.json"
    "$FLOWPREPARE_BIN" --binding-report "$tmpdir/binding.json" < "$tmpdir/optimization.json" > "$tmpdir/backend.json"
    "$FLOWLOWER_BIN" --emit-llvm "$tmpdir/program.ll" < "$tmpdir/backend.json" > "$tmpdir/lowering.json"
    clang -c "$tmpdir/program.ll" -o "$tmpdir/program.o"
    # These flags are supplied by CMake so sanitizer runtime archives link with
    # their own compiler/runtime rather than silently disabling instrumentation.
    "$FLOWGRAPH_CXX" ${FLOWGRAPH_LINK_FLAGS:-} "$tmpdir/program.o" "-Wl,-rpath,$(dirname "$FLOWGRAPH_RUNTIME")" "$FLOWGRAPH_RUNTIME" "$tmpdir/provider.so" -o "$tmpdir/program"
}
compile
FLOWCORE_GRAPH_TRACE=1 "$tmpdir/program" > "$tmpdir/output" 2> "$tmpdir/trace"
printf '3\n4\n3\n4\n' > "$tmpdir/expected"
cmp "$tmpdir/output" "$tmpdir/expected"
python3 - "$tmpdir/trace" <<'PY'
import json, sys
records = [json.loads(line) for line in open(sys.argv[1])]
enters = [r for r in records if r['event'] == 'enter']
assert [r['node_id'] for r in enters] == ['source', 'receiver', 'receiver', 'left', 'right', 'left', 'right']
assert enters[1]['input_signal_id'] == enters[2]['input_signal_id']
assert enters[1]['wire_id'] != enters[2]['wire_id']
assert enters[3]['input_signal_id'] == enters[4]['input_signal_id'] != enters[5]['input_signal_id'] == enters[6]['input_signal_id']
assert len({r['delivery_id'] for r in enters[1:]}) == 6
assert all(r['input_port'] == 'in' and r['source_port'] == 'out' and r['wire_provenance']['line'] > 0 for r in enters[1:])
assert len([r for r in records if r['event'] == 'drop']) == 4
PY
# A finite stream invokes its count once, then delivers ascending item indices
# through fresh receiver activations. The v2 schedule is dynamic at runtime;
# it is never expanded into a fabricated static activation list.
cp "$tmpdir/program.flow" "$tmpdir/scalar-root.flow"
cp "$tmpdir/selection.json" "$tmpdir/scalar-root.selection.json"
cat > "$tmpdir/stream.flow" <<'FLOW'
import "provider.flow" as host
program finite_native_stream
producer source : injected.stream
node receiver : fn observe_stream
wire source.out => receiver.in
fn observe_stream(value : c_int): c_int {
    result : c_int(0)
    host.observe(value) -> result
    return value
}
main { return 0 }
FLOW
cat > "$tmpdir/stream.selection.json" <<'JSON'
{"format":"flowcore.graph_provider_map","version":2,"providers":[{"implementation":"injected.stream","count_callable":"host.stream_count","item_callable":"host.stream_item","activation":"finite_stream_once","max_items":4096,"output_port":"out"}]}
JSON
cp "$tmpdir/stream.flow" "$tmpdir/program.flow"
cp "$tmpdir/stream.selection.json" "$tmpdir/selection.json"
compile
FLOWCORE_GRAPH_TRACE=1 "$tmpdir/program" > "$tmpdir/output" 2> "$tmpdir/trace"
printf '10\n11\n12\n' > "$tmpdir/expected"
cmp "$tmpdir/output" "$tmpdir/expected"
python3 - "$tmpdir/trace" <<'PY'
import json, sys
records = [json.loads(line) for line in open(sys.argv[1])]
enters = [r for r in records if r.get('event') == 'enter']
receivers = [r for r in enters if r.get('kind') == 'stream_receiver']
assert [r['stream_index'] for r in receivers] == [0, 1, 2]
assert [r['input_signal_id'] for r in receivers] == [1, 2, 3]
assert [r['delivery_id'] for r in receivers] == [1, 1, 1]
assert len([r for r in records if r.get('event') == 'drop']) == 3
PY
jq '.providers[0].max_items = 2' "$tmpdir/selection.json" > "$tmpdir/stream-bound.selection.json"
mv "$tmpdir/stream-bound.selection.json" "$tmpdir/selection.json"
compile
set +e
FLOWCORE_GRAPH_TRACE=1 "$tmpdir/program" > "$tmpdir/output" 2> "$tmpdir/trace"
status=$?
set -e
test "$status" -eq 70
test ! -s "$tmpdir/output"
python3 - "$tmpdir/trace" <<'PY'
import json, sys
records = [json.loads(line) for line in open(sys.argv[1])]
failure = records[-1]
assert failure['format'] == 'flowcore.graph_failure' and failure['reason'] == 'stream_bound'
assert failure['activation']['node_id'] == 'source'
assert not any('stream_index' in r for r in records)
PY
cat > "$tmpdir/stream-failure.selection.json" <<'JSON'
{"format":"flowcore.graph_provider_map","version":2,"providers":[{"implementation":"injected.stream","count_callable":"host.stream_count","item_callable":"host.stream_failure_item","activation":"finite_stream_once","max_items":4096,"output_port":"out"}]}
JSON
mv "$tmpdir/stream-failure.selection.json" "$tmpdir/selection.json"
compile
set +e
FLOWCORE_GRAPH_TRACE=1 "$tmpdir/program" > "$tmpdir/output" 2> "$tmpdir/trace"
status=$?
set -e
test "$status" -eq 70
printf '10\n' > "$tmpdir/expected"
cmp "$tmpdir/output" "$tmpdir/expected"
python3 - "$tmpdir/trace" <<'PY'
import json, sys
records = [json.loads(line) for line in open(sys.argv[1])]
failure = records[-1]
print(records, file=sys.stderr)
assert failure['format'] == 'flowcore.graph_failure' and failure['reason'] == 'source_failure'
assert failure['code'] == 31 and failure['activation']['stream_index'] == 1
assert not any(r.get('stream_index') == 2 for r in records)
PY
mv "$tmpdir/scalar-root.flow" "$tmpdir/program.flow"
mv "$tmpdir/scalar-root.selection.json" "$tmpdir/selection.json"
compile
# Independent startup roots each get their own FIFO activation sequence. The
# receiver is still entered once per delivered wire and fan-out remains local
# to the producing activation.
cp "$tmpdir/program.flow" "$tmpdir/single-root.flow"
cp "$tmpdir/selection.json" "$tmpdir/single-root.selection.json"
python3 - "$tmpdir/program.flow" <<'PY'
import sys
p = sys.argv[1]
s = open(p).read().replace('fresh_native_graph', 'multi_root_native_graph')
s = s.replace('producer source : injected.batch', 'producer first : injected.batch\nproducer second : injected.other')
s = s.replace('wire source.out => receiver.in\nwire source.out => receiver.in', 'wire first.out => receiver.in\nwire second.out => receiver.in')
s = s.replace('return calls - 1', 'return calls - 2')
open(p, 'w').write(s)
PY
jq '.providers += [{implementation:"injected.other",source_callable:"host.other",activation:"startup_once",output_port:"out"}]' \
    "$tmpdir/selection.json" > "$tmpdir/multi-root.selection.json"
mv "$tmpdir/multi-root.selection.json" "$tmpdir/selection.json"
compile
FLOWCORE_GRAPH_TRACE=1 "$tmpdir/program" > "$tmpdir/output" 2> "$tmpdir/trace"
printf '3\n4\n8\n9\n' > "$tmpdir/expected"
cmp "$tmpdir/output" "$tmpdir/expected"
python3 - "$tmpdir/trace" <<'PY'
import json, sys
records = [json.loads(line) for line in open(sys.argv[1])]
enters = [r for r in records if r['event'] == 'enter']
assert [r['node_id'] for r in enters] == [
    'first', 'receiver', 'left', 'right',
    'second', 'receiver', 'left', 'right']
first = enters[1:4]
second = enters[5:8]
assert first[0]['input_signal_id'] != second[0]['input_signal_id']
assert first[1]['input_signal_id'] == first[2]['input_signal_id']
assert second[1]['input_signal_id'] == second[2]['input_signal_id']
receiver_enters = [r for r in enters if r['kind'] == 'receiver']
assert len({r['delivery_id'] for r in receiver_enters}) == 6
PY
mv "$tmpdir/single-root.flow" "$tmpdir/program.flow"
mv "$tmpdir/single-root.selection.json" "$tmpdir/selection.json"
compile
# Native graph activation preserves a 64-bit c_long payload through a
# receiver and its fan-out, rather than narrowing it to the c_int path.
cp "$tmpdir/program.flow" "$tmpdir/scalar-root.flow"
cp "$tmpdir/selection.json" "$tmpdir/scalar-root.selection.json"
cat > "$tmpdir/wide.flow" <<'FLOW'
import "provider.flow" as host
program wide_native_graph
producer source : injected.wide
node receiver : fn identity_wide
node left : fn observe_wide
node right : fn observe_wide_plus
wire source.out => receiver.in
wire receiver.out => left.in
wire receiver.out => right.in
fn identity_wide(value : c_long): c_long {
    return value
}
fn observe_wide(value : c_long): c_long {
    result : c_int(0)
    host.observe_long(value) -> result
    return value
}
fn observe_wide_plus(value : c_long): c_long {
    result : c_int(0)
    next : c_long(0)
    one : c_long(1)
    value + one -> next
    host.observe_long(next) -> result
    return next
}
main {
    return 0
}
FLOW
cp "$tmpdir/wide.flow" "$tmpdir/program.flow"
jq '.providers += [{implementation:"injected.wide",source_callable:"host.wide",activation:"startup_once",output_port:"out"}]' \
    "$tmpdir/selection.json" > "$tmpdir/wide.selection.json"
mv "$tmpdir/wide.selection.json" "$tmpdir/selection.json"
compile
FLOWCORE_GRAPH_TRACE=1 "$tmpdir/program" > "$tmpdir/output" 2> "$tmpdir/trace"
printf '4294967297\n4294967298\n' > "$tmpdir/expected"
cmp "$tmpdir/output" "$tmpdir/expected"
python3 - "$tmpdir/trace" <<'PY'
import json, sys
records = [json.loads(line) for line in open(sys.argv[1])]
enters = [r for r in records if r['event'] == 'enter']
assert [r['node_id'] for r in enters] == ['source', 'receiver', 'left', 'right']
assert enters[1]['input_signal_id'] == enters[0]['output_signal_id']
assert enters[2]['input_signal_id'] == enters[3]['input_signal_id'] == enters[1]['output_signal_id']
assert len({r['delivery_id'] for r in enters[1:]}) == 3
PY
mv "$tmpdir/scalar-root.flow" "$tmpdir/program.flow"
mv "$tmpdir/scalar-root.selection.json" "$tmpdir/selection.json"
compile
# The remaining admitted integer carriers retain their ABI width through the
# same generic graph path. Two roots make the c_ulong and c_size_t contracts
# observable without relying on carrier-name-specific compiler dispatch.
cp "$tmpdir/program.flow" "$tmpdir/scalar-root.flow"
cat > "$tmpdir/unsigned.flow" <<'FLOW'
import "provider.flow" as host
program unsigned_native_graph
producer unsigned : injected.ulong
producer sized : injected.size
node ulong_receiver : fn identity_ulong
node size_receiver : fn identity_size
node ulong_observer : fn observe_ulong_graph
node size_observer : fn observe_size_graph
wire unsigned.out => ulong_receiver.in
wire sized.out => size_receiver.in
wire ulong_receiver.out => ulong_observer.in
wire size_receiver.out => size_observer.in
fn identity_ulong(value : c_ulong): c_ulong {
    return value
}
fn identity_size(value : c_size_t): c_size_t {
    return value
}
fn observe_ulong_graph(value : c_ulong): c_ulong {
    result : c_int(0)
    host.observe_ulong(value) -> result
    return value
}
fn observe_size_graph(value : c_size_t): c_size_t {
    result : c_int(0)
    host.observe_size(value) -> result
    return value
}
main {
    return 0
}
FLOW
cp "$tmpdir/unsigned.flow" "$tmpdir/program.flow"
jq '.providers += [
    {implementation:"injected.ulong",source_callable:"host.ulong",activation:"startup_once",output_port:"out"},
    {implementation:"injected.size",source_callable:"host.size",activation:"startup_once",output_port:"out"}
]' "$tmpdir/selection.json" > "$tmpdir/unsigned.selection.json"
mv "$tmpdir/unsigned.selection.json" "$tmpdir/selection.json"
compile
FLOWCORE_GRAPH_TRACE=1 "$tmpdir/program" > "$tmpdir/output" 2> "$tmpdir/trace"
printf '4294967297\n4294967298\n' > "$tmpdir/expected"
cmp "$tmpdir/output" "$tmpdir/expected"
python3 - "$tmpdir/trace" <<'PY'
import json, sys
records = [json.loads(line) for line in open(sys.argv[1])]
enters = [r for r in records if r['event'] == 'enter']
assert [r['node_id'] for r in enters] == [
    'unsigned', 'ulong_receiver', 'ulong_observer',
    'sized', 'size_receiver', 'size_observer']
receiver_enters = [r for r in enters if r['kind'] == 'receiver']
assert all([
    enters[1]['input_signal_id'] == enters[0]['output_signal_id'],
    enters[2]['input_signal_id'] == enters[1]['output_signal_id'],
    enters[4]['input_signal_id'] == enters[3]['output_signal_id'],
    enters[5]['input_signal_id'] == enters[4]['output_signal_id'],
    len({r['delivery_id'] for r in receiver_enters}) == 4])
PY
mv "$tmpdir/scalar-root.flow" "$tmpdir/program.flow"
compile
# A borrowed c_string is an admitted graph carrier with explicit pointer
# semantics. Fan-out reuses the captured string pointer without re-executing
# the receiver, while raw c_pointer remains refused below.
cp "$tmpdir/program.flow" "$tmpdir/scalar-root.flow"
cat > "$tmpdir/text.flow" <<'FLOW'
import "provider.flow" as host
program text_native_graph
producer source : injected.text
node receiver : fn identity_text
node left : fn observe_text_graph
node right : fn observe_text_graph
wire source.out => receiver.in
wire receiver.out => left.in
wire receiver.out => right.in
fn identity_text(value : c_string): c_string {
    return value
}
fn observe_text_graph(value : c_string): c_string {
    result : c_int(0)
    host.observe_text(value) -> result
    return value
}
main {
    return 0
}
FLOW
cp "$tmpdir/text.flow" "$tmpdir/program.flow"
jq '.providers += [{implementation:"injected.text",source_callable:"host.text",activation:"startup_once",output_port:"out"}]' \
    "$tmpdir/selection.json" > "$tmpdir/text.selection.json"
mv "$tmpdir/text.selection.json" "$tmpdir/selection.json"
compile
FLOWCORE_GRAPH_TRACE=1 "$tmpdir/program" > "$tmpdir/output" 2> "$tmpdir/trace"
printf 'graph text\ngraph text\n' > "$tmpdir/expected"
cmp "$tmpdir/output" "$tmpdir/expected"
python3 - "$tmpdir/trace" <<'PY'
import json, sys
records = [json.loads(line) for line in open(sys.argv[1])]
enters = [r for r in records if r['event'] == 'enter']
assert [r['node_id'] for r in enters] == ['source', 'receiver', 'left', 'right']
assert enters[1]['input_signal_id'] == enters[0]['output_signal_id']
assert enters[2]['input_signal_id'] == enters[3]['input_signal_id'] == enters[1]['output_signal_id']
assert len({r['delivery_id'] for r in enters[1:]}) == 3
PY
mv "$tmpdir/scalar-root.flow" "$tmpdir/program.flow"
compile
# A native graph may carry a Bool between source-defined receiver frames. The
# following chain checks typed wire matching and a Boolean branch before the
# final c_int observer.
cp "$tmpdir/program.flow" "$tmpdir/scalar-root.flow"
cat > "$tmpdir/bool.flow" <<'FLOW'
import "provider.flow" as host
program bool_native_graph
producer source : injected.batch
node classify : fn classify_bool
node accept : fn accept_bool
node display : fn observe_bool
wire source.out => classify.in
wire classify.out => accept.in
wire accept.out => display.in
fn classify_bool(value : c_int): Bool {
    result : Bool(false)
    value == 3 -> result
    return result
}
fn accept_bool(value : Bool): c_int {
    if value {
        return 7
    }
    return 8
}
fn observe_bool(value : c_int): c_int {
    result : c_int(0)
    host.observe(value) -> result
    return value
}
main {
    return 0
}
FLOW
cp "$tmpdir/bool.flow" "$tmpdir/program.flow"
compile
FLOWCORE_GRAPH_TRACE=1 "$tmpdir/program" > "$tmpdir/output" 2> "$tmpdir/trace"
printf '7\n' > "$tmpdir/expected"
cmp "$tmpdir/output" "$tmpdir/expected"
python3 - "$tmpdir/trace" <<'PY'
import json, sys
records = [json.loads(line) for line in open(sys.argv[1])]
enters = [r for r in records if r['event'] == 'enter']
assert [r['node_id'] for r in enters] == ['source', 'classify', 'accept', 'display']
assert enters[1]['input_signal_id'] == enters[0]['output_signal_id']
assert enters[2]['input_signal_id'] == enters[1]['output_signal_id']
assert enters[3]['input_signal_id'] == enters[2]['output_signal_id']
assert len({r['delivery_id'] for r in enters[1:]}) == 3
PY
mv "$tmpdir/scalar-root.flow" "$tmpdir/program.flow"
compile
# Raw pointer payloads are not part of the native graph contract. A pointer
# receiver must fail at the graph type boundary before native emission.
cp "$tmpdir/program.flow" "$tmpdir/scalar-root.flow"
cat > "$tmpdir/pointer.flow" <<'FLOW'
import "provider.flow" as host
program raw_pointer_graph
producer source : injected.batch
node receiver : fn pointer_identity
wire source.out => receiver.in
fn pointer_identity(value : c_pointer): c_pointer {
    return value
}
main {
    return 0
}
FLOW
cp "$tmpdir/pointer.flow" "$tmpdir/program.flow"
compile_args='--lowering-plan-version 2 --graph-plan-version 2'
set +e
"$FLOWMINI_BIN" --dump-frontend-bundle "$tmpdir/program.flow" > "$tmpdir/pointer.frontend.json"
"$FLOWANALYST_BIN" $compile_args --graph-providers "$tmpdir/selection.json" < "$tmpdir/pointer.frontend.json" > "$tmpdir/pointer.semantic.json"
status=$?
set -e
test "$status" -ne 0
jq -e '.status == "error" and
    any(.diagnostics[]; .code == "FLOWANALYST_GRAPH_RECEIVER_CARRIER")' \
    "$tmpdir/pointer.semantic.json" >/dev/null
mv "$tmpdir/scalar-root.flow" "$tmpdir/program.flow"
compile
# Aggregate receiver payloads are likewise outside the scalar native contract,
# even when the record body itself is a valid ordinary Flow type.
cp "$tmpdir/program.flow" "$tmpdir/scalar-root.flow"
cat > "$tmpdir/aggregate.flow" <<'FLOW'
import "provider.flow" as host
program aggregate_graph
type Packet {
    field value : int
}
producer source : injected.batch
node receiver : fn aggregate_identity
wire source.out => receiver.in
fn aggregate_identity(value : Packet): Packet {
    return value
}
main {
    return 0
}
FLOW
cp "$tmpdir/aggregate.flow" "$tmpdir/program.flow"
set +e
"$FLOWMINI_BIN" --dump-frontend-bundle "$tmpdir/program.flow" > "$tmpdir/aggregate.frontend.json"
"$FLOWANALYST_BIN" --lowering-plan-version 2 --graph-plan-version 2 \
    --graph-providers "$tmpdir/selection.json" < "$tmpdir/aggregate.frontend.json" > "$tmpdir/aggregate.semantic.json"
status=$?
set -e
test "$status" -ne 0
jq -e '.status == "error" and
    any(.diagnostics[]; .code == "FLOWANALYST_GRAPH_RECEIVER_CARRIER")' \
    "$tmpdir/aggregate.semantic.json" >/dev/null
mv "$tmpdir/scalar-root.flow" "$tmpdir/program.flow"
compile
# A startup provider failure is an activation failure: it has no normal output
# and cannot activate the receiver downstream.
cp "$tmpdir/program.flow" "$tmpdir/scalar-root.flow"
cp "$tmpdir/selection.json" "$tmpdir/scalar-root.selection.json"
cat > "$tmpdir/provider-failure.flow" <<'FLOW'
import "provider.flow" as host
program provider_failure_graph
producer source : injected.failure
node receiver : fn should_not_run
wire source.out => receiver.in
fn should_not_run(value : c_int): c_int {
    result : c_int(0)
    host.observe(value) -> result
    return value
}
main {
    return 0
}
FLOW
cp "$tmpdir/provider-failure.flow" "$tmpdir/program.flow"
jq '.providers += [{implementation:"injected.failure",source_callable:"host.failure",activation:"startup_once",output_port:"out"}]' \
    "$tmpdir/selection.json" > "$tmpdir/provider-failure.selection.json"
mv "$tmpdir/provider-failure.selection.json" "$tmpdir/selection.json"
compile
set +e
FLOWCORE_GRAPH_TRACE=1 "$tmpdir/program" > "$tmpdir/output" 2> "$tmpdir/trace"
status=$?
set -e
test "$status" -eq 70
test ! -s "$tmpdir/output"
python3 - "$tmpdir/trace" <<'PY'
import json, sys
records = [json.loads(line) for line in open(sys.argv[1])]
failure = records[-1]
assert failure['format'] == 'flowcore.graph_failure'
assert failure['reason'] == 'source_failure' and failure['code'] == 23
assert failure['activation']['node_id'] == 'source'
assert failure['activation']['kind'] == 'startup'
assert failure['activation']['wire_id'] == ''
assert not any(r.get('node_id') == 'receiver' for r in records)
PY
mv "$tmpdir/scalar-root.flow" "$tmpdir/program.flow"
mv "$tmpdir/scalar-root.selection.json" "$tmpdir/selection.json"
compile
# The static native schedule has a hard 65,536-activation bound. Generate one
# extra receiver activation and require Flowparallel to reject the plan before
# any backend lowering can observe a partial schedule.
cp "$tmpdir/program.flow" "$tmpdir/scalar-root.flow"
set +e
"$FLOWANALYST_BIN" --lowering-plan-version 2 --graph-plan-version 2 \
    --graph-providers "$tmpdir/selection.json" < "$tmpdir/frontend.json" > "$tmpdir/oversized.base.json"
jq '
    . as $root
    | ($root.lowering_plan.source_graph.syntax.wires[] | select(.wire_id == "wire:0")) as $wire
    | $root
    | .lowering_plan.source_graph.syntax.nodes =
        [.lowering_plan.source_graph.syntax.nodes[] | select(.node_id == "source" or .node_id == "receiver")]
    | .lowering_plan.source_graph.syntax.wires =
        [range(0; 65536) as $index
         | ($wire | .wire_id = ("wire:" + ($index | tostring)))]
    | .lowering_plan.source_graph.receivers =
        [.lowering_plan.source_graph.receivers[] | select(.node_id == "receiver")]
    | .lowering_plan.source_graph.providers =
        [.lowering_plan.source_graph.providers[] | select(.node_id == "source")]
' "$tmpdir/oversized.base.json" > "$tmpdir/oversized.semantic.json"
"$FLOWPARALLEL_BIN" < "$tmpdir/oversized.semantic.json" > "$tmpdir/oversized.execution.json"
status=$?
set -e
test "$status" -ne 0
test ! -s "$tmpdir/oversized.execution.json"
mv "$tmpdir/scalar-root.flow" "$tmpdir/program.flow"
compile
# Every consumer reads a durable captured file and refuses mutated scheduling.
for mutation in '.graph_schedule.steps |= reverse' '.graph_schedule.policy = "parallel"' '.graph_schedule.activation_contract = "persistent"' '.graph_schedule.steps[1].wire_id = "wrong"' '.graph_schedule.steps[2].input_signal_id = 99' '.graph_schedule.steps[1].input_port = "out"' 'del(.graph_schedule)' '.lowering_plan.source_graph.syntax.wires += [(.lowering_plan.source_graph.syntax.wires[0] | .wire_id = "cycle" | .from.node_id = "left")]' '.lowering_plan.source_graph.receivers[0].function_symbol_id = 999' '.lowering_plan.source_graph.providers[0].provider.symbol = "other_value"'; do
    jq "$mutation" "$tmpdir/execution.json" > "$tmpdir/bad.execution.json"
    if "$FLOWOPTIMIZE_BIN" < "$tmpdir/bad.execution.json" >/dev/null 2>&1; then echo 'mutated graph schedule optimized' >&2; exit 1; fi
    jq "$mutation" "$tmpdir/backend.json" > "$tmpdir/bad.backend.json"
    if "$FLOWLOWER_BIN" --emit-llvm "$tmpdir/bad.ll" < "$tmpdir/bad.backend.json" >/dev/null 2>&1; then echo 'mutated graph schedule lowered' >&2; exit 1; fi
    test ! -e "$tmpdir/bad.ll"
done
# No matching graph producer grant means no authority, even with symbol evidence.
grep -v ' input_value ' "$tmpdir/policy" > "$tmpdir/denied.policy"
if "$FLOWBIND_BIN" --policy "$tmpdir/denied.policy" < "$tmpdir/semantic.json" >/dev/null 2>&1; then echo 'ungranted graph producer accepted' >&2; exit 1; fi
# An unused provider selection cannot change emitted behavior.
cp "$tmpdir/program.ll" "$tmpdir/original.ll"
jq '.providers += [{implementation:"unused.input",source_callable:"host.other",activation:"startup_once",output_port:"out"}]' "$tmpdir/selection.json" > "$tmpdir/unused.json"
mv "$tmpdir/unused.json" "$tmpdir/selection.json"
compile
cmp "$tmpdir/program.ll" "$tmpdir/original.ll"
# Input selection is explicit, separate from receiver evaluation and scheduling.
jq '(.providers[] | select(.implementation == "injected.batch")).source_callable = "host.other"' "$tmpdir/selection.json" > "$tmpdir/other.json"
mv "$tmpdir/other.json" "$tmpdir/selection.json"
compile
"$tmpdir/program" > "$tmpdir/output" 2> "$tmpdir/trace"
printf '8\n9\n8\n9\n' > "$tmpdir/expected"
cmp "$tmpdir/output" "$tmpdir/expected"
# Wire order changes delivery order without changing receiver evaluation rules.
python3 - "$tmpdir/program.flow" <<'PYTHON'
import sys
p = sys.argv[1]
s = open(p).read().replace('fresh_native_graph', 'renamed_successful_graph')
s = s.replace('wire receiver.out => left.in\nwire receiver.out => right.in', 'wire receiver.out => right.in\nwire receiver.out => left.in')
open(p, 'w').write(s)
PYTHON
compile
"$tmpdir/program" > "$tmpdir/output" 2> "$tmpdir/trace"
printf '9\n8\n9\n8\n' > "$tmpdir/expected"
cmp "$tmpdir/output" "$tmpdir/expected"
# A failed receiver produces no normal result and never activates its fan-out.
sed 's/local + value -> local/value \/ 0 -> local/; s/program fresh_native_graph/program renamed_failed_graph/' "$tmpdir/program.flow" > "$tmpdir/failure.flow"
mv "$tmpdir/failure.flow" "$tmpdir/program.flow"
compile
set +e
FLOWCORE_GRAPH_TRACE=1 "$tmpdir/program" > "$tmpdir/output" 2> "$tmpdir/trace"
status=$?
set -e
test "$status" -eq 70
test ! -s "$tmpdir/output"
python3 - "$tmpdir/trace" <<'PY'
import json, sys
records = [json.loads(line) for line in open(sys.argv[1])]
failures = [r for r in records if r['format'] == 'flowcore.graph_failure']
assert len(failures) == 1
failure = failures[0]
assert failure['reason'] == 'invalid_division' and failure['operation_id'] >= 0
assert failure['activation']['node_id'] == 'receiver' and failure['activation']['wire_id'] == 'wire:0'
assert not any(r.get('event') == 'output' and r['node_id'] == 'receiver' for r in records)
assert not any(r.get('node_id') in ['left', 'right'] for r in records)
PY
# Source can reject an activation through an explicitly authorized runtime ABI.
jq -n --arg path "$FLOWGRAPH_RUNTIME" '{format:"flowcore.native_binding_spec",version:1,unit:"graph_failures",namespace:"runtime",provider:{soname:$path,path:$path,convention:"c"},functions:[{name:"raise",symbol:"flow_graph_raise",effect:"failure",parameters:[{name:"code",type:"c_int"}],return_type:"c_int"}]}' > "$tmpdir/runtime.spec.json"
"$root/tools/generate-flow-bindings.sh" --spec "$tmpdir/runtime.spec.json" --flow-output "$tmpdir/runtime.flow" --policy-output "$tmpdir/runtime.policy" --manifest-output "$tmpdir/runtime.manifest.json" >/dev/null
cat "$tmpdir/runtime.policy" >> "$tmpdir/policy"
sed '1i import "runtime.flow" as runtime' "$tmpdir/program.flow" | sed 's/value \/ 0 -> local/runtime.raise(17) -> local/' > "$tmpdir/raised.flow"
mv "$tmpdir/raised.flow" "$tmpdir/program.flow"
compile
set +e
FLOWCORE_GRAPH_TRACE=1 "$tmpdir/program" > "$tmpdir/output" 2> "$tmpdir/trace"
status=$?
set -e
test "$status" -eq 70
test ! -s "$tmpdir/output"
python3 - "$tmpdir/trace" <<'PY'
import json, sys
records = [json.loads(line) for line in open(sys.argv[1])]
failure = records[-1]
assert failure['format'] == 'flowcore.graph_failure' and failure['reason'] == 'source_failure'
assert failure['code'] == 17 and failure['operation_id'] >= 0
assert failure['activation']['node_id'] == 'receiver' and failure['activation']['wire_id'] == 'wire:0'
assert not any(r.get('event') == 'output' and r['node_id'] == 'receiver' for r in records)
PY
sha256sum --check --status "$tmpdir/tools.sha256"
echo 'native source graph: PASS'
