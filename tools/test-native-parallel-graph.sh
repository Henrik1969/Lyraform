#!/bin/sh
set -eu

root=${FLOWCORE_ROOT:?FLOWCORE_ROOT is required}
flowmini=${FLOWMINI_BIN:?FLOWMINI_BIN is required}
analyst=${FLOWANALYST_BIN:?FLOWANALYST_BIN is required}
bind=${FLOWBIND_BIN:?FLOWBIND_BIN is required}
parallel=${FLOWPARALLEL_BIN:?FLOWPARALLEL_BIN is required}
parallel_cpu=${FLOWPARALLEL_CPU_BIN:?FLOWPARALLEL_CPU_BIN is required}
optimizer=${FLOWOPTIMIZE_BIN:?FLOWOPTIMIZE_BIN is required}
prepare=${FLOWPREPARE_BIN:?FLOWPREPARE_BIN is required}
lower=${FLOWLOWER_BIN:?FLOWLOWER_BIN is required}
tiny_lower=${FLOWTINYLOWER_BIN:?FLOWTINYLOWER_BIN is required}
tiny_validate=${FLOWTINYVALIDATE_BIN:?FLOWTINYVALIDATE_BIN is required}
tiny_run=${FLOWTINYRUN_BIN:?FLOWTINYRUN_BIN is required}
runtime=${FLOWGRAPH_RUNTIME:?FLOWGRAPH_RUNTIME is required}
cxx=${FLOWGRAPH_CXX:?FLOWGRAPH_CXX is required}

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

cat > "$tmpdir/provider.c" <<'C'
#include <limits.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <time.h>

static _Atomic int observation_arrivals = 0;

static int overlap_observed(void) {
    if (!getenv("LYRAFORM_OBSERVATION_PROBE")) return 1;
    atomic_fetch_add_explicit(&observation_arrivals, 1, memory_order_acq_rel);
    struct timespec started;
    clock_gettime(CLOCK_MONOTONIC, &started);
    while (atomic_load_explicit(&observation_arrivals, memory_order_acquire) < 2) {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        if (now.tv_sec - started.tv_sec >= 2) return 0;
        sched_yield();
    }
    return 1;
}

int observe_left(int value) {
    return overlap_observed() ? value + 10 : INT_MIN;
}

int observe_right(int value) {
    return overlap_observed() ? value + 20 : INT_MIN;
}
C
clang -shared -fPIC "$tmpdir/provider.c" -o "$tmpdir/provider.so"
jq -n --arg path "$tmpdir/provider.so" '{format:"flowcore.native_binding_spec",version:1,unit:"parallel_graph_provider",namespace:"host",provider:{soname:$path,path:$path,convention:"c"},functions:[{name:"read_left",symbol:"observe_left",effect:"readonly",parameters:[{name:"value",type:"c_int"}],return_type:"c_int"},{name:"read_right",symbol:"observe_right",effect:"readonly",parameters:[{name:"value",type:"c_int"}],return_type:"c_int"}]}' > "$tmpdir/spec.json"
"$root/tools/generate-flow-bindings.sh" --spec "$tmpdir/spec.json" --flow-output "$tmpdir/provider.flow" --policy-output "$tmpdir/policy" --manifest-output "$tmpdir/manifest.json" >/dev/null
cat > "$tmpdir/selection.json" <<'JSON'
{"format":"flowcore.graph_provider_map","version":3,"providers":[{"implementation":"injected.pid","source_callable":"kernel.getpid","activation":"startup_once","output_port":"out","schedule_policy":"parallel_independent_v1"}]}
JSON
printf '%s\n' 'allow libc.so.6 getpid c readonly' >> "$tmpdir/policy"
cat > "$tmpdir/program.flow" <<FLOW
import "provider.flow" as host
import "$root/Lyraform/compiler/std/abi/kernel.flow"
program parallel_native_graph
producer source : injected.pid
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
if ! "$analyst" --lowering-plan-version 2 --graph-plan-version 2 --graph-providers "$tmpdir/selection.json" < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"; then
    cat "$tmpdir/semantic.json" >&2
    exit 1
fi
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
assert [r['activation_id'] for r in results] == [1, 2, 3]
assert results[1]['value'] == results[0]['value']
assert results[2]['value'] == results[0]['value'] + 1
PY

