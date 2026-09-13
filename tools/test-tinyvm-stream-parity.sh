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
graph_runtime=${FLOWGRAPH_RUNTIME:?FLOWGRAPH_RUNTIME is required}
graph_cxx=${FLOWGRAPH_CXX:?FLOWGRAPH_CXX is required}

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
cat > "$tmpdir/provider.c" <<'C'
#include <stddef.h>
size_t stream_count(void) { return 3; }
int stream_item(size_t index) { return (int)(10 + index); }
C
clang -shared -fPIC "$tmpdir/provider.c" -o "$tmpdir/provider.so"
cat > "$tmpdir/spec.json" <<JSON
{"format":"flowcore.native_binding_spec","version":1,"unit":"tinyvm_stream_provider","namespace":"graph_stream","provider":{"soname":"$tmpdir/provider.so","path":"$tmpdir/provider.so","convention":"c"},"functions":[{"name":"stream_count","symbol":"stream_count","effect":"readonly","parameters":[],"return_type":"c_size_t"},{"name":"stream_item","symbol":"stream_item","effect":"readonly","parameters":[{"name":"index","type":"c_size_t"}],"return_type":"c_int"}]}
JSON
"$root/tools/generate-flow-bindings.sh" --spec "$tmpdir/spec.json" --flow-output "$tmpdir/provider.flow" --policy-output "$tmpdir/policy" --manifest-output "$tmpdir/manifest.json" >/dev/null
cat >> "$tmpdir/policy" <<POLICY
allow $tmpdir/provider.so stream_count c readonly
allow $tmpdir/provider.so stream_item c readonly c_size_t c_int
allow libc.so.6 puts c io c_string c_int
POLICY
cat > "$tmpdir/providers.json" <<'JSON'
{"format":"flowcore.graph_provider_map","version":2,"providers":[{"implementation":"injected.stream","count_callable":"stream.stream_count","item_callable":"stream.stream_item","activation":"finite_stream_once","max_items":16,"output_port":"out"}]}
JSON
cat > "$tmpdir/program.flow" <<FLOW
import "$tmpdir/provider.flow" as stream
import "$root/Lyraform/compiler/std/abi/libc.flow"
program tinyvm_stream
producer source : injected.stream
node left : fn announce_left
node right : fn announce_right
wire source.out => left.in
wire source.out => right.in
fn announce_left(value : c_int): c_int {
    result : c_int(0)
    libc.puts("tinyvm stream left") -> result
    return value
}
fn announce_right(value : c_int): c_int {
    result : c_int(0)
    libc.puts("tinyvm stream right") -> result
    return value
}
main { return 0 }
FLOW

"$flowmini" --dump-frontend-bundle "$tmpdir/program.flow" > "$tmpdir/frontend.json"
"$analyst" --lowering-plan-version 2 --graph-plan-version 2 --graph-providers "$tmpdir/providers.json" < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
"$bind" --policy "$tmpdir/policy" < "$tmpdir/semantic.json" > "$tmpdir/binding.json"
"$parallel" < "$tmpdir/semantic.json" > "$tmpdir/execution.json"
"$optimize" < "$tmpdir/execution.json" > "$tmpdir/optimization.json"
"$prepare" --binding-report "$tmpdir/binding.json" --target-policy "$root/Flowlower/target-policies/llvm-host.json" "$tmpdir/optimization.json" > "$tmpdir/llvm.backend.json"
"$llvm_lower" --emit-llvm "$tmpdir/program.ll" "$tmpdir/llvm.backend.json" >/dev/null
clang -c "$tmpdir/program.ll" -o "$tmpdir/program.o"
"$graph_cxx" ${FLOWGRAPH_LINK_FLAGS:-} "$tmpdir/program.o" "-Wl,-rpath,$(dirname "$graph_runtime")" "$graph_runtime" "$tmpdir/provider.so" "-Wl,-rpath,$tmpdir" -o "$tmpdir/program.llvm"
"$prepare" --binding-report "$tmpdir/binding.json" --target-policy "$root/Flowlower/target-policies/tinyvm-portable.json" "$tmpdir/optimization.json" > "$tmpdir/tiny.backend.json"
"$tiny_lower" "$tmpdir/tiny.backend.json" "$tmpdir/program.tvm" > "$tmpdir/tiny.report.json"
"$tiny_validate" "$tmpdir/program.tvm" | grep -q '"status":"valid"'

