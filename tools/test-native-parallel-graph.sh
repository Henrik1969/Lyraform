#!/bin/sh
set -eu

root=${FLOWCORE_ROOT:?FLOWCORE_ROOT is required}
flowmini=${FLOWMINI_BIN:?FLOWMINI_BIN is required}
analyst=${FLOWANALYST_BIN:?FLOWANALYST_BIN is required}
bind=${FLOWBIND_BIN:?FLOWBIND_BIN is required}
parallel=${FLOWPARALLEL_BIN:?FLOWPARALLEL_BIN is required}
optimizer=${FLOWOPTIMIZE_BIN:?FLOWOPTIMIZE_BIN is required}
prepare=${FLOWPREPARE_BIN:?FLOWPREPARE_BIN is required}
lower=${FLOWLOWER_BIN:?FLOWLOWER_BIN is required}
runtime=${FLOWGRAPH_RUNTIME:?FLOWGRAPH_RUNTIME is required}
cxx=${FLOWGRAPH_CXX:?FLOWGRAPH_CXX is required}

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

cat > "$tmpdir/provider.c" <<'C'
int input_value(void) { return 3; }
C
clang -shared -fPIC "$tmpdir/provider.c" -o "$tmpdir/provider.so"
jq -n --arg path "$tmpdir/provider.so" '{format:"flowcore.native_binding_spec",version:1,unit:"parallel_graph_provider",namespace:"host",provider:{soname:$path,path:$path,convention:"c"},functions:[{name:"input",symbol:"input_value",effect:"readonly",parameters:[],return_type:"c_int"}]}' > "$tmpdir/spec.json"
"$root/tools/generate-flow-bindings.sh" --spec "$tmpdir/spec.json" --flow-output "$tmpdir/provider.flow" --policy-output "$tmpdir/policy" --manifest-output "$tmpdir/manifest.json" >/dev/null
cat > "$tmpdir/selection.json" <<'JSON'
{"format":"flowcore.graph_provider_map","version":3,"providers":[{"implementation":"injected.batch","source_callable":"host.input","activation":"startup_once","output_port":"out","schedule_policy":"parallel_independent_v1"}]}
JSON
cat > "$tmpdir/program.flow" <<'FLOW'
import "provider.flow" as host
program parallel_native_graph
producer source : injected.batch
node receiver : fn transform
node left : fn observe_left
node right : fn observe_right
wire source.out => receiver.in
wire receiver.out => left.in
wire receiver.out => right.in
fn transform(value : c_int): c_int {
    return value + 1
}
fn observe_left(value : c_int): c_int {
    return value
}
fn observe_right(value : c_int): c_int {
    return value + 1
}
main { return 0 }
FLOW

cd "$root/Lyraform/compiler"
"$flowmini" --dump-frontend-bundle "$tmpdir/program.flow" > "$tmpdir/frontend.json"
"$analyst" --lowering-plan-version 2 --graph-plan-version 2 --graph-providers "$tmpdir/selection.json" < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
"$bind" --policy "$tmpdir/policy" < "$tmpdir/semantic.json" > "$tmpdir/binding.json"
"$parallel" < "$tmpdir/semantic.json" > "$tmpdir/execution.json"
"$optimizer" < "$tmpdir/execution.json" > "$tmpdir/optimization.json"
"$prepare" --binding-report "$tmpdir/binding.json" < "$tmpdir/optimization.json" > "$tmpdir/backend.json"
"$lower" --emit-llvm "$tmpdir/program.ll" < "$tmpdir/backend.json" > "$tmpdir/lowering.json"
clang -c "$tmpdir/program.ll" -o "$tmpdir/program.o"
"$cxx" ${FLOWGRAPH_LINK_FLAGS:-} "$tmpdir/program.o" "$runtime" "$tmpdir/provider.so" "-Wl,-rpath,$(dirname "$runtime")" "-Wl,-rpath,$(dirname "$tmpdir/provider.so")" -o "$tmpdir/program"

FLOWCORE_GRAPH_TRACE=1 "$tmpdir/program" > "$tmpdir/output" 2> "$tmpdir/trace"
test ! -s "$tmpdir/output"
jq -e '.graph_schedule.version == 4 and .graph_schedule.parallel_contract == "dependency_waves_v1" and .graph_schedule.parallel_waves[1].activation_ids == [1] and .graph_schedule.parallel_waves[2].activation_ids == [2,3]' "$tmpdir/execution.json" >/dev/null
jq -e '.status == "ready"' "$tmpdir/backend.json" >/dev/null
python3 - "$tmpdir/trace" <<'PY'
import json, sys
records = [json.loads(line) for line in open(sys.argv[1])]
enters = [r for r in records if r.get('event') == 'enter']
assert [r['node_id'] for r in enters[:2]] == ['source', 'receiver']
assert {r['node_id'] for r in enters[2:]} == {'left', 'right'}
assert [r['node_id'] for r in records if r.get('event') == 'output'] == ['source', 'receiver', 'left', 'right']
results = [r for r in records if r.get('event') == 'result']
assert [(r['activation_id'], r['value']) for r in results] == [(1, 4), (2, 4), (3, 5)]
PY

# A worker body with mutable local state is not silently treated as pure. The
# graph remains analyzable, but the native parallel boundary must refuse it.
cat > "$tmpdir/impure.flow" <<'FLOW'
import "provider.flow" as host
program impure_parallel_graph
producer source : injected.batch
node receiver : fn transform
wire source.out => receiver.in
fn transform(value : c_int): c_int {
    result : c_int(0)
    value + 1 -> result
    return result
}
main { return 0 }
FLOW
"$flowmini" --dump-frontend-bundle "$tmpdir/impure.flow" > "$tmpdir/impure.frontend.json"
"$analyst" --lowering-plan-version 2 --graph-plan-version 2 --graph-providers "$tmpdir/selection.json" < "$tmpdir/impure.frontend.json" > "$tmpdir/impure.semantic.json"
"$bind" --policy "$tmpdir/policy" < "$tmpdir/impure.semantic.json" > "$tmpdir/impure.binding.json"
"$parallel" < "$tmpdir/impure.semantic.json" > "$tmpdir/impure.execution.json"
"$optimizer" < "$tmpdir/impure.execution.json" > "$tmpdir/impure.optimization.json"
"$prepare" --binding-report "$tmpdir/impure.binding.json" < "$tmpdir/impure.optimization.json" > "$tmpdir/impure.backend.json"
if "$lower" --emit-llvm "$tmpdir/impure.ll" < "$tmpdir/impure.backend.json" >/dev/null 2>&1; then
    echo 'effectful parallel receiver unexpectedly lowered' >&2
    exit 1
fi
test ! -e "$tmpdir/impure.ll"

echo 'Native parallel graph: PASS'
