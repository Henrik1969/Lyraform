#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
smoke=${FLOWPARALLEL_SMOKETEST_BIN:-$root/Flowparallel/build/flowparallel_smoketest}
report_output=''
while [ "$#" -gt 0 ]; do
    case "$1" in
        --output)
            [ "$#" -ge 2 ] || { echo 'missing value for --output' >&2; exit 2; }
            report_output=$2
            shift 2
            ;;
        --help|-h)
            echo 'Usage: run-parallel-smoketest.sh [--output PATH]'
            echo '  Keep the validated report temporarily by default; copy it to PATH when requested.'
            exit 0
            ;;
        *) echo "unknown argument: $1" >&2; exit 2 ;;
    esac
done
test -x "$smoke"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
items=${FLOWPARALLEL_SMOKETEST_ITEMS:-2000000}
minimum_speedup=${FLOWPARALLEL_MIN_SPEEDUP:-1.25}
memory_kb=${FLOWPARALLEL_SMOKETEST_MEMORY_KB:-1048576}
case "$memory_kb" in ''|*[!0-9]*) echo 'invalid FLOWPARALLEL_SMOKETEST_MEMORY_KB' >&2; exit 2 ;; esac
case "$minimum_speedup" in ''|*[!0-9.]*|.*|*.*.*) echo 'invalid FLOWPARALLEL_MIN_SPEEDUP' >&2; exit 2 ;; esac

run_limited() {
    output=$1
    shift
    affinity=$1
    shift
    if [ "$affinity" = single ] && command -v taskset >/dev/null 2>&1; then
        set -- taskset -c 0 "$@"
    fi
    if command -v timeout >/dev/null 2>&1; then
        timeout --signal=TERM 30s sh -c 'limit=$1; shift; if [ "$limit" -ne 0 ]; then ulimit -v "$limit"; fi; exec "$@"' sh "$memory_kb" "$@" > "$output"
    else
        "$@" > "$output"
    fi
}

run_limited "$tmpdir/serial.json" single "$smoke" --items "$items" --workers 1
run_limited "$tmpdir/repeated.json" single "$smoke" --items "$items" --workers 1
serial_result=$(jq -r '.result' "$tmpdir/serial.json")
repeated_result=$(jq -r '.result' "$tmpdir/repeated.json")
test "$serial_result" = "$repeated_result"

workers=${FLOWPARALLEL_SMOKETEST_WORKERS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || printf '%s' 2)}
case "$workers" in ''|*[!0-9]*) workers=2 ;; esac
test "$workers" -ge 2 || workers=2
run_limited "$tmpdir/parallel.json" unrestricted "$smoke" --items "$items" --workers "$workers"
parallel_result=$(jq -r '.result' "$tmpdir/parallel.json")
test "$serial_result" = "$parallel_result"

set +e
"$smoke" --items 10 --workers 0 >/dev/null 2>"$tmpdir/invalid.stderr"
invalid_rc=$?
set -e
test "$invalid_rc" -eq 2

jq -n \
    --argjson serial "$(cat "$tmpdir/serial.json")" \
    --argjson parallel "$(cat "$tmpdir/parallel.json")" \
    --arg repeated "$repeated_result" \
    --argjson minimum_speedup "$minimum_speedup" \
    --arg sandbox "timeout; virtual-memory-limit-kb=$memory_kb" \
    '{format:"flowparallel.smoketest_report",version:1,status:"ok",sandbox:$sandbox,serial:$serial,parallel:$parallel,cost_model:{minimum_speedup:$minimum_speedup,observed_speedup:($serial.elapsed_ns / $parallel.elapsed_ns),decision:(if ($serial.elapsed_ns / $parallel.elapsed_ns) >= $minimum_speedup then "parallel" else "serial" end)},correctness:{matching_result:($serial.result == $parallel.result),repeated_serial_match:(($serial.result|tostring) == $repeated)}}' \
    > "$tmpdir/report.json"
jq -e '.status == "ok" and .correctness.matching_result and .correctness.repeated_serial_match' "$tmpdir/report.json" >/dev/null
if [ -n "$report_output" ]; then
    cp "$tmpdir/report.json" "$report_output"
    report_display=$report_output
else
    report_display='temporary output (not retained)'
fi
echo 'Flowparallel parallel smoke test: PASS'
echo "  serial: $serial_result"
echo "  parallel: $parallel_result (workers=$workers)"
echo "  report: $report_display"