"$tmpdir/program.llvm" > "$tmpdir/llvm.stdout" 2>/dev/null
"$tiny_run" --policy "$tmpdir/policy" "$tmpdir/program.tvm" > "$tmpdir/tiny.stdout"
sed '$d' "$tmpdir/tiny.stdout" > "$tmpdir/tiny.program.stdout"
cmp "$tmpdir/llvm.stdout" "$tmpdir/tiny.program.stdout"
"$tiny_run" --engine computed --policy "$tmpdir/policy" "$tmpdir/program.tvm" > "$tmpdir/tiny.computed.stdout"
sed '$d' "$tmpdir/tiny.computed.stdout" > "$tmpdir/tiny.computed.program.stdout"
cmp "$tmpdir/tiny.program.stdout" "$tmpdir/tiny.computed.program.stdout"
test "$(wc -l < "$tmpdir/llvm.stdout")" -eq 6
test "$(tail -n 1 "$tmpdir/tiny.stdout" | jq -r .result)" -eq 0
jq -e '.graph_schedule.version == 2 and
    .graph_schedule.stream_contract == "finite_scalar_stream_v1" and
    (.graph_schedule.steps | length) == 3 and
    all(.graph_schedule.steps[1:][]; .kind == "stream_receiver" and .input_activation_id == 0 and .input_signal_id == 1 and .stream_index == "$index") and
    (.graph_schedule.streams[0].deliveries | length) == 2' "$tmpdir/execution.json" >/dev/null
jq -e '.status == "emitted" and .backend == "tinyvm"' "$tmpdir/tiny.report.json" >/dev/null
"$tiny_lower" "$tmpdir/tiny.backend.json" "$tmpdir/program-again.tvm" >/dev/null
cmp "$tmpdir/program.tvm" "$tmpdir/program-again.tvm"

cp "$tmpdir/program.flow" "$tmpdir/fanout.flow"
cat > "$tmpdir/pipeline.flow" <<FLOW
import "$tmpdir/provider.flow" as stream
import "$root/Lyraform/compiler/std/abi/libc.flow"
program tinyvm_stream_pipeline
producer source : injected.stream
node increment : fn increment
node announce : fn announce
wire source.out => increment.in
wire increment.out => announce.in
fn increment(value : c_int): c_int {
    one : c_int(1)
    return value + one
}
fn announce(value : c_int): c_int {
    result : c_int(0)
    if value == 11 {
        libc.puts("tinyvm pipeline eleven") -> result
    } else {
        libc.puts("tinyvm pipeline other") -> result
    }
    return value
}
main { return 0 }
FLOW
cp "$tmpdir/pipeline.flow" "$tmpdir/program.flow"
"$flowmini" --dump-frontend-bundle "$tmpdir/program.flow" > "$tmpdir/pipeline.frontend.json"
"$analyst" --lowering-plan-version 2 --graph-plan-version 2 --graph-providers "$tmpdir/providers.json" < "$tmpdir/pipeline.frontend.json" > "$tmpdir/pipeline.semantic.json"
"$bind" --policy "$tmpdir/policy" < "$tmpdir/pipeline.semantic.json" > "$tmpdir/pipeline.binding.json"
"$parallel" < "$tmpdir/pipeline.semantic.json" > "$tmpdir/pipeline.execution.json"
"$optimize" < "$tmpdir/pipeline.execution.json" > "$tmpdir/pipeline.optimization.json"
"$prepare" --binding-report "$tmpdir/pipeline.binding.json" --target-policy "$root/Flowlower/target-policies/llvm-host.json" "$tmpdir/pipeline.optimization.json" > "$tmpdir/pipeline.llvm.backend.json"
"$llvm_lower" --emit-llvm "$tmpdir/pipeline.ll" "$tmpdir/pipeline.llvm.backend.json" >/dev/null
clang -c "$tmpdir/pipeline.ll" -o "$tmpdir/pipeline.o"
"$graph_cxx" ${FLOWGRAPH_LINK_FLAGS:-} "$tmpdir/pipeline.o" "-Wl,-rpath,$(dirname "$graph_runtime")" "$graph_runtime" "$tmpdir/provider.so" "-Wl,-rpath,$tmpdir" -o "$tmpdir/pipeline.llvm"
"$prepare" --binding-report "$tmpdir/pipeline.binding.json" --target-policy "$root/Flowlower/target-policies/tinyvm-portable.json" "$tmpdir/pipeline.optimization.json" > "$tmpdir/pipeline.tiny.backend.json"
"$tiny_lower" "$tmpdir/pipeline.tiny.backend.json" "$tmpdir/pipeline.tvm" > "$tmpdir/pipeline.tiny.report.json"
"$tiny_validate" "$tmpdir/pipeline.tvm" | grep -q '"status":"valid"'
"$tmpdir/pipeline.llvm" > "$tmpdir/pipeline.llvm.stdout" 2>/dev/null
"$tiny_run" --policy "$tmpdir/policy" "$tmpdir/pipeline.tvm" > "$tmpdir/pipeline.tiny.stdout"
sed '$d' "$tmpdir/pipeline.tiny.stdout" > "$tmpdir/pipeline.tiny.program.stdout"
cmp "$tmpdir/pipeline.llvm.stdout" "$tmpdir/pipeline.tiny.program.stdout"
"$tiny_run" --engine computed --policy "$tmpdir/policy" "$tmpdir/pipeline.tvm" > "$tmpdir/pipeline.tiny.computed.stdout"
sed '$d' "$tmpdir/pipeline.tiny.computed.stdout" > "$tmpdir/pipeline.tiny.computed.program.stdout"
cmp "$tmpdir/pipeline.tiny.program.stdout" "$tmpdir/pipeline.tiny.computed.program.stdout"
printf 'tinyvm pipeline eleven\ntinyvm pipeline other\ntinyvm pipeline other\n' > "$tmpdir/pipeline.expected"
cmp "$tmpdir/pipeline.llvm.stdout" "$tmpdir/pipeline.expected"
test "$(tail -n 1 "$tmpdir/pipeline.tiny.stdout" | jq -r .result)" -eq 0
jq -e '.graph_schedule.version == 5 and
    .graph_schedule.stream_contract == "finite_scalar_stream_pipeline_v1" and
    (.graph_schedule.steps | length) == 3 and
    .graph_schedule.steps[1].input_activation_id == 0 and
    .graph_schedule.steps[2].input_activation_id == 1 and
    .graph_schedule.steps[1].input_signal_id == 1 and
    .graph_schedule.steps[2].input_signal_id == 2 and
    (.graph_schedule.streams[0].deliveries | length) == 2' "$tmpdir/pipeline.execution.json" >/dev/null
