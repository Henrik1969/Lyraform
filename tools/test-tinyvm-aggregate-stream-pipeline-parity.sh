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
allow ./build/libflowmini_testabi.so long_count c readonly
allow ./build/libflowmini_testabi.so long_item c readonly c_size_t LongValue
allow ./build/libflowmini_testabi.so long_sum c pure LongValue c_int
allow libc.so.6 puts c io c_string c_int
POLICY
cat > "$tmpdir/providers.json" <<'JSON'
{"format":"flowcore.graph_provider_map","version":2,"providers":[{"implementation":"wideabi.long_stream","count_callable":"wideabi.long_count","item_callable":"wideabi.long_item","activation":"finite_stream_once","max_items":2,"output_port":"out"}]}
JSON
cat > "$tmpdir/program.flow" <<FLOW
import "$root/Lyraform/compiler/std/abi/wideabi.flow"
import "$root/Lyraform/compiler/std/abi/libc.flow"
program tinyvm_aggregate_stream_pipeline
producer source : wideabi.long_stream
node pass : fn pass_long
node receiver : fn consume_long
wire source.out => pass.in
wire pass.out => receiver.in
fn pass_long(value : LongValue): LongValue {
    return value
}
fn consume_long(value : LongValue): c_int {
    result : c_int(0)
    long_sum(value) -> result
    if result == 42 {
        libc.puts("aggregate pipeline forty-two") -> result
    } else {
        libc.puts("aggregate pipeline forty-three") -> result
    }
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
"$tiny_run" --trace-graph --policy "$tmpdir/policy" "$tmpdir/program.tvm" > "$tmpdir/tiny.trace.stdout" 2> "$tmpdir/tiny.trace.jsonl"
cmp "$tmpdir/tiny.stdout" "$tmpdir/tiny.trace.stdout"
jq -s -e 'length == 5 and
    all(.[]; .format == "flowcore.tinyvm_graph_activation" and .event == "enter") and
    ([.[].sequence]) == [0, 1, 2, 3, 4] and
    .[0].kind == "stream_root" and .[0].node_id == "source" and .[0].wire_id == "-" and .[0].stream_index == null and
    ([.[1:][] | .node_id]) == ["pass", "receiver", "pass", "receiver"] and
    ([.[1:][] | .stream_index]) == [0, 0, 1, 1] and
    all(.[1:][]; .input_port == "in" and .output_port == "out" and .wire_id != "-")' "$tmpdir/tiny.trace.jsonl" >/dev/null
printf 'aggregate pipeline forty-two\naggregate pipeline forty-three\n' > "$tmpdir/expected.stdout"
cmp "$tmpdir/expected.stdout" "$tmpdir/llvm.stdout"
jq -e '.graph_schedule.version == 5 and .graph_schedule.stream_contract == "finite_aggregate_stream_pipeline_v1" and (.graph_schedule.steps | length) == 3 and .graph_schedule.steps[1].input_activation_id == 0 and .graph_schedule.steps[2].input_activation_id == 1 and .graph_schedule.steps[1].input_signal_id == 1 and .graph_schedule.steps[2].input_signal_id == 2 and (.graph_schedule.streams[0].deliveries | length) == 2' "$tmpdir/execution.json" >/dev/null
jq -e '.aggregate_abi_layouts[] | select(.name == "LongValue") | .status == "verified" and .size == 8 and .alignment == 8' "$tmpdir/tiny.backend.json" >/dev/null
jq -e '.status == "emitted" and .backend == "tinyvm"' "$tmpdir/tiny.report.json" >/dev/null
"$tiny_lower" "$tmpdir/tiny.backend.json" "$tmpdir/program-again.tvm" >/dev/null
cmp "$tmpdir/program.tvm" "$tmpdir/program-again.tvm"

jq '.graph_schedule.stream_contract = "finite_scalar_stream_pipeline_v1"' "$tmpdir/tiny.backend.json" > "$tmpdir/mismatched.backend.json"
if "$tiny_lower" "$tmpdir/mismatched.backend.json" "$tmpdir/mismatched.tvm" > "$tmpdir/mismatched.report.json" 2>/dev/null; then
    echo 'TinyVM unexpectedly admitted a scalar/aggregate stream-contract mismatch' >&2
    exit 1
fi
test ! -e "$tmpdir/mismatched.tvm"

echo 'TinyVM aggregate stream pipeline parity: PASS'