# Bounded effectful parallelism is admitted only after exact provider profiles
# bind each readonly scalar call to concurrent-observation authority.
cat > "$tmpdir/effectful.flow" <<FLOW
import "provider.flow" as host
import "$root/Lyraform/compiler/std/abi/kernel.flow"
program effectful_parallel_graph
producer source : injected.pid
node left : fn observe_left_node
node right : fn observe_right_node
wire source.out => left.in
wire source.out => right.in
fn observe_left_node(value : c_int): c_int {
    return host.read_left(value)
}
fn observe_right_node(value : c_int): c_int {
    return host.read_right(value)
}
main { return 0 }
FLOW
"$flowmini" --dump-frontend-bundle "$tmpdir/effectful.flow" > "$tmpdir/effectful.frontend.json"
"$analyst" --lowering-plan-version 2 --graph-plan-version 2 --graph-providers "$tmpdir/selection.json" < "$tmpdir/effectful.frontend.json" > "$tmpdir/effectful.unprofiled.json"
jq '{format:"flowcore.provider_effect_profiles",version:1,status:"declarative",profiles:([.lowering_plan.operations[] | select(.kind == "external_call" and (.provider.symbol == "observe_left" or .provider.symbol == "observe_right")) | .provider] | to_entries | map({format:"flowcore.provider_effect_profile",version:1,status:"declarative",profile_id:(.key + 1),capability:.value,resource_domain:("fixture:" + .value.symbol),access:"observe",concurrency:"concurrent_observation_v1",failure:"infallible_scalar_v1"}))}' "$tmpdir/effectful.unprofiled.json" > "$tmpdir/effect-profiles.json"
"$analyst" --lowering-plan-version 2 --graph-plan-version 2 --graph-providers "$tmpdir/selection.json" --effect-profiles "$tmpdir/effect-profiles.json" < "$tmpdir/effectful.frontend.json" > "$tmpdir/effectful.semantic.json"
jq -e '(.provider_effect_profiles | length) == 2 and (.effect_access_facts | length) == 2 and ([.lowering_plan.source_graph.receivers[].execution_effect.class] | sort) == ["bounded_concurrent_observation_v1","bounded_concurrent_observation_v1"]' "$tmpdir/effectful.semantic.json" >/dev/null
"$bind" --policy "$tmpdir/policy" < "$tmpdir/effectful.semantic.json" > "$tmpdir/effectful.binding.json"
"$parallel" < "$tmpdir/effectful.semantic.json" > "$tmpdir/effectful.execution.json"
jq -e '.graph_schedule.version == 6 and .graph_schedule.parallel_contract == "effect_conflict_waves_v1" and (.graph_schedule.effect_access_facts | length) == 2 and (.graph_schedule.effect_conflict_facts | length) == 1 and .graph_schedule.effect_schedule.publication == "publish_after_wave_join_v1"' "$tmpdir/effectful.execution.json" >/dev/null
"$parallel_cpu" --plan "$tmpdir/effectful.execution.json" --workers 2 --observed-speedup 2 --minimum-speedup 1.25 > "$tmpdir/effectful.parallel-selection.json"
"$parallel_cpu" --plan "$tmpdir/effectful.execution.json" --workers 2 --observed-speedup 1 --minimum-speedup 1.25 > "$tmpdir/effectful.serial-selection.json"
jq -e '.status == "ready" and .decision == "parallel" and .provider == "cpu.threadpool" and .parallel_candidates == 2' "$tmpdir/effectful.parallel-selection.json" >/dev/null
jq -e '.status == "ready" and .decision == "serial" and .provider == "cpu.serial" and .parallel_candidates == 2' "$tmpdir/effectful.serial-selection.json" >/dev/null
"$optimizer" < "$tmpdir/effectful.execution.json" > "$tmpdir/effectful.optimization.json"
for mutation in \
    '.graph_schedule.effect_conflict_facts[0].relation = "conflict"' \
    '.graph_schedule.parallel_waves[1].activation_ids = [1]'
do
    jq "$mutation" "$tmpdir/effectful.execution.json" > "$tmpdir/effectful.hostile-schedule.json"
    if "$optimizer" < "$tmpdir/effectful.hostile-schedule.json" >/dev/null 2>&1; then
        echo 'hostile effect schedule unexpectedly crossed optimization' >&2
        exit 1
    fi
done
"$prepare" --binding-report "$tmpdir/effectful.binding.json" < "$tmpdir/effectful.optimization.json" > "$tmpdir/effectful.backend.json"
jq -e '(.provider_effect_profiles | length) == 2 and (.effect_access_facts | length) == 2 and .graph_schedule.version == 6' "$tmpdir/effectful.backend.json" >/dev/null
"$lower" --emit-llvm "$tmpdir/effectful.ll" < "$tmpdir/effectful.backend.json" > "$tmpdir/effectful.lowering.json"
clang -c "$tmpdir/effectful.ll" -o "$tmpdir/effectful.o"
"$cxx" ${FLOWGRAPH_LINK_FLAGS:-} "$tmpdir/effectful.o" "$runtime" "$tmpdir/provider.so" "-Wl,-rpath,$(dirname "$runtime")" "-Wl,-rpath,$(dirname "$tmpdir/provider.so")" -o "$tmpdir/effectful"
run=1
while [ "$run" -le 8 ]; do
    LYRAFORM_OBSERVATION_PROBE=1 FLOWCORE_GRAPH_TRACE=1 "$tmpdir/effectful" \
        >/dev/null 2> "$tmpdir/effectful.$run.trace"
    run=$((run + 1))
