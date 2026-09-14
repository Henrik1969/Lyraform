#!/bin/sh
set -eu

flowkernel=${1:?flowkernel binary is required}
report=$("$flowkernel" --probe all)
printf '%s\n' "$report" | jq -e '
  .format == "flowkernel.probe_report" and
  (.results | length) == 6 and
  ([.results[].name] | unique | length) == 6
' >/dev/null
echo 'Flowkernel all-probe contract: PASS'
