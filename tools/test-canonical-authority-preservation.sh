#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
fixture="$root/Lyraform/compiler/examples/ast/parallel_independence_probe.flow"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

"$FLOWMINI_BIN" --dump-frontend-bundle "$fixture" > "$tmpdir/frontend.json"
"$FLOWANALYST_BIN" --lowering-plan-version 2 < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
"$FLOWPARALLEL_BIN" < "$tmpdir/semantic.json" > "$tmpdir/execution.json"
"$FLOWOPTIMIZE_BIN" < "$tmpdir/execution.json" > "$tmpdir/optimization.json"
"$FLOWOPTIMIZE_BIN" < "$tmpdir/semantic.json" > "$tmpdir/direct-optimization.json"
"$FLOWPREPARE_BIN" < "$tmpdir/optimization.json" > "$tmpdir/backend.json"
"$FLOWVALIDATE_BIN" < "$tmpdir/execution.json" >/dev/null
"$FLOWVALIDATE_BIN" < "$tmpdir/optimization.json" >/dev/null
"$FLOWVALIDATE_BIN" < "$tmpdir/direct-optimization.json" >/dev/null
"$FLOWVALIDATE_BIN" < "$tmpdir/backend.json" >/dev/null
"$FLOWTINYLOWER_BIN" "$tmpdir/backend.json" "$tmpdir/program.tvm" > "$tmpdir/tiny-lowering.json"
jq -e '.status == "emitted" and .backend == "tinyvm"' "$tmpdir/tiny-lowering.json" >/dev/null
test -s "$tmpdir/program.tvm"

for stage in execution optimization direct-optimization backend; do
    jq -S '{source,targets,abi_type_contracts,aggregate_abi_layouts,external_operations,effect_facts,parallel_candidates,lowering_plan}' "$tmpdir/semantic.json" > "$tmpdir/semantic.authority.json"
    jq -S '{source,targets,abi_type_contracts,aggregate_abi_layouts,external_operations,effect_facts,parallel_candidates,lowering_plan}' "$tmpdir/$stage.json" > "$tmpdir/$stage.authority.json"
    cmp "$tmpdir/semantic.authority.json" "$tmpdir/$stage.authority.json"
done

reject_execution() {
    mutation=$1
    jq "$mutation" "$tmpdir/execution.json" > "$tmpdir/hostile-execution.json"
    if "$FLOWOPTIMIZE_BIN" < "$tmpdir/hostile-execution.json" >/dev/null 2>&1; then
        echo "Flowoptimize accepted carrying-authority mutation: $mutation" >&2
        exit 1
    fi
}

reject_optimization() {
    mutation=$1
    jq "$mutation" "$tmpdir/optimization.json" > "$tmpdir/hostile-optimization.json"
    if "$FLOWLOWER_BIN" --emit-llvm "$tmpdir/hostile.ll" < "$tmpdir/hostile-optimization.json" >/dev/null 2>&1; then
        echo "Flowlower accepted carrying-authority mutation: $mutation" >&2
        exit 1
    fi
    test ! -e "$tmpdir/hostile.ll"
}

reject_backend() {
    mutation=$1
    jq "$mutation" "$tmpdir/backend.json" > "$tmpdir/hostile-backend.json"
    if "$FLOWLOWER_BIN" --emit-llvm "$tmpdir/hostile-backend.ll" < "$tmpdir/hostile-backend.json" >/dev/null 2>&1; then
        echo "Flowlower accepted backend carrying-authority mutation: $mutation" >&2
        exit 1
    fi
    test ! -e "$tmpdir/hostile-backend.ll"
    if "$FLOWTINYLOWER_BIN" "$tmpdir/hostile-backend.json" "$tmpdir/hostile-backend.tvm" >/dev/null 2>&1; then
        echo "TinyVM accepted backend carrying-authority mutation: $mutation" >&2
        exit 1
    fi
    test ! -e "$tmpdir/hostile-backend.tvm"
}

