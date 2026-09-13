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
cat > "$tmpdir/policy" <<'POLICY'
allow libc.so.6 getpid c readonly
allow libc.so.6 puts c io c_string c_int
POLICY
cat > "$tmpdir/providers.json" <<'JSON'
{"format":"flowcore.graph_provider_map","version":1,"providers":[{"implementation":"injected.pid","source_callable":"kernel.getpid","activation":"startup_once","output_port":"out"}]}
JSON
cat > "$tmpdir/program.flow" <<FLOW
import "$root/Lyraform/compiler/std/abi/kernel.flow"
import "$root/Lyraform/compiler/std/abi/libc.flow"
program tinyvm_graph
producer source : injected.pid
node transform : fn transform
node left : fn announce_left
node right : fn announce_right
wire source.out => transform.in
wire transform.out => left.in
wire transform.out => right.in
fn transform(value : c_int): c_int {
    return value
}
fn announce_left(value : c_int): c_int {
    result : c_int(0)
    libc.puts("tinyvm graph left") -> result
    return value
}
fn announce_right(value : c_int): c_int {
    result : c_int(0)
    libc.puts("tinyvm graph right") -> result
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
"$graph_cxx" ${FLOWGRAPH_LINK_FLAGS:-} "$tmpdir/program.o" "-Wl,-rpath,$(dirname "$graph_runtime")" "$graph_runtime" -o "$tmpdir/program.llvm"
"$prepare" --binding-report "$tmpdir/binding.json" --target-policy "$root/Flowlower/target-policies/tinyvm-portable.json" "$tmpdir/optimization.json" > "$tmpdir/tiny.backend.json"
"$tiny_lower" "$tmpdir/tiny.backend.json" "$tmpdir/program.tvm" > "$tmpdir/tiny.report.json"
"$tiny_validate" "$tmpdir/program.tvm" | grep -q '"status":"valid"'

"$tmpdir/program.llvm" > "$tmpdir/llvm.stdout"
"$tiny_run" --policy "$tmpdir/policy" "$tmpdir/program.tvm" > "$tmpdir/tiny.stdout"
sed '$d' "$tmpdir/tiny.stdout" > "$tmpdir/tiny.program.stdout"
cmp "$tmpdir/llvm.stdout" "$tmpdir/tiny.program.stdout"
"$tiny_run" --engine computed --policy "$tmpdir/policy" "$tmpdir/program.tvm" > "$tmpdir/tiny.computed.stdout"
sed '$d' "$tmpdir/tiny.computed.stdout" > "$tmpdir/tiny.computed.program.stdout"
cmp "$tmpdir/llvm.stdout" "$tmpdir/tiny.computed.program.stdout"
test "$(tail -n 1 "$tmpdir/tiny.stdout" | jq -r .result)" -eq 0
test "$(tail -n 1 "$tmpdir/tiny.computed.stdout" | jq -r .result)" -eq 0
jq -e '.graph_schedule.version == 1 and (.graph_schedule.steps | length) == 4 and .graph_schedule.steps[0].kind == "startup" and all(.graph_schedule.steps[1:][]; .kind == "receiver") and .graph_schedule.steps[2].input_activation_id == .graph_schedule.steps[3].input_activation_id' "$tmpdir/execution.json" >/dev/null
jq -e '.status == "emitted" and .backend == "tinyvm"' "$tmpdir/tiny.report.json" >/dev/null
"$tiny_lower" "$tmpdir/tiny.backend.json" "$tmpdir/program-again.tvm" >/dev/null
cmp "$tmpdir/program.tvm" "$tmpdir/program-again.tvm"

jq '{format:"flowcore.graph_provider_map",version:3,providers:[.providers[0] + {schedule_policy:"parallel_independent_v1"}]}' "$tmpdir/providers.json" > "$tmpdir/parallel.providers.json"
"$analyst" --lowering-plan-version 2 --graph-plan-version 2 --graph-providers "$tmpdir/parallel.providers.json" < "$tmpdir/frontend.json" > "$tmpdir/parallel.semantic.json"
"$bind" --policy "$tmpdir/policy" < "$tmpdir/parallel.semantic.json" > "$tmpdir/parallel.binding.json"
"$parallel" < "$tmpdir/parallel.semantic.json" > "$tmpdir/parallel.execution.json"
"$optimize" < "$tmpdir/parallel.execution.json" > "$tmpdir/parallel.optimization.json"
"$prepare" --binding-report "$tmpdir/parallel.binding.json" --target-policy "$root/Flowlower/target-policies/tinyvm-portable.json" "$tmpdir/parallel.optimization.json" > "$tmpdir/parallel.backend.json"
if "$tiny_lower" "$tmpdir/parallel.backend.json" "$tmpdir/parallel.tvm" > "$tmpdir/parallel.report.json" 2>/dev/null; then
    echo 'TinyVM unexpectedly admitted a parallel graph schedule' >&2
    exit 1
fi
test ! -e "$tmpdir/parallel.tvm"
jq -e '.graph_schedule.version == 4 and .graph_schedule.policy == "parallel_independent_v1"' "$tmpdir/parallel.backend.json" >/dev/null
jq -e '.status == "unsupported" and (.reason | contains("pure receiver activations"))' "$tmpdir/parallel.report.json" >/dev/null

echo 'TinyVM serial graph parity: PASS'
