#!/bin/sh
set -eu

reference=${FLOWPARALLEL_GRAPH_REFERENCE_BIN:?FLOWPARALLEL_GRAPH_REFERENCE_BIN is required}
flowmini=${FLOWMINI_BIN:?FLOWMINI_BIN is required}
analyst=${FLOWANALYST_BIN:?FLOWANALYST_BIN is required}
fixture=${FLOW_GRAPH_FIXTURE:?FLOW_GRAPH_FIXTURE is required}
test -x "$reference"; test -x "$flowmini"; test -x "$analyst"; test -f "$fixture"
diagnostic_err=$(mktemp)
trap 'rm -f "$diagnostic_err"' EXIT

report=$("$flowmini" --dump-frontend-bundle "$fixture" | "$analyst" | "$reference")
printf '%s\n' "$report" | jq -e '.format == "flowparallel.graph_analysis" and .status == "verified" and .operation == "reachability" and .semantics.semiring == "boolean" and .reachable_pairs > 0 and .provider == "cpu.reference"' >/dev/null

set +e
blocked=$(printf '%s\n' '{"format":"flowanalyst.semantic_report","version":1,"status":"error"}' | "$reference")
blocked_rc=$?
set -e
test "$blocked_rc" -eq 2
printf '%s\n' "$blocked" | jq -e '.status == "blocked"' >/dev/null

set +e
malformed=$(printf '%s\n' '{"format":"flowanalyst.semantic_report","version":1,"format":"forged","status":"ok"}' | "$reference" 2>/dev/null)
malformed_rc=$?
set -e
test "$malformed_rc" -ne 0
test -z "$malformed"
set +e
diagnostic_out=$(printf '%s\n' '{"format":"wrong"}' | "$reference" --diagnostics json 2>"$diagnostic_err")
diagnostic_rc=$?
set -e
test "$diagnostic_rc" -ne 0
test -z "$diagnostic_out"
jq -e '.status == "failed" and .code == "FLOWPARALLEL_GRAPH_REFERENCE_FAILURE" and .disposition == "no_artifact" and (.message | length > 0)' "$diagnostic_err" >/dev/null
echo 'Flowparallel graph reference: PASS'