for mutation in \
    '.external_operations[0].callee = "drifted"' \
    '.parallel_candidates[0].independent_operation_ids[0] = 999' \
    '(.effect_facts[] | select(.effect == "pure")) |= (.effect = "unknown" | .certainty = "unresolved")' \
    '.lowering_plan.operations[1].result_symbol_id = .lowering_plan.operations[0].result_symbol_id'; do
    reject_execution "$mutation"
    reject_optimization "$mutation"
    reject_backend "$mutation"
done

reject_execution '.dependency_analysis.parallel_candidates = 999'
reject_execution '.dependency_analysis.pure_callables = 999'
reject_execution 'del(.effect_facts)'
reject_execution 'del(.parallel_candidates)'
reject_optimization 'del(.effect_facts)'
reject_optimization 'del(.parallel_candidates)'
reject_backend 'del(.effect_facts)'
reject_backend 'del(.parallel_candidates)'
for mutation in \
    '.source.path = 7' \
    '.targets = [{"symbol_id":1,"name":7,"main_count":1}]' \
    '.abi_type_contracts = [{"name":7}]' \
    '.aggregate_abi_layouts = [{"name":7}]' \
    '.lowering_plan.source_operation_coverage.statements[0].operation_ids[0] = 999'; do
    reject_optimization "$mutation"
    reject_backend "$mutation"
done

# Exercise nested scalar and target facts, which are preserved inside the
# canonical lowering plan rather than copied into a second top-level model.
demo="$root/Lyraform/compiler/examples/pass/fn_canonical_demo.flow"
"$FLOWMINI_BIN" --dump-frontend-bundle "$demo" > "$tmpdir/demo.frontend.json"
"$FLOWANALYST_BIN" --lowering-plan-version 2 < "$tmpdir/demo.frontend.json" > "$tmpdir/demo.semantic.json"
"$FLOWPARALLEL_BIN" < "$tmpdir/demo.semantic.json" > "$tmpdir/demo.execution.json"
"$FLOWOPTIMIZE_BIN" < "$tmpdir/demo.execution.json" > "$tmpdir/demo.optimization.json"
"$FLOWPREPARE_BIN" < "$tmpdir/demo.optimization.json" > "$tmpdir/demo.backend.json"
for stage in execution optimization backend; do
    jq -S '{source,targets,abi_type_contracts,aggregate_abi_layouts,external_operations,effect_facts,parallel_candidates,lowering_plan}' "$tmpdir/demo.semantic.json" > "$tmpdir/demo.semantic.authority.json"
    jq -S '{source,targets,abi_type_contracts,aggregate_abi_layouts,external_operations,effect_facts,parallel_candidates,lowering_plan}' "$tmpdir/demo.$stage.json" > "$tmpdir/demo.$stage.authority.json"
    cmp "$tmpdir/demo.semantic.authority.json" "$tmpdir/demo.$stage.authority.json"
done

reject_demo() {
    artifact=$1
    mutation=$2
    jq "$mutation" "$tmpdir/demo.$artifact.json" > "$tmpdir/demo.hostile.json"
    if "$FLOWLOWER_BIN" --emit-llvm "$tmpdir/demo.hostile.ll" < "$tmpdir/demo.hostile.json" >/dev/null 2>&1; then
        echo "Flowlower accepted nested authority mutation: $mutation" >&2
        exit 1
    fi
    test ! -e "$tmpdir/demo.hostile.ll"
}

for artifact in optimization backend; do
    reject_demo "$artifact" '(.lowering_plan.operations[] | select(has("scalar_fact")) | .scalar_fact.destination_symbol_id) = 999'
    reject_demo "$artifact" '(.lowering_plan.operations[] | select(has("target_fact")) | .target_fact.base_symbol_id) = 999'
done

echo 'Canonical carrying-stage authority preservation: PASS'
