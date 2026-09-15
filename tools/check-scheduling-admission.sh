#!/usr/bin/env bash
set -euo pipefail

root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$root"

source_roots=(
    Flowanalyst/src
    Flowbind/src
    Flowcontracts/include
    Flowcontracts/src
    Flowkernel/src
    Flowlower/src
    Flowoptimize/src
    Flowparallel/src
    Frankencore/Core
    Lyraform/compiler/src
    subprojects/TinyVM/src
    subprojects/TinyVM/tools
)

mapfile -t control_authority < <(
    rg -l '"(cancellation|async|backpressure|reentrancy|nested|distributed|retry|irreversible)"' \
        "${source_roots[@]}" --glob '*.{c,cc,cpp,cxx,h,hpp}' --glob '!**/tests/**' | sort
)
expected_control_authority=(Flowcontracts/include/flowcontracts/scheduling.hpp)
if [[ "${control_authority[*]}" != "${expected_control_authority[*]}" ]]; then
    echo "scheduling-control authority inventory changed" >&2
    printf '  expected: %s\n' "${expected_control_authority[@]}" >&2
    printf '  observed: %s\n' "${control_authority[@]}" >&2
    exit 1
fi

mapfile -t policy_sites < <(
    rg -l '"schedule_policy"' "${source_roots[@]}" \
        --glob '*.{c,cc,cpp,cxx,h,hpp}' --glob '!**/tests/**' | sort
)
expected_policy_sites=(
    Flowanalyst/src/main.cpp
    Flowcontracts/include/flowcontracts/graph_execution.hpp
    Flowcontracts/include/flowcontracts/graph_provider_map.hpp
    Flowcontracts/include/flowcontracts/scheduling.hpp
    Flowcontracts/include/flowcontracts/source_graph.hpp
)
if [[ "${policy_sites[*]}" != "${expected_policy_sites[*]}" ]]; then
    echo "schedule-policy site inventory changed" >&2
    printf '  expected: %s\n' "${expected_policy_sites[@]}" >&2
    printf '  observed: %s\n' "${policy_sites[@]}" >&2
    exit 1
fi

rg -q 'validate_scheduling_request\(root, false\)' Flowcontracts/include/flowcontracts/artifacts.hpp
rg -q 'validate_scheduling_request\(root, true\)' Flowcontracts/include/flowcontracts/artifacts.hpp
rg -q 'validate_scheduling_request\(root, true, path\)' Flowcontracts/include/flowcontracts/source_graph.hpp
rg -q 'validate_scheduling_request\(item, true, path\)' Flowcontracts/include/flowcontracts/graph_provider_map.hpp
rg -q 'graph_provider_map\(provider_map\)' Flowanalyst/src/main.cpp
rg -q 'source_graph\(graph_value\)' Flowcontracts/include/flowcontracts/graph_execution.hpp
rg -q 'execution_plan\(root\)' Flowoptimize/src/main.cpp

for source in \
    Flowparallel/src/main.cpp \
    Flowparallel/src/cpu_provider.cpp \
    Flowparallel/src/cuda_provider.cpp \
    Flowparallel/src/runtime_planner.cpp; do
    rg -q 'scheduling_refusal\(' "$source"
done

echo "Scheduling admission inventory: PASS (1 control authority, 5 policy sites)"
