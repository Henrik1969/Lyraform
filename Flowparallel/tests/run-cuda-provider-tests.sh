#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cuda=${FLOWPARALLEL_CUDA_BIN:?FLOWPARALLEL_CUDA_BIN is required}
planner=${FLOWPARALLEL_BIN:?FLOWPARALLEL_BIN is required}
flowmini=${FLOWMINI_BIN:-$root/Lyraform/compiler/cmake-build-debug/flowmini}
analyst=${FLOWANALYST_BIN:-$root/Flowanalyst/build/flowanalyst}
fixture="$root/Lyraform/compiler/examples/ast/parallel_independence_probe.flow"
test -x "$cuda"
test -x "$planner"
test -x "$flowmini"
test -x "$analyst"

unsupported_plan=$(mktemp)
trap 'rm -f "$unsupported_plan"' EXIT

plan=$("$flowmini" --dump-frontend-bundle "$fixture" | "$analyst" | "$planner")
report=$(printf '%s\n' "$plan" | "$cuda" --matrix-size 64)
printf '%s\n' "$report" | jq -e '
  .format == "flowparallel.cuda_selection" and
  .status == "ready" and
  .provider == "cuda" and
  .workload.operation == "matrix_multiply" and
  .transfer_cost.calibration == "runtime" and
  .execution == "not-performed" and
  .fallback.required == true
' >/dev/null
printf '%s\n' "$plan" | jq '.cancellation = "requested"' >"$unsupported_plan"
if unsupported=$("$cuda" --plan "$unsupported_plan" 2>/dev/null); then
  echo 'CUDA provider accepted an unsupported cancellation request' >&2
  exit 1
fi
printf '%s\n' "$unsupported" | jq -e '
  .status == "unsupported" and
  .request == "cancellation" and
  .fallback.emitted == false
' >/dev/null
if printf '%s' '{"format":"flowparallel.execution_plan","format":"flowparallel.execution_plan","version":1,"status":"ready"}' | "$cuda" >/dev/null 2>&1; then
  echo 'CUDA provider accepted duplicate execution-plan authority' >&2
  exit 1
fi
echo 'Flowparallel CUDA provider boundary: PASS'
