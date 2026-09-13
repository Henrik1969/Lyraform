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
allow ./build/libflowmini_testabi.so long_input c pure
allow ./build/libflowmini_testabi.so long_sum c pure LongValue c_int
allow libc.so.6 puts c io c_string c_int
POLICY
cat > "$tmpdir/providers.json" <<'JSON'
{"format":"flowcore.graph_provider_map","version":1,"providers":[{"implementation":"wideabi.long_aggregate","source_callable":"wideabi.long_input","activation":"startup_once","output_port":"out"}]}
JSON
cat > "$tmpdir/program.flow" <<FLOW
import "$root/Lyraform/compiler/std/abi/wideabi.flow"
import "$root/Lyraform/compiler/std/abi/libc.flow"
program tinyvm_long_aggregate
producer source : wideabi.long_aggregate
node left : fn consume_long
node right : fn consume_long
wire source.out => left.in
wire source.out => right.in
fn consume_long(value : LongValue): c_int {
    result : c_int(0)
    long_sum(value) -> result
    libc.puts("42") -> result
    return result
}
main { return 0 }
FLOW

cd "$root/Lyraform/compiler"
"$flowmini" --dump-frontend-bundle "$tmpdir/program.flow" > "$tmpdir/frontend.json"
"$analyst" --lowering-plan-version 2 --graph-plan-version 2 --graph-providers "$tmpdir/providers.json" < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
"$layout" > "$tmpdir/layout.json"
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
"$tiny_run" --engine computed --policy "$tmpdir/policy" "$tmpdir/program.tvm" > "$tmpdir/tiny.computed.stdout"
sed '$d' "$tmpdir/tiny.computed.stdout" > "$tmpdir/tiny.computed.program.stdout"
cmp "$tmpdir/tiny.program.stdout" "$tmpdir/tiny.computed.program.stdout"
printf '42\n42\n' > "$tmpdir/expected.stdout"
cmp "$tmpdir/expected.stdout" "$tmpdir/llvm.stdout"
jq -e '.aggregate_abi_layouts[] | select(.name == "LongValue") | .status == "verified" and .size == 8 and .fields[0].type == "c_long"' "$tmpdir/tiny.backend.json" >/dev/null
jq -e '.status == "emitted" and .backend == "tinyvm"' "$tmpdir/tiny.report.json" >/dev/null
"$tiny_lower" "$tmpdir/tiny.backend.json" "$tmpdir/program-again.tvm" >/dev/null
cmp "$tmpdir/program.tvm" "$tmpdir/program-again.tvm"

grep -q 'declare i64 @long_input()' "$tmpdir/program.ll"
grep -q 'declare i32 @long_sum(i64)' "$tmpdir/program.ll"
echo 'TinyVM c_long aggregate parity: PASS'
