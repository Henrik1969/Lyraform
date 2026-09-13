#!/bin/sh
set -eu

root=${FLOWCORE_ROOT:?FLOWCORE_ROOT is required}
flowmini=${FLOWMINI_BIN:?FLOWMINI_BIN is required}
analyst=${FLOWANALYST_BIN:?FLOWANALYST_BIN is required}
bind=${FLOWBIND_BIN:?FLOWBIND_BIN is required}
parallel=${FLOWPARALLEL_BIN:?FLOWPARALLEL_BIN is required}
optimize=${FLOWOPTIMIZE_BIN:?FLOWOPTIMIZE_BIN is required}
prepare=${FLOWPREPARE_BIN:?FLOWPREPARE_BIN is required}
llvm_lower=${FLOWLOWER_BIN:?FLOWLOWER_BIN is required}
tiny_lower=${FLOWTINYLOWER_BIN:?FLOWTINYLOWER_BIN is required}
tiny_validate=${FLOWTINYVALIDATE_BIN:?FLOWTINYVALIDATE_BIN is required}
tiny_run=${FLOWTINYRUN_BIN:?FLOWTINYRUN_BIN is required}
layout=${FLOWTESTABI_LAYOUT_BIN:?FLOWTESTABI_LAYOUT_BIN is required}
provider_library=${FLOWTESTABI_LIBRARY:?FLOWTESTABI_LIBRARY is required}
graph_runtime=${FLOWGRAPH_RUNTIME:?FLOWGRAPH_RUNTIME is required}
graph_cxx=${FLOWGRAPH_CXX:?FLOWGRAPH_CXX is required}

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
mkdir -p "$root/Lyraform/compiler/build"
cp "$provider_library" "$root/Lyraform/compiler/build/libflowmini_testabi.so"
cat > "$tmpdir/policy" <<'POLICY'
allow ./build/libflowmini_testabi.so point_input c pure
allow ./build/libflowmini_testabi.so point_sum c pure Point c_int
allow libc.so.6 puts c io c_string c_int
POLICY
cat > "$tmpdir/providers.json" <<'JSON'
{"format":"flowcore.graph_provider_map","version":1,"providers":[{"implementation":"testabi.aggregate","source_callable":"testabi.point_input","activation":"startup_once","output_port":"out"}]}
JSON
"$layout" > "$tmpdir/layout.json"
cat > "$tmpdir/program.flow" <<FLOW
import "$root/Lyraform/compiler/std/abi/testabi.flow"
import "$root/Lyraform/compiler/std/abi/libc.flow"
program tinyvm_aggregate
producer source : testabi.aggregate
node left : fn consume_point
node right : fn consume_point
wire source.out => left.in
wire source.out => right.in
fn consume_point(point : Point): c_int {
    result : c_int(0)
    point_sum(point) -> result
    libc.puts("tinyvm aggregate") -> result
    return result
}
main { return 0 }
FLOW

cd "$root/Lyraform/compiler"
"$flowmini" --dump-frontend-bundle "$tmpdir/program.flow" > "$tmpdir/frontend.json"
"$analyst" --lowering-plan-version 2 --graph-plan-version 2 --graph-providers "$tmpdir/providers.json" < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
"$bind" --policy "$tmpdir/policy" --abi-manifest "$tmpdir/layout.json" < "$tmpdir/semantic.json" > "$tmpdir/binding.json"
"$parallel" < "$tmpdir/semantic.json" > "$tmpdir/execution.json"
"$optimize" < "$tmpdir/execution.json" > "$tmpdir/optimization.json"
"$prepare" --binding-report "$tmpdir/binding.json" --target-policy "$root/Flowlower/target-policies/llvm-host.json" "$tmpdir/optimization.json" > "$tmpdir/llvm.backend.json"
"$llvm_lower" --emit-llvm "$tmpdir/program.ll" "$tmpdir/llvm.backend.json" >/dev/null
clang -c "$tmpdir/program.ll" -o "$tmpdir/program.o"
"$graph_cxx" ${FLOWGRAPH_LINK_FLAGS:-} "$tmpdir/program.o" "-Wl,-rpath,$(dirname "$graph_runtime")" "-Wl,-rpath,$root/Lyraform/compiler/build" "$graph_runtime" "$root/Lyraform/compiler/build/libflowmini_testabi.so" -o "$tmpdir/program.llvm"
"$prepare" --binding-report "$tmpdir/binding.json" --target-policy "$root/Flowlower/target-policies/tinyvm-portable.json" "$tmpdir/optimization.json" > "$tmpdir/tiny.backend.json"
"$tiny_lower" "$tmpdir/tiny.backend.json" "$tmpdir/program.tvm" > "$tmpdir/tiny.report.json"
"$tiny_validate" "$tmpdir/program.tvm" | grep -q '"status":"valid"'

"$tmpdir/program.llvm" > "$tmpdir/llvm.stdout"
"$tiny_run" --policy "$tmpdir/policy" "$tmpdir/program.tvm" > "$tmpdir/tiny.stdout"
sed '$d' "$tmpdir/tiny.stdout" > "$tmpdir/tiny.program.stdout"
cmp "$tmpdir/llvm.stdout" "$tmpdir/tiny.program.stdout"
printf 'tinyvm aggregate\ntinyvm aggregate\n' > "$tmpdir/expected.stdout"
cmp "$tmpdir/expected.stdout" "$tmpdir/llvm.stdout"
test "$(tail -n 1 "$tmpdir/tiny.stdout" | jq -r .result)" -eq 0
jq -e '.aggregate_abi_layouts[0].status == "verified" and .graph_schedule.version == 1' "$tmpdir/tiny.backend.json" >/dev/null
jq -e '.status == "emitted" and .backend == "tinyvm"' "$tmpdir/tiny.report.json" >/dev/null
"$tiny_lower" "$tmpdir/tiny.backend.json" "$tmpdir/program-again.tvm" >/dev/null
cmp "$tmpdir/program.tvm" "$tmpdir/program-again.tvm"

echo 'TinyVM verified aggregate parity: PASS'