done
python3 - "$tmpdir" <<'PY'
import glob, json, os, sys
paths = sorted(glob.glob(os.path.join(sys.argv[1], 'effectful.*.trace')))
assert len(paths) == 8
for path in paths:
    records = [json.loads(line) for line in open(path)]
    results = [r for r in records if r.get('event') == 'result']
    executions = [r for r in records if r.get('format') == 'flowcore.graph_effect' and r.get('event') == 'execute']
    publications = [r for r in records if r.get('format') == 'flowcore.graph_effect' and r.get('event') == 'publish']
    assert [r['activation_id'] for r in results] == [1, 2]
    assert results[0]['value'] != -(2 ** 31)
    assert results[1]['value'] == results[0]['value'] + 10
    assert {r['activation_id'] for r in executions} == {1, 2}
    assert {r['worker_index'] for r in executions} == {0, 1}
    assert all(r['operation_id'] >= 0 and r['capability_identity'] and
               r['resource_domain'].startswith('fixture:observe_') and
               r['policy'] == 'parallel_independent_v1' for r in executions)
    assert [r['activation_id'] for r in publications] == [1, 2]
    assert all(r['effect_context']['activation_id'] == r['activation_id'] and
               r['effect_context']['operation_id'] >= 0 for r in publications)
PY
"$prepare" --binding-report "$tmpdir/effectful.binding.json" --target-policy "$root/Flowlower/target-policies/tinyvm-portable.json" "$tmpdir/effectful.optimization.json" > "$tmpdir/effectful.tiny.backend.json"
if ! "$tiny_lower" "$tmpdir/effectful.tiny.backend.json" "$tmpdir/effectful.tvm" > "$tmpdir/effectful.tiny.report.json"; then
    cat "$tmpdir/effectful.tiny.report.json" >&2
    exit 1
fi
"$tiny_validate" "$tmpdir/effectful.tvm" | grep -q '"status":"valid"'
awk 'NF == 8 {print $1, $2, $3, $4, $5, $6, $7; next} {print}' "$tmpdir/policy" > "$tmpdir/tiny.policy"
if ! "$tiny_run" --trace-graph --policy "$tmpdir/tiny.policy" "$tmpdir/effectful.tvm" > "$tmpdir/effectful.tiny.stdout" 2> "$tmpdir/effectful.tiny.trace"; then
    cat "$tmpdir/effectful.tiny.stdout" >&2
    cat "$tmpdir/effectful.tiny.trace" >&2
    exit 1
fi
test "$(tail -n 1 "$tmpdir/effectful.tiny.stdout" | jq -r .result)" -eq 0
jq -s -e 'length == 3 and ([.[].sequence]) == [0,1,2] and ([.[].activation_id]) == [0,1,2] and .[0].kind == "startup" and all(.[1:][]; .kind == "receiver")' "$tmpdir/effectful.tiny.trace" >/dev/null
jq -e '.status == "emitted" and .backend == "tinyvm"' "$tmpdir/effectful.tiny.report.json" >/dev/null

# The same source without profiles must fail closed before executable planning.
if "$parallel" < "$tmpdir/effectful.unprofiled.json" > "$tmpdir/effectful.unprofiled.execution.json" 2>/dev/null; then
    echo 'unprofiled effectful parallel graph unexpectedly admitted' >&2
    exit 1
fi
for mutation in \
    '(.lowering_plan.operations[] | select(.kind == "external_call") | .provider.symbol) = "drifted_symbol"' \
    '.provider_effect_profiles[0].resource_domain = "fixture:drift"'
do
    jq "$mutation" "$tmpdir/effectful.semantic.json" > "$tmpdir/effectful.hostile.json"
    if "$parallel" < "$tmpdir/effectful.hostile.json" >/dev/null 2>&1; then
        echo 'hostile effect-authority drift unexpectedly admitted' >&2
        exit 1
    fi
done

# A worker body with mutable local state is not silently treated as pure. The
# graph remains analyzable, but the native parallel boundary must refuse it.
cat > "$tmpdir/impure.flow" <<FLOW
import "provider.flow" as host
import "$root/Lyraform/compiler/std/abi/kernel.flow"
program impure_parallel_graph
producer source : injected.pid
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
