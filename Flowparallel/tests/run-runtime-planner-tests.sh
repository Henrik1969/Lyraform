#!/bin/sh
set -eu

planner=${FLOWPARALLEL_RUNTIME_PLANNER_BIN:?FLOWPARALLEL_RUNTIME_PLANNER_BIN is required}
plan=$(mktemp)
capabilities=$(mktemp)
calibration=$(mktemp)
unsupported_plan=$(mktemp)
diagnostic_out=$(mktemp)
diagnostic_err=$(mktemp)
trap 'rm -f "$plan" "$capabilities" "$calibration" "$unsupported_plan" "$diagnostic_out" "$diagnostic_err"' EXIT

printf '%s\n' '{"format":"flowparallel.execution_plan","version":1,"status":"ready","source":{"path":"test.flow"},"targets":[],"external_operations":[],"abi_type_contracts":[],"lowering_plan":{"format":"flowcore.lowering_plan","version":1,"operations":[]},"graph_projection":{"name":"region_dependency","rows":0,"columns":0,"semiring":"boolean","storage":"coo","entries":[]}}' >"$plan"
printf '%s\n' '{"format":"frankencore.runtime_capabilities","version":1,"cpu":{"logical_processors":8},"memory":{"total_bytes":1,"available_bytes":1},"cuda":{"status":"available","driver":"test","device_count":1,"diagnostic":"ok"}}' >"$capabilities"
printf '%s\n' '{"format":"flowparallel.matrix_benchmark","version":1,"status":"verified","end_to_end_speedup":3.0}' >"$calibration"

selected=$("$planner" --plan "$plan" --capabilities "$capabilities" --calibration "$calibration")
printf '%s\n' "$selected" | jq -e '.status == "selected" and .selection.provider == "cuda.cublas" and .fallback.required == true' >/dev/null

cpu=$("$planner" --plan "$plan" --capabilities "$capabilities" --calibration "$calibration" --min-speedup 4.0)
printf '%s\n' "$cpu" | jq -e '.selection.provider == "cpu.serial" and .evidence.measured_end_to_end_speedup == 3' >/dev/null

no_calibration=$("$planner" --plan "$plan" --capabilities "$capabilities")
printf '%s\n' "$no_calibration" | jq -e '.selection.provider == "cpu.serial" and .evidence.calibration_verified == false' >/dev/null

printf '%s\n' '{"format":"frankencore.runtime_capabilities","version":1,"cuda":{"status":"available","status":"unavailable","device_count":1}}' >"$capabilities"
if "$planner" --plan "$plan" --capabilities "$capabilities" >/dev/null 2>&1; then
  echo 'runtime planner accepted duplicate capability authority' >&2
  exit 1
fi

printf '%s\n' '{"format":"frankencore.runtime_capabilities","version":1,"cpu":{"logical_processors":8},"memory":{"total_bytes":1,"available_bytes":1},"cuda":{"status":"unavailable","driver":"","device_count":0,"diagnostic":"none"}}' >"$capabilities"
unavailable=$("$planner" --plan "$plan" --capabilities "$capabilities" --calibration "$calibration")
printf '%s\n' "$unavailable" | jq -e '.selection.provider == "cpu.serial" and .evidence.cuda_available == false' >/dev/null

jq '.cancellation = "requested"' "$plan" >"$unsupported_plan"
set +e
unsupported=$("$planner" --plan "$unsupported_plan" --capabilities "$capabilities" 2>/dev/null)
unsupported_rc=$?
set -e
test "$unsupported_rc" -eq 2
printf '%s\n' "$unsupported" | jq -e '.status == "unsupported" and .request == "cancellation" and .fallback.emitted == false' >/dev/null

set +e
printf '%s' '{"format":"wrong"}' | "$planner" --plan /dev/stdin --capabilities "$capabilities" --diagnostics json >"$diagnostic_out" 2>"$diagnostic_err"
diagnostic_rc=$?
set -e
test "$diagnostic_rc" -eq 1
test ! -s "$diagnostic_out"
jq -e '.status == "failed" and .code == "FLOWPARALLEL_RUNTIME_PLANNER_FAILURE" and .disposition == "no_artifact" and (.message | length > 0)' "$diagnostic_err" >/dev/null

echo 'Flowparallel runtime planner: PASS'