jq -e '.status == "emitted" and .backend == "tinyvm"' "$tmpdir/pipeline.tiny.report.json" >/dev/null
"$tiny_lower" "$tmpdir/pipeline.tiny.backend.json" "$tmpdir/pipeline-again.tvm" >/dev/null
cmp "$tmpdir/pipeline.tvm" "$tmpdir/pipeline-again.tvm"

cat > "$tmpdir/branch.flow" <<FLOW
import "$tmpdir/provider.flow" as stream
import "$root/Lyraform/compiler/std/abi/libc.flow"
program tinyvm_stream_pipeline_branch
producer source : injected.stream
node increment : fn increment
node announce : fn announce
node right : fn announce_right
wire source.out => increment.in
wire increment.out => announce.in
wire increment.out => right.in
fn increment(value : c_int): c_int {
    one : c_int(1)
    return value + one
}
fn announce(value : c_int): c_int {
    result : c_int(0)
    libc.puts("tinyvm pipeline branch") -> result
    return value
}
fn announce_right(value : c_int): c_int {
    result : c_int(0)
    libc.puts("tinyvm pipeline branch right") -> result
    return value
}
main { return 0 }
FLOW
"$flowmini" --dump-frontend-bundle "$tmpdir/branch.flow" > "$tmpdir/branch.frontend.json"
"$analyst" --lowering-plan-version 2 --graph-plan-version 2 --graph-providers "$tmpdir/providers.json" < "$tmpdir/branch.frontend.json" > "$tmpdir/branch.semantic.json"
set +e
"$parallel" < "$tmpdir/branch.semantic.json" > "$tmpdir/branch.execution.json" 2> "$tmpdir/branch.error"
branch_status=$?
set -e
test "$branch_status" -ne 0
test ! -s "$tmpdir/branch.execution.json"
grep -Fq 'cannot fan out or merge' "$tmpdir/branch.error"

mv "$tmpdir/fanout.flow" "$tmpdir/program.flow"

jq '.providers[0].max_items = 2' "$tmpdir/providers.json" > "$tmpdir/bounded.providers.json"
"$analyst" --lowering-plan-version 2 --graph-plan-version 2 --graph-providers "$tmpdir/bounded.providers.json" < "$tmpdir/frontend.json" > "$tmpdir/bounded.semantic.json"
"$bind" --policy "$tmpdir/policy" < "$tmpdir/bounded.semantic.json" > "$tmpdir/bounded.binding.json"
"$parallel" < "$tmpdir/bounded.semantic.json" > "$tmpdir/bounded.execution.json"
"$optimize" < "$tmpdir/bounded.execution.json" > "$tmpdir/bounded.optimization.json"
"$prepare" --binding-report "$tmpdir/bounded.binding.json" --target-policy "$root/Flowlower/target-policies/tinyvm-portable.json" "$tmpdir/bounded.optimization.json" > "$tmpdir/bounded.backend.json"
"$tiny_lower" "$tmpdir/bounded.backend.json" "$tmpdir/bounded.tvm" >/dev/null
set +e
"$tiny_run" --policy "$tmpdir/policy" "$tmpdir/bounded.tvm" > "$tmpdir/bounded.stdout" 2> "$tmpdir/bounded.stderr"
status=$?
set -e
test "$status" -ne 0
test "$(tail -n 1 "$tmpdir/bounded.stdout" | jq -r .status)" = faulted

echo 'TinyVM finite stream parity: PASS'
