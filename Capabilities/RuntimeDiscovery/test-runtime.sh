#!/bin/sh
set -eu

runtime_bin=${RUNTIME_BIN:?RUNTIME_BIN is required}
test -x "$runtime_bin"
report=$($runtime_bin)
printf '%s\n' "$report" | jq -e '
  .format == "frankencore.runtime_capabilities" and
  .version == 1 and
  (.cpu.logical_processors >= 1) and
  (.memory.total_bytes >= 0) and
  (.memory.available_bytes >= 0) and
  (.cuda.status == "available" or .cuda.status == "unavailable" or .cuda.status == "unknown")
' >/dev/null
echo 'Frankencore runtime discovery: PASS'
