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

# The ABI example intentionally uses its declared relative provider path. Keep
# the generated test dependency at that declared location while the pipeline
# is run from the compiler project directory.
mkdir -p "$root/Lyraform/compiler/build"
cp "$provider_library" "$root/Lyraform/compiler/build/libflowmini_testabi.so"

printf '%s\n' \
    'allow ./build/libflowmini_testabi.so point_input c pure - Point' \
    'allow ./build/libflowmini_testabi.so point_sum c pure Point c_int' \
    'allow ./build/libflowmini_testabi.so point_observe c io c_int c_int' > "$tmpdir/policy"
printf '%s\n' '{"format":"flowcore.graph_provider_map","version":1,"providers":[{"implementation":"testabi.aggregate","source_callable":"testabi.point_input","activation":"startup_once","output_port":"out"}]}' > "$tmpdir/providers.json"
"$layout" > "$tmpdir/layout.json"

cd "$root/Lyraform/compiler"
"$flowmini" --dump-frontend-bundle examples/graph/abi_aggregate_graph.flow > "$tmpdir/frontend.json"
"$analyst" --lowering-plan-version 2 --graph-plan-version 2 --graph-providers "$tmpdir/providers.json" < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
"$bind" --policy "$tmpdir/policy" --abi-manifest "$tmpdir/layout.json" < "$tmpdir/semantic.json" > "$tmpdir/binding.json"
"$parallel" < "$tmpdir/semantic.json" > "$tmpdir/execution.json"
"$optimizer" < "$tmpdir/execution.json" > "$tmpdir/optimization.json"
"$prepare" --binding-report "$tmpdir/binding.json" < "$tmpdir/optimization.json" > "$tmpdir/backend.json"
"$lower" --emit-llvm "$tmpdir/program.ll" < "$tmpdir/backend.json" > "$tmpdir/lowering.json"
clang -c "$tmpdir/program.ll" -o "$tmpdir/program.o"
"$cxx" ${FLOWGRAPH_LINK_FLAGS:-} "$tmpdir/program.o" "$runtime" "$provider_library" \
    "-Wl,-rpath,$(dirname "$runtime")" "-Wl,-rpath,$(dirname "$provider_library")" -o "$tmpdir/program"

FLOWCORE_GRAPH_TRACE=1 "$tmpdir/program" > "$tmpdir/output" 2> "$tmpdir/trace"
printf '3\n3\n' > "$tmpdir/expected"
cmp "$tmpdir/output" "$tmpdir/expected"
jq -e '.status == "ready" and .aggregate_abi_layouts[0].status == "verified" and .aggregate_abi_layouts[0].size == 8' "$tmpdir/backend.json" >/dev/null
jq -e -s '[.[] | select(.event == "enter") | .node_id] == ["source", "left", "right"]' "$tmpdir/trace" >/dev/null
grep -q 'declare i64 @point_input()' "$tmpdir/program.ll"
grep -q 'declare i32 @point_sum(i64)' "$tmpdir/program.ll"

jq '.types[0].fields[1].offset = 8' "$tmpdir/layout.json" > "$tmpdir/hostile-layout.json"
if "$bind" --policy "$tmpdir/policy" --abi-manifest "$tmpdir/hostile-layout.json" < "$tmpdir/semantic.json" > "$tmpdir/hostile-binding.json" 2>/dev/null; then
    echo 'hostile aggregate layout unexpectedly accepted' >&2
    exit 1
fi

# A consumer must independently recheck the verified layout proof; a forged
# backend artifact must not become native LLVM merely because Flowbind was
# bypassed.
jq '.aggregate_abi_layouts[0].fields[0].type = "c_long"' "$tmpdir/backend.json" > "$tmpdir/hostile-backend.json"
if "$lower" --emit-llvm "$tmpdir/hostile.ll" < "$tmpdir/hostile-backend.json" > "$tmpdir/hostile-lowering.json" 2>/dev/null; then
    echo 'hostile aggregate backend artifact unexpectedly lowered' >&2
    exit 1
fi
test ! -e "$tmpdir/hostile.ll"
jq -e '.status == "unsupported" and (.diagnostic.reason | contains("aggregate ABI"))' "$tmpdir/hostile-lowering.json" >/dev/null

echo 'Native aggregate graph: PASS'
