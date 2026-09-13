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
POLICY
cat > "$tmpdir/providers.json" <<'JSON'
{"format":"flowcore.graph_provider_map","version":3,"providers":[{"implementation":"injected.pid","source_callable":"kernel.getpid","activation":"startup_once","output_port":"out","schedule_policy":"parallel_independent_v1"}]}
JSON
cat > "$tmpdir/program.flow" <<FLOW
import "$root/Lyraform/compiler/std/abi/kernel.flow"
program tinyvm_parallel
producer source : injected.pid
node transform : fn transform
node left : fn left
node right : fn right
wire source.out => transform.in
wire transform.out => left.in
wire transform.out => right.in
fn transform(value : c_int): c_int {
    return value + 1
}
fn left(value : c_int): c_int {
    return value
}
fn right(value : c_int): c_int {
    return value + 1
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
if ! "$tiny_lower" "$tmpdir/tiny.backend.json" "$tmpdir/program.tvm" > "$tmpdir/tiny.report.json"; then
    cat "$tmpdir/tiny.report.json" >&2
    exit 1
fi
"$tiny_validate" "$tmpdir/program.tvm" | grep -q '"status":"valid"'

"$tmpdir/program.llvm" > "$tmpdir/llvm.stdout" 2>/dev/null
"$tiny_run" --policy "$tmpdir/policy" "$tmpdir/program.tvm" > "$tmpdir/tiny.stdout"
test ! -s "$tmpdir/llvm.stdout"
test "$(tail -n 1 "$tmpdir/tiny.stdout" | jq -r .result)" -eq 0
"$tiny_run" --engine computed --policy "$tmpdir/policy" "$tmpdir/program.tvm" > "$tmpdir/tiny.computed.stdout"
test "$(tail -n 1 "$tmpdir/tiny.computed.stdout" | jq -r .result)" -eq 0
jq -e '.graph_schedule.version == 4 and .graph_schedule.parallel_contract == "dependency_waves_v1" and (.graph_schedule.parallel_waves | length) == 3' "$tmpdir/execution.json" >/dev/null
jq -e '.status == "emitted" and .backend == "tinyvm"' "$tmpdir/tiny.report.json" >/dev/null
"$tiny_lower" "$tmpdir/tiny.backend.json" "$tmpdir/program-again.tvm" >/dev/null
cmp "$tmpdir/program.tvm" "$tmpdir/program-again.tvm"

echo 'TinyVM pure parallel graph parity: PASS'
