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
layout=${FLOWTESTABI_LAYOUT_BIN:?FLOWTESTABI_LAYOUT_BIN is required}
provider_library=${FLOWTESTABI_LIBRARY:?FLOWTESTABI_LIBRARY is required}
runtime=${FLOWGRAPH_RUNTIME:?FLOWGRAPH_RUNTIME is required}
cxx=${FLOWGRAPH_CXX:?FLOWGRAPH_CXX is required}

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
mkdir -p "$root/Lyraform/compiler/build"
cp "$provider_library" "$root/Lyraform/compiler/build/libflowmini_testabi.so"
printf '%s\n' \
    'allow ./build/libflowmini_testabi.so point_input c pure - Point' \
    'allow ./build/libflowmini_testabi.so point_sum c pure Point c_int' > "$tmpdir/policy"
printf '%s\n' '{"format":"flowcore.graph_provider_map","version":3,"providers":[{"implementation":"testabi.aggregate","source_callable":"testabi.point_input","activation":"startup_once","output_port":"out","schedule_policy":"parallel_independent_v1"}]}' > "$tmpdir/providers.json"
"$layout" > "$tmpdir/layout.json"

cd "$root/Lyraform/compiler"
"$flowmini" --dump-frontend-bundle examples/graph/abi_aggregate_parallel_graph.flow > "$tmpdir/frontend.json"
"$analyst" --lowering-plan-version 2 --graph-plan-version 2 --graph-providers "$tmpdir/providers.json" < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
"$bind" --policy "$tmpdir/policy" --abi-manifest "$tmpdir/layout.json" < "$tmpdir/semantic.json" > "$tmpdir/binding.json"
"$parallel" < "$tmpdir/semantic.json" > "$tmpdir/execution.json"
"$optimizer" < "$tmpdir/execution.json" > "$tmpdir/optimization.json"
"$prepare" --binding-report "$tmpdir/binding.json" < "$tmpdir/optimization.json" > "$tmpdir/backend.json"
"$lower" --emit-llvm "$tmpdir/program.ll" < "$tmpdir/backend.json" > "$tmpdir/lowering.json"
clang -c "$tmpdir/program.ll" -o "$tmpdir/program.o"
"$cxx" "$tmpdir/program.o" "$runtime" "$provider_library" "-Wl,-rpath,$(dirname "$runtime")" "-Wl,-rpath,$(dirname "$provider_library")" -o "$tmpdir/program"

FLOWCORE_GRAPH_TRACE=1 "$tmpdir/program" >/dev/null 2> "$tmpdir/trace"
jq -e '.graph_schedule.version == 4 and .graph_schedule.parallel_waves[1].activation_ids == [1,2]' "$tmpdir/execution.json" >/dev/null
jq -e '.status == "ready" and .aggregate_abi_layouts[0].status == "verified"' "$tmpdir/backend.json" >/dev/null
python3 - "$tmpdir/trace" <<'PY'
import json, sys
records = [json.loads(line) for line in open(sys.argv[1])]
results = [r for r in records if r.get('event') == 'result']
assert [(r['activation_id'], r['value']) for r in results] == [(1, 8589934593), (2, 8589934593)]
PY

echo 'Native aggregate parallel graph: PASS'
