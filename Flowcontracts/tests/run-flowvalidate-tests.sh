#!/bin/sh
set -eu

validator=${FLOWVALIDATE_BIN:?FLOWVALIDATE_BIN is required}
flowmini=${FLOWMINI_BIN:?FLOWMINI_BIN is required}
flowanalyst=${FLOWANALYST_BIN:?FLOWANALYST_BIN is required}
flowbind=${FLOWBIND_BIN:?FLOWBIND_BIN is required}
flowparallel=${FLOWPARALLEL_BIN:?FLOWPARALLEL_BIN is required}
flowoptimize=${FLOWOPTIMIZE_BIN:?FLOWOPTIMIZE_BIN is required}
flowlower=${FLOWLOWER_BIN:?FLOWLOWER_BIN is required}
fixture=${FLOWVALIDATE_FIXTURE:?FLOWVALIDATE_FIXTURE is required}

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

"$flowmini" --dump-frontend-bundle "$fixture" > "$tmpdir/frontend.json"
"$flowanalyst" < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
"$flowbind" < "$tmpdir/semantic.json" > "$tmpdir/binding.json"
"$flowparallel" < "$tmpdir/semantic.json" > "$tmpdir/execution.json"
"$flowoptimize" < "$tmpdir/execution.json" > "$tmpdir/optimization.json"
"$flowlower" < "$tmpdir/optimization.json" > "$tmpdir/lowering.json"

for artifact in frontend semantic binding execution optimization lowering; do
    "$validator" "$tmpdir/$artifact.json" | jq -e '.classification == "valid" and .format != ""' >/dev/null
    "$validator" --canonical "$tmpdir/$artifact.json" > "$tmpdir/$artifact.canonical.json"
    "$validator" --canonical "$tmpdir/$artifact.canonical.json" | cmp -s - "$tmpdir/$artifact.canonical.json"
done

jq '.lowering_plan.operations[1].id = .lowering_plan.operations[0].id' "$tmpdir/semantic.json" > "$tmpdir/duplicate-operation.json"
set +e
"$validator" "$tmpdir/duplicate-operation.json" > "$tmpdir/duplicate-result.json"
duplicate_rc=$?
set -e
test "$duplicate_rc" -eq 1
jq -e '.classification == "invalid" and (.path | contains("operations"))' "$tmpdir/duplicate-result.json" >/dev/null

set +e
printf '%s' '{"format":"future.artifact","version":1}' | "$validator" > "$tmpdir/unsupported.json"
unsupported_rc=$?
printf '%s' '{"format":"flowanalyst.semantic_report","version":1,"status":"blocked"}' | "$validator" > "$tmpdir/blocked.json"
blocked_rc=$?
printf '%s' '{"format":"flowanalyst.semantic_report","format":"flowanalyst.semantic_report","version":1,"status":"ok"}' | "$validator" > "$tmpdir/duplicate.json"
duplicate_key_rc=$?
set -e
test "$unsupported_rc" -eq 3
test "$blocked_rc" -eq 2
test "$duplicate_key_rc" -eq 1
jq -e '.classification == "unsupported"' "$tmpdir/unsupported.json" >/dev/null
jq -e '.classification == "blocked"' "$tmpdir/blocked.json" >/dev/null
jq -e '.classification == "invalid" and (.reason | contains("duplicate"))' "$tmpdir/duplicate.json" >/dev/null

set +e
printf '%s' '{"format":' | "$validator" --diagnostics json > "$tmpdir/structured.stdout" 2> "$tmpdir/structured.stderr"
structured_rc=$?
set -e
test "$structured_rc" -eq 1
test ! -s "$tmpdir/structured.stdout"
jq -e '.status == "failed" and .code == "FLOWVALIDATE_CONTRACT_FAILURE" and .stage == "contract" and (.message | length > 0) and .disposition == "no_artifact"' "$tmpdir/structured.stderr" >/dev/null

for evidence in \
  '{"format":"flowcore.abi_manifest","version":1,"provider":"test","types":[]}' \
  '{"format":"frankencore.runtime_capabilities","version":1,"cuda":{"status":"unavailable","device_count":0}}' \
  '{"format":"flowcore.runtime_capabilities","version":1,"status":"available","device_count":1}' \
  '{"format":"flowparallel.matrix_benchmark","version":1,"status":"verified","end_to_end_speedup":1.5}' \
  '{"format":"flowparallel.graph_cuda","version":1,"status":"verified","end_to_end_speedup":1.5}'
do
  printf '%s' "$evidence" | "$validator" | jq -e '.classification == "valid"' >/dev/null
done

echo 'flowvalidate tests: PASS'
