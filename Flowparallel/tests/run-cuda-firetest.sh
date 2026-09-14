#!/usr/bin/env bash
set -euo pipefail

cuda_execute=${FLOWPARALLEL_CUDA_EXECUTE_BIN:?FLOWPARALLEL_CUDA_EXECUTE_BIN is required}
provider=${FLOWPARALLEL_CUDA_BIN:?FLOWPARALLEL_CUDA_BIN is required}
plan=${FLOWPARALLEL_PLAN:?FLOWPARALLEL_PLAN is required}
test -x "$cuda_execute"
test -x "$provider"
test -f "$plan"
diagnostic_err=$(mktemp)
trap 'rm -f "$diagnostic_err"' EXIT

set +e
"$cuda_execute" --size 1 --diagnostics json >/dev/null 2>"$diagnostic_err"
diagnostic_rc=$?
set -e
test "$diagnostic_rc" -eq 1
jq -e '.status == "failed" and .code == "FLOWPARALLEL_CUDA_EXECUTE_FAILURE" and .disposition == "no_artifact" and (.message | length > 0)' "$diagnostic_err" >/dev/null

sizes=(2 3 7 16 31 64 127 256 512)
runs=0
failures=0

for size in "${sizes[@]}"; do
    expected=$((size * (size + 5) + size * (size - 1) * (size + 3)))
    for repeat in 1 2 3; do
        runs=$((runs + 1))
        report=$(timeout 45s "$cuda_execute" --size "$size")
        status=$(jq -r '.status' <<<"$report")
        actual=$(jq -r '.result_sum' <<<"$report")
        error=$(jq -r '.max_error' <<<"$report")
        devices=$(jq -r '.device_count' <<<"$report")
        if [[ "$status" != verified || "$devices" -lt 1 || "$error" != 0 || "$(printf '%.0f' "$actual")" -ne "$expected" ]]; then
            printf 'CUDA mismatch size=%s repeat=%s expected_sum=%s report=%s\n' "$size" "$repeat" "$expected" "$report" >&2
            failures=$((failures + 1))
        fi
    done
done

for bad_args in \
    '--size 0' \
    '--size 1' \
    '--size 4097' \
    '--size -2' \
    '--not-an-option'; do
    if timeout 10s "$cuda_execute" $bad_args >/dev/null 2>&1; then
        printf 'hostile argument unexpectedly accepted: %s\n' "$bad_args" >&2
        failures=$((failures + 1))
    fi
done

provider_report=$(cat "$plan" | "$provider" --matrix-size 64)
jq -e '
  .format == "flowparallel.cuda_selection" and
  .status == "ready" and
  .cuda.status == "available" and
  .cuda.devices >= 1 and
  .execution == "not-performed" and
  .fallback.required == true
' <<<"$provider_report" >/dev/null

printf 'Flowparallel CUDA firetest: runs=%s failures=%s sizes=%s\n' "$runs" "$failures" "${#sizes[@]}"
test "$failures" -eq 0
